/* $Id: dg.h,v 1.15 1997/03/11 14:31:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef dg_h
#define dg_h

#include <libs/moduleAnalysis/dependence/interface/depType.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/cd.h>

/*************************************************************************

		file:	dg.h
		author: Kathryn McKinley

	This contains the public declarations for an implementation
	of dependence edges on multiple lists of level vectors and 
	references.  The dependence graph meets the specifications
	described in 'Dependence Graph Abstraction in PARASCOPE' by 
	Jaspal Subhlok. 
	
	All the routines and data structures described here may be 
	used or accessed by programs wishing to create, change, or 
	peruse a dependence graph.

**************************************************************************/


/*--------------------------------------------------------------*/
/* DG_Instance structure - represents the dependence graph	*/
/* MAXLOOP		max loop nesting level		*/
/* MAXDIM		max dimension of array		*/
/*	now defined in dep/dg_instance.h			*/

/*-----------------------------------------------*/
/* DG_Edge structure - represents one dependence */
/*	now defined in dep/dep_dg.h		*/


/*--------------------------------------------------------------*/
/* Carried_deps structure - used to collect loop carried deps	*/
/*	now defined in dep/dep_dg.h				*/



#endif /* dg_h */
