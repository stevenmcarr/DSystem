/* $Id: balance.C,v 1.6 1994/07/11 13:43:21 carr Exp $ */

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

    
float mh_CacheBalance(int       mem_coeff[4][3][3],
		      float     PrefetchCoeff[4][3][3],
		      int       flops,
		      int       x1,
		      int       x2,
		      float       MissCost)

  {
   float PL,FL,MemBal;
   int cindex1,cindex2;

     MemBal = mh_loop_balance(mem_coeff,flops,x1,x2);
     cindex1 = get_coeff_index(x1);
     cindex2 = get_coeff_index(x2);

     PL = PrefetchCoeff[0][cindex1][cindex2] * x1 * x2 +
          PrefetchCoeff[1][cindex1][cindex2] * x1 +
	  PrefetchCoeff[2][cindex1][cindex2] * x2 +
	  PrefetchCoeff[3][cindex1][cindex2];

     FL = (float)(flops * x1 * x2);
     if (FL == 0)
       return(0.0);
     else
       return((PL*MissCost)/FL + MemBal);
  }
     
