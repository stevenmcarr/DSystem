/* $Id: treeutil.C,v 1.1 1997/06/24 17:41:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*----------------------------------------------------------------

    treeutil.c      Utilities for accessing the AST

*/


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <strings.h>

#include <sys/types.h>

#include <libs/support/memMgmt/mem.h>

#include <libs/frontEnd/ast/astlist.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/aphelper.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/treeutil.h>
#include <libs/frontEnd/ast/builtins.h>


STATIC (void, walkIDs, (AST_INDEX n, ReferenceAccessType atype, 
                        WK_IDS_CLBACK_V fn, va_list args));
STATIC (void, walkIDsEx, (AST_INDEX n, WK_IDS_EX_CLBACK_V fn, va_list args));
STATIC (void, walkIDsInStmt_internal, (AST_INDEX s, WK_IDS_CLBACK_V fn,
                                       va_list args));


/*---------------------------------------------------------------------

    gen_get_loop_control()   returns AST_INDEX of control for a loop

*/

AST_INDEX
gen_get_loop_control(AST_INDEX loop)
{
  switch (gen_get_node_type(loop))
  {
    case GEN_DO:
		return gen_DO_get_control(loop);
    case GEN_DO_ALL:
		return gen_DO_ALL_get_control(loop);
    case GEN_PARALLELLOOP:
		return gen_PARALLELLOOP_get_control(loop);
    default:
      assert(0);
  }
}

/*---------------------------------------------------------------------

    loop_level()   Returns the loop nesting level N

    0 = stmt not in loop
    N = stmt enclosed by N loop(s)

*/

int
loop_level(AST_INDEX stmt)
{
  int level;

  for (level = 0; stmt != AST_NIL; stmt = out(stmt))
  {
    if (is_loop(stmt))
      level++;
  }

  return level;
}


/*----------------------------------------------------------------

    gen_get_stmt_list()   Get the list of statements of a node

*/

AST_INDEX
gen_get_stmt_list(AST_INDEX stmt)
{
  AST_INDEX stmt_list;

  switch (gen_get_node_type(stmt))
  {
    case GEN_GLOBAL:
      stmt_list = gen_GLOBAL_get_subprogram_scope_LIST(stmt);
      break;

    case GEN_FUNCTION:
      stmt_list = gen_FUNCTION_get_stmt_LIST(stmt);
      break;

    case GEN_PROGRAM:
      stmt_list = gen_PROGRAM_get_stmt_LIST(stmt);
      break;

    case GEN_SUBROUTINE:
      stmt_list = gen_SUBROUTINE_get_stmt_LIST(stmt);
      break;

    case GEN_BLOCK_DATA:
      stmt_list = gen_BLOCK_DATA_get_stmt_LIST(stmt);
      break;

    case GEN_DO:
      stmt_list = gen_DO_get_stmt_LIST(stmt);
      break;

    case GEN_LOGICAL_IF:
      stmt_list = gen_LOGICAL_IF_get_stmt_LIST(stmt);
      break;

    case GEN_IF:
      stmt_list = gen_IF_get_guard_LIST(stmt);
      break;

    case GEN_GUARD:
      stmt_list = gen_GUARD_get_stmt_LIST(stmt);
      break;

    case GEN_DEBUG:
      stmt_list = gen_DEBUG_get_stmt_LIST(stmt);
      break;

    case GEN_PARALLEL:
      stmt_list = gen_PARALLEL_get_stmt_LIST(stmt);
      break;

    case GEN_PARALLEL_CASE:
      stmt_list = gen_PARALLEL_CASE_get_stmt_LIST(stmt);
      break;

    case GEN_PARALLELLOOP:
      stmt_list = gen_PARALLELLOOP_get_stmt_LIST(stmt);
      break;

    case GEN_DO_ALL:
      stmt_list = gen_DO_ALL_get_stmt_LIST(stmt);
      break;

    case GEN_WHERE:
      stmt_list = gen_WHERE_get_stmt_LIST(stmt);
      break;

    case GEN_WHERE_BLOCK:
      stmt_list = gen_WHERE_BLOCK_get_guard_LIST(stmt);
      break;

    case GEN_PRINT:     /* removed gen_PRINT_get_data_vars_LIST(stmt);
                         * mpal:910710 */
    case GEN_WRITE:     /* removed gen_WRITE_get_data_vars_LIST(stmt);
                         * mpal:910710 */
    case GEN_CALL:/* removed gen_INVOCATION_get_actual_args_LIST(stmt);      */
    default:
      stmt_list = AST_NIL;
      break;

  }
  return (stmt_list);
}


/*----------------------------------------------------------------

  walk_statements()   Walk over stmts in an AST

*/

