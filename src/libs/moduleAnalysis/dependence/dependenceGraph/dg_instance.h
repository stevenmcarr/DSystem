/* $Id: dg_instance.h,v 1.10 1997/06/25 15:06:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef dg_instance_h
#define dg_instance_h


/*************************************************************************

		file:	dg_instance.h
		author: Kathryn McKinley
		revisions: Mike Paleczny

	This contains the public declarations for an implementation
	of the dependence graph.  The dependence graph meets the 
	specifications described in 'Dependence Graph Abstraction 
	in PARASCOPE' by Jaspal Subhlok. 
	
	All the routines and data structures described here may be 
	used or accessed by programs wishing to create, change, or 
	peruse a dependence graph.

**************************************************************************/

#include <libs/support/misc/general.h>

#define NIL		 -1
#define END_OF_LIST      -1
#define LOOP_INDEPENDENT -1

#define MAXLOOP    10		/* max loop nesting level		*/
#define MAXDIM     5		/* max dimension of array		*/

/*--------------------------------------------------------------*/
/* DG_Instance -- the dependence graph structure 		*/

typedef	struct struct_DG_Instance	DG_Instance;

/*-----------------------------------------------*/
/* DG Edge structure - represents one dependence */

typedef struct struct_dg_edge		DG_Edge;


/*------------------------------------------------------------*/
/* Carried_deps structure - used to collect loop carried deps */

#define MAXDEPS 1000             /* max # of deps carried by loop */

typedef	struct struct_carried_deps	Carried_deps;

/*----------------------------------------------*/
/* Type of the dependence edge indices.		*/

typedef Generic	EDGE_INDEX;


/*--------------------------------------------------------------*/
/* ConsistentType indicates result of comparing the two subscript
        expressions     ,a1*I1 + a2*I2 + ... + an*In,
        and             ,b1*I1 + b2*I2 + ... + bn*In,           */
/* The I_ are induction variables, a_ and b_ are coefficiants.  */
                                        /* added mpal:910729    */
/* inconsistent         a_i - b_i  is nonzero for some coef pair*/
/* consistent_SIV       induction variable for loop which carries
                this dependence never occurs as a MIV subscript */
/* consistent_MIV       induction variable for loop which carries
                this dependence occurs as a MIV subscript.      */

typedef enum { inconsistent, consistent_SIV, consistent_MIV } ConsistentType;


/*--------------------------------------------------------------*/
/* GraphType indicates the source of dependence analysis	*/
/* when read in using readgraph.				*/
/* graph_local		analysis done within PARASCOPE		*/
/* graph_pfc		all analysis done by PFC		*/
/* graph_updated_pfc	PFC, some subtree(s) analyzed locally	*/
/* 			&/or local control dependence analysis	*/

typedef	enum { graph_unknown, graph_local, graph_pfc, graph_updated_pfc } GraphType;

#endif	/* dg_instance_h */

