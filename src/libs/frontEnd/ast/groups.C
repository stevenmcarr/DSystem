/* $Id: groups.C,v 1.2 2001/10/12 19:37:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*------------------------------------------------------------------

	groups.c	Functions to classify AST nodes
			Companion to $RN_HOME/src/include/fort/groups.h

	History
	~~~~~~~

	Jul 90  Keith - created out of groups.h
	Aug 90  Chau-Wen - moved to fort
	Sep 92  JMC collected similar functionality from elsewhere and
	        reorganized

*/

#include <stdio.h>
#include <assert.h>


#include <libs/frontEnd/ast/astlist.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/fortTree/fortsym.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/aphelper.h>

#include <libs/support/memMgmt/mem.h>

STATIC(Boolean, parent_is_f77_subprogram_stmt, (AST_INDEX n));
STATIC(Boolean, parent_is_smp_loop_stmt, (AST_INDEX stmt_list));

Boolean is_f77_subprogram_stmt(AST_INDEX n)	/* certified -- JMC 9/92 */
{
  if (n == AST_NIL) return false;
  switch(gen_get_node_type(n)) {
  case GEN_BLOCK_DATA: 		case GEN_FUNCTION: 
  case GEN_PROGRAM: 		case GEN_SUBROUTINE:
    return true;
  default:
    return false;
  }
}


Boolean is_f77_initial_specification_stmt(AST_INDEX n) /* certified -- JMC 9/92 */
{
  if (n == AST_NIL) return false;
  switch(gen_get_node_type(n)) {

     /* statements really in the "initial specification class" */
  case GEN_IMPLICIT: 		

    /* other things that are OK if intermixed */
  case GEN_PARAMETER:		case GEN_ENTRY:
  case GEN_FORMAT:
    return true;
  default:
    return false;
  }
}


Boolean is_f77_other_specification_stmt(AST_INDEX n) /* certified -- JMC 9/92 */
{
  if (n == AST_NIL) return false;
  switch(gen_get_node_type(n)) {

    /* statements really in the "other specification class" */
  case GEN_COMMON: 		case GEN_DIMENSION:
  case GEN_EQUIVALENCE: 	case GEN_EXTERNAL:
  case GEN_INTRINSIC: 		case GEN_SAVE:
  case GEN_TYPE_STATEMENT: 	

    /* other things that are OK if intermixed */
  case GEN_ENTRY: 		case GEN_FORMAT:
  case GEN_PARAMETER:		
    return true;
  default:
    return false;
  }
}


Boolean is_f77_final_specification_stmt(AST_INDEX n) /* certified -- JMC 9/92 */
{
  if (n == AST_NIL) return false;
  switch(gen_get_node_type(n)) {

    /* statements really in the "final specification class" */
  case GEN_STMT_FUNCTION:	
    
    /* other things that are OK if intermixed */
  case GEN_ENTRY: 		case GEN_FORMAT:
  case GEN_PARAMETER:		case GEN_DATA:
    return true;
  default:
    return false;
  }
}

Boolean is_f77_specification_stmt(AST_INDEX n)	/* certified -- JMC 9/92 */
{
  /* to be thought of as non-executable ... */
  return (BOOL(is_f77_initial_specification_stmt(n) ||
	       is_f77_other_specification_stmt(n) ||
	       is_f77_final_specification_stmt(n)));
}

Boolean is_f77_executable_stmt(AST_INDEX n)	/* certified -- JMC 9/92 */
{
  if (n == AST_NIL) return false;
  switch(gen_get_node_type(n)) {

  /* things that really belong in this class */
  case GEN_ASSIGN:		case GEN_ASSIGNMENT:
  case GEN_BACKSPACE_SHORT: 	case GEN_BACKSPACE_LONG: 	
  case GEN_CALL: 		case GEN_CLOSE:
  case GEN_CONTINUE:		
  case GEN_ENDFILE_SHORT:	case GEN_ENDFILE_LONG: 	
  case GEN_ASSIGNED_GOTO: 	case GEN_COMPUTED_GOTO:	
  case GEN_GOTO: 		case GEN_ARITHMETIC_IF:
  case GEN_IF: 			case GEN_LOGICAL_IF:	
  case GEN_GUARD:		case GEN_INQUIRE:
  case GEN_OPEN:		case GEN_PAUSE:
  case GEN_PRINT:		case GEN_READ_SHORT:
  case GEN_READ_LONG:		case GEN_RETURN:
  case GEN_REWIND_SHORT:	case GEN_REWIND_LONG:
  case GEN_STOP: 		case GEN_WRITE:		
    return true;
  case GEN_DO: 
    {
      AST_INDEX control = gen_DO_get_control(n);
      if (control == AST_NIL) return false;
      return (BOOL(is_inductive(control)));
    }

    /* other things that are OK if intermixed */
  case GEN_ENTRY: 		case GEN_FORMAT:
  case GEN_DATA:
    return true;
  default:
    return false;
  }
}