int
walk_statements(AST_INDEX stmt, int nesting_level, WK_STMT_CLBACK pre_action, 
                WK_STMT_CLBACK post_action, Generic parm)
{
  AST_INDEX curr, old_prev, old_next, stmt_list;
  int rc;

  rc = WALK_CONTINUE;
  if (is_list(stmt))
  {
    curr = list_first(stmt);
    while (curr != AST_NIL)
    {
      old_prev = list_prev(curr);
      old_next = list_next(curr);
      rc = walk_statements(curr,
                           nesting_level,
                           pre_action, post_action, parm);
      switch (rc)
      {
        case WALK_CONTINUE:
        case WALK_SKIP_CHILDREN:
          curr = list_next(curr);
          break;

        case WALK_ABORT:
          return (WALK_ABORT);

        case WALK_FROM_OLD_PREV:
          if (old_prev == AST_NIL)
            curr = old_next;
          else
            curr = list_next(old_prev);
          break;

        case WALK_FROM_OLD_NEXT:
          curr = old_next;
          break;

      }
    }
    return (WALK_CONTINUE);
  }
  else
  {
    if (pre_action)
      rc = (*pre_action) (stmt, nesting_level, parm);

    if (rc == WALK_CONTINUE)
    {
      stmt_list = gen_get_stmt_list(stmt);

      if (stmt_list != AST_NIL)
      {
        rc = walk_statements(stmt_list,
                             (is_do(stmt) || is_parallelloop(stmt) ?
                              nesting_level + 1 : nesting_level),
                             pre_action, post_action, parm);
      }

      if (rc == WALK_CONTINUE && post_action)
        rc = (*post_action) (stmt, nesting_level,
                             parm);
    }
  }
  return (rc);
}


/*----------------------------------------------------------------

  walk_statementsV()   Walk over stmts in an AST - allows a non-pointer va_list
                       structure to be passed

*/

static int
walk_statementsV(AST_INDEX stmt, int nesting_level, WK_STMT_CLBACK_V pre_action, 
                WK_STMT_CLBACK_V post_action, va_list parm)
{
  AST_INDEX curr, old_prev, old_next, stmt_list;
  int rc;

  rc = WALK_CONTINUE;
  if (is_list(stmt))
  {
    curr = list_first(stmt);
    while (curr != AST_NIL)
    {
      old_prev = list_prev(curr);
      old_next = list_next(curr);
      rc = walk_statementsV(curr,
			    nesting_level,
			    pre_action, post_action, parm);
      switch (rc)
      {
        case WALK_CONTINUE:
        case WALK_SKIP_CHILDREN:
          curr = list_next(curr);
          break;

        case WALK_ABORT:
          return (WALK_ABORT);

        case WALK_FROM_OLD_PREV:
          if (old_prev == AST_NIL)
            curr = old_next;
          else
            curr = list_next(old_prev);
          break;

        case WALK_FROM_OLD_NEXT:
          curr = old_next;
          break;

      }
    }
    return (WALK_CONTINUE);
  }
  else
  {
    if (pre_action)
      rc = (*pre_action) (stmt, nesting_level, parm);

    if (rc == WALK_CONTINUE)
    {
      stmt_list = gen_get_stmt_list(stmt);

      if (stmt_list != AST_NIL)
      {
        rc = walk_statementsV(stmt_list,
			      (is_do(stmt) || is_parallelloop(stmt) ?
                              nesting_level + 1 : nesting_level),
			      pre_action, post_action, parm);
      }

      if (rc == WALK_CONTINUE && post_action)
        rc = (*post_action) (stmt, nesting_level,
                             parm);
    }
  }
  return (rc);
}

/*
 * An updated version of walk_statements, allowing multiple args
 */
int walk_statements_v (AST_INDEX stmt, int nesting_level, WK_STMT_CLBACK_V pre_action,
                       WK_STMT_CLBACK_V post_action, ...)
{
  int      returnVal = 0;
  va_list  args;
  
  va_start(args, post_action);

  returnVal = walk_statementsV(stmt, nesting_level, pre_action, post_action, args);
  va_end(args);
 
  return returnVal;
}



/*----------------------------------------------------------------

  walk_statements_reverse()   Walk over stmts in an AST in reverse

*/

