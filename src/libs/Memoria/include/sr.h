/* $Id: sr.h,v 1.5 1992/11/20 13:51:19 joel Exp $ */
/*-------------------------------------------------------------

	scalar.h

*/

#ifndef sr_h
#define sr_h

#include <general.h>
#include <newdatabase.h>
#include <stdio.h>
#include <ctype.h>
#include <misc/kb.h>
#include <misc/point.h>
#include <misc/rect.h>
#include <misc/rect_list.h>
#include <mon/gfx.h>
#include <mon/font.h>
#include <misc/rn_string.h>
#include <misc/mem.h>
#include <std.h>
#include <fort/ast.h>
#include <fort/strutil.h>
#include <fort/astutil.h>
#include <fort/astlist.h>
#include <fort/astcons.h>
#include <fort/astnode.h>
#include <fort/aphelper.h>
#include <fort/astsel.h>
#include <fort/asttree.h>
#include <fort/astrec.h>
#include <fort/FortTree.h>
#include <fort/TextTree.h>
#include <fort/FortTextTree.h>
#include <fort/fortsym.h>
#include <fort/groups.h>
#include <cd.h>
#include <dp.h>
#include <cfg.h>
#include <dg.h>
#include <el.h>
#include <dt.h>
#include <dep/rsd.h>
#include <misc/list.h>
#include <fort/walk.h>
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
