/* $Id: interchange.C,v 1.16 1997/04/09 18:37:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


/****************************************************************/
/*                                                              */
/*   File:        interchange.C                                 */
/*                                                              */
/*   Description: Performs loop interchange on a loop nest to   */
/*                improve cache performance.  Each loop in a    */
/*                perfect nest is considered as the innermost   */
/*                loop of the nest and then the loops are       */
/*                from inner to outer based upon increasing     */
/*                memory costs associated with each loop when   */
/*                it is in the innermost position.  Legality    */
/*                is preserved and distribution is performed    */
/*                when necessary.                               */
/*                                                              */
/****************************************************************/


#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#include <libs/Memoria/include/mh.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/Memoria/include/mh_config.h>

#ifndef header_h
#include <libs/Memoria/include/header.h>
#endif

#ifndef dt_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>
#endif

#ifndef gi_h
#include <libs/frontEnd/include/gi.h>
#endif

#ifndef interchange_h
#include <libs/Memoria/li/interchange.h>
#endif

#ifndef analyze_h
#include <libs/Memoria/include/analyze.h>
#endif

#ifndef shape_h
#include <libs/Memoria/include/shape.h>
#endif

#ifndef mem_util_h
#include <libs/Memoria/include/mem_util.h>
#endif

#ifndef mark_h
#include <libs/Memoria/include/mark.h>
#endif

#ifndef dg_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#endif

#include <libs/Memoria/li/MemoryOrder.h>
#include <libs/frontEnd/ast/cd_branch.h>


/****************************************************************************/
/*                                                                          */
/*   Function:   set_scratch                                                */
/*                                                                          */
/*   Input:      node - a node in the AST                                   */
/*               dummy - anything                                           */
/*                                                                          */
/*   Description: This function is called by walk_expression on each AST    */
/*                node.  It sets the scratch field to NULL so no spurrious  */
/*                pointers to space exist.                                  */
/*                                                                          */
/****************************************************************************/

static int set_scratch(AST_INDEX node,
		       int       dummy)

  {
   set_scratch_to_NULL(node);
   return(WALK_CONTINUE);
  }

static model_loop *PrepareLoopForInterchange(PedInfo       ped,
					     AST_INDEX     root,
					     int           level,
					     SymDescriptor symtab,
					     arena_type    *ar)

  {
   model_loop *loop_data;
   pre_info_type pre_info;

     pre_info.stmt_num = 0;
     pre_info.loop_num = 0;
     pre_info.surrounding_do = -1;
     pre_info.abort = false;
     pre_info.ped = ped;
     pre_info.symtab = symtab;
     pre_info.ar = ar;

     walk_expression(root,set_scratch,NOFUNC,(Generic)NULL);

       /* prepare nest for analyzing, record surrounding do information */

     walk_statements(root,level,(WK_STMT_CLBACK)ut_mark_do_pre,
		     (WK_STMT_CLBACK)ut_mark_do_post,(Generic)&pre_info);
     if (pre_info.abort)
       return NULL;

     loop_data = (model_loop *)ar->arena_alloc_mem_clear(LOOP_ARENA,
					 pre_info.loop_num*sizeof(model_loop));
       /* create loop structure */

     ut_analyze_loop(root,loop_data,level,ped,symtab);
     ut_check_shape(loop_data,0);


       /* determine order of each perfect loop nest */
      
     li_ComputeMemoryOrder(loop_data,symtab,ped,ar);
   
     return loop_data;
  }

/****************************************************************/
/*                                                              */
/*   Function:     add_edge                                     */
/*                                                              */
/*   Input:        ped - depedence graph handle                 */
/*                 dg  - dependence graph edge                  */
/*                 old_edge - edge from which a copy is made    */
/*                 level - new nesting level for edge           */
/*                                                              */
/*   Description:  Move a dependence edge from one level to     */
/*                 another.  This happens as a result of        */
/*                 interchange.  Does not delete old edge       */
/*                                                              */
/****************************************************************/


