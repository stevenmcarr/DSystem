/* $Id: mark.h,v 1.5 1997/03/20 15:49:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef mark_h
#define mark_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif
#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

EXTERN(int, ut_mark_do_pre,(AST_INDEX stmt,int level,
				    Generic pre_info));
EXTERN(int, ut_mark_do_post,(AST_INDEX stmt,int level,
				     Generic pre_info));
#endif
