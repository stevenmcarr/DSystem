/* $Id: dfrgen.C,v 1.4 1992/12/11 11:22:14 carr Exp $ */
#include <general.h>
#include <sr.h>
#include <mh_ast.h>
#include <fort/walk.h>

#ifndef Arena_h
#include <Arena.h>
#endif

#ifndef dfrgen_h
#include <dfrgen.h>
#endif

#ifndef mh_config_h
#include <mh_config.h>
#endif

#ifndef dg_h
#include <dg.h>
#endif

#include <mem_util.h>
#include <pt_util.h>

static int add_gens(AST_INDEX      node,
		    rgen_info_type *rgen_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   scalar_info_type *sptr;

     if (is_subscript(node))
       {
	sptr = get_scalar_info_ptr(gen_SUBSCRIPT_get_name(node));
	ut_add_number(rgen_info->block->gen,sptr->array_num);
	if (!ut_member_number(rgen_info->LC_kill,sptr->table_index))
	  ut_add_number(rgen_info->block->LC_gen,sptr->array_num);
       }
   return(WALK_CONTINUE);
  }

static int kill_rhs(AST_INDEX      node,
		    rgen_info_type *rgen_info)

  {
   scalar_info_type *sptr;
   
     if (is_subscript(node))
       if (pt_expr_equal(node,rgen_info->lhs))
	 {
	  sptr = get_scalar_info_ptr(gen_SUBSCRIPT_get_name(node));
	  ut_delete_number(rgen_info->block->gen,sptr->array_num);
	  ut_delete_number(rgen_info->block->LC_gen,sptr->array_num);
	  ut_add_number(rgen_info->block->kill,sptr->array_num);
	 }
     return(WALK_CONTINUE);
  }

static int add_defs(AST_INDEX         node,
		    rgen_info_type   *rgen_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   scalar_info_type *sptr;
   AST_INDEX        name;
   DG_Edge          *dg;
   int              vector;
   EDGE_INDEX       edge;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	sptr = get_scalar_info_ptr(name);
	ut_add_number(rgen_info->block->gen,sptr->array_num);
	if (!ut_member_number(rgen_info->LC_kill,sptr->table_index))
	  ut_add_number(rgen_info->block->LC_gen,sptr->array_num);
	dg = dg_get_edge_structure( PED_DG(rgen_info->ped));
	vector = get_info(rgen_info->ped,name,type_levelv);
	for (edge = dg_first_sink_ref( PED_DG(rgen_info->ped),vector);
	     edge != END_OF_LIST;
	     edge = dg_next_sink_ref( PED_DG(rgen_info->ped),edge))
	  if (dg[edge].level == LOOP_INDEPENDENT)
	    {
	     sptr = get_scalar_info_ptr(dg[edge].src);
	     ut_delete_number(rgen_info->block->gen,sptr->array_num);
	     ut_add_number(rgen_info->block->kill,sptr->array_num);
	    }
       }
   return(WALK_CONTINUE);
  }
    
static void calc_gen_set(rgen_info_type   *rgen_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX  stmt;
   Boolean    in_block = true,
              contains_div = false;

    ut_clear_set(rgen_info->block->gen);
    ut_clear_set(rgen_info->block->LC_gen);
    ut_clear_set(rgen_info->block->kill);
    stmt = rgen_info->block->first;
    do
      {
       if (is_if(stmt))
         stmt = list_first(gen_IF_get_guard_LIST(stmt));
       else if (is_guard(stmt))
	 {
	  walk_expression(gen_GUARD_get_rvalue(stmt),NOFUNC,add_gens,
			  (Generic)rgen_info);
	  stmt = list_first(gen_GUARD_get_stmt_LIST(stmt));
	 }
       else if (is_logical_if(stmt))
	 {
	  walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),NOFUNC,add_gens,
			  (Generic)rgen_info);
	  stmt = list_first(gen_LOGICAL_IF_get_stmt_LIST(stmt));
	 }
       else
	 {
	  if (is_assignment(stmt))
	    {
	     if (((config_type *)PED_MH_CONFIG(rgen_info->ped))->soft_div)
	       walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
			       ut_check_div,(Generic)&contains_div);
	     if (!contains_div)
	       {
		walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
				add_gens,(Generic)rgen_info);
		walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),NOFUNC,
				add_defs,(Generic)rgen_info);
		rgen_info->lhs = gen_ASSIGNMENT_get_lvalue(stmt);
		walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
				kill_rhs,(Generic)rgen_info);
	       }
	     else
	       {		
		contains_div = false;
		ut_clear_set(rgen_info->block->gen);
		ut_clear_set(rgen_info->block->LC_gen);
		ut_clear_set(rgen_info->block->kill);
		ut_complement(rgen_info->block->kill);
		ut_clear_set(rgen_info->LC_kill);
		ut_complement(rgen_info->LC_kill);
	       }
	    }
	  else if (is_write(stmt))
	    walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,add_gens,
			    (Generic)rgen_info);
	  else if (is_read_short(stmt))
            walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),NOFUNC,
			    add_gens,(Generic)rgen_info); 
	  else if (is_arithmetic_if(stmt))
	    walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),NOFUNC,add_gens,
			    (Generic)rgen_info);
	  else if (is_call(stmt))
	    {
	     ut_clear_set(rgen_info->block->gen);
	     ut_clear_set(rgen_info->block->LC_gen);
	     ut_clear_set(rgen_info->block->kill);
	     ut_complement(rgen_info->block->kill);
	     ut_clear_set(rgen_info->LC_kill);
	     ut_complement(rgen_info->LC_kill);
	    }
	  stmt = list_next(stmt);
	 }
       if (is_null_node(stmt))
         in_block = false;
       else if(get_stmt_info_ptr(stmt)->block != rgen_info->block)
         in_block = false;
      } while(in_block);
  }

