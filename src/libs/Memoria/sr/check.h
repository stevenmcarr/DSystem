/* $Id: check.h,v 1.4 1992/12/11 11:22:32 carr Exp $ */

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
#include <Arena.h>
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
