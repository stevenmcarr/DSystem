#include <sr.h>
#include <Arena.h>
#include "table.h"
#include "profit.h"

static int chk_pav(AST_INDEX      node,
		   prof_info_type *prof_info)

  {
   scalar_info_type *sptr;

     if (is_subscript(node))
       {
	sptr = get_scalar_info_ptr(gen_SUBSCRIPT_get_name(node));
	if (sptr->gen_type == LCPAV  && sptr->generator != -1)
	  {
	   ut_add_number(prof_info->pavset,sptr->generator);
	   prof_info->array_table[sptr->generator].profit += prof_info->prob;
	  }
       }
     return(WALK_CONTINUE);
  }
    
static void count_lcpav(block_type       *block,
			Set              pavset,
			array_table_type *array_table,
			float            prob)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX      stmt;
   Boolean        in_block = true;
   prof_info_type prof_info;

    prof_info.pavset = pavset;
    prof_info.prob = prob;
    prof_info.array_table = array_table;
    stmt = block->first;
    do
      {
       if (is_if(stmt))
         stmt = list_first(gen_IF_get_guard_LIST(stmt));
       else if (is_guard(stmt))
	 {
	  walk_expression(gen_GUARD_get_rvalue(stmt),NOFUNC,chk_pav,
			  (Generic)&prof_info);
	  stmt = list_first(gen_GUARD_get_stmt_LIST(stmt));
	 }
       else if (is_logical_if(stmt))
	 {
	  walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),NOFUNC,chk_pav,
			  (Generic)&prof_info);
	  stmt = list_first(gen_LOGICAL_IF_get_stmt_LIST(stmt));
	 }
       else
	 {
	  if (is_assignment(stmt))
	    walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,chk_pav,
			    (Generic)&prof_info);
	  else if (is_write(stmt))
	    walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,chk_pav,
			    (Generic)&prof_info);
	  else if (is_arithmetic_if(stmt))
	    walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),NOFUNC,chk_pav,
			    (Generic)&prof_info);
	  stmt = list_next(stmt);
	 }
       if (is_null_node(stmt))
         in_block = false;
       else if(get_stmt_info_ptr(stmt)->block != block)
         in_block = false;
      } while(in_block);
  }

static void determine_edge_profit(block_type *block,
				  Boolean    unvisited_mark,
				  Set        pavset,
				  array_table_type *array_table)

  {
   edge_type *edge;
   float     prob = 0.0;

     if (block->mark == unvisited_mark)
       {
	block->mark = NOT(unvisited_mark);
	for (edge = block->pred;
	     edge != NULL;
	     edge = edge->next_pred)
	  {
	   determine_edge_profit(edge->from,unvisited_mark,pavset,array_table);
	   prob += edge->probability;
	  }
	if (block->block_num == 1)  /* entry */
	  prob = 1.0;
	count_lcpav(block,pavset,array_table,prob);
	for (edge = block->succ;
	     edge != NULL;
	     edge = edge->next_succ)
	   edge->probability = prob / block->num_succs;
       }
  }

static void determine_edge_cost(block_type *block,
				Boolean    unvisited_mark,
				Set        pavset,
				int        size,
				array_table_type *array_table,
				arena_type *ar)

  {
   edge_type *edge;
   int       gen;
   Set       temp;
   
     if (block->mark == unvisited_mark)
       {
	block->mark = NOT(unvisited_mark);
	temp = ut_create_set(ar,LOOP_ARENA,size);
	for (edge = block->pred;
	     edge != NULL;
	     edge = edge->next_pred)
	  {
	   ut_copy12(temp,pavset);
	   ut_forall(gen,pavset)
	     {
	      if (!ut_member_number(edge->from->LI_pavail_out,gen))
		{
		 ut_delete_number(temp,gen);
		 array_table[gen].profit -= edge->probability;
		}
	      else if (ut_member_number(edge->from->LI_avail_out,gen))
		ut_delete_number(temp,gen);
	     }
	   if (!ut_set_is_empty(temp))
	     determine_edge_cost(edge->from,unvisited_mark,temp,size,
				  array_table,ar);
	  }
	/* free(temp); */
       }
  }

void sr_perform_profit_analysis(flow_graph_type  flow_graph,
				int              size,
				array_table_type *array_table,
				arena_type       *ar)

  {
   Set pavset;
   
     pavset = ut_create_set(ar,LOOP_ARENA,size);
     determine_edge_profit(flow_graph.exit,flow_graph.exit->mark,pavset,
			   array_table);
     determine_edge_cost(flow_graph.exit,flow_graph.exit->mark,pavset,size,
			 array_table,ar);
  }
