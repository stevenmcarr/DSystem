/* $Id: bound.h,v 1.8 1997/03/20 15:49:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef bound_h
#define bound_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif
#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

EXTERN(void, ut_update_bounds,(AST_INDEX loop,AST_INDEX copy,
			       int val));
EXTERN(void, ut_update_bounds_post,(AST_INDEX loop,AST_INDEX copy,
			       int val));
#endif