int
walk_statements_reverse(AST_INDEX stmt, int nesting_level, WK_STMT_CLBACK pre_action, 
                        WK_STMT_CLBACK post_action, Generic parm)
{
  AST_INDEX curr, old_prev, old_next, stmt_list;
  int rc;

  rc = WALK_CONTINUE;
  if (is_list(stmt))
  {
    curr = list_last(stmt);
    while (curr != AST_NIL)
    {
      old_prev = list_prev(curr);
      old_next = list_next(curr);
      rc = walk_statements_reverse(curr, nesting_level,
                                   pre_action, post_action, parm);

      switch (rc)
      {
        case WALK_CONTINUE:
        case WALK_SKIP_CHILDREN:
          curr = old_prev;
          break;

        case WALK_ABORT:
          return (WALK_ABORT);

        case WALK_FROM_OLD_PREV:
          curr = old_prev;
          break;

        case WALK_FROM_OLD_NEXT:
          if (old_next == AST_NIL)
            curr = old_prev;
          else
            curr = list_prev(old_next);
          break;
      }
    }
    return (WALK_CONTINUE);
  }
  else
  {
    if (pre_action)
      rc = (*pre_action) (stmt, nesting_level, parm);

    if (rc == WALK_CONTINUE)
    {
      stmt_list = gen_get_stmt_list(stmt);

      if (stmt_list != AST_NIL)
      {
        rc = walk_statements_reverse(stmt_list,
                                     (is_do(stmt) || is_parallelloop(stmt) ?
                                      nesting_level + 1 : nesting_level),
                                     pre_action, post_action, parm);
      }

      if (rc == WALK_CONTINUE && post_action)
        rc = (*post_action) (stmt, nesting_level, parm);
    }
  }
  return (rc);
}



/*----------------------------------------------------------------

  walk_statements_reverseV()   Walk over stmts in an AST in reverse and allow
                              variable argument lists "va_list" parameters

*/

static int
walk_statements_reverseV(AST_INDEX stmt, int nesting_level, WK_STMT_CLBACK_V pre_action, 
			 WK_STMT_CLBACK_V post_action, va_list parm)
{
  AST_INDEX curr, old_prev, old_next, stmt_list;
  int rc;

  rc = WALK_CONTINUE;
  if (is_list(stmt))
  {
    curr = list_last(stmt);
    while (curr != AST_NIL)
    {
      old_prev = list_prev(curr);
      old_next = list_next(curr);
      rc = walk_statements_reverseV(curr, nesting_level,
				    pre_action, post_action, parm);

      switch (rc)
      {
        case WALK_CONTINUE:
        case WALK_SKIP_CHILDREN:
          curr = old_prev;
          break;

        case WALK_ABORT:
          return (WALK_ABORT);

        case WALK_FROM_OLD_PREV:
          curr = old_prev;
          break;

        case WALK_FROM_OLD_NEXT:
          if (old_next == AST_NIL)
            curr = old_prev;
          else
            curr = list_prev(old_next);
          break;
      }
    }
    return (WALK_CONTINUE);
  }
  else
  {
    if (pre_action)
      rc = (*pre_action) (stmt, nesting_level, parm);

    if (rc == WALK_CONTINUE)
    {
      stmt_list = gen_get_stmt_list(stmt);

      if (stmt_list != AST_NIL)
      {
        rc = walk_statements_reverseV(stmt_list,
				      (is_do(stmt) || is_parallelloop(stmt) ?
                                      nesting_level + 1 : nesting_level),
				      pre_action, post_action, parm);
      }

      if (rc == WALK_CONTINUE && post_action)
        rc = (*post_action) (stmt, nesting_level, parm);
    }
  }
  return (rc);
}


int walk_statements_reverse_v (AST_INDEX stmt, int nesting_level, WK_STMT_CLBACK_V
                               pre_action, WK_STMT_CLBACK_V post_action, ...)
{
  int      returnVal = 0;
  va_list  args;  

  va_start(args, post_action);

  returnVal =  walk_statements_reverseV(stmt, nesting_level, pre_action, 
					post_action, args);

  va_end(args);

  return returnVal;
}

/*----------------------------------------------------------------


/*----------------------------------------------------------------

  get_expressions()    Gets expressions in an AST node

 The following routine takes an individual statement (not a statement
 list) and returns the AST's of the individual expression components
 of the statement. Statements will have one or two expression
 components. AST_NIL will be returned for non existent expressions.
 If an unknown statement is sent to the routine UNKNOWN_STATEMENT
 will be returned, otherwise VALID_STATEMENT.

*/

