/* $Id: dfavail.C,v 1.7 1997/03/27 20:27:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <libs/support/misc/general.h>
#include <libs/Memoria/include/sr.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/frontEnd/include/walk.h>

#ifndef Arena_h
#include <libs/support/memMgmt/Arena.h>
#endif 

#ifndef dfavail_h
#include <libs/Memoria/sr/dfavail.h>
#endif 

#ifndef mh_config_h
#include <libs/Memoria/include/mh_config.h>
#endif 

#include <libs/Memoria/include/mem_util.h>

static void allocate_sets(block_type *block,
			  int        size,
			  arena_type *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   edge_type *edge;

     block->gen = ut_create_set(ar,LOOP_ARENA,size);
     block->LC_gen = ut_create_set(ar,LOOP_ARENA,size);
     block->kill = ut_create_set(ar,LOOP_ARENA,size);
     block->LI_avail_in = ut_create_set(ar,LOOP_ARENA,size);
     block->LI_avail_out = ut_create_set(ar,LOOP_ARENA,size);
     block->LI_rgen_in = ut_create_set(ar,LOOP_ARENA,size);
     block->LI_rgen_out = ut_create_set(ar,LOOP_ARENA,size);
     block->LI_pavail_in = ut_create_set(ar,LOOP_ARENA,size);
     block->LI_pavail_out = ut_create_set(ar,LOOP_ARENA,size);
     block->LI_antic_in = ut_create_set(ar,LOOP_ARENA,size);
     block->LI_antic_out = ut_create_set(ar,LOOP_ARENA,size);
     block->LC_avail_in = ut_create_set(ar,LOOP_ARENA,size);
     block->LC_avail_out = ut_create_set(ar,LOOP_ARENA,size);
     block->LC_rgen_in = ut_create_set(ar,LOOP_ARENA,size);
     block->LC_rgen_out = ut_create_set(ar,LOOP_ARENA,size);
     block->LC_rgen_in_if_1 = ut_create_set(ar,LOOP_ARENA,size);
     block->LC_rgen_out_if_1 = ut_create_set(ar,LOOP_ARENA,size);
     block->LC_pavail_in = ut_create_set(ar,LOOP_ARENA,size);
     block->LC_pavail_out = ut_create_set(ar,LOOP_ARENA,size);
     block->LC_avail_in_if_1 = ut_create_set(ar,LOOP_ARENA,size);
     block->LC_avail_out_if_1 = ut_create_set(ar,LOOP_ARENA,size);
     block->LC_pavail_in_if_1 = ut_create_set(ar,LOOP_ARENA,size);
     block->LC_pavail_out_if_1 = ut_create_set(ar,LOOP_ARENA,size);
     block->LC_antic_in = ut_create_set(ar,LOOP_ARENA,size);
     block->LC_antic_out = ut_create_set(ar,LOOP_ARENA,size);
     block->PP_in = ut_create_set(ar,LOOP_ARENA,size);
     block->PP_out = ut_create_set(ar,LOOP_ARENA,size);
     block->Insert = ut_create_set(ar,LOOP_ARENA,size);
     block->Transp = ut_create_set(ar,LOOP_ARENA,size);
     block->Antloc = ut_create_set(ar,LOOP_ARENA,size);
     for (edge = block->pred;
	  edge != NULL;
	  edge = edge->next_pred)
       edge->Insert = ut_create_set(ar,LOOP_ARENA,size);
  }

static int add_gens(AST_INDEX         node,
		    avail_info_type   *avail_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   scalar_info_type *scalar_info;

     if (is_subscript(node))
       {
	scalar_info = get_scalar_info_ptr(gen_SUBSCRIPT_get_name(node));
	ut_add_number(avail_info->block->gen,scalar_info->table_index);
	if (!ut_member_number(avail_info->LC_kill,scalar_info->table_index))
	  ut_add_number(avail_info->block->LC_gen,scalar_info->table_index);
	if (scalar_info->kill_set != NULL)
	  {
	   ut_difference121(avail_info->block->gen,scalar_info->kill_set);
	   ut_union121(avail_info->block->kill,scalar_info->kill_set);
	  }
       }
   return(WALK_CONTINUE);
  }
    
static void calc_gen_set(avail_info_type   *avail_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX  stmt;
   Boolean    in_block = true,
              contains_div = false;

    stmt = avail_info->block->first;
    do
      {
       if (is_if(stmt))
         stmt = list_first(gen_IF_get_guard_LIST(stmt));
       else if (is_guard(stmt))
	 {
	  walk_expression(gen_GUARD_get_rvalue(stmt),NOFUNC,
			  (WK_EXPR_CLBACK)add_gens,
			  (Generic)avail_info);
	  stmt = list_first(gen_GUARD_get_stmt_LIST(stmt));
	 }
       else if (is_logical_if(stmt))
	 {
	  walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),NOFUNC,
			  (WK_EXPR_CLBACK)add_gens,
			  (Generic)avail_info);
	  stmt = list_first(gen_LOGICAL_IF_get_stmt_LIST(stmt));
	 }
       else
	 {
	  if (is_assignment(stmt))
	    {
	     if (((config_type *)PED_MH_CONFIG(avail_info->ped))->soft_div)
	       walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
			       (WK_EXPR_CLBACK)ut_check_div,(Generic)&contains_div);
	     if (!contains_div)
	       {
		walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
				(WK_EXPR_CLBACK)add_gens,(Generic)avail_info);
		walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),NOFUNC,
				(WK_EXPR_CLBACK)add_gens,(Generic)avail_info);
	       }
	     else
	       {
		contains_div = false;
		ut_clear_set(avail_info->block->gen);
		ut_clear_set(avail_info->block->LC_gen);
		ut_clear_set(avail_info->block->kill);
		ut_complement(avail_info->block->kill);
		ut_clear_set(avail_info->LC_kill);
		ut_complement(avail_info->LC_kill);
	       }
	    }
	  else if (is_write(stmt))
	    walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,
			    (WK_EXPR_CLBACK)add_gens,
			    (Generic)avail_info);
	  else if (is_read_short(stmt))
            walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),NOFUNC,
			    (WK_EXPR_CLBACK)add_gens,(Generic)avail_info); 
	  else if (is_arithmetic_if(stmt))
	    walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),NOFUNC,
			    (WK_EXPR_CLBACK)add_gens,
			    (Generic)avail_info);
	  else if (is_call(stmt))
	    {
	     ut_clear_set(avail_info->block->gen);
	     ut_clear_set(avail_info->block->LC_gen);
	     ut_clear_set(avail_info->block->kill);
	     ut_complement(avail_info->block->kill);
	     ut_clear_set(avail_info->LC_kill);
	     ut_complement(avail_info->LC_kill);
	    }
	  stmt = list_next(stmt);
	 }
       if (is_null_node(stmt))
         in_block = false;
       else if(get_stmt_info_ptr(stmt)->block != avail_info->block)
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
   avail_info_type avail_info;

     allocate_sets(flow_graph.entry,check_info.size,check_info.ar);
     avail_info.block = flow_graph.entry;
     avail_info.LC_kill = check_info.LC_kill;
     avail_info.ped = check_info.ped;
     calc_gen_set(&avail_info);
     ut_copy12(flow_graph.entry->Transp,flow_graph.entry->kill);
     ut_complement(flow_graph.entry->Transp);
     ut_copy12(flow_graph.entry->LI_avail_out,flow_graph.entry->gen);
     ut_copy12(flow_graph.entry->LI_pavail_out,flow_graph.entry->gen);
     ut_copy12(flow_graph.entry->LC_avail_out_if_1,flow_graph.entry->gen);
     ut_copy12(flow_graph.entry->LC_pavail_out_if_1,flow_graph.entry->gen);
     ut_copy12(flow_graph.entry->LC_avail_out,flow_graph.entry->LC_gen);
     ut_copy12(flow_graph.entry->LC_pavail_out,flow_graph.entry->LC_gen);
     for (block = flow_graph.entry->next;
	  block != NULL;
	  block = block->next)
       {
	allocate_sets(block,check_info.size,check_info.ar);
	if (block != flow_graph.exit)
	  {
	   avail_info.block = block;
	   calc_gen_set(&avail_info);
	   ut_copy12(block->Transp,flow_graph.entry->kill);
	   ut_complement(block->Transp);
	  }
	ut_complement(block->LI_avail_out);
	ut_complement(block->LC_avail_out);
	ut_complement(block->LC_avail_out_if_1);
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

     ut_clear_set(block->LI_avail_in);
     ut_complement(block->LI_avail_in);
     ut_clear_set(block->LI_pavail_in);
     for (edge = block->pred;
	  edge != NULL;
	  edge = edge->next_pred)
       {
	ut_intersect121(block->LI_avail_in,edge->from->LI_avail_out);
	ut_union121(block->LI_pavail_in,edge->from->LI_pavail_out);
       }
     temp = ut_create_set(ar,LOOP_ARENA,size);
     ut_copy12(temp,block->LI_avail_in);
     ut_difference121(temp,block->kill);
     ut_union121(temp,block->gen);
     ut_copy12(block->LI_avail_out,temp);
     ut_copy12(temp,block->LI_pavail_in);
     ut_difference121(temp,block->kill);
     ut_union121(temp,block->gen);
     ut_copy12(block->LI_pavail_out,temp);
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

     ut_clear_set(block->LC_avail_in);
     ut_complement(block->LC_avail_in);
     ut_clear_set(block->LC_pavail_in);
     ut_clear_set(block->LC_avail_in_if_1);
     ut_complement(block->LC_avail_in_if_1);
     ut_clear_set(block->LC_pavail_in_if_1);
     for (edge = block->pred;
	  edge != NULL;
	  edge = edge->next_pred)
       {
	ut_intersect121(block->LC_avail_in,edge->from->LC_avail_out);
	ut_union121(block->LC_pavail_in,edge->from->LC_pavail_out);
	ut_intersect121(block->LC_avail_in_if_1,edge->from->LC_avail_out_if_1);
	ut_union121(block->LC_pavail_in_if_1,edge->from->LC_pavail_out_if_1);
       }
     temp = ut_create_set(ar,LOOP_ARENA,size);

      /* do not need to include kills because those defs killed will never
	 be added to LC_gen */

     ut_copy12(temp,block->LC_avail_in);
     ut_union121(temp,block->LC_gen);
     ut_copy12(block->LC_avail_out,temp);
     ut_copy12(temp,block->LC_pavail_in);
     ut_union121(temp,block->LC_gen);
     ut_copy12(block->LC_pavail_out,temp);

      /* check kills on one path through the flow graph to see if value can
	 be guaranteed to be available on next iteration only */

     ut_copy12(temp,block->LC_avail_in_if_1);
     ut_difference121(temp,block->kill);
     ut_union121(temp,block->gen);
     ut_copy12(block->LC_avail_out_if_1,temp);
     ut_copy12(temp,block->LC_pavail_in_if_1);
     ut_difference121(temp,block->kill);
     ut_union121(temp,block->gen);
     ut_copy12(block->LC_pavail_out_if_1,temp);
     /* free(temp); */
  }

static void LI_calc_avail(block_type    *block,
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
	  LI_calc_avail(edge->from,entry,unvisited_mark,size,ar);
	if (block != entry)
	  {
	   LI_analyze_block(block,size,ar);
	   LC_analyze_block(block,size,ar);
	  }
       }
  }

static void LC_calc_avail(block_type    *block,
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
	  LC_calc_avail(edge->from,unvisited_mark,size,ar);
        LC_analyze_block(block,size,ar);
       }
  }

void sr_perform_avail_analysis(flow_graph_type flow_graph,
			       check_info_type check_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   init_sets(flow_graph,check_info);
   LI_calc_avail(flow_graph.exit,flow_graph.entry,flow_graph.exit->mark
		 ,check_info.size,check_info.ar);

   /* d(G) always = 1 so df info will hit fix point with 1 step */
   
   LC_calc_avail(flow_graph.exit,flow_graph.exit->mark,check_info.size,
		 check_info.ar);
  }