static void add_edge(PedInfo      ped,
		     DG_Edge      **old_dg,
		     EDGE_INDEX   old_edge,
		     int          level)
		

  {
   EDGE_INDEX new_edge;
   DG_Edge    *dg;
   int        dir;
   int        source,sink,temp,
              src_stmt,sink_stmt;

     new_edge = dg_alloc_edge( PED_DG(ped),old_dg);
     dg = *old_dg;
     dg[new_edge].src_str = NULL;
     dg[new_edge].sink_str = NULL;
     dg[new_edge].type = dg[old_edge].type;
     dg[new_edge].symbolic = dg[old_edge].symbolic;
     dg[new_edge].consistent = dg[old_edge].consistent;
     dg[new_edge].level = level;
     source = dg[old_edge].src;
     sink = dg[old_edge].sink;
     if (level != LOOP_INDEPENDENT && dg[old_edge].type == dg_input)
       {

	   /* determine if an input dependence changes direction */

	dir = gen_get_dt_DIS(&dg[old_edge],level);
        switch(dir)
	 {
	  case DDATA_GT: gen_put_dt_DIS(&dg[old_edge],level,DDATA_LT);
			 temp = source;
			 source = sink;
			 sink = temp;
			 break;
	  case DDATA_GE: gen_put_dt_DIS(&dg[old_edge],level,DDATA_LE);
			 temp = source;
			 source = sink;
			 sink = temp;
			 break;
	  default:       if (dir > DDATA_BASE && dir < 0)
	                   {
			    gen_put_dt_DIS(&dg[old_edge],level,-dir);
			    temp = source;
			    source = sink;
			    sink = temp;
			   }
			}
       }
     dg[new_edge].src = source;
     dg[new_edge].sink = sink;

     /* ref lists have already been created */

     dg[new_edge].src_ref = get_info(ped,source,type_levelv);
     dg[new_edge].sink_ref = get_info(ped,sink,type_levelv);

      /* hack because ast makes no sense (IF-statement construction) */

     src_stmt = ut_get_stmt(source);
     if (is_logical_if(tree_out(src_stmt)) || is_guard(src_stmt))
       src_stmt = tree_out(src_stmt);
     sink_stmt = ut_get_stmt(sink);
     if (is_logical_if(tree_out(sink_stmt)) || is_guard(sink_stmt))
       sink_stmt = tree_out(sink_stmt);

     /* stmt level vectors have already been created */

     dg[new_edge].src_vec = get_info(ped,src_stmt,type_levelv);
     dg[new_edge].sink_vec = get_info(ped,sink_stmt,type_levelv);

     dt_copy_info( PED_DT_INFO(ped),&dg[old_edge],&dg[new_edge]);
     dt_info_str( PED_DT_INFO(ped),&dg[new_edge]);
     dg_add_edge( PED_DG(ped),new_edge);
  }


/****************************************************************/
/*                                                              */
/*   Function:     update_edges                                 */
/*                                                              */
/*   Input:        node - index of AST node                     */
/*                 info - structure containing dependendence    */
/*                        graph, etc.                           */
/*                                                              */
/*   Description:  Walk the dependence edges for a node and     */
/*                 create loop independent edges for those that */
/*                 cross loop bodies.  This is done after       */
/*                 distribution.                                */
/*                                                              */
/****************************************************************/

static int update_edges(AST_INDEX node,
			level_info_type *info)

  {
   EDGE_INDEX edge,next_edge;;
   int        vector,j,surrounding_do;
   DG_Edge    *dg;
   AST_INDEX  name;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	vector = get_info(info->ped,name,type_levelv);
	dg = dg_get_edge_structure( PED_DG(info->ped));
	surrounding_do = get_stmt_info_ptr(ut_get_stmt(node))->surrounding_do;
	for (edge = dg_first_src_ref( PED_DG(info->ped),vector);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_src_ref( PED_DG(info->ped),edge);

	     /* look for carried edges that cross loop bodies, these will be
		loop independent after distribution */

	   if (dg[edge].level == info->level && 
	       get_stmt_info_ptr(ut_get_stmt(dg[edge].sink))->surrounding_do !=
	       surrounding_do)
	     {
	      if (dg[edge].type != dg_input ||
		  get_stmt_info_ptr(ut_get_stmt(dg[edge].src))->stmt_num >
		  get_stmt_info_ptr(ut_get_stmt(dg[edge].sink))->stmt_num)
		{
		 for (j = 1; j <= gen_get_dt_LVL(&dg[edge]);j++)
		   gen_put_dt_DIS(&dg[edge],j,0);
		 add_edge(info->ped,&dg,edge,LOOP_INDEPENDENT);
		}
	      dg_delete_free_edge( PED_DG(info->ped),edge);
	     }
	  }
       }
     return(WALK_CONTINUE);
  }


