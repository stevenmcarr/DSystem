/* $Id: do_unroll.h,v 1.4 1992/12/07 10:23:06 carr Exp $ */

#ifndef do_unroll_h
#define do_unroll_h

#include <Arena.h>

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

EXTERN(void, mh_do_unroll_and_jam,(model_loop *loop_data,
					   PedInfo ped,
					   SymDescriptor symtab,
					   int num_loops,
					   arena_type *ar));
EXTERN(int, mh_copy_edges,(AST_INDEX node,
				   Generic ped));
#endif
