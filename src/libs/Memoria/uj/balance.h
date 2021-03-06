/* $Id: balance.h,v 1.11 1997/03/27 20:28:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef balance_h
#define balance_h

#include <libs/support/misc/general.h>
#include <libs/Memoria/uj/compute_uj.h>

EXTERN(int, mh_fp_register_pressure,(int reg_coeff[4][3][3],int *scalar_coeff,
				  int x1, int x2));
EXTERN(int, mh_addr_register_pressure,(int addr_coeff[4][3][3],int x1,int x2));
EXTERN(float, mh_loop_balance,(int mem_coeff[4][3][3],int flops,int x1,
			       int x2));
EXTERN(int, mh_memref, (int   mem_coeff[4][3][3], int x1, int x2));
EXTERN(float,mh_CacheBalance,(int mem_coeff[4][3][3],float PrefetchCoeff[4][3][3],
			      int flops,int x1, int x2,float MissCost,
			      PrefetchCoeffComponentType Comp[4][3][3],
			      int LineSize, float IM));
EXTERN(float, mh_PrefetchRequirements,(float     PrefetchCoeff[4][3][3],
				       int       flops,
				       int       x1,
				       int       x2,
				       PrefetchCoeffComponentType Comp[4][3][3],
				       int LineSize));

#define POSITIVE_PART(x) (x > 0 ? x : 0)
#endif
