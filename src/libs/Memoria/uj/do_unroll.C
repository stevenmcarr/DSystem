/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <mh.h>
#include "do_unroll.h"
#include "ujam.h"
#include <mark.h>
#include <mem_util.h>
#include <gi.h>
#include <label.h>
#include <mem_util.h>
#include <analyze.h>
#include <shape.h>
#include "compute_uj.h"
#include "log.h"
#include <bound.h>

static void walk_loops_to_unroll(model_loop    *loop_data,
				 int           loop,
				 int           unroll_loop,
				 PedInfo       ped,
				 SymDescriptor symtab,
				 AST_INDEX     step,
				 arena_type    *ar);

static AST_INDEX mh_gen_ident(SymDescriptor symtab,
			      char          *name,
			      int           asttype,
			      AST_INDEX     src)

  {
   AST_INDEX   node;
   fst_index_t index;
   
     node = pt_gen_ident(name);
     gen_put_converted_type(node,asttype);
     gen_put_real_type(node,asttype);
     index = fst_Index(symtab,name);
     fst_PutFieldByIndex(symtab,index,NEW_VAR,true);
     fst_PutFieldByIndex(symtab,index,SYMTAB_TYPE,asttype);
     fst_PutFieldByIndex(symtab,index,RDX_VAR,src);
     return(node);
  }

static int update_refs(AST_INDEX     node,
		       ref_info_type *ref_info)

  {
   char new_var[30];
   subscript_info_type *sptr;
   
     if (is_identifier(node))
       if ((sptr = get_subscript_ptr(node)) != NULL && 
	   fst_GetField(ref_info->symtab,gen_get_text(node),
			SYMTAB_NUM_DIMS) > 0)
	 {
	  sptr->surrounding_do = ref_info->surrounding_do;
	  sptr->surround_node = ref_info->surround_node;
	 }
       else
         if (fst_GetField(ref_info->symtab,gen_get_text(node),EXPAND_LVL) >= 
	     ref_info->level)
	   {
	     sprintf(new_var,"%s_%d$",gen_get_text(node),ref_info->val);
	     pt_tree_replace(node,mh_gen_ident(ref_info->symtab,new_var,
					       gen_get_real_type(node),
					       (Generic)NULL));
	    }
     return(WALK_CONTINUE);
  }
 
static AST_INDEX find_end_stmt(AST_INDEX stmt)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   while(!is_do(list_next(stmt)) && list_next(stmt) != (Generic)NULL)
     stmt = list_next(stmt);
   return(stmt);
  }

static void set_stmt_ptrs_to_NULL(AST_INDEX stmt_list)

  {
   AST_INDEX stmt;

     for (stmt = list_first(stmt_list);
	  stmt != AST_NIL;
	  stmt = list_next(stmt))
       set_scratch_to_NULL(stmt);
  }

static void replicate_statements(AST_INDEX     stmt_list,
				 int           val,
				 int           level,
				 char          *ivar,
				 AST_INDEX     step,
				 SymDescriptor symtab,
				 int           surrounding_do,
				 AST_INDEX     surround_node,
				 arena_type    *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX node,
             new_node,
             start_node,
             last_node,
             cnode;
   int       i,step_v;
   ref_info_type ref_info;
   copy_info_type copy_info;

     if (step == AST_NIL)
       step_v = 1;
     else 
       (void)pt_eval(step,&step_v);
     node = list_first(stmt_list);
     ref_info.level = level;
     ref_info.symtab = symtab;
     ref_info.surrounding_do = surrounding_do;
     ref_info.surround_node = surround_node;
     copy_info.val = val;
     copy_info.ar = ar;
     copy_info.symtab = symtab;
     while (node != ast_null_node)
       if (is_do(node))
	 node = list_next(node);
       else
	 {
	  start_node = node;
	  last_node = find_end_stmt(node);
	  node = list_next(last_node);
	  if (start_node != last_node || !is_continue(start_node))
	    {
	     for (cnode = last_node;
		  cnode != list_prev(start_node);
		  cnode = list_prev(cnode))
	       walk_expression(cnode,ut_init_copies,NOFUNC,
			       (Generic)&copy_info);
	     for (i = val; i >= 1; i--)
	       {
		ref_info.val = i-1;
		for (cnode = last_node;
		     cnode != list_prev(start_node);
		     cnode = list_prev(cnode))
		  {
		   new_node = ut_tree_copy_with_type(cnode,i-1,ar);
		   walk_expression(new_node,update_refs,NOFUNC,
				   (Generic)&ref_info);
		   /* set_scratch_to_NULL(new_node); */
		   pt_var_add(new_node,ivar,step_v*i);
		   list_insert_after(last_node,new_node);
		   ut_update_labels(new_node,symtab);
		  }
	       }
	    }
	 }
  }

static int create_ref_lists(AST_INDEX node,
			    PedInfo   ped)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX name;
   int       ref;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	if (get_info(ped,get_subscript_ptr(name)->original,type_levelv) >= 0)
	  {
	   ref = dg_alloc_ref_list( PED_DG(ped));
	   create_info(PED_INFO(ped),name);
	   put_info(ped,name,type_levelv,ref);
	  }
       }
     return(WALK_CONTINUE);
  }
	  

static void set_level_vectors(AST_INDEX old_list,
			      AST_INDEX new_list,
			      PedInfo   ped)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX old,
             newa;
   int       vector,
             v_size;

     for (old = list_first(old_list), newa = list_first(new_list);
	  old != (Generic)NULL;
	  old = list_next(old), newa = list_next(newa))
       {
	if (is_do(old))
	  set_level_vectors(gen_DO_get_stmt_LIST(old),
			    gen_DO_get_stmt_LIST(newa),ped);
	else if (is_guard(old))
	  set_level_vectors(gen_GUARD_get_stmt_LIST(old),
			    gen_GUARD_get_stmt_LIST(newa),ped);
	else if (is_if(old))
	  set_level_vectors(gen_IF_get_guard_LIST(old),
			    gen_IF_get_guard_LIST(newa),ped);
	else if (is_logical_if(old))
	  set_level_vectors(gen_LOGICAL_IF_get_stmt_LIST(old),
			    gen_LOGICAL_IF_get_stmt_LIST(newa),ped);
	if (get_info(ped,old,type_levelv) >= 0)
	  {
	   v_size = dg_length_level_vector( PED_DG(ped),
					   get_info(ped,old,type_levelv));
	   vector = dg_alloc_level_vector( PED_DG(ped),v_size);
	   create_info(PED_INFO(ped),newa);
	   put_info(ped,newa,type_levelv,vector);
	   walk_expression(newa,create_ref_lists,NOFUNC,(Generic)ped);
	  }
       }
  }

static int replace_array(AST_INDEX node,
			 rep_info_type *rep_info)

  {
   EDGE_INDEX edge,
              next_edge;
   int        vector;
   AST_INDEX  name;

     if (is_subscript(node))
       if (pt_expr_equal(node,rep_info->array))
	 {
	  name = gen_SUBSCRIPT_get_name(node);
	  vector = get_info(rep_info->ped,name,type_levelv);
	  for (edge = dg_first_src_ref( PED_DG(rep_info->ped),vector);
	       edge != END_OF_LIST;
	       edge = next_edge)
	    {
	     next_edge = dg_next_src_ref( PED_DG(rep_info->ped),edge);
	     dg_delete_free_edge( PED_DG(rep_info->ped),edge);
	    }
	  for (edge = dg_first_sink_ref( PED_DG(rep_info->ped),vector);
	       edge != END_OF_LIST;
	       edge = next_edge)
	    {
	     next_edge = dg_next_sink_ref( PED_DG(rep_info->ped),edge);
	     dg_delete_free_edge( PED_DG(rep_info->ped),edge);
	    }
	  /* free((char *)get_subscript_ptr(name)); */
	  pt_tree_replace(node,pt_gen_ident(rep_info->new_var));
	 }
     return(WALK_CONTINUE);
  }
static int update_rdx_var(AST_INDEX     stmt,
			  int           level,
			  dupd_info_type *upd_info)
  
  {
   AST_INDEX lval,name,copy;
   char      *var,new_var[10];
   subscript_info_type *subp;
   rep_info_type rep_info;
   
     if (is_assignment(stmt))
       {
	lval = gen_ASSIGNMENT_get_lvalue(stmt);
	if (gen_get_converted_type(lval) == TYPE_DOUBLE_PRECISION ||
	    gen_get_converted_type(lval) == TYPE_REAL)
	  if (is_identifier(lval) && 
	      pt_find_var(gen_ASSIGNMENT_get_rvalue(stmt),gen_get_text(lval)))
	    {
	     var = gen_get_text(lval);
	     sprintf(new_var,"%s_%d_0",var,upd_info->val);
	     pt_var_replace(stmt,var,mh_gen_ident(upd_info->symtab,new_var,
						  gen_get_real_type(lval),
						  tree_copy_with_type(lval)));
	    }
	  else if (is_subscript(lval))
	    {

	       /* will not update names in other copied statements,
		  fix this! */

	     name = gen_SUBSCRIPT_get_name(lval);
	     copy = tree_copy_with_type(lval);
	     /* set_scratch_to_NULL(gen_SUBSCRIPT_get_name(copy)); */
	     subp = get_subscript_ptr(get_subscript_ptr(name)->original);
	     if (subp->is_scalar[2] && !subp->prev_sclr[2])
	       {
		sprintf(rep_info.new_var,"%s_%d_0",gen_get_text(name),
			upd_info->val);
		tree_free(mh_gen_ident(upd_info->symtab,rep_info.new_var,
				       gen_get_real_type(copy),
				       tree_copy_with_type(copy)));
		rep_info.array = tree_copy_with_type(copy);
		rep_info.ped = upd_info->ped;
		walk_expression(stmt,NOFUNC,replace_array,(Generic)&rep_info);
		tree_free(rep_info.array);
		tree_free(copy);
	       }
	    }
       }
     return(WALK_CONTINUE);
  }
     