#ifdef SOLARIS
extern "C" 
#endif
AST_INDEX first_executable_stmt(Boolean (*pre_cond)(AST_INDEX), 
				Boolean (*spec_cond)(AST_INDEX), 
                                Boolean (*post_cond)(AST_INDEX), 
				AST_INDEX stmt_list)
{
  AST_INDEX stmt;
  assert(stmt_list != AST_NIL && pre_cond(stmt_list));
  stmt = list_first(stmt_list);
  while (stmt != AST_NIL && (spec_cond(stmt) || is_comment(stmt) ) ) {
    stmt = list_next(stmt);
  }
  assert(stmt == AST_NIL || post_cond(stmt));
  return stmt;
}


static Boolean parent_is_f77_subprogram_stmt(AST_INDEX n)
{
  return is_f77_subprogram_stmt(out(n));
}


AST_INDEX first_f77_executable_stmt(AST_INDEX stmt_list)
{
   return first_executable_stmt(parent_is_f77_subprogram_stmt, 
				is_f77_specification_stmt, 
				is_f77_executable_stmt, stmt_list);
}


/* certified -- JMC 9/92 */
Boolean is_smp_extension_other_specification_stmt(AST_INDEX n) 
{
  if (n == AST_NIL) return false;
  switch(gen_get_node_type(n)) {
    /* things that really belong in this class */
  case GEN_TASK_COMMON:
    return true;
  default:
    return false;
  }	
}

Boolean is_smp_specification_stmt(AST_INDEX n)	/* certified -- JMC 9/92 */
{
  /* to be thought of as non-executable ... */
  return (BOOL(is_f77_specification_stmt(n) || 
	       is_smp_extension_other_specification_stmt(n)));
}


Boolean is_smp_extension_executable_stmt(AST_INDEX n) /* certified -- JMC 9/92 */
{
  if (n == AST_NIL) return false;
  switch(gen_get_node_type(n)) {
  case GEN_TASK:	 	case GEN_PARALLEL:
  case GEN_PARALLEL_CASE: 	case GEN_DO_ALL:		

  case GEN_LOCK:		case GEN_UNLOCK:		

  case GEN_WAIT: 		case GEN_POST:
  case GEN_CLEAR:		

  case GEN_SET_BARRIER: 	case GEN_BLOCK:

  case GEN_PARALLELLOOP:	
  case GEN_STOPLOOP:
    return true;
  case GEN_DO:  /* Fortran 90 loop control */
    {
      AST_INDEX control = gen_DO_get_control(n);
      if (control == AST_NIL) return false;
      return NOT(BOOL(is_inductive(control)));
    }
  default:
    return false;
  }
}


Boolean is_smp_loop_specification_stmt(AST_INDEX n)
{
  if (n == AST_NIL) return false;
  return BOOL(is_private(n));
}


Boolean is_smp_executable_stmt(AST_INDEX n)	/* certified -- JMC 9/92 */
{
  /* to be thought of as executable ... */
  return (BOOL(is_f77_executable_stmt(n) || 
	       is_smp_extension_executable_stmt(n)));
}

AST_INDEX first_smp_executable_stmt(AST_INDEX stmt_list)
{
   return first_executable_stmt(parent_is_f77_subprogram_stmt, 
				is_smp_specification_stmt, 
				is_smp_executable_stmt, stmt_list);
}


