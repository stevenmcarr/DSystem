/* $Id: ped_dt.C,v 1.1 1997/06/25 14:41:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*-----------------------------------------------------------------------

	dt.c		Dependence Test Module

	History
	~~~~~~~
	6 Feb 90  cwt :    Created
	4 Oct 91  seema :  Modified Loop_data to include symbolic information
*/


#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/ast.h>

#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/dt_info.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/dep_dt.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>

/*-------------------*/ 
/* global functions  */

void     ped_dt_update();


/*-----------------------------------------------------------------------

ped_dt_update()		Update dependence vectors

	Assumes that the dependence edges are in place
	Updates local dependence testing info for edges
*/
void
ped_dt_update(PedInfo ped, AST_INDEX root)
{
  /* first update loop & ref info */
  /* then recalculate dep test info in area	*/

  dt_update( PED_DG(ped), PED_DT_INFO(ped), PED_INFO(ped), root);
}


