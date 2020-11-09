/* $Id: dep_dg.h,v 1.16 1997/03/11 14:35:45 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef dep_dg_h
#define dep_dg_h

/*************************************************************************

		file:	dep_dg.h
		author: Kathryn McKinley

	------------------------------------------------------------
	WARNING: the following public definitions are successively
	being made private.  Continued use of the details of these
	structures outside of dependence analysis or transformations
	will be hazardous!
	------------------------------------------------------------

	This contains the public declarations for an implementation
	of dependence edges on multiple lists of level vectors and 
	references.  The dependence graph meets the specifications
	described in 'Dependence Graph Abstraction in PARASCOPE' by 
	Jaspal Subhlok. 
	
	All the routines and data structures described here may be 
	used or accessed by programs wishing to create, change, or 
	peruse a dependence graph.

**************************************************************************/


#ifndef	cd_branch_h
#include <libs/frontEnd/ast/cd_branch.h>
#endif
#ifndef	dg_instance_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#endif
#ifndef	dg_header_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#endif

#include <libs/frontEnd/fortTree/FortTree.h>
/*-----------------------------------------------*/
/* DG_Edge structure - represents one dependence */

struct struct_dg_edge 
{
    AST_INDEX src;      /* source of dependence         */
    AST_INDEX sink;     /* sink of dependence           */
    int       level;	/* level (depth) of dependence  */
    DepType   type;     /* type of dependence           */
    char     *src_str;  /* the RSD for src              */
    char     *sink_str; /* the RSD for sink             */

    /* flags for edge    */

    Boolean	ic_preventing;	/* loop interchange illegal        */
    Boolean	ic_sensitive;	/* loop interchange profitable     */
    ConsistentType consistent;	/* consistent across iterations    */
    Boolean	   symbolic;	/* consistent distance is symbolic */
    Boolean	 used;		/* edge is currently in use        */
    CdBranchType cdtype;	/* control dependence identifier   */
    int		 cdlabel;	/* cd branching label              */

    /* handles returned from dg_alloc_ref_list()    */

    int src_ref;     /* list of deps with ref as source  */
    int sink_ref;    /* list of deps with ref as sink    */

    /* handles returned from dg_alloc_level_vector() */

    int src_vec;     /* list of deps with stmt as source */
    int sink_vec;    /* list of deps with stmt as sink   */

    int label;       /* label nodes for interchange - jass April 88 */

    /* dependence test info */

    int dt_type;		/* type of dt_info		*/
    int dt_data[MAXLOOP];	/* actual data on dep		*/
    char *dt_str;		/* string to be displayed	*/
} ;


/*------------------------------------------------------------*/
/* Carried_deps structure - used to collect loop carried deps */

#define MAXDEPS 1000             /* max # of deps carried by loop */

struct struct_carried_deps
{
  int all_num;                   /* total number of dependneces  */
  int true_num;                  /* number of true dependences   */
  int anti_num;                  /* number of anti dependences   */
  int out_num;                   /* number of output dependences */
  DG_Edge *true_deps[MAXDEPS];   /* list of true dependences     */
  DG_Edge *anti_deps[MAXDEPS];   /* list of output dependences   */
  DG_Edge *out_deps[MAXDEPS];    /* list of output dependences   */
} ;




/* ------------------------------------------------------------ */
/*	The following functions are in dep/dg/dg_util.c		*/
/* ------------------------------------------------------------ */


EXTERN(int,	dg_print_deps,
		(AST_INDEX  root, FortTree ft, DG_Instance * dg, SideInfo * infoPtr) );
/* Prints all dependences in program on REF lists
 */

EXTERN(Carried_deps *, dg_carried_deps, 
		(DG_Instance * dg, SideInfo * infoPtr, AST_INDEX loop));
/* Returns list of dependences carried by loop.
 */





#endif /* dep_dg_h */
