/* $Id: bound.h,v 1.4 1992/12/11 11:19:43 carr Exp $ */

#ifndef bound_h
#define bound_h

#ifndef general_h
#include <general.h>       /* for EXTERN */
#endif
#ifndef ast_h
#include <fort/ast.h>     /* for AST_INDEX */
#endif

EXTERN(void, ut_update_bounds,(AST_INDEX loop,AST_INDEX copy,
			       int val));
#endif