/****************************************************************/
/*                                                              */
/*   Function:     distribute_loop                              */
/*                                                              */
/*   Input:        loop_data - loop structure                   */
/*                 loop - index of loop to distribute           */
/*                 ped  - dependence graph handle               */
/*                 symtab - symbol table                        */
/*                 ar - arena for memory allocation             */
/*                                                              */
/*   Description:  Distribute a loop across each of the loop    */
/*                 nests at the next inner nesting level.       */
/*                                                              */
/****************************************************************/


static void distribute_loop(model_loop    *loop_data,
			    int           loop,
			    PedInfo       ped,
			    SymDescriptor symtab,
			    arena_type    *ar)
  {
   AST_INDEX stmt,
             stmt_list[MAXLOOP],
             next_stmt,
             new_do;
   int       i,j;
   level_info_type info;

     i = 0;
     stmt = list_first(gen_DO_get_stmt_LIST(loop_data[loop].node));

       /* create blocks of statements around which we distribute the loop.
	  a block consists of a list of non-do statements or a just one do-statement */

     while (stmt != AST_NIL)
       {
	stmt_list[i] = list_create(AST_NIL);
	while (!is_do(stmt) && stmt != AST_NIL)
	  {
	   next_stmt = list_next(stmt);
	   list_remove_node(stmt);
	   stmt_list[i] = list_insert_last(stmt_list[i],stmt);
	   stmt = next_stmt;
	  } 
	if (is_do(stmt))
	  {
	   if (!list_empty(stmt_list[i]))
	     {
	      i++;
	      stmt_list[i] = list_create(AST_NIL);
	     }
	   next_stmt = list_next(stmt);
	   list_remove_node(stmt);
	   stmt_list[i] = list_insert_last(stmt_list[i],stmt);
	   stmt = next_stmt;
	  }
	i++;
       }
     fprintf(stderr,"Loop at level %d has been distributed around %d statement blocks\n",loop_data[loop].level,i);
     info.ped = ped;
     info.level = loop_data[loop].level;
     tree_free(gen_DO_get_stmt_LIST(loop_data[loop].node));
     gen_DO_put_stmt_LIST(loop_data[loop].node,AST_NIL);

       /* distribute the loop around each block of statements */

     for (j = i-1; j > 0; j--)
       {
	walk_expression(stmt_list[j],(WK_EXPR_CLBACK)update_edges,NOFUNC,(Generic)&info);
	new_do = tree_copy_with_type(loop_data[loop].node);
	gen_DO_put_stmt_LIST(new_do,stmt_list[j]);
	list_insert_after(loop_data[loop].node,new_do);

	   /* perform interchange on each new nest if it is separate*/
	
	if (loop_data[loop].parent == -1)
	   memory_loop_interchange(ped,new_do,loop_data[loop].level,symtab,ar);
				   
       }
     walk_expression(stmt_list[0],(WK_EXPR_CLBACK)update_edges,NOFUNC,(Generic)&info);
     gen_DO_put_stmt_LIST(loop_data[loop].node,stmt_list[0]);
     if (loop_data[loop].parent == -1)

	   /* perform interchange on each new nest if it is separate*/
	
       memory_loop_interchange(ped,loop_data[loop].node,loop_data[loop].level,
			       symtab,ar);
     else
       /* perform interchange on whole new nest (may need outer distributions) */
       {
	for (i = loop;
	     loop_data[i].parent != -1;
	     i = loop_data[i].parent);

	   /* interchange only on outermost loop */

	memory_loop_interchange(ped,loop_data[i].node,loop_data[i].level,
				symtab,ar);
       }
  }


