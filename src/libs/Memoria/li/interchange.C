#include <mh.h>
#include <gi.h>
#include "interchange.h"
#include <analyze.h>
#include <shape.h>
#include <mem_util.h>
#include <mark.h>



static int remove_edges(AST_INDEX stmt,
			int       level,
			PedInfo   ped)

  {
   DG_Edge    *dg;
   int        vector;
   EDGE_INDEX edge,
              next_edge;
   int        i;

     dg = dg_get_edge_structure((Generic)ped);
     vector = get_info(ped,stmt,type_levelv);
     for (i = 1;i <= level; i++)
       {
	for (edge = dg_first_src_stmt((Generic)ped,vector,i);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_src_stmt((Generic)ped,edge);
	   if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	       dg[edge].type == dg_call || dg[edge].type == dg_control)
	     dg_delete_free_edge((Generic)ped,edge);
	  }
	for (edge = dg_first_sink_stmt((Generic)ped,vector,i);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_sink_stmt((Generic)ped,edge);
	   if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	       dg[edge].type == dg_call || dg[edge].type == dg_control)
	     dg_delete_free_edge((Generic)ped,edge);
	  }
       }
     for (edge = dg_first_src_stmt((Generic)ped,vector,LOOP_INDEPENDENT);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_src_stmt((Generic)ped,edge);
	if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	    dg[edge].type == dg_call || dg[edge].type == dg_control)
	  dg_delete_free_edge((Generic)ped,edge);
       }
     for (edge = dg_first_sink_stmt((Generic)ped,vector,LOOP_INDEPENDENT);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_sink_stmt((Generic)ped,edge);
	if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	    dg[edge].type == dg_call || dg[edge].type == dg_control)
	   dg_delete_free_edge((Generic)ped,edge);
       }
     return(WALK_CONTINUE);
  }

static Boolean not_in_other_positions(AST_INDEX node,
				      char      *var)

  {
     for (node = list_next(list_first(node));
	  node != AST_NIL;
	  node = list_next(node))
       if (pt_find_var(node,var))
         return(false);
     return(true);
  }


static Boolean can_move_to_innermost(DG_Edge *edge)

  {
   int i;
   
     for (i = edge->level; i < gen_get_dt_LVL(edge);i++)
       if (gen_get_dt_DIS(edge,i) != 0)
         return(false);
     return(true);
  }


static RefType reference_type(AST_INDEX node,
			      int       level,
			      PedInfo   ped)

  {
   AST_INDEX name;
   DG_Edge   *dg;
   int       vector;
   EDGE_INDEX edge;
   RefType   rtype;

     rtype = MISS;
     name = gen_SUBSCRIPT_get_name(node);
     dg = dg_get_edge_structure((Generic)ped);
     vector = get_info(ped,name,type_levelv);
     for (edge = dg_first_sink_ref((Generic)ped,vector);
	  edge != END_OF_LIST;
	  edge = dg_next_sink_ref((Generic)ped,edge))
       if (dg[edge].consistent != inconsistent && !dg[edge].symbolic)
	 if ((dg[edge].level == level && can_move_to_innermost(&dg[edge])) || 
	    dg[edge].level == LOOP_INDEPENDENT)
           if (dg[edge].type == dg_true || dg[edge].type == dg_input)
	     if (dg[edge].consistent == consistent_SIV)
	       return(REGISTER);
	     else
	       rtype = AHIT;
	   else if (((config_type *)PED_MH_CONFIG(ped))->write_back)
	     rtype = AHIT;
	   else;
	 else if (dg[edge].src == dg[edge].sink && rtype == MISS)
	   rtype = IHIT;
     return(rtype);
  }


