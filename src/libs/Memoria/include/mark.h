/* $Id: mark.h,v 1.3 1992/12/07 10:17:27 carr Exp $ */

#ifndef mark_h
#define mark_h

EXTERN(int, ut_mark_do_pre,(AST_INDEX stmt,int level,
				    Generic pre_info));
EXTERN(int, ut_mark_do_post,(AST_INDEX stmt,int level,
				     Generic pre_info));
#endif
