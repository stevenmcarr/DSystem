/* $Id: mh.h,v 1.26 1997/10/30 15:12:13 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef mh_h
#define mh_h 

/*-------------------------------------------------------------

	mh.h

*/

#ifndef general_h
#include <libs/support/misc/general.h>
#endif
#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif
#ifndef list_h
#include <libs/support/lists/list.h>
#endif
#ifndef cgen_set_h
#include <libs/Memoria/include/cgen_set.h>
#endif
#ifndef fortsym_h
#include <libs/frontEnd/fortTree/fortsym.h>
#endif
#ifndef FloatList_h
#include <libs/Memoria/include/FloatList.h>
#endif
#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif 
#ifndef groups_h
#include <libs/frontEnd/ast/groups.h>
#endif 
#ifndef header_h
#include <libs/Memoria/include/header.h>
#endif
#ifndef mem_util_h
#include <libs/Memoria/include/mem_util.h>
#endif
#include <libs/Memoria/include/UniformlyGeneratedSets.h>

extern Boolean mc_extended_cache;
extern int PartitionUnrollAmount;

#define  MAX_LEVEL  20

typedef enum {COMPLEX,RECT,TRI_UL,TRI_LL,TRI_UR,TRI_LR,TRAP,RHOM,MULT} 
             loop_shape_type;

typedef enum {FN_MIN,FN_MAX,FN_BOTH} trap_fn_type;

typedef struct heaptype {
  int index,
      stride;
 } heap_type;

typedef struct loop_struct model_loop;
struct loop_struct {
  AST_INDEX       node,
                  surround_node,
                  rhom_const,
                  tri_const;
  int             parent,
                  inner_loop,
                  next_loop,
                  tri_loop,
                  tri_coeff,
                  trap_loop,
                  level,
                  max,
                  count,
                  stride,
                  scalar_array_refs,
	          val,
                  registers,
                  inner_stmts,
                  outer_stmts,
                  CarriedDependences;
  float           rho,
                  L_L,
		  P_L,
                  initial_L_L,
                  initial_P_L,
                  ibalance,
                  fbalance;
  Boolean         stmts,
                  interchange,
                  DependencesHandled,
                  transform,
                  distribute,
                  expand,
                  unroll,
		  reversed,
                  reduction;
  loop_shape_type type;
  trap_fn_type    trap_fn;
  UtilList        *split_list;
  heap_type       *heap;
  int             *unroll_vector;
  int             MemoryOrder[MAX_LEVEL],
                  FinalOrder[MAX_LEVEL],
                  OutermostLvl,
                  DistributeNumber;
  FloatList       InvariantCostList,
                  SpatialCostList,
                  OtherSpatialCostList,
                  TemporalCostList,
                  NoneCostList;
  UtilList        *GroupList;
  Set             PreventLvl[MAX_LEVEL];
  Boolean         NoImprovement,
                  Distribute,
                  Interchange;
  Boolean         InterlockCausedUnroll;
  UniformlyGeneratedSets *UGS;
 };


typedef struct subscriptinfotype {
  int       surrounding_do;
  AST_INDEX surround_node;
  Boolean   is_scalar[3],
            prev_sclr[3],
            uses_regs,
            MIV,
            visited,
            eliminated;
  AST_INDEX *copies;
  AST_INDEX original;
  UtilNode  *lnode;
  Boolean   store;
  LocalityType Locality;
  int          GroupTemporalDistance;
  int          GroupSpatialDistance;
 } subscript_info_type;

typedef struct stmtinfotype {
  int  stmt_num;
  int  loop_num;
  int  surrounding_do;
  AST_INDEX surround_node;
  int  level;
  Boolean pre_loop;
 } stmt_info_type;

typedef struct preinfotype {
  int     stmt_num,
          loop_num,
          surrounding_do;
  AST_INDEX surround_node;
  Boolean abort;  
  PedInfo ped;
  SymDescriptor symtab;
  arena_type    *ar;
 } pre_info_type;

#define set_scratch_to_NULL(n) \
  ast_put_scratch(n,(Generic)NULL)

#define create_stmt_info_ptr(n,ar) \
  ast_put_scratch(n,(Generic)ar->arena_alloc_mem_clear(LOOP_ARENA,sizeof(stmt_info_type)))

#define get_stmt_info_ptr(n) \
  ((stmt_info_type *)ast_get_scratch(n))

#define create_subscript_ptr(n,ar) \
  ast_put_scratch(n,(Generic)ar->arena_alloc_mem_clear(LOOP_ARENA,sizeof(subscript_info_type)))

#define get_subscript_ptr(n) \
  ((subscript_info_type *)ast_get_scratch(n))

#define Self_loop(e) \
  (e.src == e.sink)

#define is_binary_op(n) \
  (is_binary_plus(n) || is_binary_minus(n) || is_binary_times(n) || \
   is_binary_divide(n) || is_binary_exponent(n))

#define set_label_sym_index(n,i) \
  ast_put_scratch(n,i)

#define get_label_sym_index(n) \
  ((int)ast_get_scratch(n))

#define NEW_VAR "sr: new_var"
#define EXPAND_LVL "mh: expand_lvl"

#define LOG_UNROLL 1
#define LOG_SCALAR 2
#define LOG_ALL    3

#define LOOP_ARENA 0
#define ARENAS     1

#endif