static int new_level(DG_Edge *edge,
		     int     level)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int lvl;
   
     lvl = level + 1;
     while(lvl <= gen_get_dt_LVL(edge))
       if (gen_get_dt_DIS(edge,lvl) != 0)
         return(lvl);
       else
         lvl++;
     return(LOOP_INDEPENDENT);
  }

static void add_edge(AST_INDEX    source,
		     AST_INDEX    sink,
		     AST_INDEX    src_stmt,
		     AST_INDEX    sink_stmt,
		     EDGE_INDEX   old_edge,
		     int          new_thresh,
		     int          level,
		     PedInfo      ped)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   EDGE_INDEX new_edge;
   AST_INDEX  temp;
   DG_Edge    *dg;
   int        i,dist,
              src_ref,
              sink_ref;

     dg = dg_get_edge_structure( PED_DG(ped));
     new_edge = dg_alloc_edge( PED_DG(ped),&dg);
     dg[new_edge].src_str = NULL;
     dg[new_edge].sink_str = NULL;
     dg[new_edge].type = dg[old_edge].type;
     dg[new_edge].symbolic = dg[old_edge].symbolic;
     dg[new_edge].consistent = dg[old_edge].consistent;
     dt_copy_info( PED_DT_INFO(ped),&dg[old_edge],&dg[new_edge]);
     if (new_thresh == 0 && level == dg[old_edge].level &&
	 level != LOOP_INDEPENDENT)
       {
	dg[new_edge].level = new_level(&dg[old_edge],level);

	  /*  this can only happen for input dependences.  The dependence
	      direction is reversed after unroll-and-jam. */

	if ((dist = gen_get_dt_DIS(&dg[new_edge],dg[new_edge].level)) < 0)
	  {
	   temp = source;
	   source = sink;
	   sink = temp;
	   temp = src_stmt;
	   src_stmt = sink_stmt;
	   sink_stmt = temp;
	   gen_put_dt_DIS(&dg[new_edge],dg[new_edge].level,-dist);
	  }
       }
     else
	dg[new_edge].level = dg[old_edge].level;
     dg[new_edge].src = source;
     dg[new_edge].sink = sink;

     /* ref lists have already been created */

     dg[new_edge].src_ref = get_info(ped,source,type_levelv);
     dg[new_edge].sink_ref = get_info(ped,sink,type_levelv);

     /* stmt level vectors have already been created */

      /* hack because ast makes no sense */

     if (is_logical_if(tree_out(src_stmt)) || is_guard(src_stmt))
       src_stmt = tree_out(src_stmt);
     dg[new_edge].src_vec = get_info(ped,src_stmt,type_levelv);
     if (is_logical_if(tree_out(sink_stmt)) || is_guard(sink_stmt))
       sink_stmt = tree_out(sink_stmt);
     dg[new_edge].sink_vec = get_info(ped,sink_stmt,type_levelv);

     gen_put_dt_DIS(&dg[new_edge],level,new_thresh);
     dt_info_str( PED_DT_INFO(ped),&dg[new_edge]);
     dg_add_edge( PED_DG(ped),new_edge);
  }

static void add_inner_edge(AST_INDEX    source,
			   AST_INDEX    sink,
			   AST_INDEX    src_stmt,
			   AST_INDEX    sink_stmt,
			   EDGE_INDEX   old_edge,
			   int          new_thresh,
			   int          level,
			   PedInfo      ped)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   EDGE_INDEX new_edge;
   AST_INDEX  temp;
   DG_Edge    *dg;
   int        i,dist,
              src_ref,
              sink_ref;

     dg = dg_get_edge_structure( PED_DG(ped));
     new_edge = dg_alloc_edge( PED_DG(ped),&dg);
     dg[new_edge].src_str = NULL;
     dg[new_edge].sink_str = NULL;
     dg[new_edge].type = dg[old_edge].type;
     dg[new_edge].symbolic = dg[old_edge].symbolic;
     dg[new_edge].consistent = dg[old_edge].consistent;
     dg[new_edge].src = source;
     dg[new_edge].sink = sink;
     dg[new_edge].level = level;

     /* ref lists have already been created */

     dg[new_edge].src_ref = get_info(ped,source,type_levelv);
     dg[new_edge].sink_ref = get_info(ped,sink,type_levelv);

     /* stmt level vectors have already been created */

      /* hack because ast makes no sense */

     if (is_logical_if(tree_out(src_stmt)) || is_guard(src_stmt))
       src_stmt = tree_out(src_stmt);
     dg[new_edge].src_vec = get_info(ped,src_stmt,type_levelv);
     if (is_logical_if(tree_out(sink_stmt)) || is_guard(sink_stmt))
       sink_stmt = tree_out(sink_stmt);
     dg[new_edge].sink_vec = get_info(ped,sink_stmt,type_levelv);

     for (i = 1; i < level; i++)
       gen_put_dt_DIS(&dg[new_edge],level,0);
     gen_put_dt_DIS(&dg[new_edge],level,new_thresh);
     dt_info_str( PED_DT_INFO(ped),&dg[new_edge]);
     dg_add_edge( PED_DG(ped),new_edge);
  }

static void do_siv(AST_INDEX    source,
		   AST_INDEX    sink,
		   AST_INDEX    src_stmt,
		   AST_INDEX    sink_stmt,
		   EDGE_INDEX   old_edge,
		   int          level,
		   char         *ivar,
		   PedInfo      ped)

  {
   AST_INDEX sub1,sub2,expr;
   int       con1,con2,coeff,thresh;
   Boolean   lin;

     sub1 = tree_out(source);
     sub2 = tree_out(sink);
     for (sub1 = list_first(gen_SUBSCRIPT_get_rvalue_LIST(sub1)),
          sub2 = list_first(gen_SUBSCRIPT_get_rvalue_LIST(sub2));
	  !pt_find_var(sub1,ivar) && sub1 != AST_NIL;
	  sub1 = list_next(sub1),
	  sub2 = list_next(sub2));
     if (sub1 != AST_NIL)
       {
	pt_fold_term(sub1,&expr,&con1);
	pt_fold_term(sub2,&expr,&con2);
	pt_get_coeff(sub1,ivar,&lin,&coeff);
	if ((con1 - con2) % coeff == 0)
          {
	   thresh = (con1 - con2)/coeff;
	   if (thresh < 0)
	     add_inner_edge(sink,source,src_stmt,ut_get_stmt(sink),old_edge,
			    -thresh,level,ped);
	   else if (thresh > 0)
	     add_inner_edge(source,sink,src_stmt,ut_get_stmt(sink),old_edge,
			    thresh,level,ped);
	   else 
	     add_inner_edge(source,sink,src_stmt,ut_get_stmt(sink),old_edge,
			    thresh,LOOP_INDEPENDENT,ped);
	  }
       }
     else 
       add_inner_edge(sink,sink,src_stmt,ut_get_stmt(sink),old_edge,
		      DDATA_ANY,level,ped);
  }
				     

