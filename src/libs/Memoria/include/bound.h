/* $Id: bound.h,v 1.5 1995/07/07 11:38:51 carr Exp $ */

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
EXTERN(void, ut_update_bounds_post,(AST_INDEX loop,AST_INDEX copy,
			       int val));
#endif
