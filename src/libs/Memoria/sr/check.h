/* $Id: check.h,v 1.3 1992/12/07 10:20:09 carr Exp $ */

#ifndef check_h
#define check_h

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
