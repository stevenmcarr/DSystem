/* $Id: gavail.C,v 1.3 1992/12/11 11:22:15 carr Exp $ */
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <general.h>
#include <sr.h>
#include <mh_ast.h>
#include <fort/walk.h>
#include <gavail.h>

#include <mem_util.h>

#ifndef mh_config_h
#include <mh_config.h>
#endif

static int update_LI_avail(AST_INDEX         node,
			   gavail_info_type  *gavail_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   scalar_info_type *scalar_info;

     if (is_subscript(node))
       {
	scalar_info = get_scalar_info_ptr(gen_SUBSCRIPT_get_name(node));
	ut_add_number(gavail_info->LI_avail,scalar_info->table_index);
	ut_add_number(gavail_info->LI_pavail,scalar_info->table_index);
	if (scalar_info->kill_set != NULL)
	  {
	   ut_difference121(gavail_info->LI_avail,scalar_info->kill_set);
	   ut_difference121(gavail_info->LI_pavail,scalar_info->kill_set);
	  }
       }
   return(WALK_CONTINUE);
  }

static int chk_avail(AST_INDEX        node,
		     gavail_info_type *gavail_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {	
   scalar_info_type *sptr;

     if (is_subscript(node))
       {
	sptr = get_scalar_info_ptr(gen_SUBSCRIPT_get_name(node));
	if (sptr->generator != -1)
	  if (sptr->gen_distance == 0)
	    if (ut_member_number(gavail_info->LI_avail,sptr->generator))
	      sptr->gen_type = LIAV;
	    else
	      sptr->gen_type = LIPAV;
	  else
            if (ut_member_number(gavail_info->exit_block->LC_avail_out,
			      sptr->generator) ||
		(ut_member_number(gavail_info->exit_block->LC_avail_out_if_1,
			       sptr->generator) && sptr->gen_distance == 1))
	      sptr->gen_type = LCAV;
	    else
	      sptr->gen_type = LCPAV;
       }
     return(WALK_CONTINUE);
  }

static void update_gavail(gavail_info_type   *gavail_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX  stmt;
   Boolean    in_block = true,
              contains_div = false;

    ut_copy12(gavail_info->LI_avail,gavail_info->block->LI_avail_in);
    ut_copy12(gavail_info->LI_pavail,gavail_info->block->LI_pavail_in);
    stmt = gavail_info->block->first;
    do
      {
       if (is_if(stmt))
         stmt = list_first(gen_IF_get_guard_LIST(stmt));
       else if (is_guard(stmt))
	 {
	  walk_expression(gen_GUARD_get_rvalue(stmt),NOFUNC,chk_avail,
			  (Generic)gavail_info);
	  walk_expression(gen_GUARD_get_rvalue(stmt),NOFUNC,update_LI_avail,
			  (Generic)gavail_info);
	  stmt = list_first(gen_GUARD_get_stmt_LIST(stmt));
	 }
       else if (is_logical_if(stmt))
	 {
	  walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),NOFUNC,chk_avail,
			  (Generic)gavail_info);
	  walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),NOFUNC,
			  update_LI_avail,(Generic)gavail_info);
	  stmt = list_first(gen_LOGICAL_IF_get_stmt_LIST(stmt));
	 }
       else
	 {
	  if (is_assignment(stmt))
	    {
	     if (((config_type *)PED_MH_CONFIG(gavail_info->ped))->soft_div)
	       walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
			       ut_check_div,(Generic)&contains_div);
	     if (!contains_div)
	       {
		walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
				chk_avail,(Generic)gavail_info);
		walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
				update_LI_avail,(Generic)gavail_info);
		walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),NOFUNC,
				update_LI_avail,(Generic)gavail_info);
	       }
	     else
	       {
		contains_div = false;
		ut_clear_set(gavail_info->block->gen);
		ut_clear_set(gavail_info->block->LC_gen);
		ut_clear_set(gavail_info->block->kill);
		ut_complement(gavail_info->block->kill);
	       }
	    }
	  else if (is_write(stmt))
	    {
	     walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,
			     chk_avail,(Generic)gavail_info);
	     walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,
			     update_LI_avail,(Generic)gavail_info);
	    }
	  else if (is_read_short(stmt))
	   walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),NOFUNC,
			   update_LI_avail,(Generic)gavail_info);
	  else if (is_arithmetic_if(stmt))
	    {
	     walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),NOFUNC,
			     chk_avail,(Generic)gavail_info);
	     walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),NOFUNC,
			     update_LI_avail,(Generic)gavail_info);
	    }
	  else if (is_call(stmt))
	    {
	     ut_clear_set(gavail_info->block->gen);
	     ut_clear_set(gavail_info->block->LC_gen);
	     ut_clear_set(gavail_info->block->kill);
	     ut_complement(gavail_info->block->kill);
	    }
	  stmt = list_next(stmt);
	 }
       if (is_null_node(stmt))
         in_block = false;
       else if(get_stmt_info_ptr(stmt)->block != gavail_info->block)
         in_block = false;
      } while(in_block);
  }

void sr_redo_gen_avail(flow_graph_type flow_graph,
		       int             size,
		       PedInfo         ped,
		       arena_type      *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   gavail_info_type gavail_info;

     gavail_info.LI_avail = ut_create_set(ar,LOOP_ARENA,size);
     gavail_info.LI_pavail = ut_create_set(ar,LOOP_ARENA,size);
     gavail_info.exit_block = flow_graph.exit;
     gavail_info.ped = ped;
     for (gavail_info.block = flow_graph.entry;
	  gavail_info.block != flow_graph.exit;
	  gavail_info.block = gavail_info.block->next)
       update_gavail(&gavail_info);
  }
