/* $Id: dfantic.C,v 1.3 1992/10/03 15:48:58 rn Exp $ */
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <sr.h>
#include <Arena.h>
#include <dfantic.h>

static int mark_def(AST_INDEX    node,
		    Set          kill)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   if (is_subscript(node))
     ut_add_number(kill,get_scalar_info_ptr(gen_SUBSCRIPT_get_name(node))->
		   table_index);
   return(WALK_CONTINUE);
  }

static int add_LI_gens(AST_INDEX         node,
		       block_type        *block)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int  gen;

     if (is_subscript(node))
       {
	gen = get_scalar_info_ptr(gen_SUBSCRIPT_get_name(node))->generator;
        if (gen != -1)
	  {
	   ut_add_number(block->LC_gen,gen);
	   if (!ut_member_number(block->kill,gen))
	     ut_add_number(block->gen,gen);
	  }
       }
     return(WALK_CONTINUE);
  }


void sr_calc_local_antic(block_type  *block,
			 int         size,
			 PedInfo     ped)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX  stmt;
   Boolean    in_block = true,
              contains_div = false;

    stmt = block->first;
    ut_clear_set(block->gen);
    ut_clear_set(block->LC_gen);
    ut_clear_set(block->kill);
    if (is_null_node(stmt))
      in_block = false;
    while (in_block)
      {
       if (is_if(stmt))
         stmt = list_first(gen_IF_get_guard_LIST(stmt));
       else if (is_guard(stmt))
	 {
	  walk_expression(gen_GUARD_get_rvalue(stmt),NOFUNC,add_LI_gens,
			  (Generic)block);
	  stmt = list_first(gen_GUARD_get_stmt_LIST(stmt));
	 }
       else if (is_logical_if(stmt))
	 {
	  walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),NOFUNC,add_LI_gens,
			  (Generic)block);
	  stmt = list_first(gen_LOGICAL_IF_get_stmt_LIST(stmt));
	 }
       else
	 {
	  if (is_assignment(stmt))
	    {
	     if (((config_type *)PED_MH_CONFIG(ped))->soft_div)
	       walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
			       ut_check_div,(Generic)&contains_div);
	     if (!contains_div)
	       {
		walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
				add_LI_gens,(Generic)block);
		walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),NOFUNC,
				mark_def,(Generic)block->kill);
	       }
	     else
	       {
		contains_div = false;
		ut_clear_set(block->gen);
		ut_clear_set(block->LC_gen);
		ut_clear_set(block->kill);
		ut_complement(block->kill);
	       }
	    }
	  else if (is_write(stmt))
	    walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,
			    add_LI_gens,(Generic)block);
	  else if (is_read_short(stmt))
	    walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),NOFUNC,
			    mark_def,(Generic)block->kill);
	  else if (is_arithmetic_if(stmt))
	    walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),NOFUNC,
			    add_LI_gens,(Generic)block);
	  else if (is_call(stmt))
	    {
	     ut_clear_set(block->gen);
	     ut_clear_set(block->LC_gen);
	     ut_clear_set(block->kill);
	     ut_complement(block->kill);
	    }
	  stmt = list_next(stmt);
	 }
       if (is_null_node(stmt))
         in_block = false;
       else if(get_stmt_info_ptr(stmt)->block != block)	    
         in_block = false;
      }
  }

static void analyze_block_antic(block_type   *block,
				int          size,
				arena_type   *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   edge_type  *edge;
   Set         temp;

     temp = ut_create_set(ar,LOOP_ARENA,size);
     ut_clear_set(block->LI_antic_out);
     ut_complement(block->LI_antic_out);
     for (edge = block->succ;
	  edge != NULL;
	  edge = edge->next_succ)
       {
	ut_intersect121(block->LI_antic_out,edge->to->LI_antic_in);
	ut_intersect121(block->LC_antic_out,edge->to->LC_antic_in);
       }
     ut_copy12(temp,block->LI_antic_out);
     ut_difference121(temp,block->kill);
     ut_union121(temp,block->gen);
     ut_copy12(block->LI_antic_in,temp);

     /* there are no kills for sinks of LC values on the same iteration as
	the use */

     ut_copy12(temp,block->LC_antic_out);
     ut_union121(temp,block->LC_gen);
     ut_copy12(block->LC_antic_in,temp);
     /* free(temp); */
  }

static void calc_antic(block_type *exit,
		       block_type *block,
		       Boolean    unvisited_mark,
		       int        size,
		       arena_type *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   edge_type *edge;

     if (block->mark == unvisited_mark)
       {
	block->mark = (Boolean)!unvisited_mark;
	for (edge = block->succ;
	     edge != NULL;
	     edge = edge->next_succ)
	   calc_antic(exit,edge->to,unvisited_mark,size,ar);
	if (block != exit)
	  analyze_block_antic(block,size,ar);
       }
  }

void sr_perform_antic_analysis(flow_graph_type  flow_graph,
			       int              size,
			       PedInfo          ped,
			       arena_type       *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   block_type *block;
   
     for (block = flow_graph.entry;
	  block != NULL;
	  block = block->next)
       {
        sr_calc_local_antic(block,size,ped);
	ut_copy12(block->LI_antic_in,block->gen);
       }
     calc_antic(flow_graph.exit,flow_graph.entry,flow_graph.entry->mark,size,
		ar);
     ut_copy12(flow_graph.exit->LC_antic_out,flow_graph.entry->LC_antic_in);
     ut_copy12(flow_graph.exit->LC_antic_in,flow_graph.exit->LC_antic_out);
  }