static void init_sets(flow_graph_type  flow_graph,
		      check_info_type  check_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   block_type      *block;
   rgen_info_type rgen_info;

     rgen_info.block = flow_graph.entry;
     rgen_info.LC_kill = check_info.LC_kill;
     rgen_info.ped = check_info.ped;
     calc_gen_set(&rgen_info);
     ut_copy12(flow_graph.entry->LI_rgen_out,flow_graph.entry->gen);
     ut_copy12(flow_graph.entry->LC_rgen_out_if_1,flow_graph.entry->gen);
     ut_copy12(flow_graph.entry->LC_rgen_out,flow_graph.entry->LC_gen);
     for (block = flow_graph.entry->next;
	  block != NULL;
	  block = block->next)
       {
	if (block != flow_graph.exit)
	  {
	   rgen_info.block = block;
	   calc_gen_set(&rgen_info);
	  }
       }
  }

static void LI_analyze_block(block_type  *block,
			     int         size,
			     arena_type  *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   edge_type *edge;
   Set       temp;

     ut_clear_set(block->LI_rgen_in);
     for (edge = block->pred;
	  edge != NULL;
	  edge = edge->next_pred)
       ut_union121(block->LI_rgen_in,edge->from->LI_rgen_out);
     temp = ut_create_set(ar,LOOP_ARENA,size);
     ut_copy12(temp,block->LI_rgen_in);
     ut_difference121(temp,block->kill);
     ut_union121(temp,block->gen);
     ut_copy12(block->LI_rgen_out,temp);
     /* free(temp); */
  }

static void LC_analyze_block(block_type  *block,
			     int         size,
			     arena_type  *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   edge_type *edge;
   Set       temp;

     ut_clear_set(block->LC_rgen_in);
     ut_clear_set(block->LC_rgen_in_if_1);
     for (edge = block->pred;
	  edge != NULL;
	  edge = edge->next_pred)
       {
	ut_union121(block->LC_rgen_in,edge->from->LC_rgen_out);
	ut_union121(block->LC_rgen_in_if_1,edge->from->LC_rgen_out_if_1);
       }
     temp = ut_create_set(ar,LOOP_ARENA,size);

      /* do not need to include kills because those defs killed will never
	 be added to LC_gen */

     ut_copy12(temp,block->LC_rgen_in);
     ut_union121(temp,block->LC_gen);
     ut_copy12(block->LC_rgen_out,temp);
   
     ut_copy12(temp,block->LC_rgen_in_if_1);
     ut_difference121(temp,block->kill);
     ut_union121(temp,block->gen);
     ut_copy12(block->LC_rgen_out_if_1,temp);
     /* free(temp); */
  }

static void LI_calc_rgen(block_type    *block,
			 block_type    *entry,
			 Boolean       unvisited_mark,
			 int           size,
			 arena_type    *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   edge_type  *edge;

     if (block->mark == unvisited_mark)
       {
	block->mark = (Boolean)!unvisited_mark;
	for (edge = block->pred;
	     edge != NULL;
	     edge = edge->next_pred)
	  LI_calc_rgen(edge->from,entry,unvisited_mark,size,ar);
	if (block != entry)
	  {
	   LI_analyze_block(block,size,ar);
	   LC_analyze_block(block,size,ar);
	  }
       }
  }

static void LC_calc_rgen(block_type    *block,
			 Boolean       unvisited_mark,
			 int           size,
			 arena_type    *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   edge_type  *edge;

     if (block->mark == unvisited_mark)
       {
	block->mark = (Boolean)!unvisited_mark;
	for (edge = block->pred;
	     edge != NULL;
	     edge = edge->next_pred)
	  LC_calc_rgen(edge->from,unvisited_mark,size,ar);
        LC_analyze_block(block,size,ar);
       }
  }

void sr_perform_rgen_analysis(flow_graph_type flow_graph,
			      check_info_type check_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   init_sets(flow_graph,check_info);
   LI_calc_rgen(flow_graph.exit,flow_graph.entry,flow_graph.exit->mark
		 ,check_info.size,check_info.ar);

   /* d(G) always = 1 so df info will hit fix point with 1 step */
   
   LC_calc_rgen(flow_graph.exit,flow_graph.exit->mark,check_info.size,
		check_info.ar);
  }
