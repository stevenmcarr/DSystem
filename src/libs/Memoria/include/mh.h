#ifndef mh_h
#define mh_h 

/*-------------------------------------------------------------

	mh.h

*/

#include <general.h>
#include <database.h>
#include <stdio.h>
#include <ctype.h>
#include <kb.h>
#include <point.h>
#include <rect.h>
#include <rect_list.h>
#include <gfx.h>
#include <font.h>
#include <rn_string.h>
#include <std.h>
#include <mem.h>
#include <ast.h>
#include <strutil.h>
#include <astutil.h>
#include <astlist.h>
#include <astcons.h>
#include <astnode.h>
#include <aphelper.h>
#include <astsel.h>
#include <asttree.h>
#include <astrec.h>
#include <FortTree.h>
#include <TextTree.h>
#include <FortTextTree.h>
#include <fortsym.h>
#include <groups.h>
#include <cd.h>
#include <dg.h>
#include <el.h>
#include <dp.h>
#include <dt.h>
#include <rsd.h>
#include <walk.h>
#include <mh_config.h>
#include <pt_util.h>
#include <malloc.h>
#include <Arena.h>
#include <header.h>

#define  MAX_LEVEL  20

typedef enum {COMPLEX,RECT,TRI_UL,TRI_LL,TRI_UR,TRI_LR,TRAP,RHOM,MULT} 
             loop_shape_type;

typedef enum {FN_MIN,FN_MAX,FN_BOTH} trap_fn_type;

typedef struct {
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
                  outer_stmts;
  float           rho,
                  ibalance,
                  fbalance;
  Boolean         stmts,
                  interchange,
                  transform,
                  distribute,
                  unroll,
                  reduction;
  loop_shape_type type;
  trap_fn_type    trap_fn;
  UtilList        *split_list;
  heap_type       *heap;
  int             *unroll_vector;
 };

typedef struct {
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
 } subscript_info_type;

typedef struct {
  int  stmt_num;
  int  loop_num;
  int  surrounding_do;
  AST_INDEX surround_node;
  int  level;
  Boolean pre_loop;
 } stmt_info_type;

typedef struct {
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

EXTERN_FUNCTION(void message,(char *msg));
#endif