int
get_expressions(AST_INDEX stmt, AST_INDEX *expr1, AST_INDEX *expr2)
{
  AST_INDEX temp;

  if (stmt == AST_NIL)
    return UNKNOWN_STATEMENT;

  switch (gen_get_node_type(stmt))
  {
    case GEN_ASSIGNMENT:
      *expr1 = gen_ASSIGNMENT_get_lvalue(stmt);
      *expr2 = gen_ASSIGNMENT_get_rvalue(stmt);
      break;

    case GEN_IF:
      *expr1 = gen_IF_get_guard_LIST(stmt);
      *expr2 = AST_NIL;
      break;

    case GEN_LOGICAL_IF:
      *expr1 = gen_LOGICAL_IF_get_rvalue(stmt);
      *expr2 = AST_NIL;
      break;

    case GEN_ARITHMETIC_IF:
      *expr1 = gen_ARITHMETIC_IF_get_rvalue(stmt);
      *expr2 = AST_NIL;
      break;

    case GEN_DO:
    case GEN_DO_ALL:
    case GEN_PARALLELLOOP:
      temp = gen_get_loop_control(stmt);
      *expr1 = gen_INDUCTIVE_get_rvalue1(temp);
      *expr2 = gen_INDUCTIVE_get_rvalue2(temp);
      /* what about: gen_INDUCTIVE_get_rvalue3(temp);  */
      break;

    case GEN_CALL:
      temp = gen_CALL_get_invocation(stmt);
      *expr1 = gen_INVOCATION_get_name(temp);
      *expr2 = gen_INVOCATION_get_actual_arg_LIST(temp);
      break;

    case GEN_GUARD:
      *expr1 = gen_GUARD_get_rvalue(stmt);
      *expr2 = AST_NIL;
      break;

    case GEN_PRINT:		/* added, mpal:920127	*/
      *expr1 = gen_PRINT_get_data_vars_LIST(stmt);
      *expr2 = AST_NIL;
      break;
 
      /* These I/O cases are being tested: mpal, 910625    */
    case GEN_WRITE:
      *expr1 = gen_WRITE_get_data_vars_LIST(stmt);
      *expr2 = AST_NIL;
      break;

    case GEN_READ_SHORT:
      *expr1 = gen_READ_SHORT_get_data_vars_LIST(stmt);
      *expr2 = AST_NIL;
      break;

    case GEN_READ_LONG:
      *expr1 = gen_READ_LONG_get_io_LIST(stmt);
      *expr2 = AST_NIL;
      break;

    case GEN_WHERE_BLOCK:
      *expr1 = gen_WHERE_BLOCK_get_guard_LIST(stmt);
      *expr2 = AST_NIL;
      break;

    case GEN_WHERE:
      *expr1 = gen_WHERE_get_rvalue(stmt);
      *expr2 = AST_NIL;
      break;

    default:
      *expr1 = AST_NIL;
      *expr2 = AST_NIL;
      return UNKNOWN_STATEMENT;
  }

  return VALID_STATEMENT;
}


/*----------------------------------------------------------------

  walk_expression()   Walk over nodes of an expression AST

  robust in the presence of a post_action that performs a tree replace 
  of the current node; a pre_action that replaces the current node will
  cause undefined behavior  -- JMC 5/93

*/

int
walk_expression(AST_INDEX expr, WK_EXPR_CLBACK pre_action, 
                WK_EXPR_CLBACK post_action, Generic parm)
{
  int i, rc, n;
  AST_INDEX from, curr;
  
  rc = WALK_CONTINUE;
  
  if (is_list(expr)) {
    /* walking of list elements is robust in the presence of
     * a post_action that replaces the current node. to accomplish
     * this, the successor of the first element in the list
     * is calculated forward from the (possibly new) list head and
     * any other node's successor is calculated from its 
     * predecessor.  -- JMC 5/93
     */
    AST_INDEX next;
    
    curr = list_first(expr);
    if (curr != AST_NIL) {
      
      /*  This was changed from the previous version according to
       *  enable correct walking.  The following changes were made
       *  according to Steve Carr. 
       *  Changes were made under patton's uid.  She had the file
       *  checked out to correct it for prototypes.
       *    -- KLC 7/93 
       */
    while (curr != AST_NIL) 
      {
         next = list_next(curr);
         rc = walk_expression(curr, pre_action, post_action, parm);
         if (rc == WALK_ABORT) return WALK_ABORT;
         curr = next;
      }
    }
  } else {
    if (pre_action) {
      rc = (*pre_action) (expr, parm);
      if (rc == WALK_ABORT)
	return WALK_ABORT;
    }
    if (rc == WALK_CONTINUE) {
      n = gen_how_many_sons(gen_get_node_type(expr));
      for (i = 1; i <= n; i++) {
	from = ast_get_son_n(expr, i);
	if (from != AST_NIL) {
	  rc = walk_expression(from, pre_action, post_action, parm);
	  if (rc == WALK_ABORT)
	    return (WALK_ABORT);
	}
      }
      if (post_action) {
	rc = (*post_action) (expr, parm);
	if (rc == WALK_ABORT)
	  return (WALK_ABORT);
      }
    }
  }
  return (WALK_CONTINUE);
}


