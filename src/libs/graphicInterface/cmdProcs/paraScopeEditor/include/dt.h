/* $Id: dt.h,v 1.14 1997/03/11 14:31:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*-----------------------------------------------------------------------

	dt.h		Dependence Test Module Header File

	History
	~~~~~~~
	15 Feb 90  cwt  Created
	22 May 91  cwt  Add support for REF
	30 Aug 92  mpal Separated the ped specific functions
*/

#include <libs/frontEnd/ast/ast.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/dep_dt.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>

/* dependence test entry points in ped_cp/dt/dt_build.c				*/

/* Builds DG and LI for entire program given DG_Instance, ...		*/
/* Requires that the DG and LI structures already exist.		*/
EXTERN( void,	ped_dg_build, ( PedInfo ped, AST_INDEX root) );


/* Rebuild DG and LI for selected subtree			*/
EXTERN( void,	ped_dg_update, ( PedInfo ped, AST_INDEX root) );

