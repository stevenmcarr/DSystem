/* $Id: ped_rsd.C,v 1.1 1997/06/25 14:41:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*-----------------------------------------------------------------------

	rsd.c		Regular Section Descriptors Module

	Currently, severely restricted

	History
	~~~~~~~
	25 May 90  roth    Created
	 8 Jun 91  tseng   Modifed to use DT_REF info

*/

#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/dt_info.h>
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>

#include <libs/moduleAnalysis/dependence/dependenceTest/rsd.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>


/*----------------------------------------------------------------------

    ped_rsd_vector_init()  
       
    Builds Rsd_vectors for all statements in tree.

*/

void
ped_rsd_vector_init(PedInfo ped, AST_INDEX root)
{
  rsd_vector_init( PED_DT_INFO(ped), PED_INFO(ped), root);
}

