/* $Id: call.h,v 1.2 1997/03/27 20:30:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

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

