/*-------------------------------------------------------------

	scalar.h

*/

#ifndef sr_h
#define sr_h

#include <general.h>
#include <database.h>
#include <stdio.h>
#include <ctype.h>
#include <kb.h>					/* keyboard constants and functions	*/
#include <point.h>				/* coordinate utilities			*/
#include <rect.h>
#include <rect_list.h>
#include <gfx.h>
#include <font.h>
#include <rn_string.h>
#include <mem.h>
#include <std.h>
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
#include <dp.h>
#include <cfg.h>
#include <dg.h>
#include <el.h>
#include <dt.h>
#include <rsd.h>
#include <list.h>
#include <walk.h>
#include <mh_config.h>
#include <pt_util.h>
#include <global.h>
#include <block.h>
#include <mem_util.h>
#include <header.h>

typedef enum {BOGUS,LIAV,LIPAV,LCAV,LCPAV} GenTypes;

typedef struct {
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

typedef struct {
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
  ast_put_scratch(n,NULL)

#define create_stmt_info_ptr(n,ar) \
  ast_put_scratch(n,(int) ar->arena_alloc_mem_clear(LOOP_ARENA,sizeof(stmt_info_type)))

#define get_stmt_info_ptr(n) \
  ((stmt_info_type *)ast_get_scratch(n))

#define set_scratch_to_NULL(n)\
  ast_put_scratch(n,NULL)

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

EXTERN_FUNCTION(void message,(char *str));
#endif
