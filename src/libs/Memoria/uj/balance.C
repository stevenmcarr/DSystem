/* $Id: balance.C,v 1.5 1994/01/18 14:26:23 carr Exp $ */

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
   float num,denom;
   int cindex1,cindex2;

     cindex1 = get_coeff_index(x1);
     cindex2 = get_coeff_index(x2);
     num = (float)(mem_coeff[0][cindex1][cindex2] * x1 * x2 + 
		   mem_coeff[1][cindex1][cindex2] * x1 + 
		   mem_coeff[2][cindex1][cindex2] * x2 + 
		   mem_coeff[3][cindex1][cindex2]);
     denom = (float)(flops * x1 * x2);
     if (denom == 0)
       return(0.0);
     else
       return(num/denom);
  }

    
float mh_loop_balance_cache(int       mem_coeff[4][3][3],
			    CoeffType PrefetchCoeff,
			    int       flops,
			    int       LineSize,
			    int       x1,
			    int       x2)

  {
   float P0,PC,PI,denom;
   int cindex1,cindex2;

     cindex1 = get_coeff_index(x1);
     cindex2 = get_coeff_index(x2);

     P0 = ((float)PrefetchCoeff.V0[0][cindex1][cindex2] * x1 * x2) / (float)LineSize +
          (float)PrefetchCoeff.V0[1][cindex1][cindex2] * x2 * ceil_ab(x1,LineSize) +
	  (float)PrefetchCoeff.V0[2][cindex1][cindex2] * x1 * ceil_ab(x2,LineSize) +
	  (float)PrefetchCoeff.V0[3][cindex1][cindex2] * x1 * x2;
     PC = ((float)(PrefetchCoeff.VC[0][0][cindex1][cindex2] * x1 * x2 + 
		   PrefetchCoeff.VC[0][1][cindex1][cindex2] * x1 + 
		   PrefetchCoeff.VC[0][2][cindex1][cindex2] * x2 + 
		   PrefetchCoeff.VC[0][3][cindex1][cindex2]) / LineSize) +
          (float)(PrefetchCoeff.VC[1][0][cindex1][cindex2] * x2 * ceil_ab(x1,LineSize) +
		  PrefetchCoeff.VC[1][1][cindex1][cindex2] * ceil_ab(x1,LineSize) +
		  ceil_ab(PrefetchCoeff.VC[1][3][cindex1][cindex2],LineSize)) +
          (float)(PrefetchCoeff.VC[2][0][cindex1][cindex2] * x1 * ceil_ab(x2,LineSize) +
		  PrefetchCoeff.VC[2][2][cindex1][cindex2] * ceil_ab(x2,LineSize) +
		  ceil_ab(PrefetchCoeff.VC[1][3][cindex1][cindex2],LineSize)) +
	  (float)(PrefetchCoeff.VC[3][0][cindex1][cindex2] * x1 * x2 + 
		  PrefetchCoeff.VC[3][1][cindex1][cindex2] * x1 + 
		  PrefetchCoeff.VC[3][2][cindex1][cindex2] * x2 + 
		  PrefetchCoeff.VC[3][3][cindex1][cindex2]);
     PI = ((float)(PrefetchCoeff.VI[0][1][cindex1][cindex2] * x1 +
		   PrefetchCoeff.VI[0][2][cindex1][cindex2] * x2 +
		   PrefetchCoeff.VI[0][3][cindex1][cindex2]) / (float)LineSize) +
          ((float)(PrefetchCoeff.VI[1][2][cindex1][cindex2] * x2))/(float)LineSize +
          ((float)(PrefetchCoeff.VI[2][1][cindex1][cindex2] * x1))/(float)LineSize +
          ((float)(PrefetchCoeff.VI[0][1][cindex1][cindex2] * x1 +
		   PrefetchCoeff.VI[0][2][cindex1][cindex2] * x2 +
		   PrefetchCoeff.VI[0][3][cindex1][cindex2]));
     denom = (float)(flops * x1 * x2);
     return((P0 + PC + PI)/denom);
  }
     
