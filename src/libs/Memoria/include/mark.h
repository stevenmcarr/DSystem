#ifndef mark_h
#define mark_h

EXTERN_FUNCTION(int ut_mark_do_pre,(AST_INDEX stmt,int level,
				    Generic pre_info));
EXTERN_FUNCTION(int ut_mark_do_post,(AST_INDEX stmt,int level,
				     Generic pre_info));
#endif