static int update_graph(AST_INDEX     node,
			dupd_info_type *upd_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX  name,
              sink,      
              source,
              src_stmt;
   EDGE_INDEX edge,
              next_edge;
   DG_Edge    *dg;
   int        vector,
              stmt_vector,
              thresh,
              new_thresh,
              i,index,upb;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	dg = dg_get_edge_structure( PED_DG(upd_info->ped));
	vector = get_info(upd_info->ped,name,type_levelv);
	for (edge = dg_first_src_ref( PED_DG(upd_info->ped),vector);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_src_ref( PED_DG(upd_info->ped),edge);
	   thresh = gen_get_dt_DIS(&dg[edge],upd_info->level);
	   if ((dg[edge].type == dg_true || dg[edge].type == dg_anti ||
		dg[edge].type == dg_input || dg[edge].type == dg_output) &&
	       (get_subscript_ptr(dg[edge].src)->surround_node ==
		get_subscript_ptr(dg[edge].sink)->surround_node))
	     if (dg[edge].consistent == consistent_SIV && !dg[edge].symbolic)
	       {
		if (dg[edge].level != LOOP_INDEPENDENT)
		  if (gen_is_dt_DIS(&dg[edge]) || thresh > DDATA_BASE)
		    {
		     source = dg[edge].src;
		     i = mod(thresh, upd_info->val + 1);
		     new_thresh = floor_ab(thresh,upd_info->val + 1);
		     if (i == 0)
		       sink = dg[edge].sink;
		     else
		       sink = get_subscript_ptr(dg[edge].sink)->copies[i-1];
		     add_edge(source,sink, ut_get_stmt(source),
			      ut_get_stmt(sink),edge,new_thresh,
			      upd_info->level,upd_info->ped);
		     for (index = 0; index < upd_info->val; index++)
		       {
			source = get_subscript_ptr(name)->copies[index]; 
			i = mod(thresh + index + 1, upd_info->val + 1);
			new_thresh = floor_ab(thresh,upd_info->val + 1);
			if (i < index + 1)
			 new_thresh++;
			if (i == 0)
			  sink = dg[edge].sink;
			else
			  sink = get_subscript_ptr(dg[edge].sink)->copies[i-1];
			add_edge(source,sink, ut_get_stmt(source),
				 ut_get_stmt(sink),edge,new_thresh,
				 upd_info->level,upd_info->ped);
		       }
		     dg_delete_free_edge( PED_DG(upd_info->ped),edge);
		    }
		  else
		    {
		     /*  scalar array references  */


		      /* copy self loop for each stmt */

		     for (index = 0;index < upd_info->val;index++)
		       {
			source = get_subscript_ptr(name)->copies[index]; 
			sink =get_subscript_ptr(dg[edge].sink)->copies[index]; 
			add_edge(source,sink, ut_get_stmt(source),
				 ut_get_stmt(sink),edge,DDATA_ANY,
				 dg[edge].level,upd_info->ped);
		       } 
		     if (dg[edge].level == upd_info->level)
		       {

			  /* copy loop independent to successive stmts */
			
			source = dg[edge].src;
			src_stmt = ut_get_stmt(source);
			for (index = 0; index < upd_info->val; index++)
			  {
			   sink = get_subscript_ptr(dg[edge].sink)->
			                                 copies[index]; 
			   add_edge(source,sink,src_stmt,ut_get_stmt(sink),
				    edge,0,upd_info->level,upd_info->ped);
			  } 
			for (index = 0; index < upd_info->val-1; index++)
			  {
			   source = get_subscript_ptr(name)->copies[index]; 
			   src_stmt = ut_get_stmt(source);
			   for (i = index+1; i < upd_info->val; i++)
			     {
			      sink = get_subscript_ptr(dg[edge].sink)->
			                                        copies[i]; 
			      add_edge(source,sink, ut_get_stmt(source),
				       ut_get_stmt(sink),edge,0,
				       upd_info->level,upd_info->ped);
				} 
			  }
			
			   /* scalar edges between scalar array references are 
			      unnecessary, so not added */
		       }
		    }
		else

		    /* copy loop independent edges */

		  for (index = 0; index < upd_info->val; index++)
		    {
		     source = get_subscript_ptr(name)->copies[index]; 
		     sink = get_subscript_ptr(dg[edge].sink)->copies[index]; 
		     add_edge(source,sink, ut_get_stmt(source),
			      ut_get_stmt(sink),edge,0,upd_info->level,
			      upd_info->ped);
		    } 
	       }
	     else if (dg[edge].consistent == consistent_MIV && 
		      !dg[edge].symbolic)
	       {
		source = dg[edge].src;
		src_stmt = ut_get_stmt(source);
		for (index = 0; index < upd_info->val; index++)
		  {
		   sink = get_subscript_ptr(dg[edge].sink)->copies[index]; 
		   do_siv(source,sink,src_stmt,ut_get_stmt(sink),edge,
			  upd_info->inner_level,upd_info->ivar,upd_info->ped);
		  } 
		for (index = 0; index < upd_info->val; index++)
		  {
		   source = get_subscript_ptr(name)->copies[index];  
		   src_stmt = ut_get_stmt(source);
		   for (i = index + 1; i < upd_info->val; i++)
		     {
		      sink = get_subscript_ptr(dg[edge].sink)->copies[i]; 
		      do_siv(source,sink,src_stmt,ut_get_stmt(sink),edge,
			     upd_info->inner_level,upd_info->ivar,
			     upd_info->ped);
		     }
		  }
	       }
	     else
	       {
		/* copy other edges */

		source = dg[edge].src;
		src_stmt = ut_get_stmt(source);
		thresh = gen_get_dt_DIS(&dg[edge],upd_info->level);
		if (thresh == DDATA_LT)
		  thresh = DDATA_LE;
		for (index = 0; index < upd_info->val; index++)
		  {
		   sink = get_subscript_ptr(dg[edge].sink)->copies[index]; 
		   add_edge(source,sink,src_stmt,ut_get_stmt(sink),edge,
			    thresh,upd_info->level,upd_info->ped);
		  } 
		for (index = 0; index < upd_info->val; index++)
		  {
		   source = get_subscript_ptr(name)->copies[index];  
		   src_stmt = ut_get_stmt(source);
		   for (i = 0; i < upd_info->val; i++)
		     if (i != index)
		       {
			sink = get_subscript_ptr(dg[edge].sink)->copies[i]; 
			add_edge(source,sink,src_stmt,ut_get_stmt(sink),edge,
				 thresh,upd_info->level,upd_info->ped);
		       }
		  }
	       }
	  }
       }
     return(WALK_CONTINUE);
  }

static void replicate_body(AST_INDEX     stmt_list,
			   int           val,
			   int           level,
			   char          *ivar,
			   AST_INDEX     step,
			   PedInfo       ped,
			   SymDescriptor symtab,
			   Boolean       inner_rdx,
			   int           surrounding_do,
			   AST_INDEX     surround_node,
			   char          *inner_ivar,
			   int           inner_level,
			   arena_type    *ar)
  
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   dupd_info_type upd_info;
   AST_INDEX     *new_code;
   int           i,step_v,
                 vector;
   ref_info_type ref_info;
   
     if (step == AST_NIL)
       step_v = 1;
     else 
       (void)pt_eval(step,&step_v);
     new_code = (AST_INDEX *)ar->arena_alloc_mem(LOOP_ARENA,
						 val * sizeof(AST_INDEX));
     ref_info.symtab = symtab;
     ref_info.level = level;
     ref_info.surrounding_do = surrounding_do;
     ref_info.surround_node = surround_node;
     for (i = 0; i < val-1; i++)
       {
	new_code[i] = ut_tree_copy_with_type(stmt_list,i,ar);
	ref_info.val  = i;
	walk_expression(new_code[i],update_refs,NOFUNC,(Generic)&ref_info);
	/* set_stmt_ptrs_to_NULL(new_code[i]); */
	pt_var_add(new_code[i],ivar,step_v*(i+1));
	set_level_vectors(stmt_list,new_code[i],ped);
	ut_update_labels(new_code[i],symtab);
       }
     new_code[val-1] = ut_tree_copy_with_type(stmt_list,val-1,ar);
     ref_info.val = val-1;
     walk_expression(new_code[val-1],update_refs,NOFUNC,(Generic)&ref_info);
     /* set_stmt_ptrs_to_NULL(new_code[val-1]); */
     pt_var_add(new_code[val-1],ivar,step_v*val);
     set_level_vectors(stmt_list,new_code[val-1],ped);
     ut_update_labels(stmt_list,symtab);
     upd_info.level = level;
     upd_info.ped = ped;
     upd_info.symtab = symtab;
     upd_info.val = val;
     upd_info.ivar = inner_ivar;
     upd_info.inner_level = inner_level;
     walk_expression(stmt_list,update_graph,NOFUNC,(Generic)&upd_info);
     if (inner_rdx)
       for (i = 0; i < val; i++)
	 {
	  upd_info.val = i;
	  walk_statements(new_code[i],level,update_rdx_var,NOFUNC,
			  (Generic)&upd_info);
	 } 
     for (i = 0; i < val; i++)
       stmt_list = list_append(stmt_list,new_code[i]);
  }
	
int mh_copy_edges(AST_INDEX node,
		  Generic   ped)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX  source,
              sink,
              name;
   EDGE_INDEX edge;
   DG_Edge    *dg;
   int        vector;   

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	dg = dg_get_edge_structure( PED_DG(((PedInfo)ped)));
	vector = get_info((PedInfo)ped,name,type_levelv);
	for (edge = dg_first_src_ref( PED_DG(((PedInfo)ped)),vector);
	     edge != END_OF_LIST;
	     edge = dg_next_src_ref( PED_DG(((PedInfo)ped)),edge))
         if ((dg[edge].type == dg_true || dg[edge].type == dg_anti ||
	      dg[edge].type == dg_input || dg[edge].type == dg_output) &&
	     (get_subscript_ptr(dg[edge].src)->surround_node == 
	      get_subscript_ptr(dg[edge].sink)->surround_node))
	   {
	    source = get_subscript_ptr(dg[edge].src)->copies[0];
	    sink = get_subscript_ptr(dg[edge].sink)->copies[0];
	    add_edge(source,sink,ut_get_stmt(source),ut_get_stmt(sink),
		     edge,gen_get_dt_DIS(&dg[edge],dg[edge].level),
		     dg[edge].level,(PedInfo)ped);
	   }
       }
     return(WALK_CONTINUE);
  }

static int add_level_to_edges(AST_INDEX node,
			      edge_info_type *edge_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX  name;
   EDGE_INDEX edge;
   DG_Edge    *dg;
   int        vector;   
   int        i;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	dg = dg_get_edge_structure( PED_DG(edge_info->ped));
	vector = get_info(edge_info->ped,name,type_levelv);
	for (edge = dg_first_src_ref( PED_DG(edge_info->ped),vector);
	     edge != END_OF_LIST;
	     edge = dg_next_src_ref( PED_DG(edge_info->ped),edge))
         if (dg[edge].level >= edge_info->level)
	   {
	    gen_put_dt_LVL(&dg[edge],gen_get_dt_LVL(&dg[edge])+1);
	    for (i = gen_get_dt_LVL(&dg[edge]);i > edge_info->level;i--)
	       gen_put_dt_DIS(&dg[edge],i,gen_get_dt_DIS(&dg[edge],i-1));
	    gen_put_dt_DIS(&dg[edge],edge_info->level,0);
	   dg[edge].level++;
	   }
       }
     return(WALK_CONTINUE);
  }

static int count_loops(model_loop *loop_data,
		       int        loop)

  {
   int next;
   int count = 1;

     for (next = loop_data[loop].inner_loop;
	  next != -1;
	  next = loop_data[next].next_loop)
       count += count_loops(loop_data,next);
     return(count);
  }

