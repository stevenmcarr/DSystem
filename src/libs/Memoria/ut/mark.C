/* $Id: mark.C,v 1.3 1992/10/03 15:50:40 rn Exp $ */
#include <mh.h>
#include <mark.h>

static int set_surrounding_do(AST_INDEX       node,
			      pre_info_type   *pre_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   subscript_info_type *sptr;

     if (is_identifier(node))
       {
	if (fst_GetField(pre_info->symtab,gen_get_text(node),SYMTAB_NUM_DIMS)
	    > 0)
	  {
	   create_subscript_ptr(node,pre_info->ar);
	   sptr = get_subscript_ptr(node);
	   sptr->surrounding_do = pre_info->surrounding_do;
	   sptr->surround_node = pre_info->surround_node;
	  }
       }
     return(WALK_CONTINUE);
  }

int ut_mark_do_pre(AST_INDEX       stmt,
		   int             level,
		   Generic         pre_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   create_stmt_info_ptr(stmt,((pre_info_type *)pre_info)->ar);
   get_stmt_info_ptr(stmt)->stmt_num = ((pre_info_type *)pre_info)->stmt_num++;
   get_stmt_info_ptr(stmt)->surrounding_do = 
             ((pre_info_type *)pre_info)->surround_node;
   get_stmt_info_ptr(stmt)->surrounding_do = 
             ((pre_info_type *)pre_info)->surround_node;
   get_stmt_info_ptr(stmt)->level = level;
   if (is_do(stmt))
     {
      walk_expression(gen_DO_get_control(stmt),set_surrounding_do,NOFUNC,
		      pre_info);
      get_stmt_info_ptr(stmt)->loop_num =((pre_info_type *)pre_info)->loop_num;
      ((pre_info_type *)pre_info)->surrounding_do = 
                   ((pre_info_type *)pre_info)->loop_num++;
      ((pre_info_type *)pre_info)->surround_node = stmt;
     }
   else if (is_assignment(stmt))
     {
      walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),set_surrounding_do,
		      NOFUNC,pre_info);
      walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),set_surrounding_do,
		      NOFUNC,pre_info);
     }
   else if (is_guard(stmt))
     walk_expression(gen_GUARD_get_rvalue(stmt),set_surrounding_do,NOFUNC,
		     pre_info);
   else if (is_write(stmt))
     walk_expression(gen_WRITE_get_data_vars_LIST(stmt),set_surrounding_do,
		     NOFUNC,pre_info);
   else if (is_read_short(stmt))
     walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),
		     set_surrounding_do,NOFUNC,pre_info);
   else if (is_logical_if(stmt))
     walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),set_surrounding_do,NOFUNC,
		     pre_info);
   else if (is_arithmetic_if(stmt))
     walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),set_surrounding_do,
		     NOFUNC,pre_info);
   else if (is_computed_goto(stmt))
     walk_expression(gen_COMPUTED_GOTO_get_rvalue(stmt),set_surrounding_do,
		     NOFUNC,pre_info);
   else if (is_call(stmt))
     walk_expression(gen_CALL_get_invocation(stmt),set_surrounding_do,NOFUNC,
		     pre_info);
   else if (is_return(stmt))
     walk_expression(gen_RETURN_get_rvalue(stmt),set_surrounding_do,NOFUNC,
		     pre_info);
   else if (is_if(stmt) || is_continue(stmt) || is_goto(stmt) ||
            is_assigned_goto(stmt));
   else if (executable_stmt(stmt))
     { 
      char errmsg[30];
      
      sprintf(errmsg,"Statement not handled %d\n",NT(stmt));
      message(errmsg);
      ((pre_info_type *)pre_info)->abort = true;
      return(WALK_ABORT);
     }
   return(WALK_CONTINUE);
  }

int ut_mark_do_post(AST_INDEX       stmt,
		    int             level,
		    Generic         pre_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   if (is_do(stmt))
     {
      ((pre_info_type *)pre_info)->surrounding_do = 
                   get_stmt_info_ptr(stmt)->surrounding_do;
      ((pre_info_type *)pre_info)->surround_node = 
                   get_stmt_info_ptr(stmt)->surround_node;
     }
   return(WALK_CONTINUE);
  }
