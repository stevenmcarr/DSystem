/* $Id: compute_uj.h,v 1.7 1993/06/21 13:48:51 carr Exp $ */

#ifndef compute_uj_h
#define compute_uj_h

#ifndef general_h
#include <general.h>
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

#ifndef list_h
#include <misc/list.h>
#endif

#ifndef mh_config_h
#include <mh_config.h>
#endif

#ifndef mh_h
#include <mh.h>
#endif

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

typedef struct CoeffTypeTag {
  int V0[4][3][3],
      VC[4][4][3][3],
      VI[4][4][3][3];
 } CoeffType;

typedef struct depinfotype {
  int      reg_coeff[4][3][3],
           mem_coeff[4][3][3],
           addr_coeff[4][3][3],
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
  model_loop *loop_data;
  CoeffType  PrefetchCoeff;
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

typedef enum {S_NONE,SELF,SELF1,SELF2,GROUP} SpatialLocalityType;

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
