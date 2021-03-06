/* $Id: compute_uj.h,v 1.14 2001/10/12 19:22:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef compute_uj_h
#define compute_uj_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif

#ifndef fortsym_h
#include <libs/frontEnd/fortTree/fortsym.h>
#endif

#ifndef Arena_h
#include <libs/support/memMgmt/Arena.h>
#endif

#ifndef list_h
#include <libs/support/lists/list.h>
#endif

#ifndef mh_config_h
#include <libs/Memoria/include/mh_config.h>
#endif

#ifndef mh_h
#include <libs/Memoria/include/mh.h>
#endif

typedef struct exprinfotype {
  Boolean found;
  AST_INDEX node;
 } expr_info_type;

typedef struct compinfotype {
  model_loop *loop_data;
  int     loop_stack[MAX_LEVEL];
  int     **count;
  int     num_loops;
  int     level;
  PedInfo ped;
  SymDescriptor symtab;
 } comp_info_type;

typedef struct prefectcomponent {
  float  unit,
         ceil_fraction,
         ceil_min_fraction_x,
         ceil_min_fraction_d,
         fraction;
 } PrefetchCoeffComponentType;

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
  float    PrefetchCoeff[4][3][3];
  PrefetchCoeffComponentType PrefetchComponent[4][3][3];
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

#ifdef SOLARIS
#undef S_NONE
#endif

typedef enum {S_NONE,SELF,SELF1,SELF2,GROUP} SpatialLocalityType;
typedef enum {UNIT,CEIL_FRACTION,FRACTION,CEIL_MIN_FRACTION_X,
	     CEIL_MIN_FRACTION_D} ComponentType;

#define get_vec_DIS(vec,lvl)		(vec[(lvl)-1])
#define put_vec_DIS(vec,lvl,d)		(vec[(lvl)-1] = d)

#define put_label(n,v) \
   ast_put_scratch(n,v)

#define get_label(n) \
  (int)ast_get_scratch(n)

#define FIRST  "mh: first"

EXTERN(int, mh_increase_unroll,(int max,int denom,float rhoL_lp,dep_info_type *dep_info));
EXTERN(void, mh_compute_unroll_amounts,(model_loop *loop_data,
					int size,int num_loops,
					PedInfo ped,
					SymDescriptor symtab,
					arena_type    *ar));

extern dep_info_type *machine_info;

#endif