static void copy_unroll_amounts(model_loop *loop_data,
				model_loop *split_loop,
				int        loop,
				int        sloop)

  {
   int next,
       snext;

     split_loop[sloop].max = loop_data[loop].max;
     split_loop[sloop].val = loop_data[loop].val;
     split_loop[sloop].count = loop_data[loop].count;
     split_loop[sloop].unroll = loop_data[loop].unroll;
     for (next = loop_data[loop].inner_loop,
	  snext = split_loop[sloop].inner_loop;
	  next != -1;
	  next = loop_data[next].next_loop,
	  snext = split_loop[snext].next_loop)
       copy_unroll_amounts(loop_data,split_loop,next,snext);
  }

static void unroll_rhomboid(model_loop    *loop_data,
			    int           loop,
			    int           uloop,
			    PedInfo       ped,
			    SymDescriptor symtab,
			    AST_INDEX     step,
			    arena_type    *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int       size;
   AST_INDEX loop_copy,arg_list,max,control1,control2,new_control,
             ivar,new_do,min;
   char      new_ivar[20];
   pre_info_type pre_info;
   edge_info_type edge_info;
   model_loop *split_loop;
   copy_info_type copy_info;

     copy_info.val = 1;
     copy_info.ar = ar;
     copy_info.symtab = symtab;
     control1 = gen_DO_get_control(loop_data[uloop].node);
       
           /* create first loop */

     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),ut_init_copies,
		     NOFUNC,(Generic)&copy_info);
     loop_copy = ut_tree_copy_with_type(loop_data[loop].node,0,ar);
     ut_update_labels(loop_copy,symtab);
     ivar = gen_INDUCTIVE_get_name(gen_DO_get_control(loop_data[uloop].node));
     sprintf(new_ivar,"%s$%d",gen_get_text(ivar),loop_data[uloop].level);
     pt_var_replace(gen_DO_get_stmt_LIST(loop_copy),gen_get_text(ivar),
		    pt_gen_ident(new_ivar));
     control2 = gen_DO_get_control(loop_copy);
     arg_list = list_create(pt_simplify_expr(
				 pt_gen_add(
				   pt_gen_mul(
				     pt_gen_int(loop_data[uloop].tri_coeff),
			             pt_gen_add(tree_copy_with_type(ivar),
				       pt_gen_int(loop_data[uloop].val-1))),
			           tree_copy_with_type(
						loop_data[uloop].tri_const))));
     arg_list = list_insert_last(arg_list, pt_simplify_expr(
				 pt_gen_add(
				   pt_gen_mul(
				     pt_gen_int(loop_data[uloop].tri_coeff),
			             pt_gen_add(tree_copy_with_type(ivar),
				       pt_gen_int(loop_data[uloop].val-1))),
			           tree_copy_with_type(
				       loop_data[uloop].rhom_const))));
     min = gen_INVOCATION(pt_gen_ident("min"),arg_list);
     pt_tree_replace(gen_INDUCTIVE_get_rvalue1(control2),pt_simplify_expr(
		     pt_gen_add(
		        pt_gen_mul(pt_gen_int(loop_data[uloop].tri_coeff),
				   pt_gen_ident(new_ivar)),
			tree_copy_with_type(loop_data[uloop].tri_const))));
     pt_tree_replace(gen_INDUCTIVE_get_rvalue2(control2),min);
     new_control = gen_INDUCTIVE(pt_gen_ident(new_ivar),
				 tree_copy_with_type(ivar),
				 pt_simplify_expr(
			           pt_gen_add(tree_copy_with_type(ivar),
				         pt_gen_int(loop_data[uloop].val-1))),
				 AST_NIL);
     new_do=gen_DO(AST_NIL,AST_NIL,AST_NIL,new_control,list_create(loop_copy));

     (void)list_insert_before(loop_data[loop].node,new_do);
     set_level_vectors(gen_DO_get_stmt_LIST(loop_data[loop].node),
		       gen_DO_get_stmt_LIST(loop_copy),ped);
     walk_expression(loop_data[loop].node,mh_copy_edges,NOFUNC,(Generic)ped);
     edge_info.ped = ped;
     edge_info.level = loop_data[uloop].level;
     walk_expression(loop_copy,add_level_to_edges,NOFUNC,(Generic)&edge_info);
     size = count_loops(loop_data,loop) + 1;
     split_loop = (model_loop *)ar->arena_alloc_mem_clear(LOOP_ARENA,
						  size*sizeof(model_loop));
     pre_info.stmt_num = 0;
     pre_info.loop_num = 0;
     pre_info.surrounding_do = 0;
     pre_info.surround_node = loop_data[loop_data[loop].parent].node;
     pre_info.ped = ped;
     pre_info.symtab = symtab;
     pre_info.ar = ar;
     walk_statements(new_do,loop_data[loop].level,ut_mark_do_pre,
		     ut_mark_do_post,(Generic)&pre_info);
     ut_analyze_loop(new_do,split_loop,loop_data[loop].level,ped,symtab);
     split_loop[0].max = 0;
     split_loop[0].val = 0;
     copy_unroll_amounts(loop_data,split_loop,loop,split_loop[0].inner_loop);
     ut_check_shape(split_loop,0);
     util_append(loop_data[loop].split_list,
		 util_node_alloc((Generic)split_loop,"split-list"));

           /* create last loop */

     copy_info.val = 1;
     copy_info.ar = ar;
     copy_info.symtab = symtab;
     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),ut_init_copies,
		     NOFUNC,(Generic)&copy_info);
     loop_copy = ut_tree_copy_with_type(loop_data[loop].node,0,ar);
     ut_update_labels(loop_copy,symtab);
     ivar = gen_INDUCTIVE_get_name(gen_DO_get_control(loop_data[uloop].node));
     sprintf(new_ivar,"%s$%d",gen_get_text(ivar),loop_data[uloop].level);
     pt_var_replace(gen_DO_get_stmt_LIST(loop_copy),gen_get_text(ivar),
		    pt_gen_ident(new_ivar));
     control2 = gen_DO_get_control(loop_copy);
     arg_list = list_create(pt_simplify_expr(
				 pt_gen_add(
				   pt_gen_mul(
				     pt_gen_int(loop_data[uloop].tri_coeff),
			             pt_gen_add(tree_copy_with_type(ivar),
						pt_gen_int(1))),
					    tree_copy_with_type(
						loop_data[uloop].tri_const))));
     arg_list = list_insert_last(arg_list, pt_simplify_expr(
				 pt_gen_add(
				   pt_gen_mul(
				     pt_gen_int(loop_data[uloop].tri_coeff),
			             pt_gen_add(tree_copy_with_type(ivar),
						pt_gen_int(1))),
					    tree_copy_with_type(
				               loop_data[uloop].rhom_const))));
     max = gen_INVOCATION(pt_gen_ident("max"),arg_list);
     pt_tree_replace(gen_INDUCTIVE_get_rvalue1(control2),max);
     pt_tree_replace(gen_INDUCTIVE_get_rvalue2(control2),pt_simplify_expr(
		     pt_gen_add(pt_gen_mul(
					pt_gen_int(loop_data[uloop].tri_coeff),
					pt_gen_ident(new_ivar)),
			   tree_copy_with_type(loop_data[uloop].rhom_const))));
     new_control = gen_INDUCTIVE(pt_gen_ident(new_ivar),
				 pt_gen_add(tree_copy_with_type(ivar),
					    pt_gen_int(1)),
				 pt_simplify_expr(
				   pt_gen_add(
				     tree_copy_with_type(ivar),
				     pt_gen_int(loop_data[uloop].val))),
				 AST_NIL);
     new_do=gen_DO(AST_NIL,AST_NIL,AST_NIL,new_control,list_create(loop_copy));
     (void)list_insert_after(loop_data[loop].node,new_do);
     set_level_vectors(gen_DO_get_stmt_LIST(loop_data[loop].node),
		       gen_DO_get_stmt_LIST(loop_copy),ped);
     walk_expression(loop_data[loop].node,mh_copy_edges,NOFUNC,(Generic)ped);
     edge_info.ped = ped;
     edge_info.level = loop_data[uloop].level;
     walk_expression(loop_copy,add_level_to_edges,NOFUNC,(Generic)&edge_info);
     size = count_loops(loop_data,loop) + 1;
     split_loop = (model_loop *)ar->arena_alloc_mem_clear(LOOP_ARENA,
						  size*sizeof(model_loop));
     pre_info.stmt_num = 0;
     pre_info.loop_num = 0;
     pre_info.surrounding_do = 0;
     pre_info.surround_node = loop_data[loop_data[loop].parent].node;
     pre_info.ped = ped;
     pre_info.symtab = symtab;
     pre_info.ar = ar;
     walk_statements(new_do,loop_data[loop].level,ut_mark_do_pre,
		     ut_mark_do_post,(Generic)&pre_info);
     ut_analyze_loop(new_do,split_loop,loop_data[loop].level,ped,symtab);
     split_loop[0].max = 0;
     split_loop[0].val = 0;
     copy_unroll_amounts(loop_data,split_loop,loop,split_loop[0].inner_loop);
     ut_check_shape(split_loop,0);
     util_append(loop_data[loop].split_list,
		 util_node_alloc((Generic)split_loop,"split-list"));

        /* update node bounds */

     pt_tree_replace(gen_INDUCTIVE_get_rvalue1(gen_DO_get_control(
                     loop_data[loop].node)),pt_simplify_expr(
		     pt_gen_add(
		       pt_gen_mul(pt_gen_int(loop_data[uloop].tri_coeff),
			          pt_gen_add(tree_copy_with_type(ivar),
				    pt_gen_int(loop_data[uloop].val))),
		       tree_copy_with_type(loop_data[uloop].tri_const))));
							  

        /* continue unroll-and-jam */

     if (loop_data[loop].inner_loop == -1)
      {
       copy_info.val = loop_data[uloop].val;
       walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
		       ut_init_copies,NOFUNC,(Generic)&copy_info);
       replicate_body(gen_DO_get_stmt_LIST(loop_data[loop].node),
	              loop_data[uloop].val,loop_data[uloop].level,
		      gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
		      loop_data[uloop].node))),step,ped,symtab,false,
		      get_stmt_info_ptr(loop_data[loop].node)->loop_num,
		      loop_data[loop].node,
		      gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
                      loop_data[loop].node))),
		      loop_data[loop].level,ar);
      }
     else
       {
	if (loop_data[loop].stmts)
	  replicate_statements(gen_DO_get_stmt_LIST(loop_data[loop].node),
			       loop_data[uloop].val,loop_data[uloop].level,
			       gen_get_text(gen_INDUCTIVE_get_name(
			       gen_DO_get_control(loop_data[uloop].node))),
			       step,symtab,loop,loop_data[loop].node,ar);
	walk_loops_to_unroll(loop_data,loop_data[loop].inner_loop,uloop,ped,
			     symtab,step,ar);
       }
  }

