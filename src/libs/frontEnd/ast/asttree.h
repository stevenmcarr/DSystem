/* $Id: asttree.h,v 1.9 1997/03/11 14:29:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef asttree_h
#define asttree_h

#include <stdio.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

EXTERN(AST_INDEX, tree_copy, (AST_INDEX node));
EXTERN(AST_INDEX, tree_copy_with_type, (AST_INDEX node));
EXTERN(void, tree_free, (AST_INDEX node));
EXTERN(void, tree_replace, (AST_INDEX oldnode, AST_INDEX newnode));
EXTERN(void, tree_replace_free, (AST_INDEX old_node, AST_INDEX new_node));
EXTERN(void, tree_print, (AST_INDEX node));
EXTERN(void, tree_print_w_sides, (AST_INDEX node));
EXTERN(AST_INDEX, tree_out, (AST_INDEX node));
EXTERN(void, tree_print_to_file, (AST_INDEX node, FILE *fp));

#endif
