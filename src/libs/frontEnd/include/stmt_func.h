/* $Id: stmt_func.h,v 1.3 1997/03/11 14:30:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef stmt_func_h
#define stmt_func_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef FortTree_h
#include <libs/frontEnd/fortTree/FortTree.h>
#endif

EXTERN(AST_INDEX, stmt_func_invocation_to_expression,
		(SymDescriptor d, AST_INDEX invocation_node));

EXTERN(AST_INDEX, expand_stmt_funcs,
		(SymDescriptor d, AST_INDEX invocation_node));

#endif
