#ifndef mem_util_h
#define mem_util_h

#include "Arena.h"

typedef struct list_struct *listp;

struct list_struct {
  EDGE_INDEX edge;
  listp      prev,
             next;
 };

typedef struct {
  listp head,
        tail,
        ptail;
 } listnode;

typedef struct {
  int        val;
  arena_type *ar;
  SymDescriptor symtab;
 } copy_info_type;

#define Head(l) (l.head)

#define Length1(l) (l.head == l.tail)

#define Mark_end(l) l.ptail = l.tail

#define Not_null(l) (l.tail != l.ptail)

#define Null(l) (l.tail == l.ptail)


EXTERN_FUNCTION(AST_INDEX ut_get_stmt,(AST_INDEX node));
EXTERN_FUNCTION(AST_INDEX ut_tree_copy,(AST_INDEX node,int index,
					arena_type *ar));
EXTERN_FUNCTION(void ut_new_tail,(listnode *list,EDGE_INDEX edge,
				  arena_type *ar));
EXTERN_FUNCTION(void ut_empty_new,(listnode *list));
EXTERN_FUNCTION(listp ut_get_start,(listnode list));
EXTERN_FUNCTION(listp ut_remove,(listp p,listnode *list));
EXTERN_FUNCTION(void ut_free_list,(listnode list));

EXTERN_FUNCTION(int floor_ab,(int a, int b));
EXTERN_FUNCTION(int mod,(int a,int b));
EXTERN_FUNCTION(int gcd,(int a,int b));
EXTERN_FUNCTION(int lcm,(int a,int b));
EXTERN_FUNCTION(AST_INDEX ut_gen_ident,(SymDescriptor symtab,
					char *name,int asttype));
EXTERN_FUNCTION(int ut_check_div,(AST_INDEX node,Generic contains_div));

EXTERN_FUNCTION(void ut_update_bounds,(AST_INDEX loop,AST_INDEX copy,
				       int val));
EXTERN_FUNCTION(int ut_init_copies,(AST_INDEX node,
				    Generic copy_info));
#endif
