/* $Id: treeutil.h,v 1.25 1997/03/11 14:29:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef treeutil_h
#define treeutil_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

#ifndef rn_varargs_h
#include <include/rn_varargs.h>
#endif

/**************************************************************************
 *
 * external declarations for fort/treeutil.c                    
 **************************************************************************/ 


/*************************************************************************** 
 *  loop_level()  returns the loop nesting level
 **************************************************************************/ 
EXTERN(int, loop_level, (AST_INDEX stmt));


/*************************************************************************** 
 *  gen_get_stmt_list()  extract the list of statements (if any) from a node 
 **************************************************************************/ 
EXTERN(AST_INDEX, gen_get_stmt_list, (AST_INDEX stmt));


/*************************************************************************** 
 *  gen_get_loop_control()  extract the loop control from a loop node 
 **************************************************************************/ 
EXTERN(AST_INDEX, gen_get_loop_control, (AST_INDEX loop));


/*************************************************************************** 
 *  ast_equiv()  Determine whether two AST subtrees are equivalent
 **************************************************************************/ 
EXTERN(Boolean, ast_equiv, (AST_INDEX left, AST_INDEX right));

/*************************************************************************** 
 *  ast_out_if_subscript_id()  if node is the id of a subscript node, return
 *             the subscript node, else return the node itself
 **************************************************************************/ 
EXTERN(AST_INDEX, ast_out_if_subscript_id, (AST_INDEX node));

/*************************************************************************** 
 *  ast_eval()  eval function for simple integer expressions
 *
 *  Returns:    true    if symbolics/complex exprs found
 *              false   otherwise (constant expression)
 **************************************************************************/ 
EXTERN(Boolean, ast_eval, (AST_INDEX node, int *iptr));


/***************************************************************************
 *  constants and function declarations to use AST walk routines
 *
 *  Reformatted to use EXTERN and typed args.
 ***************************************************************************/

#define WALK_CONTINUE          1
#define WALK_SKIP_CHILDREN     2
#define WALK_ABORT             3                
#define WALK_FROM_OLD_PREV     4
#define WALK_FROM_OLD_NEXT     5

#define UNKNOWN_STATEMENT      0
#define VALID_STATEMENT        1

#define LEVEL1                 1
#define NOFUNC                 NULL


/******************************
 *  Call-back function types  *
 ******************************/

typedef FUNCTION_POINTER(int, WK_STMT_CLBACK,
                         (AST_INDEX stmt, int nesting_level, Generic parm));

typedef FUNCTION_POINTER(int, WK_STMT_CLBACK_V,
                         (AST_INDEX stmt, int nesting_level, va_list arg_list));

typedef FUNCTION_POINTER(int, WK_EXPR_CLBACK,
                         (AST_INDEX expr, Generic parm));

typedef enum {
  at_invalid, 
  at_decl, 
  at_invoc, 
  at_noaccess, 
  at_decl_dim_ref, 
  at_ref, 
  at_mod
} ReferenceAccessType;

typedef FUNCTION_POINTER(void, WK_IDS_CLBACK_V, 
			 (AST_INDEX stmt, ReferenceAccessType atype, 
			  va_list arg_list));

typedef FUNCTION_POINTER(void, WK_IDS_EX_CLBACK_V,
                         (AST_INDEX stmt, ...));

/******************
 * function decls *
 ******************/


/****************************************************************************
 *  walk_statements
 *
 *  takes a statement and recursively walks the tree,
 *  calling pre-action before the walk and post_action as soon as an
 *  individual statement is encountered.  Pre_action and Post_action should
 *  return one of the WALK instructions defined above.  Parm is the
 *  parameter structure which will be passed to pre_action and
 *  post_action.
 *****************************************************************************/
EXTERN(int, walk_statements,
		(AST_INDEX stmt,
		 int nesting_level,
		 WK_STMT_CLBACK pre_action,
		 WK_STMT_CLBACK post_action,
		 Generic parm)); 