static int chk_array_refs(AST_INDEX     node,
			  int_info_type *int_info)

  {
   AST_INDEX sub_list,
             sub;
   Boolean   lin;
   int       coeff,words,loop,hit,miss,in_cache;
   int       level_val,dims;
   char      *var;
   UtilNode  *lnode;

     if (is_subscript(node))
       {
	if (gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION ||
	    gen_get_converted_type(node) == TYPE_COMPLEX)
	  words = (((config_type *)PED_MH_CONFIG(int_info->ped))->line) >> 3; 
	else
	  words = (((config_type *)PED_MH_CONFIG(int_info->ped))->line) >> 2; 
	hit = ((config_type *)PED_MH_CONFIG(int_info->ped))->hit_cycles;
	miss = ((config_type *)PED_MH_CONFIG(int_info->ped))->miss_cycles;
	sub_list = gen_SUBSCRIPT_get_rvalue_LIST(node);
	for (lnode = UTIL_HEAD(int_info->loop_list);
	     lnode != NULLNODE;
	     lnode = UTIL_NEXT(lnode))
	  {
	   loop = (int)UTIL_NODE_ATOM(lnode);
	   var = gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
				   int_info->loop_data[loop].node)));
	   if (pt_find_var(sub_list,var))
	     {
	      sub = list_first(sub_list);
	      if (pt_find_var(sub,var) && not_in_other_positions(sub_list,var))
		{
		 pt_get_coeff(sub,var,&lin,&coeff);
		 if (coeff < words && lin)
		   {
		    in_cache = words/coeff - 1;
		    switch(reference_type(node,int_info->loop_data[loop].level,
					  int_info->ped))
		      {
		       case IHIT: 
		         int_info->loop_data[loop].stride +=
			   (in_cache * hit +  (words - in_cache) * miss);
			 break;
	       	       case AHIT: 
			 int_info->loop_data[loop].stride += (words*hit);
			 break;
		       default:;
			}
		   }
		 else
		   int_info->loop_data[loop].stride += (words * miss);
		}
	      else 
	        switch(reference_type(node,int_info->loop_data[loop].level,
				      int_info->ped))
		  {
		   case MISS:
		     int_info->loop_data[loop].stride += (words * miss);
	             break;
		   case IHIT: 
		     int_info->loop_data[loop].stride +=
		       (in_cache * hit +  (words - in_cache) * miss);
		     break;
		   case AHIT:
	             int_info->loop_data[loop].stride += (words * hit);
	             break;
		   default:;
		  }
	     }
	  }
	return(WALK_SKIP_CHILDREN);
       }
     return(WALK_CONTINUE);
  }

static void heapify(heap_type *heap,
		    int       i,
		    int       j,
		    int       n)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int k,i1,i2,
       temp_index,
       temp_stride;

     if (i < ((j+1) >> 1))
       {
	i1 = (i << 1) + 1;
	i2 = (i << 1) + 2;
	if (i2 > n) 
	  i2 = i1;
	if ((heap[i1].stride < heap[i].stride || 
	     (heap[i1].stride == heap[i].stride &&
	      heap[i1].index > heap[i].index)) ||
	    (heap[i2].stride < heap[i].stride ||
	     (heap[i2].stride == heap[i].stride &&
	      heap[i2].index > heap[i].index)))
	  {
	   if (heap[i1].stride < heap[i2].stride)
	     k = i1;
	   else
	     k = i2;
	   temp_index = heap[i].index;
	   temp_stride = heap[i].stride;
	   heap[i].index = heap[k].index;
	   heap[i].stride = heap[k].stride;
	   heap[k].index = temp_index;
	   heap[k].stride = temp_stride;
	   heapify(heap,k,j,n);
	  }
       }
  }
     
static void order_loops_for_stride(model_loop *loop_data,
				   int        num_loops,
				   int        outermost_lvl,
				   PedInfo    ped,
				   SymDescriptor symtab,
				   UtilList      *loop_list,
				   heap_type     *heap)

  {
   int           i,pos;
   int           temp_index,temp_stride;
   upd_info_type upd_info;
   Boolean       do_interchange = false;
   UtilNode      *node;
   int           *loop_order;

     for (node = UTIL_HEAD(loop_list),i = 0;
	  node != NULLNODE;
	  node = UTIL_NEXT(node),i++)
       {
	if (loop_data[UTIL_NODE_ATOM(node)].level < outermost_lvl)
	  {
	   heap[i].index = UTIL_NODE_ATOM(node);
	   heap[i].stride = MAXINT - i;
	  }
	else
	  {
	   heap[i].index = UTIL_NODE_ATOM(node);
	   heap[i].stride = loop_data[heap[i].index].stride;
	  }
       }
     for (i = (num_loops-1) >> 1; i >= 0; i--)
       heapify(heap,i,num_loops-1,num_loops-1);
     for (i = num_loops-1; i > 0; i--)
       {
	temp_index = heap[i].index;
	temp_stride = heap[i].stride;
	heap[i].index = heap[0].index;
	heap[i].stride = heap[0].stride;
	heap[0].index = temp_index;
	heap[0].stride = temp_stride;
	heapify(heap,0,i-1,i-1);
       }
  }



static void check_interchange(model_loop    *loop_data,
			      int           loop,
			      int           num_loops,
			      int           outermost_lvl,
			      SymDescriptor symtab,
			      PedInfo       ped,
			      UtilList      *loop_list,
			      heap_type     *heap)

  {
   int_info_type int_info;

     int_info.symtab = symtab;
     int_info.ped = ped;
     int_info.loop_data = loop_data;
     int_info.loop_list = loop_list;
     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),chk_array_refs,
		     NOFUNC,(Generic)&int_info);
     order_loops_for_stride(loop_data,num_loops,outermost_lvl,
			    ped,symtab,loop_list,heap);
  }
   

