/* $Id: bound.h,v 1.6 1995/08/18 10:13:50 trsuchyt Exp $ */

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
