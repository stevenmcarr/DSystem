/* $Id: sr.h,v 1.7 1992/12/11 11:20:13 carr Exp $ */

/*-------------------------------------------------------------

	scalar.h

*/

#ifndef sr_h
#define sr_h

#ifndef ast_h
#include <fort/ast.h>                /* for AST_INDEX */
#endif
#ifndef general_h
#include <general.h>                 /* for Boolean, EXTERN */
#endif
#ifndef list_h
#include <misc/list.h>               /* for UtilList */
#endif
#ifndef cgen_set_h
#include <cgen_set.h>                /* for Set */
#endif
#ifndef dp_h
#include <dp.h>                     /* for PedInfo */
#endif 
#ifndef block_h
#include <block.h>
#endif

typedef enum {BOGUS,LIAV,LIPAV,LCAV,LCPAV} GenTypes;

typedef struct scalarinfotype {
  Set               kill_set;
  int               generator,
                    array_num,
                    table_index,
                    surrounding_do,
                    def,
                    num_regs,
                    gen_distance;
  Boolean           is_generator,
                    constant,
                    is_consistent,
                    no_store,
                    visited,
                    recurrence,
                    prevent_rec,
                    scalar,
                    prevent_slr;
  GenTypes          gen_type;
  UtilNode          *list_index;
}  scalar_info_type;

typedef struct stmtinfotype {
  int           stmt_num;
  block_type    *block;
  Boolean       generated;
} stmt_info_type;

#define LOOP_ARENA 0

#define create_scalar_info_ptr(n,ar) \
  ast_put_scratch(n,(int) ar->arena_alloc_mem_clear(LOOP_ARENA,sizeof(scalar_info_type)))

#define get_scalar_info_ptr(n) \
  ((scalar_info_type *)ast_get_scratch(n))

#define create_NULL_stmt_info_ptr(n) \
  ast_put_scratch(n,(Generic)NULL)

#define create_stmt_info_ptr(n,ar) \
  ast_put_scratch(n,(int) ar->arena_alloc_mem_clear(LOOP_ARENA,sizeof(stmt_info_type)))

#define get_stmt_info_ptr(n) \
  ((stmt_info_type *)ast_get_scratch(n))

#define set_scratch_to_NULL(n)\
  ast_put_scratch(n,(Generic)NULL)

#define set_label_sym_index(n,i) \
  ast_put_scratch(n,i)

#define get_label_sym_index(n) \
  ((int)ast_get_scratch(n))

#define REFS          "sr: refs"
#define LBL_STMT      "sr: lbl_stmt"
#define NEW_LBL_INDEX "sr: new_lbl_index"
#define NEW_VAR       "sr: new_var"

#define LOG_UNROLL 1
#define LOG_SCALAR 2
#define LOG_ALL    3

EXTERN(void, message,(char *str));
#endif