static void unroll_triangular(model_loop    *loop_data,
			      int           loop,
			      int           uloop,
			      PedInfo       ped,
			      SymDescriptor symtab,
			      AST_INDEX     step,
			      arena_type    *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int       size;
   AST_INDEX loop_copy,arg_list,max,control1,control2,new_control,
             ivar,new_do,min;
   char      new_ivar[20];
   pre_info_type pre_info;
   edge_info_type edge_info;
   model_loop *split_loop;
   copy_info_type copy_info;

     copy_info.val = 1;
     copy_info.ar = ar;
     copy_info.symtab = symtab;
     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),ut_init_copies,
		     NOFUNC,(Generic)&copy_info);
     loop_copy = ut_tree_copy_with_type(loop_data[loop].node,0,ar);
     ut_update_labels(loop_copy,symtab);
     control1 = gen_DO_get_control(loop_data[loop].node);
     ivar = gen_INDUCTIVE_get_name(gen_DO_get_control(loop_data[uloop].node));
     sprintf(new_ivar,"%s$%d",gen_get_text(ivar),loop_data[uloop].level);
     pt_var_replace(loop_copy,gen_get_text(ivar),pt_gen_ident(new_ivar));
     new_control = gen_INDUCTIVE(pt_gen_ident(new_ivar),AST_NIL,AST_NIL,
				 AST_NIL);
     new_do=gen_DO(AST_NIL,AST_NIL,AST_NIL,new_control,list_create(loop_copy));
     control2 = gen_DO_get_control(loop_copy);
     switch(loop_data[uloop].type) {
       case TRI_UL:
         gen_INDUCTIVE_put_rvalue1(new_control,tree_copy_with_type(ivar));
	 gen_INDUCTIVE_put_rvalue2(new_control,pt_gen_add(tree_copy_with_type(ivar),
				   pt_gen_int(loop_data[uloop].val-1)));
	 pt_tree_replace(gen_INDUCTIVE_get_rvalue1(control2),pt_simplify_expr(
			 pt_gen_add(
			   pt_gen_mul(pt_gen_int(loop_data[uloop].tri_coeff),
				      pt_gen_ident(new_ivar)),
			   tree_copy_with_type(loop_data[uloop].tri_const))));
	 arg_list =list_create(tree_copy_with_type(gen_INDUCTIVE_get_rvalue2(control2)));
	 arg_list = list_insert_last(arg_list,pt_simplify_expr(
			 pt_gen_add(
			   pt_gen_mul(
                             pt_gen_int(loop_data[uloop].tri_coeff),
			     pt_gen_add(
                               tree_copy_with_type(ivar),
                               pt_gen_int(loop_data[uloop].val))),
                           pt_gen_sub(
			     tree_copy_with_type(loop_data[uloop].tri_const),
			     pt_gen_int(1)))));
	 min = gen_INVOCATION(pt_gen_ident("min"),arg_list);
	 pt_tree_replace(gen_INDUCTIVE_get_rvalue2(control2),min);
	 pt_tree_replace(gen_INDUCTIVE_get_rvalue1(control1),pt_simplify_expr(
			 pt_gen_add(
			   pt_gen_mul(
                             pt_gen_int(loop_data[uloop].tri_coeff),
			     pt_gen_add(
                               tree_copy_with_type(ivar),
                               pt_gen_int(loop_data[uloop].val))),
			   tree_copy_with_type(loop_data[uloop].tri_const))));
	 (void)list_insert_before(loop_data[loop].node,new_do);
         break;
       case TRI_LL:
         gen_INDUCTIVE_put_rvalue1(new_control,tree_copy_with_type(ivar));
	 gen_INDUCTIVE_put_rvalue2(new_control,pt_gen_add(tree_copy_with_type(ivar),
				   pt_gen_int(loop_data[uloop].val-1)));
	 arg_list =list_create(tree_copy_with_type(gen_INDUCTIVE_get_rvalue1(control2)));
	 arg_list = list_insert_last(arg_list,pt_simplify_expr(
			 pt_gen_add(
			   pt_gen_mul(
                             pt_gen_int(loop_data[uloop].tri_coeff),
			     pt_gen_add(
                               tree_copy_with_type(ivar),
                               pt_gen_int(loop_data[uloop].val))),
                           pt_gen_add(
			     tree_copy_with_type(loop_data[uloop].tri_const),
			     pt_gen_int(1)))));
         max = gen_INVOCATION(pt_gen_ident("max"),arg_list);
         pt_tree_replace(gen_INDUCTIVE_get_rvalue1(control2),max);
         pt_tree_replace(gen_INDUCTIVE_get_rvalue2(control2),pt_simplify_expr(
			 pt_gen_add(
			   pt_gen_mul(
			     pt_gen_int(loop_data[uloop].tri_coeff),
			     pt_gen_ident(new_ivar)),
			   tree_copy_with_type(loop_data[uloop].tri_const))));
	 pt_tree_replace(gen_INDUCTIVE_get_rvalue2(control1),pt_simplify_expr(
			 pt_gen_add(
			   pt_gen_mul(
                             pt_gen_int(loop_data[uloop].tri_coeff),
			     pt_gen_add(
                               tree_copy_with_type(ivar),
                               pt_gen_int(loop_data[uloop].val))),
			   tree_copy_with_type(loop_data[uloop].tri_const))));
	 (void)list_insert_after(loop_data[loop].node,new_do);
         break;
       case TRI_UR:
         gen_INDUCTIVE_put_rvalue1(new_control,pt_gen_add(tree_copy_with_type(ivar),
							  pt_gen_int(1)));
         gen_INDUCTIVE_put_rvalue2(new_control,pt_gen_add(tree_copy_with_type(ivar),
				   pt_gen_int(loop_data[uloop].val)));
	 pt_tree_replace(gen_INDUCTIVE_get_rvalue1(control2),pt_simplify_expr(
			 pt_gen_add(
			   pt_gen_mul(
                             pt_gen_int(loop_data[uloop].tri_coeff),
                             pt_gen_ident(new_ivar)),
			   tree_copy_with_type(loop_data[uloop].tri_const))));
	 arg_list =list_create(tree_copy_with_type(gen_INDUCTIVE_get_rvalue2(control2)));
	 arg_list = list_insert_last(arg_list,pt_simplify_expr(
			 pt_gen_add(
			   pt_gen_mul(
			     pt_gen_int(loop_data[uloop].tri_coeff),
			     tree_copy_with_type(ivar)),
			   pt_gen_sub(
                             tree_copy_with_type(loop_data[uloop].tri_const),
			     pt_gen_int(1)))));
	 min = gen_INVOCATION(pt_gen_ident("min"),arg_list);
         pt_tree_replace(gen_INDUCTIVE_get_rvalue2(control2),min);
	 (void)list_insert_before(loop_data[loop].node,new_do);
         break;
       case TRI_LR:
         gen_INDUCTIVE_put_rvalue1(new_control,pt_gen_add(tree_copy_with_type(ivar),
							  pt_gen_int(1)));
         gen_INDUCTIVE_put_rvalue2(new_control,pt_gen_add(tree_copy_with_type(ivar),
				   pt_gen_int(loop_data[uloop].val)));
         arg_list =list_create(tree_copy_with_type(gen_INDUCTIVE_get_rvalue1(control2)));
         arg_list = list_insert_last(arg_list,pt_simplify_expr(
				pt_gen_add(
				  pt_gen_add(
			            pt_gen_mul(
				      pt_gen_int(loop_data[uloop].tri_coeff),
				      tree_copy_with_type(ivar)),
			            tree_copy_with_type(loop_data[uloop].tri_const)),
				  pt_gen_int(1))));
         max = gen_INVOCATION(pt_gen_ident("max"),arg_list);
         pt_tree_replace(gen_INDUCTIVE_get_rvalue1(control2),max);
         pt_tree_replace(gen_INDUCTIVE_get_rvalue2(control2),pt_simplify_expr(
			 pt_gen_add(
			   pt_gen_mul(
			     pt_gen_int(loop_data[uloop].tri_coeff),
			     pt_gen_ident(new_ivar)),
			   tree_copy_with_type(loop_data[uloop].tri_const))));
	 (void)list_insert_after(loop_data[loop].node,new_do);
         break;
      }
     set_level_vectors(gen_DO_get_stmt_LIST(loop_data[loop].node),
		       gen_DO_get_stmt_LIST(loop_copy),ped);
     walk_expression(loop_data[loop].node,mh_copy_edges,NOFUNC,(Generic)ped);
     edge_info.ped = ped;
     edge_info.level = loop_data[uloop].level;
     walk_expression(loop_copy,add_level_to_edges,NOFUNC,(Generic)&edge_info);
     size = count_loops(loop_data,loop) + 1;
     split_loop = (model_loop *)ar->arena_alloc_mem_clear(LOOP_ARENA,
						  size*sizeof(model_loop));
     pre_info.stmt_num = 0;
     pre_info.loop_num = 0;
     pre_info.surrounding_do = 0;
     pre_info.surround_node = loop_data[loop_data[loop].parent].node;
     pre_info.ped = ped;
     pre_info.symtab = symtab;
     pre_info.ar = ar;
     walk_statements(new_do,loop_data[loop].level,ut_mark_do_pre,
		     ut_mark_do_post,(Generic)&pre_info);
     ut_analyze_loop(new_do,split_loop,loop_data[loop].level,ped,symtab);
     split_loop[0].max = 0;
     split_loop[0].val = 0;
     copy_unroll_amounts(loop_data,split_loop,loop,split_loop[0].inner_loop);
     ut_check_shape(split_loop,0);
     util_append(loop_data[loop].split_list,
		 util_node_alloc((Generic)split_loop,"split-list"));
     if (loop_data[loop].inner_loop == -1)
      {
       copy_info.val = loop_data[uloop].val;
       walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
		       ut_init_copies,NOFUNC,(Generic)&copy_info);
       replicate_body(gen_DO_get_stmt_LIST(loop_data[loop].node),
	              loop_data[uloop].val,loop_data[uloop].level,
		      gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
		      loop_data[uloop].node))),step,ped,symtab,false,
		      get_stmt_info_ptr(loop_data[loop].node)->loop_num,
		      loop_data[loop].node,
		      gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
                      loop_data[loop].node))),
		      loop_data[loop].level,ar);
      }
     else
       {
	if (loop_data[loop].stmts)
	  replicate_statements(gen_DO_get_stmt_LIST(loop_data[loop].node),
			       loop_data[uloop].val,loop_data[uloop].level,
			       gen_get_text(gen_INDUCTIVE_get_name(
			       gen_DO_get_control(loop_data[uloop].node))),
			       step,symtab,loop,loop_data[loop].node,ar);
	walk_loops_to_unroll(loop_data,loop_data[loop].inner_loop,uloop,ped,
			     symtab,step,ar);
       }
  }

