/* $Id: ped_el.C,v 1.1 1997/06/25 14:40:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/PEditorCP/el/el.c					*/
/*									*/
/*	el.c -- The edge-list abstraction. Used to manipulate the	*/
/*		edges in the dependence graph.				*/
/*									*/
/************************************************************************/

/*		I N C L U D E    F I L E S 			*/

#include <libs/frontEnd/ast/ast.h>
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_instance.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_header.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/el.h>


/*	  I N T E R F A C E    F U N C T I O N S		*/



/* -----------------------------------------------------------------------
   ped_el_new_loop() - collect the list of dependences for this loop in an 
   edge list structure, and return the number of dependences found.
   "node" is the ast index of the loop header statement. 
  -----------------------------------------------------------------------*/

int 
ped_el_new_loop (PedInfo ped, AST_INDEX node)
{
    EL_Instance	*el	= (EL_Instance *)(PED_EL(ped));
    LI_Instance	*li	= (LI_Instance *)(PED_LI(ped));
    SideInfo	*infoPtr= PED_INFO(ped);
    DG_Instance	*dg	= PED_DG(ped);
    AST_INDEX	 loop	= PED_SELECTED_LOOP(ped);

    int		 result	= el_new_loop( el, li, infoPtr, dg, loop );

    return (result);
}


void
ped_li_free(PedInfo ped)
{
  li_free(PED_LI(ped));
  PED_LI(ped)	= NULL;
}