Boolean is_smp_loop_stmt(AST_INDEX n)		/* certified -- JMC 9/92 */
{
  if (n == AST_NIL) return false;
  switch(gen_get_node_type(n)) {
  case GEN_DO:			case GEN_DO_ALL:
  case GEN_PARALLELLOOP:
    return true;
  default:
    return false;
  }
}

static Boolean parent_is_smp_loop_stmt(AST_INDEX stmt_list)
{
  return (BOOL(is_smp_loop_stmt(stmt_list) && 
	       is_smp_extension_executable_stmt(stmt_list)));
}


/* certified -- JMC 9/92 */
AST_INDEX first_smp_executable_stmt_in_loop(AST_INDEX stmt_list)
{
   return first_executable_stmt(parent_is_smp_loop_stmt, 
				is_smp_loop_specification_stmt, 
				is_smp_executable_stmt, stmt_list);
}

Boolean is_f77_statement(AST_INDEX n)		/* certified -- JMC 9/92 */
{
    /* what about statement function def and data statement ? */
  return (BOOL(is_f77_subprogram_stmt(n) || is_f77_specification_stmt(n) ||
	       is_f77_executable_stmt(n))); 
}


Boolean is_f77_operator(AST_INDEX n)		/* certified -- JMC 9/92 */
{
  switch(gen_get_node_type(n)) {
  case GEN_SUBSCRIPT:		case GEN_SUBSTRING:
  case GEN_INVOCATION:		case GEN_BINARY_EXPONENT:
  case GEN_BINARY_TIMES:	case GEN_BINARY_DIVIDE:
  case GEN_BINARY_PLUS:		case GEN_BINARY_MINUS:
  case GEN_BINARY_CONCAT:	case GEN_BINARY_AND:
  case GEN_BINARY_OR:		case GEN_BINARY_EQ:
  case GEN_BINARY_NE:		case GEN_BINARY_GE:
  case GEN_BINARY_GT:		case GEN_BINARY_LE:
  case GEN_BINARY_LT:		case GEN_BINARY_EQV:
  case GEN_BINARY_NEQV:		case GEN_UNARY_MINUS:
  case GEN_UNARY_NOT:
    return true;
  default:
    return false;
  }
}



/*----------------------------- deprecated ----------------------------*/

/* should have one and only one client -- the control dependence code  -- JMC 9/92*/
Boolean is_executable(AST_INDEX stmt) 
{
  switch (gen_get_node_type(stmt)) {
  case GEN_BLOCK_DATA:
  case GEN_COMMON:
  case GEN_DIMENSION:
  case GEN_EQUIVALENCE:
  case GEN_TYPE_STATEMENT:
  case GEN_EXTERNAL:
  case GEN_IMPLICIT:
  case GEN_PARAMETER:
  case GEN_SAVE:
  case GEN_COMMENT:
  case GEN_DATA:
    return false;
    
  default:
    return true;
  }
}

Boolean is_loop(AST_INDEX n) /* certified to be equivalent to is_loop from treeutil.c -- JMC 9/92 */
{
  return is_loop_stmt(n);
}

Boolean cprop_operator(AST_INDEX n)
{
  return is_operator(n);
}

Boolean cprop_loop_stmt(AST_INDEX n)
{
  return is_loop_stmt(n);
}

Boolean cprop_executable_stmt(AST_INDEX n)
{
  return is_executable_stmt(n);
}



Boolean 
cprop_sub_expr_op(AST_INDEX n)
{
  switch(gen_get_node_type(n))
  {
	case GEN_SUBSCRIPT:		case GEN_SUBSTRING:
	case GEN_INVOCATION:
		return true;

	default:
		return false;
  }
}

Boolean is_io_keyword(AST_INDEX n)		/* certified -- JMC 9/92 */
{
  switch(gen_get_node_type(n)) {
  case GEN_EXIST_QUERY:		case GEN_OPENED_QUERY:	
  case GEN_NUMBER_QUERY:	case GEN_NAMED_QUERY:	
  case GEN_NAME_QUERY:		case GEN_ERR_SPECIFY:
  case GEN_ACCESS_QUERY:	case GEN_SEQUENTIAL_QUERY:
  case GEN_DIRECT_QUERY:	case GEN_FORM_QUERY:
  case GEN_FORMATTED_QUERY:	case GEN_UNFORMATTED_QUERY:
  case GEN_RECL_QUERY:		case GEN_BLANK_QUERY:
  case GEN_NEXTREC_QUERY:	case GEN_IOSTAT_QUERY:
  case GEN_FILE_SPECIFY:	case GEN_FMT_SPECIFY:	
  case GEN_REC_SPECIFY:		case GEN_END_SPECIFY:
  case GEN_STATUS_SPECIFY:	case GEN_ACCESS_SPECIFY:
  case GEN_FORM_SPECIFY:	case GEN_RECL_SPECIFY:	
  case GEN_BLANK_SPECIFY:	case GEN_UNIT_SPECIFY:
    return true;
  default:
    return false;
  }
}


