/* $Id: expr.h,v 1.5 1997/03/11 14:29:58 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef Expr_Types
#define Expr_Types

#include <stdio.h>
#include <libs/frontEnd/ast/ast.h>
/*---------------------------------------------------------*/
/* type of expression: i.e. symbolic, constant, expression */
/* in the future once symbolic analysis is in place modify */
/* to include auxiliary induction variables                */
/*---------------------------------------------------------*/

 enum Expr_type
{
   Expr_simple_sym = 1,       /* bound contains symbolic (not index vars) */
   Expr_constant = 2,         /* is a constant   e.g. 10                  */
   Expr_linear = 3,           /* contains a linear expression  e.g. n+1   */
   Expr_invocation = 4,       /* contain a function e.g. mod(n-1, 10)     */
   Expr_linear_ivar_only = 5, /* linear expr containing only index vars   */
   Expr_index_var = 6,        /* an index var                             */
   Expr_complex = 7          /* previous classifications don't apply     */
};


/*----------------------------- structs ----------------------------*/

/*----------------------------------------------*/
/* general expression (for simple symbolic analysis) */

typedef struct expr
{
	enum Expr_type type;     /* type of the expression    */
	int val;            /* integer value if constant */ 
	AST_INDEX ast;      /* AST index if non-constant */
	char *str;          /* string if simple var      */
} Expr;

#endif
