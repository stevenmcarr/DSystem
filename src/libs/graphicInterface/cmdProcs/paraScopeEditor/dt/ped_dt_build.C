/* $Id: ped_dt_build.C,v 1.1 1997/06/25 14:41:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>

/*-----------------------------------------------------------------------

	ped_dg_build()		Builds DG and LI for entire program from scratch
			Requires that the DG and LI structures already exist.

*/
void
ped_dg_build( PedInfo ped, AST_INDEX root )
//     PedInfo ped;
//     AST_INDEX root;
{
    dg_build(root, (FortTree)PED_FT(ped), PED_DG(ped), PED_INFO(ped), 
	     PED_DT_INFO(ped), PED_LI(ped), PED_PGM_CALLGRAPH(ped), 
	     PED_CFG(ped) );
}


/*-----------------------------------------------------------------------

	ped_dg_update()		Rebuild DG and LI for selected subtree

*/
void
ped_dg_update(PedInfo ped, AST_INDEX root)
// 	PedInfo		 ped;
// 	AST_INDEX	 root;
{
	dg_update( root, (FortTree)PED_FT(ped), PED_DG(ped), 
		  PED_INFO(ped), PED_DT_INFO(ped), PED_LI(ped) );
}