/****************************************************************/
/*                                                              */
/*   Function:      heap_position                               */
/*                                                              */
/*   Input:         heap - array of loops sorted order          */
/*                  loop - index of loop to be located          */
/*                  size - size of the array                    */
/*                                                              */
/*   Description:   Locate a loop index in the array of loops   */
/*                                                              */
/****************************************************************/


static int heap_position(heap_type *heap,
			 int       loop,
			 int       size)

  {
   int i;
   
     i = 0; 
     while (i <= size)
       {
	if (heap[i].index == loop)
	  return i;
	i++;
       }
     return -1;
  }


/****************************************************************/
/*                                                              */
/*   Function:      shift_loop                                  */
/*                                                              */
/*   Input:         loop_data - loop structure                  */
/*                  outer - index of loop to be moved inward    */
/*                  inner - index of loop inside which outer    */
/*                          loop is to be moved.                */
/*                  heap - sorted array of loop indices         */
/*                  size - size of heap                         */
/*                                                              */
/*   Description:   Move a loop inward to a new nesting level.  */
/*                  Handle triangular loops also.               */
/*                                                              */
/****************************************************************/


static void shift_loop(model_loop *loop_data,
		       int        outer,
		       int        inner,
		       heap_type  *heap,
		       int        size)

  {
   AST_INDEX stmt,
             slist1,
             slist2,
             lwb1,lwb2,upb1,upb2,control1,control2,
             ivar,const1;
   int       coeff;
             

             /* perform the loop shifting */

     slist1 = gen_DO_get_stmt_LIST(loop_data[outer].node);
     slist2 = gen_DO_get_stmt_LIST(loop_data[inner].node);
     gen_DO_put_stmt_LIST(loop_data[outer].node,AST_NIL);
     gen_DO_put_stmt_LIST(loop_data[inner].node,AST_NIL);
     stmt = list_remove_first(slist1);
     tree_free(slist1);
     tree_replace(loop_data[outer].node,stmt);
     gen_DO_put_stmt_LIST(loop_data[outer].node,slist2);
     gen_DO_put_stmt_LIST(loop_data[inner].node,
			  list_create(loop_data[outer].node));

             /* update triangular bounds if necessary */

     if (loop_data[outer].type != RECT && 
	 heap_position(heap,outer,size) >= 
	 heap_position(heap,loop_data[outer].tri_loop,size))
       {
	control1 = gen_DO_get_control(loop_data[outer].node);
	lwb1 = gen_INDUCTIVE_get_rvalue1(control1);
	upb1 = gen_INDUCTIVE_get_rvalue2(control1);
	control2=gen_DO_get_control(loop_data[loop_data[outer].tri_loop].node);
	lwb2 = gen_INDUCTIVE_get_rvalue1(control2);
	upb2 = gen_INDUCTIVE_get_rvalue2(control2);
	ivar = gen_INDUCTIVE_get_name(control2);
	coeff = loop_data[outer].tri_coeff;
	const1 = loop_data[outer].tri_const;
	switch(loop_data[outer].type) {
	  case TRI_LL:
	              pt_tree_replace(upb2,pt_simplify_expr(
					   pt_gen_add(pt_gen_mul(
                                                      pt_gen_int(coeff),
						      tree_copy_with_type(lwb1)),
						      tree_copy_with_type(const1))));
		      pt_tree_replace(upb1,pt_simplify_expr(
					   pt_gen_div(
					   pt_gen_sub(tree_copy_with_type(ivar),
						      tree_copy_with_type(const1)),
					   pt_gen_int(coeff))));
		      break;
	  case TRI_LR:
	              pt_tree_replace(upb2,pt_simplify_expr(
					   pt_gen_add(pt_gen_mul(
                                                      pt_gen_int(coeff),
						      tree_copy_with_type(upb1)),
						      tree_copy_with_type(const1))));
		      pt_tree_replace(lwb1,pt_simplify_expr(
					   pt_gen_div(
					   pt_gen_sub(tree_copy_with_type(ivar),
						      tree_copy_with_type(const1)),
					   pt_gen_int(coeff))));
		      break;
	  case TRI_UL:
	              pt_tree_replace(lwb2,pt_simplify_expr(
					   pt_gen_add(pt_gen_mul(
                                                      pt_gen_int(coeff),
						      tree_copy_with_type(lwb1)),
						      tree_copy_with_type(const1))));
		      pt_tree_replace(upb1,pt_simplify_expr(
					   pt_gen_div(
					   pt_gen_sub(tree_copy_with_type(ivar),
						      tree_copy_with_type(const1)),
					   pt_gen_int(coeff))));
		      break;
	  case TRI_UR:
	              pt_tree_replace(lwb2,pt_simplify_expr(
                                          pt_gen_add(pt_gen_mul(
                                                      pt_gen_int(coeff),
						      tree_copy_with_type(upb1)),
						      tree_copy_with_type(const1))));
		      pt_tree_replace(lwb1,pt_simplify_expr(
					   pt_gen_div(
					   pt_gen_sub(tree_copy_with_type(ivar),
						      tree_copy_with_type(const1)),
					   pt_gen_int(coeff))));
		      break;
	 }
       }
  }


