/* $Id: mark.h,v 1.4 1992/12/11 11:19:48 carr Exp $ */

#ifndef mark_h
#define mark_h

#ifndef general_h
#include <general.h>       /* for Generic */
#endif
#ifndef ast_h
#include <fort/ast.h>      /* for AST_INDEX */
#endif

EXTERN(int, ut_mark_do_pre,(AST_INDEX stmt,int level,
				    Generic pre_info));
EXTERN(int, ut_mark_do_post,(AST_INDEX stmt,int level,
				     Generic pre_info));
#endif
