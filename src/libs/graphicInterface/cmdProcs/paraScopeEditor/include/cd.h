/* $Id: cd.h,v 1.14 1997/03/11 14:31:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef cd_h
#define cd_h

#include <libs/moduleAnalysis/dependence/interface/depType.h>
#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/moduleAnalysis/dependence/controlDependence/cd_graph.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>

#include <libs/moduleAnalysis/dependence/utilities/strong.h>



/************************************************************************/
/*									*/
/*	/rn/src/rn/ped_cp/ped_include/cd.h --				*/
/*	    Control dependence representation, data structures for a    */
/*	    control dependence graph, and external routine declarations.*/
/*									*/
/************************************************************************/

/*
 *  Types of control flow to handle (besides procedure calls themselves)
 *  ===============================
 *  
 *  This specifies how we will label control dependence edges.  The first
 *  requirement on the labels that they indicate which conditional branch
 *  was taken.  We could just order the branches and number them from 1..n
 *  
 *  We do that with two exceptions:
 *	* use 0 to indicate fallthrough or default
 *	* use -1, 0, 1 for I/O conditions (to keep contiguous label space)
 *
 *  The arbitrary numeric values need to stay consistent with the PSERVE
 *  dependence graph format.  Label in field 5, source type in field 9.
 *
 *  Modification history: (hopefully sparse)
 *  ---------------------
 *	5 August 1991 -- Paul Havlak
 *	Changed alt. entry numbering so that normal entry is labeled 1,
 *	alternate entries 2:(n+1).  This is so that 0-labeled edges can
 *	be added from the main entry to its postdominators, resulting in
 *	a rooted control dependence graph.
 *  
 *  Statement			Arity	Labels	Source Type
 *  ---------			-----	------	-----------
 *  GOTO			1	0	UNCONDITIONAL
 *  PARALLEL CASES		n	0	UNCONDITIONAL
 *  IF () THEN/stmt		2	0..1	LOGICAL_IF
 *  DO ...			1	0..1	DO_LOOP
 *  PARALLEL LOOP		2	0..1	DO_LOOP
 *  IF () L1,L2,L3		3	1..3	ARITHMETIC_IF
 *  GOTO (L1..Ln) i		n+1	0..n	COMPUTED_GOTO
 *  GOTO i (L1..Ln)		n	1..n	ASSIGNED_GOTO
 *  GOTO i 			N	1..N	OPEN_ASSIGNED_GOTO
 *  I/O (normal, ERR, END)	3	-1..1	I_O (input/output stmt)
 *  CASE (n cases, default)	n+1	0..n	CASE
 *  alt. return (n of `em)	n+1	0..n	ALTERNATE_RETURN (call/fn inv.)
 *  alt. entries (n of `em)	n+1	1..n+1	ALTERNATE_ENTRY (subroutine)
 */

/**************************************************************************/
/* The CDG structure represents explicitly  both the nodes
 * and the edges in the control dependence graph.  All of the
 * lists are doubley linked for easy access. This makes
 * deletion tricky because nodes and edges may be part of
 * several lists.
 * The cdg also has specialized usages in loop distribution, performance
 * estimation, and the intermediate representation for parallel
 * code generation.
 */


/* These entry points are in /rn/src/ped_cp/dg/ped_cd.c.
 */

EXTERN(ControlDep *, ped_dg_build_cdg, 
		(PedInfo ped, AST_INDEX root, double extra));
/* Extracts the control dependences in ped's dg for the subprogram
 * rooted at root.  A root may be for a module, a subroutine, a loop, etc.
 * The control dependences are placed in a graph by themselves which is
 * returned.  The graph is cyclic.  It is also rooted, but only if all the 
 * executable statements in the module are reachable.  Extra multiplies
 * the number of edges and nodes in the cdg, and that additional number
 * of nodes and edges is also allocated.  The default for extra is 1.
 */

/* This entry point is in /rn/src/ped_cp/dg/ped_cd.c
 */
EXTERN(Boolean, ped_dg_delete_cds, 
		(PedInfo ped, AST_INDEX root, Boolean check));
/* Starts at root and deletes control dependences for every statement
 * in root's scope.  If check is true, it returns false if there are 
 * edges into or out of the scope that are not deleted, otherwise it
 * returns true.
 */


/* This entry point is in /rn/src/ped_cp/pt/dstr_graph.c
 */

EXTERN(ControlDep *, dstr_graph, 
		(PedInfo ped, AST_INDEX loop));
/* Contructs a mini dependence graph for a loop with control and data edges.
 * Duplicate data edges are pruned.
 */


#endif /* cd_h */
