/* $Id: table.h,v 1.3 1992/12/11 11:22:50 carr Exp $ */
#ifndef table_h
#define table_h

#ifndef general_h
#include <general.h>
#endif

#ifndef ast_h
#include <ast.h>
#endif

EXTERN(int, sr_build_table,(AST_INDEX stmt,int level,
			    Generic prelim_info));
  
#endif
