/* $Id: check.h,v 1.6 1997/03/27 20:27:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef check_h
#define check_h

#ifndef cgen_set_h
#include <libs/Memoria/include/cgen_set.h>
#endif

#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif

#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

#ifndef Arena_h
#include <libs/support/memMgmt/Arena.h>
#endif

typedef struct checkinfotype {
  int     size;
  Set     LC_kill;
  PedInfo ped;
  int     level;
  arena_type *ar;
 } check_info_type;

EXTERN(void, sr_check_inconsistent_edges,(AST_INDEX root,
					  check_info_type *check_info));
#endif
