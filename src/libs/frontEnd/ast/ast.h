/* $Id: ast.h,v 1.5 1997/03/11 14:29:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
	ast.h -- for users of the AST who need to AST_INDEX, but do not need
		to directly access the contents of the AST.

	Prerequisites:
		"general.h"
*/
#ifndef ast_h
#define ast_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

typedef unsigned int		AST_INDEX; 

/* Global null node index (0) */
extern AST_INDEX 		ast_null_node; 
# define AST_NIL            	ast_null_node

/* Pointer to the current asttab */
extern struct AsttabStruct	*asttab;
#endif
