/* $Id: mark.C,v 1.11 1997/03/27 20:29:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/****************************************************************************/
/*                                                                          */
/*    File:         mark.C                                                  */
/*                                                                          */
/*    Description:  Store surrounding do information in statement and       */
/*                  subscript nodes.                                        */
/*                                                                          */
/****************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/Memoria/include/mark.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/message.h>


/****************************************************************************/
/*                                                                          */
/*    Function:     set_surrounding_do                                      */
/*                                                                          */
/*    Input:        node - AST index of a subscript                         */
/*                  pre_info - structure containing various info            */
/*                                                                          */
/*    Description:  Create a subscript_info_type structure and store the    */
/*                  pointer in the AST scratch field.  Set the surrounding  */
/*                  DO-loop of the node.                                    */
/*                                                                          */
/****************************************************************************/


static int set_surrounding_do(AST_INDEX       node,
			      pre_info_type   *pre_info)
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


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_mark_do_pre                                          */
/*                                                                          */
/*    Input:        stmt - AST index of a statement                         */
/*                  level - nesting level of stmt                           */
/*                  pre_info - structure containing various info            */
/*                                                                          */
/*    Description:  Set the surrounding do of a statement and walk the      */
/*                  statement to set it for the subscript nodes.            */
/*                                                                          */
/****************************************************************************/


int ut_mark_do_pre(AST_INDEX       stmt,
		   int             level,
		   Generic         pre_info)

  {
   create_stmt_info_ptr(stmt,((pre_info_type *)pre_info)->ar);
   get_stmt_info_ptr(stmt)->stmt_num = ((pre_info_type *)pre_info)->stmt_num++;
   get_stmt_info_ptr(stmt)->surrounding_do = 
             ((pre_info_type *)pre_info)->surrounding_do;
   get_stmt_info_ptr(stmt)->surround_node = 
             ((pre_info_type *)pre_info)->surround_node;
   get_stmt_info_ptr(stmt)->level = level;
   if (is_do(stmt))
     {
      walk_expression(gen_DO_get_control(stmt),(WK_EXPR_CLBACK)set_surrounding_do,(WK_EXPR_CLBACK)NOFUNC,
		      pre_info);
      get_stmt_info_ptr(stmt)->loop_num =((pre_info_type *)pre_info)->loop_num;
      ((pre_info_type *)pre_info)->surrounding_do = 
                   ((pre_info_type *)pre_info)->loop_num++;
      ((pre_info_type *)pre_info)->surround_node = stmt;
     }
   else if (is_assignment(stmt))
     {
      walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),(WK_EXPR_CLBACK)set_surrounding_do,(WK_EXPR_CLBACK)
		      NOFUNC,pre_info);
      walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),(WK_EXPR_CLBACK)set_surrounding_do,(WK_EXPR_CLBACK)
		      NOFUNC,pre_info);
     }
   else if (is_guard(stmt))
     walk_expression(gen_GUARD_get_rvalue(stmt),(WK_EXPR_CLBACK)set_surrounding_do,(WK_EXPR_CLBACK)NOFUNC,
		     pre_info);
   else if (is_write(stmt))
     walk_expression(gen_WRITE_get_data_vars_LIST(stmt),
		     (WK_EXPR_CLBACK)set_surrounding_do,(WK_EXPR_CLBACK)NOFUNC,
		     pre_info);
   else if (is_print(stmt))
     walk_expression(gen_PRINT_get_data_vars_LIST(stmt),
		     (WK_EXPR_CLBACK)set_surrounding_do,(WK_EXPR_CLBACK)NOFUNC,
		     pre_info);
   else if (is_read_short(stmt))
     walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),(WK_EXPR_CLBACK)
		     set_surrounding_do,(WK_EXPR_CLBACK)NOFUNC,pre_info);
   else if (is_read_long(stmt))
     walk_expression(gen_READ_LONG_get_io_LIST(stmt),(WK_EXPR_CLBACK)
		     set_surrounding_do,(WK_EXPR_CLBACK)NOFUNC,pre_info);
   else if (is_logical_if(stmt))
     walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),(WK_EXPR_CLBACK)set_surrounding_do,(WK_EXPR_CLBACK)NOFUNC,
		     pre_info);
   else if (is_arithmetic_if(stmt))
     walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),(WK_EXPR_CLBACK)set_surrounding_do,(WK_EXPR_CLBACK)
		     NOFUNC,pre_info);
   else if (is_computed_goto(stmt))
     walk_expression(gen_COMPUTED_GOTO_get_rvalue(stmt),(WK_EXPR_CLBACK)set_surrounding_do,(WK_EXPR_CLBACK)
		     NOFUNC,pre_info);
   else if (is_call(stmt))
     walk_expression(gen_CALL_get_invocation(stmt),(WK_EXPR_CLBACK)set_surrounding_do,(WK_EXPR_CLBACK)NOFUNC,
		     pre_info);
   else if (is_return(stmt))
     walk_expression(gen_RETURN_get_rvalue(stmt),(WK_EXPR_CLBACK)set_surrounding_do,(WK_EXPR_CLBACK)NOFUNC,
		     pre_info);
   else if (is_if(stmt) || is_continue(stmt) || is_goto(stmt) ||
            is_assigned_goto(stmt) || is_format(stmt) || is_stop(stmt));
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


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_mark_do_post                                         */
/*                                                                          */
/*    Input:        stmt - AST index of a statement                         */
/*                  level - nesting level of stmt                           */
/*                  pre_info - structure containing various info            */
/*                                                                          */
/*    Description:  Update surrounding do info when we move out a level     */
/*                                                                          */
/****************************************************************************/


int ut_mark_do_post(AST_INDEX       stmt,
		    int             level,
		    Generic         pre_info)

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
