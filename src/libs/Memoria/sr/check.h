/* $Id: check.h,v 1.5 1993/06/21 13:46:47 carr Exp $ */

#ifndef check_h
#define check_h

#ifndef cgen_set_h
#include <cgen_set.h>
#endif

#ifndef dp_h
#include <dp.h>
#endif

#ifndef ast_h
#include <ast.h>
#endif

#ifndef Arena_h
#include <misc/Arena.h>
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
