/* $Id: balance.C,v 1.2 1992/10/03 15:49:45 rn Exp $ */
#include <balance.h>

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

int mh_compute_x(int regs,
		 int int_regs,
		 int *reg_coeff,
		 int *scalar_coeff,
		 int *addr_coeff,
		 int x,
		 int i)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int denom;
   int rt,at;

     if (x == 1)
       {
	denom = reg_coeff[0] + scalar_coeff[0] + reg_coeff[3-i];
	if (denom == 0) 
	  if (scalar_coeff[i] == 0)
	    rt = 1;
	  else
	    rt = regs;
	else
	  rt = (regs - reg_coeff[3] - (reg_coeff[i] + scalar_coeff[i]))/denom;
       }
     else  
       {
	denom =  (reg_coeff[0] + scalar_coeff[0]) * x +
	          reg_coeff[3-i] + scalar_coeff[3-i];
	if (denom == 0)
	  rt = 1; 
	else
	  rt = (regs - reg_coeff[3] - ((reg_coeff[i] + scalar_coeff[i])* x))
	        / denom;
       }
     denom = addr_coeff[0]*x + addr_coeff[3-i];
     if (denom == 0)
       at = rt;
     else
       at = (int_regs - addr_coeff[3] - addr_coeff[i] * x)/denom;
     if (at < rt)
       return at;
     else
       return rt;
  }

int mh_register_pressure(int *reg_coeff,
			 int *scalar_coeff,
			 int x1,
			 int x2)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int regs;
   
     regs = (reg_coeff[0] + scalar_coeff[0]) * x1 * x2 + reg_coeff[1] * x1 + 
            reg_coeff[2] * x2 + reg_coeff[3];
     if (x1 != 1)
       regs += scalar_coeff[2] * x2;
     if (x2 != 1)
       regs += scalar_coeff[1] * x1;
     return(regs);
  }

float mh_loop_balance(int   *mem_coeff,
		      int   flops,
		      int   x1,
		      int   x2)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   float num,denom;

     num = (float)(mem_coeff[0] * x1 * x2 + mem_coeff[1] * x1 + mem_coeff[2] *
		   x2 + mem_coeff[3]);
     denom = (float)(flops * x1 * x2);
     return(num/denom);
  }

    
