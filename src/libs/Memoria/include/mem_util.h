/* $Id: mem_util.h,v 1.5 1992/12/11 11:19:49 carr Exp $ */

#ifndef mem_util_h
#define mem_util_h

#ifndef general_h
#include <general.h>         /* for Generic, EXTERN */
#endif
#ifndef Arena_h
#include <Arena.h>            /* for arena_type */
#endif 
#ifndef ast_h
#include <fort/ast.h>         /* for AST_INDEX */
#endif
#ifndef fortsym_h
#include <fort/fortsym.h>     /* for SymDescriptor */
#endif
#ifndef context_h
#include <context.h> 
#endif
#ifndef el_h
#include <el.h>               /* for EDGE_INDEX */
#endif

typedef struct list_struct *listp;

struct list_struct {
  EDGE_INDEX edge;
  listp      prev,
             next;
 };

typedef struct list_node {
  listp head,
        tail,
        ptail;
 } listnode;

typedef struct copyinfotype {
  int        val;
  arena_type *ar;
  SymDescriptor symtab;
 } copy_info_type;

#define Head(l) (l.head)

#define Length1(l) (l.head == l.tail)

#define Mark_end(l) l.ptail = l.tail

#define Not_null(l) (l.tail != l.ptail)

#define Null(l) (l.tail == l.ptail)


EXTERN(AST_INDEX, ut_get_stmt,(AST_INDEX node));
EXTERN(AST_INDEX, ut_tree_copy_with_type,(AST_INDEX node,int index,
					arena_type *ar));
EXTERN(void, ut_new_tail,(listnode *list,EDGE_INDEX edge,
				  arena_type *ar));
EXTERN(void, ut_empty_new,(listnode *list));
EXTERN(listp, ut_get_start,(listnode list));
EXTERN(listp, ut_remove,(listp p,listnode *list));
EXTERN(void, ut_free_list,(listnode list));

EXTERN(int, floor_ab,(int a, int b));
EXTERN(int, mod,(int a,int b));
EXTERN(int, gcd,(int a,int b));
EXTERN(int, lcm,(int a,int b));
EXTERN(AST_INDEX, ut_gen_ident,(SymDescriptor symtab,
					char *name,int asttype));
EXTERN(int, ut_check_div,(AST_INDEX node,Generic contains_div));

EXTERN(void, ut_update_bounds,(AST_INDEX loop,AST_INDEX copy,
				       int val));
EXTERN(int, ut_init_copies,(AST_INDEX node,
				    Generic copy_info));
#endif
