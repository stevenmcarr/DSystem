/* $Id: ped_cd.C,v 1.1 1997/06/25 14:40:06 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	File:	ped_cp/PEditorCP/dg/cd.c				*/
/*	Author:	Kathryn McKinley					*/
/*									*/
/*	Routines:							*/
/*	   Externally available:					*/
/*		ControlDep *ped_dg_build_cdg (ped, root, extra)		*/
/*		Boolean	    ped_dg_delete_cds( ped, root, check)	*/
/*									*/
/************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/ast.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/cd.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>




/************************************************************************/
/*									*/
/* ped_dg_build_cdg -- copies the control dependences from the		*/
/*		dependence graph, forming and returning a rooted, 	*/
/*		ordered control dependence graph.			*/
/*									*/
/************************************************************************/
ControlDep *
ped_dg_build_cdg( PedInfo ped, AST_INDEX root, double extra)
//     PedInfo	ped;
//     AST_INDEX   root;
//     double	extra;		/* float automatically promoted	*/
{
    ControlDep	*cdp;

    cdp	= dg_build_cdg( PED_DG(ped), PED_INFO(ped), root, extra );

    return	cdp;
}

/************************************************************************/
/*									*/
/* ped_dg_delete_cds -- Starts at root and deletes control dependences 	*/
/*	for every statement in root's scope.  If check is true, it  	*/
/*	returns false if there are edges into or out of the scope that 	*/
/*	are not deleted, otherwise it returns true.			*/
/*									*/
/************************************************************************/
Boolean
ped_dg_delete_cds( PedInfo ped, AST_INDEX root, Boolean check)
//     PedInfo	 ped;
//     AST_INDEX	 root;
//     Boolean	 check;
{
    Boolean	 result;

    if (root == PED_ROOT(ped))
	check = false;

    result	= dg_delete_cds( PED_DG(ped), PED_INFO(ped), root, check );

    return	(result);
}

