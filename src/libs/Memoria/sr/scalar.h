/* $Id: scalar.h,v 1.7 1994/06/30 14:36:06 carr Exp $ */

#ifndef scalar_h
#define scalar_h

#include <LoopStats.h>

#ifndef general_h
#include <general.h>
#endif

#ifndef ast_h
#include <ast.h>
#endif

#ifndef dp_h
#include <dp.h>
#endif

#ifndef fortsym_h
#include <fort/fortsym.h>
#endif

#ifndef Arena_h
#include <misc/Arena.h>
#endif

#ifndef groups_h
#include <fort/groups.h>
#endif

#ifndef mh_config_h
#include <mh_config.h>
#endif

typedef struct arraytabletype {
  AST_INDEX   node;
  int         def,
              regs;
  float       profit;
  Boolean     partial;
 } array_table_type;

typedef struct preliminfotype {
  int              array_refs,
                   scalar_regs,
                   surrounding_do,
                   def_num,
                   level,
                   stmt_num;
  Boolean          contains_cf,
                   contains_goto_or_label,
		   premature_exit,
		   backjump,
                   illegal_jump,
                   jumps_ok;
  array_table_type *array_table;
  SymDescriptor    symtab;
  PedInfo          ped;
  arena_type       *ar;
}  prelim_info_type;

typedef struct doinfotype {
  int      inner_do,
           do_num;
  PedInfo  ped;
  Boolean  abort;
  SymDescriptor symtab;
  arena_type *ar;
  LoopStatsType *LoopStats;
 } do_info_type;

typedef struct reginfotype {
  config_type *config;
  int         expr_regs;
 } reg_info_type;

typedef struct balinfotype {
  int        mem,
             flops;
  PedInfo    ped;
 } bal_info_type;


#define put_label(n,v) \
   ast_put_scratch(n,v)

#define get_label(n) \
   (int)ast_get_scratch(n)

#define is_binary_op(n) \
  (is_binary_plus(n) || is_binary_minus(n) || is_binary_times(n) || \
   is_binary_divide(n) || is_binary_exponent(n))

#endif
