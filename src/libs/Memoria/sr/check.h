#ifndef check_h
#define check_h

typedef struct {
  int     size;
  Set     LC_kill;
  PedInfo ped;
  int     level;
  arena_type *ar;
 } check_info_type;

EXTERN_FUNCTION(void sr_check_inconsistent_edges,(AST_INDEX root,
					  check_info_type *check_info));
#endif
