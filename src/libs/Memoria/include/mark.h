/* $Id: mark.h,v 1.2 1992/10/03 15:51:03 rn Exp $ */
#ifndef mark_h
#define mark_h

EXTERN(int, ut_mark_do_pre,(AST_INDEX stmt,int level,
				    Generic pre_info));
EXTERN(int, ut_mark_do_post,(AST_INDEX stmt,int level,
				     Generic pre_info));
#endif
