/* $Id: table.h,v 1.4 1997/03/27 20:27:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef table_h
#define table_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

EXTERN(int, sr_build_table,(AST_INDEX stmt,int level,
			    Generic prelim_info));
  
#endif
