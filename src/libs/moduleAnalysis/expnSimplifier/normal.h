/* $Id: normal.h,v 1.6 1997/03/11 14:36:06 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef	normal_h
#define	normal_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#include <libs/frontEnd/ast/ast.h>

/**********************************************/
/* This routine takes as an argument an       */
/* expression and transforms it into a sum    */
/* of products. The argument is an AST_INDEX  */
/* The routine is going to replace the passed */
/* node with the normalized node.             */
/**********************************************/


EXTERN(void, normalize_expr, (AST_INDEX passed_node));
EXTERN(int, walk_assign, (AST_INDEX stmt));
EXTERN(int, pre_distribute, (AST_INDEX expr, Generic parm));
EXTERN(int, post_distribute, (AST_INDEX passed_node, Generic parm));
EXTERN(AST_INDEX, normalize, (AST_INDEX passed_node));
EXTERN(AST_INDEX, push_star, (AST_INDEX passed_node));
EXTERN(AST_INDEX, distribute, (AST_INDEX passed_node, AST_INDEX distribute_expr,
                               AST_INDEX over_expr, int from_left));
EXTERN(AST_INDEX, left_lean, (AST_INDEX passed_node, AST_INDEX new_root));
EXTERN(AST_INDEX, push_minus, (AST_INDEX passed_node, int sign));
EXTERN(AST_INDEX, juggle_minus, (AST_INDEX passed_node));
EXTERN(void, replace_left_most_minus, (AST_INDEX node));
EXTERN(int, distributive, (AST_INDEX expr)); 


#endif