/****************************************************************/
/*                                                              */
/*   Function:     rearrange_loops                              */
/*                                                              */
/*   Input:        loop_data - loop structure                   */
/*                 heap      - sorted array of loop indices     */
/*                 num_loops - number of loops in perfect nest  */
/*                 innermost_loop - index of innermost loop     */
/*                                                              */
/*   Description:  Permute the loop order based upon the sorted */
/*                 array "heap".  This will put the loop in     */
/*                 memory order.                                */
/*                                                              */
/****************************************************************/

static void rearrange_loops(model_loop *loop_data,
			    heap_type  *heap, 
			    int        num_loops,
			    int        innermost_loop)

  {
   int i,j;
   
     for  (j = loop_data[innermost_loop].level,i = num_loops-1;i >= 0;i--,j--)
       if (loop_data[heap[i].index].level < j)
	 shift_loop(loop_data,heap[i].index,innermost_loop,heap,num_loops-1);
       else
	 innermost_loop = loop_data[innermost_loop].parent;
  }


/****************************************************************/
/*                                                              */
/*   Function:     new_level                                    */
/*                                                              */
/*   Input:        edge - dependence edge                       */
/*                 max - max nesting level possible             */
/*                 init_level - initial edge level              */
/*                                                              */
/*   Description:  Determine the new level of an edge after     */
/*                 interchange.                                 */
/*                                                              */
/****************************************************************/

static int new_level(DG_Edge edge,
		     int     max,
		     int     init_level)

  {
   int i;
   
     i = init_level;
     while (gen_get_dt_DIS(&edge,i) == 0 && i <= max)
       i++;
     if (i <= max)
       return(i);
     else
       return(LOOP_INDEPENDENT);
  }


/****************************************************************/
/*                                                              */
/*   Function:     new_dir                                      */
/*                                                              */
/*   Input:        dir - direction vector entry                 */
/*                                                              */
/*   Description:  Return the opposite direction vector entry   */
/*                 This happens if an input edge is reversed    */
/*                                                              */
/****************************************************************/

static int new_dir(int dir)

  {
   switch(dir) {
     case DDATA_LT: return(DDATA_GT);
     case DDATA_GT: return(DDATA_LT);
     case DDATA_LE: return(DDATA_GE);
     case DDATA_GE: return(DDATA_LE);
     default:   
       return(dir);
    }
  }


/****************************************************************/
/*                                                              */
/*   Function:      update_vectors                              */
/*                                                              */
/*   Input:         node - AST node                             */
/*                  upd_info - structure with dependence graph, */
/*                             etc.                             */
/*                                                              */
/*   Description:   Update the direction vectors for the new    */
/*                  loop order.                                 */
/*                                                              */
/****************************************************************/