/*
 * walk all of the nodes in stmt, calling
 *
 *	void fn(id_node, accesstype,  ... )
 *
 * for each IDENTIFIER contained therein
 *
 * where:
 *	id_node		node num of the IDENTIFIER
 *      accesstype      type of access the id represents
 *
 */
void walkIDsInStmt (AST_INDEX stmt, WK_IDS_CLBACK_V fn, ...)
{
  va_list    args;
  AST_INDEX  ctl;

  va_start(args, fn);

  walkIDsInStmt_internal(stmt, fn, args);

  va_end(args);

  return;
}


/* walkIDsInStmt_internal
 *
 * NOTES:
 * (1) each statement in the statement list must be inspected in a
 *     separate call -- JMC 9/92
 */
static void walkIDsInStmt_internal(AST_INDEX s, WK_IDS_CLBACK_V fn, va_list args)
{
  if ( s != AST_NIL ) {
    if (is_subprogram_stmt(s)) {
      walkIDs(get_name_in_entry(s), at_decl, fn, args);
      walkIDs(get_formals_in_entry(s), at_decl, fn, args);
      /* see note (1) */
    } else if (is_specification_stmt(s)) {
      walkIDs(s, at_decl, fn, args);
    } else {	
      switch (gen_get_node_type(s)) {
	/* Source for docs is ANSI X3.9-1978	*/
      case GEN_ASSIGN:		/* ASSIGN stmt_label TO int_var_name	*/
	fn(gen_ASSIGN_get_name(s), at_mod, args);
	break;

      case GEN_ASSIGNMENT:	/* var = expression	*/
	walkIDs(gen_ASSIGNMENT_get_lvalue(s), at_mod,  fn, args);
	walkIDs(gen_ASSIGNMENT_get_rvalue(s), at_ref, fn, args);
	break;
	
	/*  case GEN_..._IF:		IF logical, arithmetic, block	*/
      case GEN_LOGICAL_IF:	/* IF (bool_exp) stmt	*/
	walkIDs(gen_LOGICAL_IF_get_rvalue(s), at_ref, fn, args);
	/* see note (1) */
	break;
	
      case GEN_ARITHMETIC_IF:	/* IF (int_real_exp) label1,label2,label3 */
	walkIDs(gen_ARITHMETIC_IF_get_rvalue(s), at_ref, fn, args);
	break;
	
      case GEN_IF:
	/* nothing to do here. nested guards handled as the guard statements
	 * are walked -- JMC 9/92 
	 */
	break;
	
      case GEN_DO:		/* DO s [,] i= exp1,exp2 [,exp3]	*/
      case GEN_PARALLELLOOP:	/* PARALLEL DO 	*/
      case GEN_DO_ALL:	{ /* DOALL 	*/
	AST_INDEX ctl = gen_get_loop_control(s);
	
	if ( gen_get_node_type(ctl) == GEN_INDUCTIVE ) {
	  walkIDs(gen_INDUCTIVE_get_rvalue1(ctl), at_ref, fn, args);
	  walkIDs(gen_INDUCTIVE_get_rvalue2(ctl), at_ref, fn, args);
	  walkIDs(gen_INDUCTIVE_get_rvalue3(ctl), at_ref, fn, args);
	} else if ( gen_get_node_type(ctl) == GEN_CONDITIONAL ) {
	  walkIDs(gen_CONDITIONAL_get_rvalue(ctl),at_ref, fn,args);
	} else if ( gen_get_node_type(ctl) == GEN_REPETITIVE ) {
	  walkIDs(gen_REPETITIVE_get_rvalue(ctl), at_ref, fn,args);
	}
      }
	break;
	
      case GEN_PRIVATE:		
	walkIDs(gen_PRIVATE_get_name_LIST(s), at_decl, fn, args);
	break;
	
      case GEN_CALL:		/* CALL sub [([arg [,arg]...])]	*/
	walkIDs(gen_CALL_get_invocation(s), at_noaccess, fn, args);
	break;
	
      case GEN_GUARD:             /* fragment of an if construct ... */
	walkIDs(gen_GUARD_get_rvalue(s), at_ref, fn, args);
	/* see note (1) */
	break;
	
      case GEN_PRINT:		/* PRINT f [,iolist]	*/
	walkIDs(gen_PRINT_get_data_vars_LIST(s), at_ref, fn, args);
	break;
	
      case GEN_WRITE:		/* WRITE (cilist) [iolist]	*/
	walkIDs(gen_WRITE_get_data_vars_LIST(s), at_ref, fn, args);
	break;
	
      case GEN_READ_SHORT:	/* 	*/
	walkIDs(gen_READ_SHORT_get_data_vars_LIST(s), at_mod, fn, args);
	break;
	
      case GEN_READ_LONG:		/* READ (cilist) | f    [iolist]	*/
	walkIDs(gen_READ_LONG_get_io_LIST(s), at_mod, fn, args);
	break;
	
      case GEN_COMPUTED_GOTO:	/* GO TO (s [,s]...) [,] i	*/
	walkIDs(gen_COMPUTED_GOTO_get_rvalue(s), at_ref, fn, args);
	break;
	
      case GEN_RETURN:		/* RETURN [e]	*/
	walkIDs(gen_RETURN_get_rvalue(s), at_ref, fn, args);
	break;
	

      case GEN_AT:		/* ???	*/
      case GEN_BACKSPACE_LONG:	/* BACKSPACE unit | (alist)	*/
      case GEN_BACKSPACE_SHORT:	/* 	*/
      case GEN_CLOSE:		/* CLOSE (cllist)	*/
      case GEN_DATA:		/* DATA nlist/clist/ [[,]nlist/clist/]... */
      case GEN_DEBUG:		/* 	*/
      case GEN_ENDFILE_LONG:	/* ENDFILE u	*/
      case GEN_ENDFILE_SHORT:	/* ENDFILE	*/
      case GEN_ENTRY:	/* ENTRY en [([d [,d]...])] ;d == var|array|proc_name*/
      case GEN_FORMAT:		/* FORMAT fs	*/
      case GEN_GLOBAL:	/* main prog, common blocks, subprog, externals	*/
      case GEN_GOTO:		/* GO TO s	*/
      case GEN_IMPLICIT:		/* IMPLICIT typ (a [,a]...) ...	*/
      case GEN_INQUIRE:		/* INQUIRE (iflist) | (iulist)	*/
      case GEN_INTRINSIC:		/* INTRINSIC fun [,fun]	*/
      case GEN_OPEN:		/* OPEN (olist)	*/
      case GEN_PARAMETER:		/* PARAMETER (p=e [,p=e] ...)	*/
      case GEN_PAUSE:		/* PAUSE [char_string | char_const]	*/
      case GEN_REWIND_LONG:	/* REWIND unit | (alist)	*/
      case GEN_REWIND_SHORT:	/* 	*/
      case GEN_SAVE:		/* SAVE [a [,a]...] ; a==saved_common_blk_name	*/
      case GEN_STOP:		/* STOP [char_string | char_const]	*/
	break;

	/* no handling needed -- JMC 9/92 */
      case GEN_COMMENT:		/* C or * in column 1, or blank in 1->72  */
      case GEN_CONTINUE:		/* CONTINUE	*/
	break;

	/* need missing special handling -- JMC 9/92 */
      case GEN_ASSIGNED_GOTO:	/* GO TO int_var_name [stmt_label_list]	*/
      case GEN_STMT_FUNCTION:	/* fun ([d [,d]...]) = e	*/
      case GEN_BLOCK_DATA:	/* BLOCK DATA [sub]	*/
	break;
	
      /**** begin Fortran 90 cases */
      case GEN_ALLOCATE:		/* ALLOCATE object_list */
	walkIDs(gen_ALLOCATE_get_object_LIST(s), at_mod, fn, args);
	break;
	
      case GEN_DEALLOCATE:		/* DEALLOCATE object_list */
	walkIDs(gen_DEALLOCATE_get_object_LIST(s), at_mod, fn, args);
	break;
	
      case GEN_WHERE:	/* WHERE (bool_exp) stmt	*/
	walkIDs(gen_WHERE_get_rvalue(s), at_ref, fn, args);
	/* see note (1) */
	break;
	
      case GEN_WHERE_BLOCK:	/* WHERE (bool_exp) stmts ELSEWHERE stmts ENDWHERE */
	/* nothing to do here. nested guards handled as the guard statements
	 * are walked  */
	break;
      /**** end Fortran 90 cases */
	
      default:
	walkIDs(s, at_invalid, fn, args);
	break;
      } 
    }
  }
}


