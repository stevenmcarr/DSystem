/* $Id: cd_branch.h,v 1.1 1997/03/11 14:29:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Id: cd_branch.h,v 1.1 1997/03/11 14:29:25 carr Exp $ 
*/

#ifndef cd_branch_h
#define cd_branch_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

/************************************************************************/
/*									*/
/*	fort/cd_branch.h  --						*/
/*	    Codes used in representing control information.		*/
/*	    Used both by control dependence and control flow packages.	*/
/*									*/
/*	Feb 1993 -- extracted from dep/cd_graph.h and put here	--paco	*/
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
 *  dependence graph format.  Label in field 5, branch type in field 9.
 *
 *  Modification history: (hopefully sparse)
 *  ---------------------
 *	5 August 1991 -- Paul Havlak
 *	Changed alt. entry numbering so that normal entry is labeled 1,
 *	alternate entries 2:(n+1).  This is so that 0-labeled edges can
 *	be added from the main entry to its postdominators, resulting in
 *	a rooted control dependence graph.
 *  
 *  Statement			Arity	Labels	Branch Type
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

typedef enum		/* Edge Labeling/Numbering	*/
{					/* -----------------------------*/
    CD_UNCONDITIONAL		= 0,	/* All edges are 0		*/
    CD_LOGICAL_IF		= 1,	/* false/else (0), true/then (1)*/
    CD_DO_LOOP			= 2,	/* fallthrough (0), enter (1)	*/
    CD_ARITHMETIC_IF		= 3,	/* neg (1), 0 (2), pos (3)	*/
    CD_COMPUTED_GOTO		= 4,	/* fallthrough(0), first(1)..last(n) */
    CD_ASSIGNED_GOTO		= 5,	/* Number first label (1) to last (n)*/
    CD_OPEN_ASSIGNED_GOTO	= 6,	/* arbitrary 1..# of visible labels  */
    CD_I_O			= 7,	/* fallthrough (0), ERR(1), END(-1)  */
    CD_CASE			= 8,	/* default(0), first case(1)..last(n)*/
    CD_ALTERNATE_RETURN		= 9,	/* fallthrough (0), arbitrary 1..n   */
    CD_ALTERNATE_ENTRY		= 10,	/* fallthrough (0), arbitrary 1..n   */
    CD_NUM_TYPES		= 11	
} CdBranchType;

/*
 *  Mnemonics for common labels
 */
#define CD_FALSE	0
#define CD_DEFAULT	0
#define CD_FALLTHROUGH	0
#define CD_TRUE		1
#define CD_ENTER	1
#define CD_FIRST	1
#define CD_NEGATIVE	1
#define CD_ZERO		2
#define CD_POSITIVE	3
#define CD_IO_ERR	1
#define CD_IO_END	-1
#define CD_INVALID	-2

/* --------------------------------------------------------------------	*/
/*  These entry points are in fort/cd_branch.c
 */

EXTERN(CdBranchType, cd_branch_type, (AST_INDEX n));
/*
 *  Classifies the type of branch node that gives rise to the dependence.
 *  Answers can be misleading if fanout <= 1.
 */

EXTERN(char *, cd_branch_text, (CdBranchType type));
/*
 *  Returns a (static) string for the type name.
 */

#endif /* cd_branch_h */
