/* $Id: astlist.h,v 1.6 1997/03/11 14:29:22 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef astlist_h
#define astlist_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

/* VISIBLE */
EXTERN(AST_INDEX, list_create, (AST_INDEX node));
EXTERN(AST_INDEX, list_append, (AST_INDEX list1, AST_INDEX list2));
EXTERN(Boolean, is_list, (AST_INDEX node));
EXTERN(Boolean, in_list, (AST_INDEX node));
EXTERN(Boolean, is_aux, (AST_INDEX node));
EXTERN(Boolean, list_empty, (AST_INDEX list));
EXTERN(Generic, list_length, (AST_INDEX list));
EXTERN(AST_INDEX, list_head, (AST_INDEX node));
EXTERN(AST_INDEX, list_first, (AST_INDEX list));
EXTERN(AST_INDEX, list_last, (AST_INDEX list));
EXTERN(AST_INDEX, list_next, (AST_INDEX node));
EXTERN(AST_INDEX, list_prev, (AST_INDEX node));
EXTERN(AST_INDEX, list_retrieve, (AST_INDEX list, Generic element_number));
EXTERN(int, list_element, (AST_INDEX element));
EXTERN(AST_INDEX, list_get_aux, (AST_INDEX node));
EXTERN(AST_INDEX, list_insert_before, (AST_INDEX node, AST_INDEX element));
EXTERN(AST_INDEX, list_insert_after, (AST_INDEX node, AST_INDEX element));
EXTERN(AST_INDEX, list_insert_first, (AST_INDEX list, AST_INDEX element));
EXTERN(AST_INDEX, list_insert_last, (AST_INDEX list, AST_INDEX element));
EXTERN(AST_INDEX, list_remove_first, (AST_INDEX list));
EXTERN(AST_INDEX, list_remove_last, (AST_INDEX list));
EXTERN(AST_INDEX, list_remove_node, (AST_INDEX node));
EXTERN(AST_INDEX, list_remove, (AST_INDEX list, Generic element_number));

#endif
