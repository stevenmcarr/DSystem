/* $Id: balance.C,v 1.3 1992/12/17 14:50:57 carr Exp $ */
#include <balance.h>

/****************************************************************************/
/*                                                                          */
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

int mh_fp_register_pressure(int reg_coeff[4][3][3],
			    int *scalar_coeff,
			    int x1,
			    int x2)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

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

int mh_addr_register_pressure(int addr_coeff[4][3][3],
			      int x1,
			      int x2)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

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

float mh_loop_balance(int   mem_coeff[4][3][3],
		      int   flops,
		      int   x1,
		      int   x2)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

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
     return(num/denom);
  }

    