static void walk_loops(model_loop    *loop_data,
		       int           loop,
		       int           outermost_lvl,
		       int           num_loops,
		       UtilList      *loop_list,
		       heap_type     *heap,
		       SymDescriptor symtab,
		       PedInfo       ped,
		       arena_type    *ar)
		       
  {
   int next;

     util_append(loop_list,util_node_alloc(loop,"loop node"));
     fst_PutField(symtab,gen_get_text(gen_INDUCTIVE_get_name(
		  gen_DO_get_control(loop_data[loop].node))),INDEX,loop);
     if (!loop_data[loop].transform || !loop_data[loop].interchange ||
	 !loop_data[loop].distribute || loop_data[loop].type == COMPLEX ||
	 loop_data[loop].type == TRAP)
       outermost_lvl = loop_data[loop].level+1;
     if (loop_data[loop].inner_loop == -1)
       check_interchange(loop_data,loop,num_loops,outermost_lvl,symtab,
			 ped,loop_list,heap);
     else
       {
	next = loop_data[loop].inner_loop;
	while (next != -1)
	  {
	   walk_loops(loop_data,next,outermost_lvl,num_loops+1,loop_list,heap,
		      symtab,ped,ar);
	   next = loop_data[next].next_loop;
	   if (next != -1)
	     {
	      loop_data[next].heap=(heap_type *)ar->arena_alloc_mem(LOOP_ARENA,
								    MAXLOOP * 
							   sizeof(heap_type));
	      heap = loop_data[next].heap;
	     }
	  }
       }
     util_pluck(UTIL_TAIL(loop_list));
  }


static void add_edge(PedInfo      ped,
		     DG_Edge      *dg,
		     EDGE_INDEX   old_edge,
		     int          level)
		

  {
   EDGE_INDEX new_edge;
   int        j;
   int        dir;
   int        source,sink,temp,
              src_stmt,sink_stmt;

     new_edge = dg_alloc_edge((Generic)ped,&dg);
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

     /* stmt level vectors have already been created */

      /* hack because ast makes no sense */

     src_stmt = ut_get_stmt(source);
     if (is_logical_if(tree_out(src_stmt)) || is_guard(src_stmt))
       src_stmt = tree_out(src_stmt);
     dg[new_edge].src_vec = get_info(ped,src_stmt,type_levelv);
     sink_stmt = ut_get_stmt(sink);
     if (is_logical_if(tree_out(sink_stmt)) || is_guard(sink_stmt))
       sink_stmt = tree_out(sink_stmt);
     dg[new_edge].sink_vec = get_info(ped,sink_stmt,type_levelv);
     dt_copy_info(ped,&dg[old_edge],&dg[new_edge]);
     dt_info_str(ped,&dg[new_edge]);
     dg_add_edge((Generic)ped,new_edge);
  }

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
	dg = dg_get_edge_structure((Generic)info->ped);
	surrounding_do = get_stmt_info_ptr(ut_get_stmt(node))->surrounding_do;
	for (edge = dg_first_src_ref((Generic)info->ped,vector);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_src_ref((Generic)info->ped,edge);
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
		 add_edge(info->ped,dg,edge,LOOP_INDEPENDENT);
		}
	      dg_delete_free_edge((Generic)info->ped,edge);
	     }
	  }
       }
     return(WALK_CONTINUE);
  }


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
     info.ped = ped;
     info.level = loop_data[loop].level;
     tree_free(gen_DO_get_stmt_LIST(loop_data[loop].node));
     for (j = i-1; j > 0; j--)
       {
	walk_expression(stmt_list[j],update_edges,NOFUNC,(Generic)&info);
	new_do = tree_copy_with_type(loop_data[loop].node);
	gen_DO_put_stmt_LIST(new_do,stmt_list[j]);
	list_insert_after(loop_data[loop].node,new_do);
	memory_loop_interchange(ped,new_do,loop_data[loop].level,symtab,ar);
       }
     walk_expression(stmt_list[0],update_edges,NOFUNC,(Generic)&info);
     gen_DO_put_stmt_LIST(loop_data[loop].node,stmt_list[0]);
     memory_loop_interchange(ped,loop_data[loop].node,loop_data[loop].level,
			     symtab,ar);
  }


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
  }


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