static int update_vectors(AST_INDEX     node,
			  upd_info_type *upd_info)

  {
   EDGE_INDEX edge,next_edge;
   int        vector,i;
   DG_Edge    *dg;
   int        temp_vec[MAXLOOP],dist,nlevel;
   AST_INDEX  name;

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
	 if (dg[edge].level != LOOP_INDEPENDENT)
	   {

	       /* create a copy of the direction vector so that the real one
		  can be modified */

	    for (i = 0; i < upd_info->num_loops;i++)
	      temp_vec[i] = gen_get_dt_DIS(&dg[edge],i+1);

	       /* rearrange the edge's direction vector */

	    for (i = 0; i < upd_info->num_loops;i++)
	      if (upd_info->loop_data[upd_info->heap[i].index].reversed)
		{
		 dist = temp_vec[upd_info->loop_data[upd_info->heap[i].index].level
				 -1];
		 if (dist < DDATA_BASE)
		   gen_put_dt_DIS(&dg[edge],i+1,new_dir(dist));
		 else
		   gen_put_dt_DIS(&dg[edge],i+1,-dist);
		}
	      else
	         gen_put_dt_DIS(&dg[edge],i+1,temp_vec[upd_info->
			        loop_data[upd_info->heap[i].index].level-1]);

	       /* get the new nesting level */

	    nlevel = new_level(dg[edge],upd_info->num_loops,dg[edge].level);
	    if (dg[edge].type == dg_input)
	      {

	          /* input dependence can reverse direction, so we must check that
		     here */

	       dist = gen_get_dt_DIS(&dg[edge],nlevel);
	       if ((dist < 0 && dist >= DDATA_BASE) || dist == DDATA_GT ||
		   dist == DDATA_GT)
		 {
		  if (dist < DDATA_BASE)
		    gen_put_dt_DIS(&dg[edge],nlevel,new_dir(dist));
		  else
		    gen_put_dt_DIS(&dg[edge],nlevel,-dist);
		  for (i = nlevel+1; i <= upd_info->num_loops; i++)
		    {
		     dist = gen_get_dt_DIS(&dg[edge],i);
		     if (dist < DDATA_BASE)
		       gen_put_dt_DIS(&dg[edge],i,new_dir(dist));
		     else
		       gen_put_dt_DIS(&dg[edge],i,-dist);
		    }
		 }
	      }

	      /* add a new edge with the new distance vector */

	    add_edge(upd_info->ped,&dg,edge,nlevel);
	    i = nlevel;

	        /* may be multiple new edges because of the vector is a summary */

	    while((gen_get_dt_DIS(&dg[edge],i) == DDATA_ANY ||
		   gen_get_dt_DIS(&dg[edge],i) == DDATA_GE ||
		   gen_get_dt_DIS(&dg[edge],i) == DDATA_LE ||
		   gen_get_dt_DIS(&dg[edge],i) == 0) && 
		  i < upd_info->num_loops)
	      if (gen_get_dt_DIS(&dg[edge],i) != 0)
		{
		 if ((nlevel = new_level(dg[edge],upd_info->num_loops,i+1))
		     != LOOP_INDEPENDENT)
		   {
		    add_edge(upd_info->ped,&dg,edge,nlevel);
		    i++;
		   }
		 else
		   i = upd_info->num_loops;
		}
	      else
	        i++;

	       /* remove original edge */

	    dg_delete_free_edge( PED_DG(upd_info->ped),edge);
	   }
	}
     }
   return(WALK_CONTINUE);
  }


/****************************************************************/
/*                                                              */
/*   Function:     perform_interchange                          */
/*                                                              */
/*   Input:        loop_data - loop structure                   */
/*                 heap      - sorted array of loop indices     */
/*                 innermost_loop - index of innermost loop     */
/*                 num_loops - number of loops in perfect nest  */
/*                 symtab - symbol table                        */
/*                 ped - dependence graph handle                */
/*                 ar - arena for memory allocation             */
/*                                                              */
/*   Description:  Determine if interchange must be done and    */
/*                 do it if it is legal.                        */
/*                                                              */
/****************************************************************/

