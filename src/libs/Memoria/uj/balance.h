/* $Id: balance.h,v 1.4 1992/12/17 14:51:12 carr Exp $ */

#ifndef balance_h
#define balance_h

#include <general.h>

EXTERN(int, mh_fp_register_pressure,(int reg_coeff[4][3][3],int *scalar_coeff,
				  int x1, int x2));
EXTERN(int, mh_addr_register_pressure,(int addr_coeff[4][3][3],int x1,int x2));
EXTERN(float, mh_loop_balance,(int mem_coeff[4][3][3],int flops,int x1,
			       int x2));

#endif