static void walk_loops_to_unroll(model_loop    *loop_data,
				 int           loop,
				 int           unroll_loop,
				 PedInfo       ped,
				 SymDescriptor symtab,
				 AST_INDEX     step,
				 arena_type    *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int      next;
   UtilNode *listnode;
   copy_info_type copy_info;

    for (listnode = UTIL_HEAD(loop_data[loop].split_list);
	 listnode != NULLNODE;
	 listnode = UTIL_NEXT(listnode))
       walk_loops_to_unroll((model_loop *)util_node_atom(listnode),0,
			    unroll_loop,ped,symtab,step,ar);
    if (loop_data[unroll_loop].tri_loop == loop)
      if (loop_data[unroll_loop].type == RHOM)
        unroll_rhomboid(loop_data,loop,unroll_loop,ped,symtab,step,ar);
      else
        unroll_triangular(loop_data,loop,unroll_loop,ped,symtab,step,ar);
    else if (loop_data[loop].inner_loop == -1)
      {
       copy_info.val = loop_data[unroll_loop].val;
       copy_info.ar = ar;
       copy_info.symtab = symtab;
       walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
		       ut_init_copies,NOFUNC,(Generic)&copy_info);
       replicate_body(gen_DO_get_stmt_LIST(loop_data[loop].node),
	              loop_data[unroll_loop].val,loop_data[unroll_loop].level,
		      gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
		      loop_data[unroll_loop].node))),step,ped,symtab,false,
		      get_stmt_info_ptr(loop_data[loop].node)->loop_num,
		      loop_data[loop].node,
		      gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
                      loop_data[loop].node))),
		      loop_data[loop].level,ar);
      }
    else
      {
       if (loop_data[loop].stmts)
         replicate_statements(gen_DO_get_stmt_LIST(loop_data[loop].node),
		  	      loop_data[unroll_loop].val,
			      loop_data[unroll_loop].level,gen_get_text(
                              gen_INDUCTIVE_get_name(gen_DO_get_control(
                              loop_data[unroll_loop].node))),step,symtab,loop,
			      loop_data[loop].node,ar);
       for (next = loop_data[loop].inner_loop;
	    next != -1;
	    next = loop_data[next].next_loop)
         walk_loops_to_unroll(loop_data,next,unroll_loop,ped,symtab,step,ar);
      }
  }

static model_loop *split_trap_upb(model_loop    *loop_data,
				  int           uloop,
				  int           loop,
				  PedInfo       ped,
				  SymDescriptor symtab,
				  arena_type    *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int       size,coeff1,coeff;
   AST_INDEX loop_copy,arg_list,max,control1,control2,
             ivar,min,con,con1,con2,fac,temp,lwb,upb;
   char      new_ivar[20];
   pre_info_type pre_info;
   Boolean   lin;
   model_loop *split_loop;
   copy_info_type copy_info;

     copy_info.val = 1;
     copy_info.ar = ar;
     copy_info.symtab = symtab;
     walk_expression(gen_DO_get_stmt_LIST(loop_data[uloop].node),
		     ut_init_copies,NOFUNC,(Generic)&copy_info);
     control1 = gen_DO_get_control(loop_data[loop].node);
     ivar = gen_INDUCTIVE_get_name(gen_DO_get_control(loop_data[uloop].node));
     lwb = gen_INDUCTIVE_get_rvalue1(control1);
     pt_separate_linear(lwb,gen_get_text(ivar),&lin,&fac,&con);
     pt_fold_term(fac,&temp,&coeff);
     upb = gen_INDUCTIVE_get_rvalue2(control1);
     arg_list = gen_INVOCATION_get_actual_arg_LIST(upb);
     if (pt_find_var(list_first(arg_list),gen_get_text(ivar)))
       {
	pt_separate_linear(list_first(arg_list),gen_get_text(ivar),&lin,&fac,
			   &con1);
	pt_fold_term(fac,&temp,&coeff1);
	con2 = list_last(arg_list);
       }
     else
       {
	pt_separate_linear(list_last(arg_list),gen_get_text(ivar),&lin,&fac,
			   &con1);
	pt_fold_term(fac,&temp,&coeff1);
	con2 = list_first(arg_list);
       }
     tree_replace(upb,pt_simplify_expr(pt_gen_add(pt_gen_mul(
							 pt_gen_int(coeff1),
							 tree_copy_with_type(ivar)),
						  tree_copy_with_type(con1))));
     loop_copy = ut_tree_copy_with_type(loop_data[uloop].node,0,ar);
     ut_update_labels(loop_copy,symtab);
     control2 = gen_DO_get_control(loop_copy);
     arg_list =list_create(tree_copy_with_type(gen_INDUCTIVE_get_rvalue2(control2)));
     arg_list=list_insert_last(arg_list,pt_simplify_expr(
			       pt_gen_div(pt_gen_sub(tree_copy_with_type(con2),
						     tree_copy_with_type(con1)),
					  tree_copy_with_type(pt_gen_int(coeff1)))));
     min = gen_INVOCATION(pt_gen_ident("min"),arg_list);
     pt_tree_replace(gen_INDUCTIVE_get_rvalue2(control1),tree_copy_with_type(con2));
     arg_list = list_create(pt_gen_add(tree_copy_with_type(min),pt_gen_int(1)));
     arg_list = list_insert_first(arg_list,tree_copy_with_type(gen_INDUCTIVE_get_rvalue1(
								   control2)));
     pt_tree_replace(gen_INDUCTIVE_get_rvalue2(control2),min);
     control2 = gen_DO_get_control(loop_data[uloop].node);
     max = gen_INVOCATION(pt_gen_ident("max"),arg_list);
     pt_tree_replace(gen_INDUCTIVE_get_rvalue1(control2),max);
     tree_free(upb);
     set_level_vectors(gen_DO_get_stmt_LIST(loop_data[uloop].node),
		       gen_DO_get_stmt_LIST(loop_copy),ped);
     walk_expression(loop_data[uloop].node,mh_copy_edges,NOFUNC,(Generic)ped);
     size = count_loops(loop_data,uloop) + 1;
     split_loop = (model_loop *)ar->arena_alloc_mem_clear(LOOP_ARENA,
						  size*sizeof(model_loop));
     pre_info.stmt_num = 0;
     pre_info.loop_num = 0;
     pre_info.surrounding_do = 0;
     pre_info.surround_node = loop_data[uloop].surround_node;
     pre_info.ped = ped;
     pre_info.symtab = symtab;
     pre_info.ar = ar;
     walk_statements(loop_copy,loop_data[uloop].level,ut_mark_do_pre,
		     ut_mark_do_post,(Generic)&pre_info);
     ut_analyze_loop(loop_copy,split_loop,loop_data[uloop].level,ped,
		     symtab);
     copy_unroll_amounts(loop_data,split_loop,uloop,0);
     ut_check_shape(split_loop,0);
     loop_data[uloop].type = RECT;
     ut_compare_loops(loop_data,uloop,loop);
     list_insert_before(loop_data[uloop].node,loop_copy);
     return(split_loop);
  }

