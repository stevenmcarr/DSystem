/* $Id: do_unroll.h,v 1.7 1993/07/20 16:35:08 carr Exp $ */

#ifndef do_unroll_h
#define do_unroll_h

#ifndef general_h
#include <general.h>
#endif

#ifndef Arena_h
#include <misc/Arena.h>
#endif

#ifndef fortsym_h
#include <fort/fortsym.h>
#endif

#ifndef dp_h
#include <dp.h>
#endif

#ifndef ast_h
#include <ast.h>
#endif

#ifndef mh_h
#include <mh.h>
#endif

typedef struct dupdinfotype {
  SymDescriptor symtab;
  int       level,
            inner_level,
            val;
  PedInfo   ped;
  char      *ivar;
 } dupd_info_type;

typedef struct lableinfotype {
  SymDescriptor symtab;
  char          *fieldn;
 } label_info_type;

typedef struct edgeinfotype {
  PedInfo ped;
  int     level;
 } edge_info_type;

typedef struct rdxstmtstype {
  AST_INDEX prev,
            post;
 } rdx_stmts_type;

typedef struct flopinfotype {
  PedInfo ped;
  int     flops;
 } flop_info_type;

typedef struct repinfotype {
  PedInfo ped;
  char    new_var[30];
  AST_INDEX array;
 } rep_info_type;

typedef struct refinfotype {
  SymDescriptor symtab;
  int           level,
                surrounding_do,
                val;
  AST_INDEX     surround_node;
 } ref_info_type;

#define RDX_VAR "mh: rdx_var"

EXTERN(void, set_level_vectors, (AST_INDEX old, AST_INDEX newa, PedInfo ped));

EXTERN(void, mh_do_unroll_and_jam,(model_loop *loop_data,
				   PedInfo ped,
				   SymDescriptor symtab,
				   int num_loops,
				   arena_type *ar,
				   LoopStatsType *LoopStats));
EXTERN(int, mh_copy_edges,(AST_INDEX node,
			   Generic ped));
EXTERN(void, mh_replicate_body,(AST_INDEX     stmt_list,
				int           val,
				int           level,
				char          *ivar,
				AST_INDEX     step,
				PedInfo       ped,
				SymDescriptor symtab,
				Boolean       inner_rdx,
				int           surrounding_do,
				AST_INDEX     surround_node,
				char          *inner_ivar,
				int           inner_level,
				arena_type    *ar));
#endif
