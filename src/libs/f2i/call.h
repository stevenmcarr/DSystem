/* $Id: call.h,v 1.1 1997/03/20 14:27:37 carr Exp $ */
/*
 * call.h
 *
 *	the structure declarations required to interface to 
 *	GenerateCall(), the routine that worries about the 
 *	mechanics of argument passing, type conversion, and
 *	interprocedural side effects.
 *
 */

struct CallTemplate
{
  char 	*name;		/* name of the routine being called (no '_')	*/
  AST_INDEX CallSite;	/* index of the call site - to pass idfa rtns	*/
  int	InLibrary;	/* TRUE => library code, FALSE => user code	*/
  int	NumParms;	/* number of parameters in Actuals[]		*/
  int   ReturnReg;	/* non-zero => returns type ActualType[0]	*/

  AST_INDEX *Actuals;	/* AST_INDEX for each actual parameter		*/
  int	*ActualTypes;	/* ILOC_TYPE for actual (may need conversion)	*/
  int	*ActualMods;	/* Summary info by parameter			*/
  int	*ActualUses;	/*   ditto					*/
			/* **** ActualReg is optional ****		*/
  int	*ActualReg;	/* Register containing the actual parameter	*/
};

#define MAXPARMS 256