static void walkIDs (AST_INDEX n, ReferenceAccessType atype, WK_IDS_CLBACK_V fn, 
                     va_list args)
{
  if ( n == AST_NIL ) return;
  
  switch(gen_get_node_type(n)) {
  case GEN_IDENTIFIER: /* base case */ 
    assert(atype != at_invalid);
    fn(n, atype, args);
    break;
  case GEN_SUBSCRIPT:
    walkIDs(gen_SUBSCRIPT_get_name(n), atype, fn, args);
    walkIDs(gen_SUBSCRIPT_get_rvalue_LIST(n), at_ref, fn, args);
    break;
  case GEN_INVOCATION: 
    {
      AST_INDEX name = gen_INVOCATION_get_name(n);
      ReferenceAccessType arg_atype = 
	(builtins_isBuiltinFunction(gen_get_text(name))? at_ref : at_noaccess);
      fn(name, at_invoc, args);
      walkIDs(gen_INVOCATION_get_actual_arg_LIST(n), arg_atype, fn, args);
    }	
    break;
  case GEN_GUARD:
    walkIDsInStmt_internal(n, fn, args);
    break;
  case GEN_LIST_OF_NODES:
    {
      /* visit each element of the list */
      AST_INDEX listel = list_first(n);
      while (listel != AST_NIL) {
	walkIDs(listel, atype, fn, args);
	listel = list_next(listel);
      }
    }
    break;
  case GEN_ARRAY_DECL_LEN:
    walkIDs(gen_ARRAY_DECL_LEN_get_name(n), at_decl, fn, args);
    walkIDs(gen_ARRAY_DECL_LEN_get_dim_LIST(n), at_decl_dim_ref, fn, args);
    break;
  case GEN_IMPLIED_DO:
    walkIDs(gen_IMPLIED_DO_get_imp_elt_LIST(n), atype, fn, args);
    walkIDs(gen_IMPLIED_DO_get_name(n), at_mod, fn, args);
    walkIDs(gen_IMPLIED_DO_get_rvalue1(n), at_ref, fn, args);
    walkIDs(gen_IMPLIED_DO_get_rvalue2(n), at_ref, fn, args);
    walkIDs(gen_IMPLIED_DO_get_rvalue3(n), at_ref, fn, args);
    break;
  case GEN_TRIPLET:
    walkIDs(gen_TRIPLET_get_lower(n), at_ref, fn, args);
    walkIDs(gen_TRIPLET_get_upper(n), at_ref, fn, args);
    walkIDs(gen_TRIPLET_get_step(n), at_ref, fn, args);
    break;
  default: 
    {
      /* for operators other than invocation and subscript which are
       * covered above, all operators cause all of their operands to
       * be referenced 
       */ 
      ReferenceAccessType son_atype;
      int nsons = gen_how_many_sons(gen_get_node_type(n));
      int i;
      if (atype != at_decl_dim_ref && is_operator(n))
	son_atype = at_ref;
      else son_atype = atype;
      for(i = 1; i <= nsons; i++) 
	walkIDs(ast_get_son_n(n, i), son_atype, fn, args);
    }
  }
}


