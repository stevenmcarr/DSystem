#ifndef do_unroll_h
#define do_unroll_h

#include <Arena.h>

typedef struct {
  SymDescriptor symtab;
  int       level,
            val;
  PedInfo   ped;
 } dupd_info_type;

typedef struct {
  SymDescriptor symtab;
  char          *fieldn;
 } label_info_type;

typedef struct {
  PedInfo ped;
  int     level;
 } edge_info_type;

typedef struct {
  AST_INDEX prev,
            post;
 } rdx_stmts_type;

typedef struct {
  PedInfo ped;
  int     flops;
 } flop_info_type;

typedef struct {
  PedInfo ped;
  char    new_var[30];
  AST_INDEX array;
 } rep_info_type;

typedef struct {
  SymDescriptor symtab;
  int           level,
                surrounding_do,
                val;
 } ref_info_type;

#define RDX_VAR "mh: rdx_var"

EXTERN_FUNCTION(AST_INDEX mh_do_unroll_and_jam,(model_loop *loop_data,
						PedInfo ped,
						SymDescriptor symtab,
						int num_loops,
						arena_type *ar));
EXTERN_FUNCTION(int mh_copy_edges,(AST_INDEX node,
				   Generic ped));
#endif