static void perform_interchange(model_loop *loop_data,
				heap_type  *heap,
				int        innermost_loop,
				int        num_loops,
				SymDescriptor symtab,
				PedInfo    ped,
				arena_type *ar)
  {
   int i,j;
   Boolean do_interchange = false;
   upd_info_type upd_info;

     for (j = loop_data[innermost_loop].level,i = num_loops-1;i >= 0 && 
	  !do_interchange;i--,j--)

          /* Is interchange necessary? */

       if (loop_data[heap[i].index].level != j)
	   do_interchange = true;

     if (do_interchange)
       {

	   /* reorder the loop nest based upon heap */

	rearrange_loops(loop_data,heap,num_loops,innermost_loop);
	upd_info.ped = ped;
	upd_info.heap = heap;
	upd_info.loop_data = loop_data;
	upd_info.num_loops = num_loops;

	   /* update the dependence graph */

	walk_expression(loop_data[heap[num_loops-1].index].node,
			(WK_EXPR_CLBACK)update_vectors,NOFUNC,(Generic)&upd_info);
       }
  }


/****************************************************************/
/*                                                              */
/*   Function:     outer_levels_ok                              */
/*                                                              */
/*   Input:        heap - sorted array of loop indices          */
/*                 loop_data - loop structure                   */
/*                 chk_level - level where original outer loops */
/*                             are not to move inside of        */
/*                 outermost - index of outermost loop that can */
/*                             be interchanged inward.          */
/*                                                              */
/*   Description:  Determine if the new loops originally        */
/*                 outside of chk_level have stayed outside     */
/*                 that level                                   */
/*                                                              */
/****************************************************************/

static Boolean outer_levels_ok(heap_type *heap,
			       model_loop *loop_data,
			       int        chk_level,
			       int        outermost)

  {
   int i,level_val;
   int next;
   
     for (i = loop_data[outermost].level-1; 
	  i < chk_level;
	  i++)
       if (loop_data[heap[i].index].level > chk_level)
         return(false);
     return(true);
  }


/****************************************************************/
/*                                                              */
/*   Function:      levels_in_order                             */
/*                                                              */
/*   Input:         heap - sorted array of loop indices         */
/*                  loop_data - loop structure                  */
/*                  loop - index of loop to be checked          */
/*                  chk_level - the level outside of which the  */
/*                              nest of loops will be checked   */
/*                  outermost - outermost level where           */
/*                              interchange can occur safely    */
/*                                                              */
/*   Description:   Determines if no loop that was originally   */
/*                  between chk_level and outermost has moved   */
/*                  inside of chk_level after interchange       */
/*                                                              */
/****************************************************************/


static Boolean levels_in_order(heap_type *heap,
			      model_loop *loop_data,
			      int        loop,
			      int        chk_level,
			      int        outermost)

  {
   int i,level_val;
   int next;
   
     next = loop;
     while (next != -1)
       {
	if (loop_data[next].inner_loop != -1)
	  if (NOT(levels_in_order(heap,loop_data,loop_data[next].inner_loop,
				  chk_level,outermost)))
	    return(false);
	if (NOT(outer_levels_ok(heap,loop_data,chk_level,outermost)))
	  return(false);
	next = loop_data[next].next_loop;
	heap = loop_data[next].heap;
       }
     return(true);
  }


/****************************************************************/
/*                                                              */
/*   Function:     walk_loops_to_interchange                    */
/*                                                              */
/*   Input:        loop_data - loop structure                   */
/*                 loop - index of loop being examined          */
/*                 outermost - outermost loop that can be       */
/*                             interchanged inward              */
/*                 num_loops - number of loops in perfect nest  */
/*                 heap - sorted array of loop indices          */
/*                 symtab - symbol table                        */
/*                 ped - dependence graph handle                */
/*                 ar - arena for memory allocation             */
/*                                                              */
/*   Description:  Walk the loop structure performing           */
/*                 interchage as necessary.                     */
/*                                                              */
/****************************************************************/


