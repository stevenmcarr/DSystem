/* $Id: ped_util_dg.C,v 1.1 1997/06/25 14:40:06 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>

/*---------------------------------------------------------------------
 
  ped_print_deps()   Prints all dependences in program on REF lists

  Uses print_deps() as helper function for walk_expressions().

*/

void
ped_print_deps(PedInfo ped)
{
  dg_print_deps( PED_ROOT(ped), PED_DG(ped), (SideInfo *)PED_SIDE_INFO(ped) );
}