/*
 * walk all of the nodes in expr, calling
 *
 *	void fn(id_node, ... )
 *
 * for each IDENTIFIER contained therein
 *
 * where:
 *	id_node		node num of the IDENTIFIER
 *
 */
void walkIDsInExpr (AST_INDEX expr, WK_IDS_EX_CLBACK_V fn, ...)
{
  va_list  args;

  va_start(args, fn);

  walkIDsEx(expr, fn, args);

  va_end(args);

  return;
}


static void walkIDsEx (AST_INDEX n, WK_IDS_EX_CLBACK_V fn, va_list args)
{
  int		 nodetype;
  int		 i;
  AST_INDEX	 son, expr, arg_list;

  if ( n == AST_NIL )
    return;

  nodetype = gen_get_node_type(n);

  switch ( nodetype ) {
  case GEN_IDENTIFIER:
    fn(n, args);
    break;
    
  case GEN_LIST_OF_NODES:	/* must visit each elt of list */
    expr = list_first(n);
    while ( expr != AST_NIL ) {
      walkIDsEx(expr, fn, args);
      expr = list_next(expr);
    }
    break;
    
  default:
    for( i = 1; i <= (int)gen_how_many_sons(gen_get_node_type(n)); i++ ) {
      son = ast_get_son_n(n, i);
      walkIDsEx(son, fn, args);
    }
    break;
  }
}


/*----------------------------------------------------------------

    ast_equiv()  Determine whether two AST are equivalent

*/

Boolean
ast_equiv(AST_INDEX left, AST_INDEX right)
{
  AST_INDEX rcurr, lcurr;
  int i, n; 
  Boolean rc = true;

  if (gen_get_node_type(left) != gen_get_node_type(right))
  {
    return false;
  }
  if (left == ast_null_node)
  {
    return true;
  }
  if (is_identifier(left) || is_constant(left))
  {
    if (ast_get_symbol(left) != ast_get_symbol(right) &&
        strcmp(gen_get_text(left), gen_get_text(right)))
    {
      return false;
    }
    return true;
  }
  if (is_list(left))
  {
    rcurr = list_first(left);
    lcurr = list_first(right);
    while (rcurr != ast_null_node)
    {
      if (!ast_equiv(rcurr, lcurr))
        return false;

      rcurr = list_next(rcurr);
      lcurr = list_next(lcurr);
    }
  }
  else
  {
    n = gen_how_many_sons(gen_get_node_type(left));
    for (i = 1; i <= n; i++)
    {
      rc = ast_equiv(ast_get_son_n(left, i), ast_get_son_n(right, i));
      if (!rc)
        return false;
    }
  }
  return true;
}