static void walk_loops_to_interchange(model_loop *loop_data,
				      int        loop,
				      int        outermost,
				      int        num_loops,
				      heap_type  *heap,
				      SymDescriptor symtab,
				      PedInfo    ped,
				      arena_type *ar)
  {
   int next,temp;
   AST_INDEX stmt_list[MAXLOOP];
   level_info_type info;
   int i,j;
   int dstr_index = -1;
   AST_INDEX stmt,next_stmt,new_do;

     if (!loop_data[loop].transform || !loop_data[loop].distribute || 
	 loop_data[loop].type == COMPLEX || loop_data[loop].type == TRAP)
       {

	   /* we've found a level where interchange is illegal, recurse and 
	      note that fact in outer.  The safety of the order has already 
	      been determined.  This is recorded for indexing purposes. */

	if (loop_data[loop].inner_loop != -1)
	  walk_loops_to_interchange(loop_data,loop_data[loop].inner_loop,
				    loop_data[loop].inner_loop,num_loops+1,
				    heap,symtab,ped,ar);
	for (next = loop_data[loop].next_loop;
	     next != -1;
	     next = temp)
	  {

	        /* isolate each loop body since outer loops will remain 
		   in order */

	   temp = loop_data[next].next_loop;
	   loop_data[next].next_loop = -1;

	   heap = loop_data[next].heap;
	   walk_loops_to_interchange(loop_data,next,next,num_loops,heap,symtab,
				     ped,ar);
	   loop_data[next].next_loop = temp;
	  }
	}  
     else if (loop_data[loop].next_loop == -1) 
       if (!loop_data[loop].stmts || loop_data[loop].inner_loop == -1
	   || levels_in_order(heap,loop_data,loop,loop_data[loop].level,
			      outermost))
	 {

	     /* no distribution is necessary */

	  if (loop_data[loop].inner_loop == -1)
	    perform_interchange(loop_data,heap,loop,num_loops,symtab,ped,ar);
	  else
	    walk_loops_to_interchange(loop_data,loop_data[loop].inner_loop,
				      outermost,num_loops+1,heap,symtab,ped,ar);
	 }
       else
	 distribute_loop(loop_data,loop,ped,symtab,ar);
     else
       {

	    /* imperfect nest.  Determine if an outer level moves inside of this
	       level to see if distribution is necessary */

	if (NOT(levels_in_order(heap,loop_data,loop_data[loop].parent,
				loop_data[loop].level-1,outermost)))
	  distribute_loop(loop_data,loop_data[loop].parent,ped,symtab,ar);
	else
	  {

	       /* all common levels are identical */

	   next = loop;
	   while(next != -1)
	     {

	        /* isolate each loop body since outer loops will remain 
		   in order */

	      temp = loop_data[next].next_loop;
	      loop_data[next].next_loop = -1;

	      walk_loops_to_interchange(loop_data,next,next,num_loops,
					heap,symtab,ped,ar);	   
	      loop_data[next].next_loop = temp;
	      next = temp;
	      heap = loop_data[next].heap;
	     }
	  }
       }
  }


/****************************************************************/
/*                                                              */
/*   Function:     memory_loop_interchange                      */
/*                                                              */
/*   Input:        ped - dependence graph handle                */
/*                 root - AST index of an outermost loop        */
/*                 level - nesting level of root                */
/*                 symtab - symbol table                        */
/*                 ar - arena for memory allocation             */
/*                                                              */
/*   Description:  Order a loop nest based upon memory          */
/*                 performance                                  */
/*                                                              */
/****************************************************************/


void memory_loop_interchange(PedInfo       ped,
			     AST_INDEX     root,
			     int           level,
			     SymDescriptor symtab,
			     arena_type    *ar)

  {
   model_loop *loop_data;


     if ((loop_data = PrepareLoopForInterchange(ped,root,level,symtab,ar)) == NULL)
       return;

     /* reorder loop nests as necessary */

     walk_loops_to_interchange(loop_data,0,0,1,loop_data[0].heap,symtab,ped,
			       ar);

  }