Boolean cprop_common_stmt(AST_INDEX n)
{
  switch(gen_get_node_type(n)) {
  case GEN_COMMON:		case GEN_TASK_COMMON:
    return true;
  default:
    return false;
  }
}


Boolean 
cprop_labelled_stmt(AST_INDEX n)
{
  switch(gen_get_node_type(n))
  {
	case GEN_FUNCTION:		case GEN_PROGRAM:	
	case GEN_SUBROUTINE:		case GEN_BLOCK_DATA:	
	case GEN_DIMENSION:		case GEN_EQUIVALENCE:
	case GEN_SAVE:			case GEN_TYPE_STATEMENT:
	case GEN_EXTERNAL:		case GEN_IMPLICIT:	
	case GEN_DATA:			case GEN_INTRINSIC:	
	case GEN_PARAMETER:		case GEN_ENTRY:	
	case GEN_STMT_FUNCTION:		case GEN_FORMAT:	
	case GEN_AT:			case GEN_PRIVATE:	
	case GEN_DEBUG:			case GEN_TRACEON:	
	case GEN_TRACEOFF:		case GEN_BLOCK:	
	case GEN_LOCK:			case GEN_UNLOCK:
	case GEN_COMMON:		case GEN_TASK_COMMON:	

	case GEN_ASSIGN:		case GEN_ASSIGNMENT:
	case GEN_CALL:			case GEN_BACKSPACE_SHORT:
	case GEN_BACKSPACE_LONG:	case GEN_CLOSE:
	case GEN_CONTINUE:		case GEN_INQUIRE:
	case GEN_ENDFILE_SHORT:		case GEN_TASK:
	case GEN_ENDFILE_LONG:		case GEN_ASSIGNED_GOTO:
	case GEN_COMPUTED_GOTO:		case GEN_IF:
	case GEN_GOTO:			case GEN_ARITHMETIC_IF:
	case GEN_LOGICAL_IF:		case GEN_GUARD:
	case GEN_OPEN:			case GEN_PAUSE:
	case GEN_PRINT:			case GEN_WAIT:
	case GEN_READ_SHORT:		case GEN_READ_LONG:
	case GEN_RETURN:		case GEN_PARALLEL:
	case GEN_REWIND_SHORT:		case GEN_REWIND_LONG:
	case GEN_STOP:			case GEN_DO:
	case GEN_DO_ALL:		case GEN_PARALLELLOOP:
	case GEN_WRITE:			case GEN_STOPLOOP:
	case GEN_PARALLEL_CASE:		case GEN_POST:
	case GEN_SET_BARRIER:		case GEN_CLEAR:
		return true;

	default:
		return false;
  }
}


Boolean 
cprop_new_instance(AST_INDEX n)
{
  switch(gen_get_node_type(n))
  {
	case GEN_GLOBAL:		case GEN_PROGRAM:	
	case GEN_FUNCTION:		case GEN_SUBROUTINE:	
	case GEN_BLOCK_DATA:
		return true;

	default:
		return false;
  }
}


