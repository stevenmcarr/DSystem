/* $Id: balance.h,v 1.5 1993/07/20 16:34:59 carr Exp $ */

#ifndef balance_h
#define balance_h

#include <general.h>
#include <compute_uj.h>

EXTERN(int, mh_fp_register_pressure,(int reg_coeff[4][3][3],int *scalar_coeff,
				  int x1, int x2));
EXTERN(int, mh_addr_register_pressure,(int addr_coeff[4][3][3],int x1,int x2));
EXTERN(float, mh_loop_balance,(int mem_coeff[4][3][3],int flops,int x1,
			       int x2));
EXTERN(float,mh_loop_balance_cache,(int mem_coeff[4][3][3],CoeffType PrefetchCoeff,
				    int LineSize, int flops,int x1, int x2));

#endif