static model_loop *split_trap_lwb(model_loop    *loop_data,
				  int           uloop,
				  int           loop,
				  PedInfo       ped,
				  SymDescriptor symtab,
				  arena_type    *ar)
				  
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int       size,coeff1,coeff;
   AST_INDEX loop_copy,arg_list,max,control1,control2,
             ivar,min,con,con1,con2,fac,temp,lwb,upb;
   char      new_ivar[20];
   pre_info_type pre_info;
   Boolean   lin;
   model_loop *split_loop;
   copy_info_type copy_info;

     copy_info.val = 1;
     copy_info.ar = ar;
     copy_info.symtab = symtab;
     walk_expression(gen_DO_get_stmt_LIST(loop_data[uloop].node),
		     ut_init_copies,NOFUNC,(Generic)&copy_info);
     control1 = gen_DO_get_control(loop_data[loop].node);
     ivar = gen_INDUCTIVE_get_name(gen_DO_get_control(loop_data[uloop].node));
     upb = gen_INDUCTIVE_get_rvalue2(control1);
     pt_separate_linear(upb,gen_get_text(ivar),&lin,&fac,&con);
     pt_fold_term(fac,&temp,&coeff);
     lwb = gen_INDUCTIVE_get_rvalue1(control1);
     arg_list = gen_INVOCATION_get_actual_arg_LIST(lwb);
     if (pt_find_var(list_first(arg_list),gen_get_text(ivar)))
       {
	pt_separate_linear(list_first(arg_list),gen_get_text(ivar),&lin,&fac,
			   &con1);
	pt_fold_term(fac,&temp,&coeff1);
	con2 = list_last(arg_list);
       }
     else
       {
	pt_separate_linear(list_last(arg_list),gen_get_text(ivar),&lin,&fac,
			   &con1);
	pt_fold_term(fac,&temp,&coeff1);
	con2 = list_first(arg_list);
       }
     tree_replace(lwb,pt_simplify_expr(pt_gen_add(pt_gen_mul(
							 pt_gen_int(coeff1),
							 tree_copy_with_type(ivar)),
						  tree_copy_with_type(con1))));
     loop_copy = ut_tree_copy_with_type(loop_data[uloop].node,0,ar);
     pt_tree_replace(gen_INDUCTIVE_get_rvalue1(control1),tree_copy_with_type(con2));
     ut_update_labels(loop_copy,symtab);
     control2 = gen_DO_get_control(loop_data[uloop].node);
     arg_list =list_create(tree_copy_with_type(gen_INDUCTIVE_get_rvalue2(control2)));
     arg_list=list_insert_last(arg_list,pt_simplify_expr(
			       pt_gen_div(pt_gen_sub(tree_copy_with_type(con2),
						     tree_copy_with_type(con1)),
					  tree_copy_with_type(pt_gen_int(coeff1)))));
     min = gen_INVOCATION(pt_gen_ident("min"),arg_list);
     arg_list = list_create(pt_gen_add(tree_copy_with_type(min),pt_gen_int(1)));
     pt_tree_replace(gen_INDUCTIVE_get_rvalue2(control2),min);
     control2 = gen_DO_get_control(loop_copy);
     arg_list = list_insert_first(arg_list,tree_copy_with_type(gen_INDUCTIVE_get_rvalue1(
								   control2)));
     max = gen_INVOCATION(pt_gen_ident("max"),arg_list);
     pt_tree_replace(gen_INDUCTIVE_get_rvalue1(control2),max);
     tree_free(lwb);
     set_level_vectors(gen_DO_get_stmt_LIST(loop_data[uloop].node),
		       gen_DO_get_stmt_LIST(loop_copy),ped);
     walk_expression(loop_data[uloop].node,mh_copy_edges,NOFUNC,(Generic)ped);
     size = count_loops(loop_data,uloop) + 1;
     split_loop = (model_loop *)ar->arena_alloc_mem_clear(LOOP_ARENA,
						  size*sizeof(model_loop));
     pre_info.stmt_num = 0;
     pre_info.loop_num = 0;
     pre_info.surrounding_do = 0;
     pre_info.surround_node = loop_data[uloop].surround_node;
     pre_info.ped = ped;
     pre_info.symtab = symtab;
     pre_info.ar = ar;
     walk_statements(loop_copy,loop_data[uloop].level,ut_mark_do_pre,
		     ut_mark_do_post,(Generic)&pre_info);
     ut_analyze_loop(loop_copy,split_loop,loop_data[uloop].level,ped,
		     symtab);
     copy_unroll_amounts(loop_data,split_loop,uloop,0);
     ut_check_shape(split_loop,0);
     loop_data[uloop].type = RECT;
     ut_compare_loops(loop_data,uloop,loop);
     list_insert_after(loop_data[uloop].node,loop_copy);
     return(split_loop);
  }

static void unroll_and_jam_pre_loop(PedInfo       ped,
				    AST_INDEX     stmt_list,
				    int           level,
				    int           num_loops,
				    SymDescriptor symtab,
				    arena_type    *ar)

  {
   AST_INDEX stmt,
             next_stmt;

     for (stmt = list_first(stmt_list);
	  stmt != AST_NIL;
	  stmt = next_stmt)
       {
	next_stmt = list_next(stmt);
	if (is_do(stmt))
          (void)memory_unroll_and_jam(ped,stmt,level,num_loops,symtab,ar);
       }
  }

static void create_pre_loop(model_loop    *loop_data,
			    int           loop,
			    int           num_loops,
			    int           loops_unrolled,
			    PedInfo       ped,
			    SymDescriptor symtab,
			    arena_type    *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
  

  {
   AST_INDEX control,new_loop,next,
             lwb,upb,step;
   Boolean   need_pre_loop = false;
   int       lwb_v,upb_v,step_v;
   copy_info_type copy_info;

     control = gen_DO_get_control(loop_data[loop].node);
     lwb = gen_INDUCTIVE_get_rvalue1(control);
     if (pt_eval(lwb,&lwb_v))
       need_pre_loop = true;
     else
       {
	upb = gen_INDUCTIVE_get_rvalue2(control);
	if (pt_eval(upb,&upb_v))
	  need_pre_loop = true;
	else
	  {
	   step = gen_INDUCTIVE_get_rvalue3(control);
	   if (step == AST_NIL)
	     step_v = 1;
	   else if (pt_eval(step,&step_v))
	     need_pre_loop = true;
	   if (!need_pre_loop)
	     if (mod((upb_v - lwb_v + 1)/step_v, loop_data[loop].val + 1)
	           != 0)
	       need_pre_loop = true;
	  }
       }
     if (need_pre_loop)
       {
	copy_info.ar = ar;
	copy_info.val = 1;
	copy_info.symtab = symtab;
	walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
			ut_init_copies,NOFUNC,(Generic)&copy_info);
	new_loop = ut_tree_copy_with_type(loop_data[loop].node,0,ar);
	set_level_vectors(gen_DO_get_stmt_LIST(loop_data[loop].node),
			  gen_DO_get_stmt_LIST(new_loop),ped);
        walk_expression(loop_data[loop].node,mh_copy_edges,NOFUNC,
			(Generic)ped);
	ut_update_labels(new_loop,symtab);
	ut_update_bounds(loop_data[loop].node,new_loop,loop_data[loop].val);
	if ((num_loops > 0 && loop_data[loop].inner_loop != -1) || 
	    (loops_unrolled == 0))
	  unroll_and_jam_pre_loop(ped,gen_DO_get_stmt_LIST(new_loop),
				  loop_data[loop].level+1,num_loops,symtab,
				  ar);
	list_insert_before(loop_data[loop].node,new_loop);
       }
     else 
       ut_update_bounds(loop_data[loop].node,AST_NIL,loop_data[loop].val);
  }
  

static int check_labels(AST_INDEX       stmt,
			int             level,
			label_info_type *label_info)

  {
   AST_INDEX label,
             lbl_ref;
   int       index;
   char      *lbl_str;

     if (labelled_stmt(stmt))
       if ((label = gen_get_label(stmt)) != AST_NIL)
	 {
	  set_label_sym_index(label,fst_QueryIndex(label_info->symtab,
						   gen_get_text(label)));
	  fst_PutFieldByIndex(label_info->symtab,get_label_sym_index(label),
			      label_info->fieldn,stmt);
	 }
     return(WALK_CONTINUE);
  }

static int count_flops(AST_INDEX      node,
		       flop_info_type *flop_info)

  {
   int ops;

     if (is_binary_op(node) || is_unary_minus(node))
        if (!is_binary_times(node) || !is_binary_plus(tree_out(node)) ||
	    !((config_type *)PED_MH_CONFIG(flop_info->ped))->mult_accum)
	  if (gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION ||
	      gen_get_converted_type(node) == TYPE_COMPLEX ||
	      gen_get_converted_type(node) == TYPE_REAL)
	    {
	     if (gen_get_converted_type(node) == TYPE_COMPLEX)
	       ops = 2;
	     else
	       ops = 1;
	     if (is_binary_times(node))
	       flop_info->flops+=((config_type *)PED_MH_CONFIG(flop_info->ped))
	                          ->mul_cycles * ops;
	     else if (is_binary_plus(node) || is_binary_minus(node))
	       flop_info->flops+=((config_type *)PED_MH_CONFIG(flop_info->ped))
	                          ->add_cycles * ops;
	     else if (is_binary_divide(node))
	       flop_info->flops+=((config_type *)PED_MH_CONFIG(flop_info->ped))
	                          ->div_cycles * ops;
	     else
               flop_info->flops += ops; 
	    }
     return(WALK_CONTINUE);
  }

