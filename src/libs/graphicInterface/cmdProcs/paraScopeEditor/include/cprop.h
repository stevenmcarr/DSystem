/* $Id: cprop.h,v 1.5 1997/03/11 14:31:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *  Public include file for decremented version of constant propagation
 *
 *  havlak, March 1992
 */

#ifndef _cprop_h_
#define _cprop_h_

/*
 *  cprop_build_structs
 *
 *	Constructs the global constant propagation structures,
 *	which you can hang onto if you'd like.  Arguments mean:
 *
 *	cpropInfo	an old global cprop object -- will be freed if !NULL
 *	ft		the ((Generic) FortTree) to build on
 *	makeChanges	change the tree?
 *	print		print a dump of the constants?
 *	killGraphs	free the global cprop object before returning?
 *	doValueTable	construct auxiliary ValueTable for accessing constants?
 *	doExecAlg	propagate constants? (else just build SSA structures)
 */
EXTERN(Generic, cprop_build_structs,
		(Generic cpropInfo,
		 Generic ft,
		 int makeChanges,
		 int print,
		 int killGraphs,
		 int doValueTable,
		 int doExecAlg));

/*
 *  cprop_change_ast
 *
 *	Calling cprop_build_structs with (makeChanges = 0), then calling this,
 *	is equivalent to just calling cprop_build_structs with (makeChanges = 1)
 */
EXTERN(void, cprop_change_ast, (Generic cpropInfo));

/*
 *  cprop_free_globals
 *
 *	Frees the global cprop object and auxiliary structures.
 */
EXTERN(void, cprop_free_globals, (Generic cpropInfo));

#endif /* _cprop_h_ */