static int update_vectors(AST_INDEX     node,
			  upd_info_type *upd_info)

  {
   EDGE_INDEX edge,next_edge;
   int        vector,i,l;
   DG_Edge    *dg;
   int        temp_vec[MAXLOOP],dist,nlevel;
   AST_INDEX  name;

   if (is_subscript(node))
     {
      name = gen_SUBSCRIPT_get_name(node);
      dg = dg_get_edge_structure((Generic)upd_info->ped);
      vector = get_info(upd_info->ped,name,type_levelv);
      for (edge = dg_first_src_ref((Generic)upd_info->ped,vector);
	   edge != END_OF_LIST;
	   edge = next_edge)
	{
	 next_edge = dg_next_src_ref((Generic)upd_info->ped,edge);
	 if (dg[edge].level != LOOP_INDEPENDENT)
	   {
	    for (i = 0; i < upd_info->num_loops;i++)
	      temp_vec[i] = gen_get_dt_DIS(&dg[edge],i+1);
	    for (i = 0; i < upd_info->num_loops;i++)
	      gen_put_dt_DIS(&dg[edge],i+1,temp_vec[upd_info->
			     loop_data[upd_info->heap[i].index].level-1]);
	    nlevel = new_level(dg[edge],upd_info->num_loops,dg[edge].level);
	    if (dg[edge].type == dg_input)
	      {
	       dist = gen_get_dt_DIS(&dg[edge],nlevel);
	       if ((dist < 0 && dist >= DDATA_BASE) || dist == DDATA_GT ||
		   dist == DDATA_GT)
		 {
		  if (dist < DDATA_BASE)
		    gen_put_dt_DIS(&dg[edge],nlevel,new_dir(dist));
		  else
		    gen_put_dt_DIS(&dg[edge],i,-dist);
		  for (i = nlevel+1; i <= upd_info->num_loops; i++)
		    {
		     dist = gen_get_dt_DIS(&dg[edge],i);
		     if (dist < DDATA_BASE)
		       gen_put_dt_DIS(&dg[edge],nlevel,new_dir(dist));
		     else
		       gen_put_dt_DIS(&dg[edge],i,-dist);
		    }
		 }
	      }
	    add_edge(upd_info->ped,dg,edge,nlevel);
	    i = nlevel;
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
		    add_edge(upd_info->ped,dg,edge,nlevel);
		    i++;
		   }
		 else
		   i = upd_info->num_loops;
		}
	      else
	        i++;
	    dg_delete_free_edge((Generic)upd_info->ped,edge);
	   }
	}
     }
   return(WALK_CONTINUE);
  }

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
       if (loop_data[heap[i].index].level != j)
         if (loop_data[heap[i].index].interchange)
	   do_interchange = true;
     if (do_interchange)
       {
	rearrange_loops(loop_data,heap,num_loops,innermost_loop);
	upd_info.ped = ped;
	upd_info.heap = heap;
	upd_info.loop_data = loop_data;
	upd_info.num_loops = num_loops;
	walk_expression(loop_data[heap[num_loops-1].index].node,
			update_vectors,NOFUNC,(Generic)&upd_info);
       }
  }

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

     if (!loop_data[loop].transform || !loop_data[loop].interchange ||
	 !loop_data[loop].distribute || loop_data[loop].type == COMPLEX ||
	 loop_data[loop].type == TRAP)
       {
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
	if (NOT(levels_in_order(heap,loop_data,loop_data[loop].parent,
				loop_data[loop].level-1,outermost)))
	  distribute_loop(loop_data,loop_data[loop].parent,ped,symtab,ar);
	else
	  {
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


void memory_loop_interchange(PedInfo       ped,
			 AST_INDEX     root,
			 int           level,
			 SymDescriptor symtab,
			 arena_type    *ar)

  {
   pre_info_type pre_info;
   model_loop    *loop_data;
   UtilList      *loop_list;

     pre_info.stmt_num = 0;
     pre_info.loop_num = 0;
     pre_info.surrounding_do = -1;
     pre_info.abort = false;
     pre_info.ped = ped;
     pre_info.symtab = symtab;
     pre_info.ar = ar;
     walk_statements(root,level,ut_mark_do_pre,ut_mark_do_post,
		     (Generic)&pre_info);
     if (pre_info.abort)
       return;
     walk_statements(root,level,remove_edges,NOFUNC,(Generic)ped);
     loop_data = (model_loop *)ar->arena_alloc_mem_clear(LOOP_ARENA,
					 pre_info.loop_num*sizeof(model_loop));
     ut_analyze_loop(root,loop_data,level,ped,symtab);
     ut_check_shape(loop_data,0);
     fst_InitField(symtab,INDEX,-1,0);
     loop_list = util_list_alloc(NULL,"loop-list");
     loop_data[0].heap = (heap_type *)ar->arena_alloc_mem(LOOP_ARENA,
							  MAXLOOP*
							  sizeof(heap_type));
     walk_loops(loop_data,0,0,1,loop_list,loop_data[0].heap,symtab,ped,ar);
     util_list_free(loop_list);
     fst_KillField(symtab,INDEX);
     walk_loops_to_interchange(loop_data,0,0,1,loop_data[0].heap,symtab,ped,
			       ar);
  }