Boolean 
cprop_poss_gets_node(AST_INDEX n)
{
  switch(gen_get_node_type(n))
  {
	case GEN_PARAMETER: 		case GEN_ENTRY:
	case GEN_FUNCTION:		case GEN_SUBROUTINE:	
	case GEN_BLOCK_DATA:		case GEN_PROGRAM:	

	case GEN_ASSIGN:		case GEN_ASSIGNMENT:
	case GEN_CALL:			case GEN_BACKSPACE_SHORT:
	case GEN_BACKSPACE_LONG:	case GEN_CLOSE:
	case GEN_CONTINUE:		case GEN_INQUIRE:
	case GEN_ENDFILE_SHORT:		case GEN_TASK:
	case GEN_ENDFILE_LONG:		case GEN_ASSIGNED_GOTO:
	case GEN_COMPUTED_GOTO:		case GEN_IF:
	case GEN_GOTO:			case GEN_ARITHMETIC_IF:
	case GEN_LOGICAL_IF:		case GEN_GUARD:
	case GEN_OPEN:			case GEN_PAUSE:
	case GEN_PRINT:			case GEN_WAIT:
	case GEN_READ_SHORT:		case GEN_READ_LONG:
	case GEN_RETURN:		case GEN_PARALLEL:
	case GEN_REWIND_SHORT:		case GEN_REWIND_LONG:
	case GEN_STOP:			case GEN_DO:
	case GEN_DO_ALL:		case GEN_PARALLELLOOP:
	case GEN_WRITE:			case GEN_STOPLOOP:
	case GEN_PARALLEL_CASE:		case GEN_POST:
	case GEN_SET_BARRIER:		case GEN_CLEAR:
		return true;

	default:
		return false;
  }
}


Boolean 
cprop_file_pos_stmt(AST_INDEX n)
{
  switch(gen_get_node_type(n))
  {
	case GEN_BACKSPACE_SHORT:	case GEN_BACKSPACE_LONG:
	case GEN_ENDFILE_SHORT:		case GEN_ENDFILE_LONG:
	case GEN_REWIND_SHORT:		case GEN_REWIND_LONG:
		return true;

	default:
		return false;
  }
}

Boolean 
cprop_io_stmt(AST_INDEX n)
{
  switch(gen_get_node_type(n))
  {
	case GEN_READ_SHORT:		case GEN_READ_LONG:
	case GEN_WRITE:			case GEN_PRINT:
	case GEN_OPEN:			case GEN_CLOSE:
	case GEN_INQUIRE:
	case GEN_BACKSPACE_SHORT:	case GEN_BACKSPACE_LONG:
	case GEN_ENDFILE_SHORT:		case GEN_ENDFILE_LONG:
	case GEN_REWIND_SHORT:		case GEN_REWIND_LONG:
		return true;

	default:
		return false;
  }
}

Boolean 
cprop_stmt_containing_expr(AST_INDEX n)
{
  switch(gen_get_node_type(n))
  {
	case GEN_ASSIGN:		case GEN_ASSIGNMENT:
	case GEN_RETURN:		case GEN_COMPUTED_GOTO:
	case GEN_ARITHMETIC_IF:		case GEN_LOGICAL_IF:
	case GEN_GUARD:			case GEN_CALL:
	case GEN_PARAMETER:		case GEN_DATA:
	case GEN_FUNCTION:		case GEN_SUBROUTINE:
	case GEN_ENTRY:			case GEN_ASSIGNED_GOTO:
	case GEN_CLEAR:			case GEN_POST:
	case GEN_PARALLEL_CASE:		case GEN_PARALLEL:
	case GEN_TASK:			case GEN_SET_BARRIER:
	case GEN_WAIT:
	case GEN_DO:			case GEN_DO_ALL:
	case GEN_PARALLELLOOP:

	case GEN_READ_SHORT:		case GEN_READ_LONG:
	case GEN_WRITE:			case GEN_PRINT:
	case GEN_OPEN:			case GEN_CLOSE:
	case GEN_INQUIRE:
	case GEN_BACKSPACE_SHORT:	case GEN_BACKSPACE_LONG:
	case GEN_ENDFILE_SHORT:		case GEN_ENDFILE_LONG:
	case GEN_REWIND_SHORT:		case GEN_REWIND_LONG:

	case GEN_COMMON:		case GEN_TASK_COMMON:
		return true;

	default:
		return false;
  }
}


Boolean 
cprop_has_stmt_list(AST_INDEX n)
{
  switch(gen_get_node_type(n))
  {
	case GEN_IF:			case GEN_LOGICAL_IF:	
	case GEN_GUARD:			case GEN_DEBUG:	
	case GEN_PARALLEL:		case GEN_PARALLEL_CASE:

	case GEN_GLOBAL:		case GEN_PROGRAM:	
	case GEN_FUNCTION:		case GEN_SUBROUTINE:	
	case GEN_BLOCK_DATA:

	case GEN_DO:			case GEN_DO_ALL:
	case GEN_PARALLELLOOP:
		return true;

	default:
		return false;
  }
}

