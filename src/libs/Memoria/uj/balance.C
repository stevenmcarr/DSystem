/* $Id: balance.C,v 1.10 1996/02/19 13:55:07 carr Exp $ */

/****************************************************************************/
/*                                                                          */
/*    File:   balance.C                                                     */
/*                                                                          */
/*    Description:  Functions to compute the balance of a loop given a set  */
/*                  of unroll values.                                       */
/*                                                                          */
/****************************************************************************/

#include <balance.h>
#include <compute_uj.h>
#include <mem_util.h>


/****************************************************************************/
/*                                                                          */
/*    Function:     get_coeff_index                                         */
/*                                                                          */
/*    Input:        x - unroll amount                                       */
/*                                                                          */
/*    Description:  Determine which coefficient to use in computing balance */
/*                  distance 0, distance 1 or 0 or all distances            */
/*                                                                          */
/****************************************************************************/


static int get_coeff_index(int x)

  {
     switch (x-1)
       {
	case 0: 
	  return 0;
	case 1: 
          return 1;
	default:
	  return 2;
       }
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     mh_fp_register_pressure                                 */
/*                                                                          */
/*    Input:        reg_coeff - coefficients to the register pressure       */
/*                              pressure formula                            */
/*                  scalar_coeff - coefficients for scalar array refs       */
/*                  x1,x2 - unroll amounts for the two loops.               */
/*                                                                          */
/*    Description:  Determine the number of floating point registers        */
/*                  required by a loop, given the unroll amounts for two    */
/*                  loops.  This comes from Carr's thesis.                  */
/*                                                                          */
/****************************************************************************/


int mh_fp_register_pressure(int reg_coeff[4][3][3],
			    int *scalar_coeff,
			    int x1,
			    int x2)

  {
   int regs,cindex1,cindex2;
   
     cindex1 = get_coeff_index(x1);
     cindex2 = get_coeff_index(x2);
     regs = (reg_coeff[0][cindex1][cindex2] + scalar_coeff[0]) * x1 * x2 +
             reg_coeff[1][cindex1][cindex2] * x1 + 
	     reg_coeff[2][cindex1][cindex2] * x2 + 
	     reg_coeff[3][cindex1][cindex2];
     if (x1 != 1)
       regs += scalar_coeff[2] * x2;
     if (x2 != 1)
       regs += scalar_coeff[1] * x1;
     return(regs);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     mh_addr_register_pressure                               */
/*                                                                          */
/*    Input:        addr_coeff - coefficients for determining address       */
/*                               register pressure                          */
/*                  x1,x2 - unroll amounts for two loops                    */
/*                                                                          */
/*    Description:  Determine the address-register pressure in a loop given */
/*                  two unroll amounts.                                     */
/*                                                                          */
/****************************************************************************/


int mh_addr_register_pressure(int addr_coeff[4][3][3],
			      int x1,
			      int x2)

  {
   int regs,cindex1,cindex2;
   
     cindex1 = get_coeff_index(x1);
     cindex2 = get_coeff_index(x2);
     regs = addr_coeff[0][cindex1][cindex2] * x1 * x2 +
            addr_coeff[1][cindex1][cindex2] * x1 + 
	    addr_coeff[2][cindex1][cindex2] * x2 + 
	    addr_coeff[3][cindex1][cindex2];
     return(regs);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     mh_loop_balance                                         */
/*                                                                          */
/*    Input:        mem_coeff - coefficients for formula to compute the     */
/*                              number of memory references in a loop.      */
/*                  flops - number of floating-point ops in a loop.         */
/*                  x1,x2 - unroll amount for two loops.                    */
/*                                                                          */
/*    Description:  Determine the ratio of memory ops to flops in a loop    */
/*                  nest given unroll amounts for two loops.  This is from  */
/*                  Carr's thesis.                                          */
/*                                                                          */
/****************************************************************************/


float mh_loop_balance(int   mem_coeff[4][3][3],
		      int   flops,
		      int   x1,
		      int   x2)

  {
   float ML,FL;
   int cindex1,cindex2;

     cindex1 = get_coeff_index(x1);
     cindex2 = get_coeff_index(x2);
     ML = (float)(mem_coeff[0][cindex1][cindex2] * x1 * x2 + 
		   mem_coeff[1][cindex1][cindex2] * x1 + 
		   mem_coeff[2][cindex1][cindex2] * x2 + 
		   mem_coeff[3][cindex1][cindex2]);
     FL = (float)(flops * x1 * x2);
     if (FL == 0)
       return(0.0);
     else
       return(ML/FL);
  }

/* Return How Many Memory References in the loop */

int mh_memref(int   mem_coeff[4][3][3],
                      int   x1,
                      int   x2)

  {
   int cindex1,cindex2;
   int num_memref;

     cindex1 = get_coeff_index(x1);
     cindex2 = get_coeff_index(x2);
     num_memref = (mem_coeff[0][cindex1][cindex2] * x1 * x2 +
                   mem_coeff[1][cindex1][cindex2] * x1 +
                   mem_coeff[2][cindex1][cindex2] * x2 +
                   mem_coeff[3][cindex1][cindex2]);
       return(num_memref);
  }


float mh_PrefetchRequirements(float     PrefetchCoeff[4][3][3],
			      int       flops,
			      int       x1,
			      int       x2,
			      PrefetchCoeffComponentType Comp[4][3][3],
			      int       LineSize)
  {
   int cindex1,cindex2;

     cindex1 = get_coeff_index(x1);
     cindex2 = get_coeff_index(x2);

     return(x1*x2*(Comp[0][cindex1][cindex2].unit + 
		   Comp[0][cindex1][cindex2].fraction/(float)LineSize) +
	    ceil_ab(x1*x2,LineSize) * (Comp[0][cindex1][cindex2].ceil_fraction +
				       Comp[0][cindex1][cindex2].ceil_min_fraction_x) +
	    
	    x1*(Comp[1][cindex1][cindex2].unit + 
		Comp[1][cindex1][cindex2].fraction/(float)LineSize +
		Comp[1][cindex1][cindex2].ceil_min_fraction_d) +
	    ceil_ab(x1,LineSize) * (Comp[1][cindex1][cindex2].ceil_fraction +
				    Comp[1][cindex1][cindex2].ceil_min_fraction_x) +

	    x2*(Comp[2][cindex1][cindex2].unit + 
		Comp[2][cindex1][cindex2].fraction/(float)LineSize +
		Comp[2][cindex1][cindex2].ceil_min_fraction_d) +
	    ceil_ab(x2,LineSize) * (Comp[2][cindex1][cindex2].ceil_fraction +
				    Comp[2][cindex1][cindex2].ceil_min_fraction_x) +

	    Comp[3][cindex1][cindex2].ceil_fraction + 
	    Comp[3][cindex1][cindex2].unit + 
	    Comp[3][cindex1][cindex2].fraction/(float)LineSize +
	    Comp[3][cindex1][cindex2].ceil_min_fraction_d);

  }

    
float mh_CacheBalance(int       mem_coeff[4][3][3],
		      float     PrefetchCoeff[4][3][3],
		      int       flops,
		      int       x1,
		      int       x2,
		      float       MissCost,
		      PrefetchCoeffComponentType Comp[4][3][3],
		      int       LineSize,
		      float     IM)

  {
   float PL,FL,MemBal,LL;

     MemBal = mh_loop_balance(mem_coeff,flops,x1,x2);
     PL = mh_PrefetchRequirements(PrefetchCoeff,flops,x1,x2,Comp,LineSize);
     FL = (float)(flops * x1 * x2);
     LL = MemBal >= 1.0 ? MemBal*FL : FL;
     if (FL == 0)
       return(0.0);
     else
       return((POSITIVE_PART(PL - IM*LL)*MissCost)/FL + MemBal);
  }
     