EXTERN(int, walk_statements_v,
		(AST_INDEX stmt,
		 int nesting_level,
		 WK_STMT_CLBACK pre_action,
		 WK_STMT_CLBACK post_action,
		 ...)); 	/* the arguments to pass to pre_ and post_ */



/***************************************************
 *  walk_statements_reverse
 ***************************************************/
EXTERN(int, walk_statements_reverse,
		(AST_INDEX stmt,
		 int nesting_level,
		 WK_STMT_CLBACK pre_action,
		 WK_STMT_CLBACK post_action,
		 Generic parm)); 

EXTERN(int, walk_statements_reverse_v,
		(AST_INDEX stmt,
		 int nesting_level,
		 WK_STMT_CLBACK pre_action,
		 WK_STMT_CLBACK post_action,
		 ...));	/* the arguments to pass to pre_ and post_ */


/*****************************************************************************
 *  walkIDsInStmt
 *
 * walk all of the nodes in stmt, calling
 *
 *	void fn(id_node, atype, ... )
 *
 * for each IDENTIFIER contained therein
 *
 * where:
 *	id_node		node num of the IDENTIFIER
 *	atype	        type of access for this reference (noaccess for actuals
 *                      passed to procedure)
 *****************************************************************************/

EXTERN(void, walkIDsInStmt, (AST_INDEX stmt, WK_IDS_CLBACK_V fn, ...));	


/*****************************************************************************
 *  walkIDsInExpr
 *
 * walk all of the nodes in expr, calling
 *
 *	void fn(id_node, ... )
 *
 * for each IDENTIFIER contained therein
 *
 * where:
 *	id_node		node num of the IDENTIFIER encountered
 *****************************************************************************/

EXTERN(void, walkIDsInExpr, (AST_INDEX  expr, WK_IDS_EX_CLBACK_V fn, ...));


/*****************************************************************************
 *  get_expressions
 *
 *  The following routine takes an individual statement (not a statement
 *  list) and returns the AST of the individual expression components of
 *  the statement. Statements will have one or two expression components.
 *  AST_NIL will be returned for non existent expressions. If an invalid
 *  statement is sent to the routine INVALID_STATEMENT will be returned,
 *  otherwise VALID_STATEMENT.
 *****************************************************************************/
EXTERN(int, get_expressions, 
       (AST_INDEX stmt, AST_INDEX *expr1, AST_INDEX *expr2));



/*****************************************************************************
 *  walk_expression
 *
 *  recursively walks an expression tree calling pre_action
 *  before the expression is walked and calling post_action after.
 *  Pre_action and Post_action should return one of the WALK instructions
 *  defined above. Parm is the parameter structure which will be passed to
 *  pre_action and post_action.
 *****************************************************************************/
EXTERN(int, walk_expression, 
		(AST_INDEX expr,
		 WK_EXPR_CLBACK pre_action,
		 WK_EXPR_CLBACK post_action,
		 Generic parm)); 


/*****************************************************************************
 *  subtree_apply_when_pred
 *
 *  a function meant to be executed by walk_statements_v. the arguments
 *  are as follows
 *    AST_INDEX node -- the root of a subtree
 *    int nesting_level -- standard walker arg (ignored)
 *  and in the var_args list ...
 *    Boolean (*pred)(AST_INDEX node) -- a predicate on the subtree root
 *    void (*fn)(AST_INDEX node, va_list args) -- a function 
 *  for a node that satisfies the predicate, the provided function is applied
 *  and the children of the subtree root are skipped in the walk
 *****************************************************************************/

EXTERN(int, subtree_apply_when_pred, (AST_INDEX node, int nesting_level, 
				      va_list arg_list));

/*****************************************************************************
 *   tree_out_to_enclosing_stmt(AST_INDEX node)  
 *    Must be called on a node within a statement. Returns
 *    the AST_INDEX of the statement enclosing this node.
 *
 *****************************************************************************/

EXTERN(AST_INDEX, tree_out_to_enclosing_stmt, (AST_INDEX node));

#endif