Boolean 
cprop_has_close_lbl_def(AST_INDEX n)
{
  switch(gen_get_node_type(n))
  {
	case GEN_FUNCTION:		case GEN_PROGRAM:	
	case GEN_BLOCK_DATA:		case GEN_SUBROUTINE:	
	case GEN_IF:			case GEN_DEBUG:	
	case GEN_PARALLEL:		

	case GEN_DO:			case GEN_DO_ALL:
	case GEN_PARALLELLOOP:
		return true;

	default:
		return false;
  }
}

Boolean 
cprop_used_node(AST_INDEX n)
{
  switch(gen_get_node_type(n))
  {
	case GEN_FREED_NODE:		case GEN_NULL_NODE:	
	case GEN_LIST_OF_NODES:		case GEN_ERROR:
	case GEN_PLACE_HOLDER:
		return false;

	default:
		return true;
  }
}

Boolean 
cprop_control_flow(AST_INDEX n)
{
  switch(gen_get_node_type(n))
  {
	case GEN_GOTO:			case GEN_COMPUTED_GOTO:	
	case GEN_ASSIGNED_GOTO:		case GEN_IF:
	case GEN_ARITHMETIC_IF:		case GEN_RETURN:
	case GEN_PARALLEL:		case GEN_PARALLEL_CASE:
		return true;

	default:
		return false;
  }
}

Boolean 
cprop_exit_stmt(AST_INDEX n)
{
  switch(gen_get_node_type(n))
  {
	case GEN_GOTO:			case GEN_COMPUTED_GOTO:	
	case GEN_ASSIGNED_GOTO:		case GEN_RETURN:
	case GEN_STOP:			case GEN_STOPLOOP:
		return true;

	default:
		return false;
  }
}

Boolean 
cprop_struct_guard(AST_INDEX n)
{
  switch(gen_get_node_type(n))
  {
	case GEN_IF:			case GEN_GUARD:	
	case GEN_ARITHMETIC_IF:		case GEN_LOGICAL_IF:
		return true;

	default:
		return false;
  }
}

Boolean 
cprop_poss_alternate_edges(AST_INDEX n)
{
  switch(gen_get_node_type(n))
  {
	case GEN_READ_LONG:		case GEN_WRITE:
	case GEN_CALL:			case GEN_INQUIRE:
	case GEN_OPEN:			case GEN_CLOSE:
	case GEN_BACKSPACE_LONG:	case GEN_ENDFILE_LONG:
	case GEN_REWIND_LONG:		case GEN_ENTRY:
	case GEN_DO:			case GEN_DO_ALL:
	case GEN_PARALLELLOOP:

	/*
	 *  Added because of new edges from special root node
	 *  to entry points -- paco 27 May 92
	 */
	case GEN_PROGRAM:
	case GEN_SUBROUTINE:		case GEN_FUNCTION:

		return true;

	default:
		return false;
  }
}


Boolean 
cprop_usable_exp(Generic t, AST_INDEX n)
{

  if (is_operator(n) == true && 
      (gen_get_node_type(n) != GEN_SUBSCRIPT))
     return true;
  if (gen_get_node_type(n) == GEN_IDENTIFIER && (!array_ident(t,n)))
     return true;
  return false;
}

Boolean
cprop_array_ident(Generic t, AST_INDEX n)
{
    int index;

    index = fst_QueryIndex((SymDescriptor) t, gen_get_text(n));

    if (!fst_index_is_valid(index))
	return false;

    return BOOL(FS_IS_ARRAY((SymDescriptor) t, index));
}

Boolean
cprop_dummy_proc(Generic t, AST_INDEX n)
{
    int index;

    index = fst_QueryIndex((SymDescriptor) t, gen_get_text(n));

    if (!fst_index_is_valid(index))
	return false;

    return BOOL(FS_IS_DUMMY_PARAM_FOR_ENTRY_OR_PROCEDURE((SymDescriptor) t,
							 index));
}
