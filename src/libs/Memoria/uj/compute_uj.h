/* $Id: compute_uj.h,v 1.4 1992/12/07 10:23:03 carr Exp $ */

#ifndef compute_uj_h
#define compute_uj_h

typedef struct exprinfotype {
  Boolean found;
  AST_INDEX node;
 } expr_info_type;

typedef struct compinfotype {
  int     loop_stack[MAX_LEVEL];
  int     **count;
  int     num_loops;
  int     level;
  PedInfo ped;
  SymDescriptor symtab;
 } comp_info_type;

typedef struct depinfotype {
  int      reg_coeff[4],
           mem_coeff[4],
           addr_coeff[4],
           scalar_coeff[3],
           step[3],
           scalar_regs,
           flops,
           x1,x2,
           level1,
           level2,
           inner_level;
  char     *index[3];
  UtilList *vector_list,
           *partition;
  SymDescriptor symtab;
  PedInfo  ped;
  arena_type *ar;
 } dep_info_type;

typedef struct reginfotype {
  config_type *config;
  int         expr_regs;
  SymDescriptor symtab;
 } reg_info_type;

typedef struct vectorinfotype {
  AST_INDEX src;
  int       vector[MAXLOOP];
 } vector_info_type;


#define get_vec_DIS(vec,lvl)		(vec[(lvl)-1])
#define put_vec_DIS(vec,lvl,d)		(vec[(lvl)-1] = d)

#define put_label(n,v) \
   ast_put_scratch(n,v)

#define get_label(n) \
  (int)ast_get_scratch(n)

#define FIRST  "mh: first"

EXTERN(int, mh_increase_unroll,(int max,int denom,float rhoL_lp));
EXTERN(void, mh_compute_unroll_amounts,(model_loop *loop_data,
					       int size,int num_loops,
					       PedInfo ped,
					       SymDescriptor symtab,
					       arena_type    *ar));
#endif
