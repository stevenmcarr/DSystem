/* $Id: mem_util.h,v 1.14 1995/08/24 14:25:53 yguan Exp $ */

#ifndef mem_util_h
#define mem_util_h

#ifndef general_h
#include <general.h>         /* for Generic, EXTERN */
#endif

#ifndef Arena_h
#include <misc/Arena.h>            /* for arena_type */
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

#ifndef dg_h
#include <dg.h>
#endif

#ifndef dt_h
#include <dt.h>
#endif

#ifndef dp_h
#include <dp.h>
#endif

#include <UniformlyGeneratedSets.h>

typedef struct loop_struct model_loop;
typedef enum {UNDEFINED,NONE,SELF_TEMPORAL,GROUP_TEMPORAL,SELF_SPATIAL,
	      GROUP_SPATIAL,SELF_TEMPORAL_CACHE,GROUP_TEMPORAL_CACHE} LocalityType;

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

typedef struct StatsStruct {
  PedInfo ped;
  float   flops,
          mops;
  int     level;
  Boolean UseCache;
  model_loop *loop_data;
  int         loop;
  UniformlyGeneratedSets *UGS;
 } StatsInfoType;

typedef struct cycletype {
  int MemCycles;
  int FlopCycles;
  PedInfo ped;
 } CycleInfoType;

#define Head(l) (l.head)

#define Length1(l) (l.head == l.tail)

#define Mark_end(l) l.ptail = l.tail

#define Not_null(l) (l.tail != l.ptail)

#define Null(l) (l.tail == l.ptail)


EXTERN(AST_INDEX, ut_get_stmt,(AST_INDEX node));
EXTERN(AST_INDEX, ut_GetSubprogramStmtList,(AST_INDEX stmt));
EXTERN(AST_INDEX, ut_tree_copy_with_type,(AST_INDEX node,int index,
					arena_type *ar));

EXTERN(int, floor_ab,(int a, int b));
EXTERN(int, ceil_ab,(int a, int b));
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

EXTERN(int, ut_ComputeBalance,(AST_INDEX     node,
			       StatsInfoType *Stats));
EXTERN(LocalityType, ut_GetReferenceType, (AST_INDEX  node,
					   model_loop *loop_data,
					   int        loop,
					   PedInfo    ped,
					   UniformlyGeneratedSets *UGS));
EXTERN(void, ut_GetSubscriptText, (AST_INDEX Node, char *Text));

EXTERN(int,ut_change_logical_to_block_if, (AST_INDEX stmt,
					   int       level,
					   int       dummy));
EXTERN(int, ut_CyclesPerIteration, (AST_INDEX Node, PedInfo   ped));

EXTERN(int, ut_LoopSize, (AST_INDEX Node, PedInfo   ped));

#endif