static void insert_stmts(SymDescriptor  symtab,
			 fst_index_t    index,
			 rdx_stmts_type *rdx_stmts)

  {
   char      *new_var;
   AST_INDEX rval;
   int       i;

     if ((rval = fst_GetFieldByIndex(symtab,index,RDX_VAR)) != AST_NIL)
       {
	new_var = (char *)fst_GetFieldByIndex(symtab,index,SYMTAB_NAME);
	list_insert_first(rdx_stmts->prev,
			  gen_ASSIGNMENT(AST_NIL,pt_gen_ident(new_var),
					 tree_copy_with_type(rval)));
        list_insert_first(rdx_stmts->post,
			  gen_ASSIGNMENT(AST_NIL,tree_copy_with_type(rval),
					 pt_gen_add(tree_copy_with_type(rval),
						    pt_gen_ident(new_var))));
       }
  }

static void cleanup_rdx_var(AST_INDEX node)

  {
   tree_free(node);
  }

static void unroll_reduction(model_loop      *loop_data,
			     int             loop,
			     PedInfo         ped,
			     SymDescriptor   symtab,
			     label_info_type *label_info,
			     arena_type      *ar)

  {
   rdx_stmts_type  rdx_stmts;
   AST_INDEX       node,step;
   flop_info_type  flop_info;
   float           rhoL_lp;
   copy_info_type  copy_info;
   
     walk_statements_reverse(loop_data[loop].node,loop_data[loop].level,
			     NOFUNC,check_labels,(Generic)label_info);
     flop_info.ped = ped;
     flop_info.flops = 0;
     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),count_flops,
		     NOFUNC,(Generic)&flop_info);
     rhoL_lp = loop_data[loop].rho * 
               ((config_type *)PED_MH_CONFIG(ped))->pipe_length;
     if (rhoL_lp > (float)flop_info.flops)
       loop_data[loop].val = mh_increase_unroll(loop_data[loop].max,
						flop_info.flops,rhoL_lp) - 1;
     else
       if (loop_data[loop].val > loop_data[loop].max)
         loop_data[loop].val = loop_data[loop].max;
     if (loop_data[loop].val < 1)
       return;
     step = tree_copy_with_type(gen_INDUCTIVE_get_rvalue3(gen_DO_get_control(
                      loop_data[loop].node)));
     create_pre_loop(loop_data,loop,0,1,ped,symtab,ar);
     copy_info.val = loop_data[loop].val;
     copy_info.ar = ar;
     copy_info.symtab = symtab;
     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),ut_init_copies,
		     NOFUNC,(Generic)&copy_info);
     fst_InitField(symtab,RDX_VAR,AST_NIL,cleanup_rdx_var);
     replicate_body(gen_DO_get_stmt_LIST(loop_data[loop].node),
		    loop_data[loop].val,loop_data[loop].level,
		    gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
		    loop_data[loop].node))),step,ped,symtab,true,
		    get_stmt_info_ptr(loop_data[loop].node)->loop_num,
		    loop_data[loop].node,
		    gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
                    loop_data[loop].node))),
		    loop_data[loop].level,ar);
     rdx_stmts.prev = list_create(AST_NIL);
     rdx_stmts.post = list_create(AST_NIL);
     fst_ForAll(symtab,insert_stmts,(Generic)&rdx_stmts);
     if (!list_empty(rdx_stmts.prev))
       {
	for (node = list_remove_first(rdx_stmts.prev);
	     node != AST_NIL;
	     node = list_remove_first(rdx_stmts.prev))
	  list_insert_before(loop_data[loop].node,node);
	for (node = list_remove_first(rdx_stmts.post);
	     node != AST_NIL;
	     node = list_remove_first(rdx_stmts.post))
	  list_insert_after(loop_data[loop].node,node);
       }
     fst_KillField(symtab,RDX_VAR);
     tree_free(rdx_stmts.prev);
     tree_free(rdx_stmts.post);
  }

static void walk_loops(model_loop    *loop_data,
		       int           loop,
		       int           num_loops,
		       PedInfo       ped,
		       SymDescriptor symtab,
		       char          *fieldn,
		       int           loops_unrolled,
		       arena_type    *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/


  {
   int             next;
   label_info_type label_info;
   UtilNode        *listnode;
   model_loop      *split_loop,
                   *split_loop1,
		   *split_loop2;
   AST_INDEX       step;

     for (listnode = UTIL_HEAD(loop_data[loop].split_list);
	  listnode != NULLNODE;
	  listnode = UTIL_NEXT(listnode))
       walk_loops((model_loop *)util_node_atom(listnode),0,num_loops,ped,
		  symtab,fieldn,loops_unrolled,ar);
     label_info.symtab = symtab;
     label_info.fieldn = fieldn;
     if (loop_data[loop].val > 0 && loop_data[loop].type == TRAP)
       {
	switch(loop_data[loop].trap_fn) {
	  case FN_MIN:
	    split_loop = split_trap_upb(loop_data,loop,
					loop_data[loop].trap_loop,ped,symtab,
					ar);
	    break;
	  case FN_MAX:
	    split_loop = split_trap_lwb(loop_data,loop,
					loop_data[loop].trap_loop,ped,symtab,
					ar);
	    break;
	  case FN_BOTH:
	    split_loop2 = split_trap_lwb(loop_data,loop,
					 loop_data[loop].trap_loop,ped,symtab,
					 ar);
	    walk_statements_reverse(split_loop2[0].node,loop_data[loop].level,
				    NOFUNC,check_labels,(Generic)&label_info);
	    util_append(loop_data[loop].split_list,
			util_node_alloc((Generic)split_loop2,"split-list"));
	    split_loop1 = split_trap_upb(split_loop2,0,
					 split_loop2[0].trap_loop,ped,symtab,
					 ar);
	    walk_statements_reverse(split_loop1[0].node,loop_data[loop].level,
				    NOFUNC,check_labels,(Generic)&label_info);
	    util_append(split_loop2[0].split_list,
			util_node_alloc((Generic)split_loop1,"split-list"));
	    split_loop = split_trap_upb(loop_data,loop,
					loop_data[loop].trap_loop,ped,symtab,
					ar);
	    if (split_loop1[0].val > 0)
	      create_pre_loop(split_loop1,0,0,loops_unrolled,ped,symtab,ar);
	    if (split_loop2[0].val > 0)
	      create_pre_loop(split_loop2,0,0,loops_unrolled,ped,symtab,ar);
	    break;
	   }
	walk_statements_reverse(split_loop[0].node,loop_data[loop].level,
				NOFUNC,check_labels,(Generic)&label_info);
	if (split_loop[0].val > 0)
	  create_pre_loop(split_loop,0,0,loops_unrolled,ped,symtab,ar);
	util_append(loop_data[loop].split_list,
		    util_node_alloc((Generic)split_loop,"split-list"));
       }
     if (loop_data[loop].val > 0)

       /* after splitting a trapezoidal loop, the unroll val may become
	  zero; therefore, it is checked again */

       {
	walk_statements_reverse(loop_data[loop].node,loop_data[loop].level,
				NOFUNC,check_labels,(Generic)&label_info);
	step = tree_copy_with_type(gen_INDUCTIVE_get_rvalue3(gen_DO_get_control(
                         loop_data[loop].node)));
	create_pre_loop(loop_data,loop,num_loops-loops_unrolled,loops_unrolled,
			ped,symtab,ar); 
	walk_loops_to_unroll(loop_data,loop,loop,ped,symtab,step,ar);
	loops_unrolled++;
       }
    else if (loop_data[loop].inner_loop == -1 &&
	      loop_data[loop].reduction && loops_unrolled == 0)
       unroll_reduction(loop_data,loop,ped,symtab,&label_info,ar);
    for (next = loop_data[loop].inner_loop;
	 next != -1;
	 next = loop_data[next].next_loop)
      walk_loops(loop_data,next,num_loops,ped,symtab,fieldn,loops_unrolled,ar);
  }

void mh_do_unroll_and_jam(model_loop    *loop_data,
			  PedInfo       ped,
			  SymDescriptor symtab,
			  int           num_loops,
			  arena_type    *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
 
  {
   static int field = 0;
   char fieldn[20];
   int  logval;
   FILE *logfile;
   int  temp,loop;

     logval = ((config_type *)PED_MH_CONFIG(ped))->logging;
     logfile = ((config_type *)PED_MH_CONFIG(ped))->logfile;
     if ((logval == LOG_UNROLL) || (logval == LOG_ALL) &&
	 logfile != NULL)
       mh_log_data(loop_data,logfile);
     sprintf(fieldn,"mh: lbl_stmt%d",field++);
     if (field == 100)
       field = 0;
     fst_InitField(symtab,fieldn,AST_NIL,0);
     loop = 0;
     do
       {
	temp = loop_data[loop].next_loop;
	loop_data[loop].next_loop = -1;
	walk_loops(loop_data,loop,num_loops,ped,symtab,fieldn,2-num_loops,ar);
	loop_data[loop].next_loop = temp;
	loop = temp;
       } while (loop != -1); 
     fst_KillField(symtab,fieldn);
     logval = ((config_type *)PED_MH_CONFIG(ped))->logging;
     logfile = ((config_type *)PED_MH_CONFIG(ped))->logfile;
  }