/*-----------------------------------------------------------------------

    ast_eval()  eval function for simple integer expressions

    Returns:    true        if symbolics/complex exprs found
                false       otherwise (constant expression)
*/

Boolean
ast_eval(AST_INDEX node, int *iptr)
{
  int val1, val2;  /* intermediate results */
  Boolean status;

  /*---------------------------------------------------*/
  /* unary minus */

  if (is_unary_minus(node))
  {
    if (status = ast_eval(gen_UNARY_MINUS_get_rvalue(node), &val1))
      return true;

    *iptr = -val1;
    return false;  /* found constant expression    */
  }

  /*---------------------------------------------------*/
  /* constants */

  if (is_constant(node))
  {
    *iptr = atoi(gen_get_text(node));
    return false;  /* found constant expression    */
  }

  /*---------------------------------------------------*/
  /* identifier */

  if (is_identifier(node))
  {
    return true;

#ifdef WAITING_FOR_PACO
    if (NOT(val_exp_is_const(node)))
      return true;

    *iptr = (int) val_exp_int(node);
    return false;  /* found constant expression    */
#endif

  }

  /*---------------------------------------------------*/
  /* plus */

  if (is_binary_plus(node))
  {
    if (status = ast_eval(gen_BINARY_PLUS_get_rvalue1(node), &val1))
      return true;

    if (status = ast_eval(gen_BINARY_PLUS_get_rvalue2(node), &val2))
      return true;

    *iptr = val1 + val2;
    return false;  /* found constant expression    */
  }

  /*---------------------------------------------------*/
  /* minus */

  if (is_binary_minus(node))
  {
    if (status = ast_eval(gen_BINARY_MINUS_get_rvalue1(node), &val1))
      return true;

    if (status = ast_eval(gen_BINARY_MINUS_get_rvalue2(node), &val2))
      return true;

    *iptr = val1 - val2;
    return false;  /* found constant expression    */
  }

  /*---------------------------------------------------*/
  /* multiply */

  if (is_binary_times(node))
  {
    if (status = ast_eval(gen_BINARY_TIMES_get_rvalue1(node), &val1))
      return true;

    if (status = ast_eval(gen_BINARY_TIMES_get_rvalue2(node), &val2))
      return true;

    *iptr = val1 * val2;
    return false;  /* found constant expression    */
  }

  /*---------------------------------------------------*/
  /* divide   */

  if (is_binary_divide(node))
  {
    if (status = ast_eval(gen_BINARY_DIVIDE_get_rvalue1(node), &val1))
      return true;

    if (status = ast_eval(gen_BINARY_DIVIDE_get_rvalue2(node), &val2))
      return true;

    if (!val2)
    {
      printf("ast_eval(): division by zero found\n");
      return true;
    }

    *iptr = val1 / val2;
    return false;  /* found constant expression    */

  }

  /*---------------------------------------------------*/
  /* not in proper normalized form    */

  /* 2/7/94 RvH: I guess this should be *true* instead: */
  /* return false; */
  return true;
}

int subtree_apply_when_pred(AST_INDEX node, int nesting_level, va_list arg_list)
{
  typedef FUNCTION_POINTER(Boolean, node_predicate, (AST_INDEX node));
  typedef FUNCTION_POINTER(int, subtree_fn, (AST_INDEX node, va_list alist));

  node_predicate pred;
  subtree_fn function;

  pred = va_arg(arg_list, node_predicate);
  function = va_arg(arg_list, subtree_fn);

  if (pred(node)) {
    function(node, arg_list);
    return WALK_SKIP_CHILDREN;
  } else {
    return WALK_CONTINUE;
  }
}

/*------------------------------------------------------------------------
    ast_out_if_subscript_id()  
      if node is the id of a subscript node, return the subscript node
      else return the node itself
 */

AST_INDEX ast_out_if_subscript_id(AST_INDEX node)
{
  if(is_subscript(node)) return node;
  else {
    AST_INDEX sub_node = tree_out(node);
    if ((sub_node != AST_NIL) && is_subscript(sub_node) && 
	(node == gen_SUBSCRIPT_get_name(sub_node)))
      return sub_node; 
    else return node;
  }
}


/*------------------------------------------------------------------------
    tree_out_to_enclosing_stmt(node)  
      Must be called on a node within a statement. Returns
      the AST_INDEX of the statement enclosing this node.
 */

AST_INDEX tree_out_to_enclosing_stmt(AST_INDEX node)
{
  while(node != AST_NIL && !is_statement(node))
    node = tree_out(node);

  return node; 
}
