/* $Id: balance.h,v 1.2 1992/10/03 15:49:47 rn Exp $ */
#ifndef balance_h
#define balance_h

#include <general.h>

EXTERN(int, mh_compute_x,(int regs,int int_regs,int *reg_coeff,
				  int *scalar_coeff,int *addr_coeff,int x,
				  int i));
EXTERN(int, mh_register_pressure,(int *reg_coeff,int *scalar_coeff,
					  int x1, int x2));
EXTERN(float, mh_loop_balance,(int *mem_coeff,int flops,int x1,
				       int x2));

#endif
