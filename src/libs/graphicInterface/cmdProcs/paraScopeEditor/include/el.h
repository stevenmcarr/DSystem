/* $Id: el.h,v 1.11 1997/03/11 14:31:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/***************************************************************************

   NEW: Views on top of the dependence filter facility (leads to a pretty
   sorry user interface, but at least a decent functionality).
   kats 9/90

   Data structures to implement the Edge List abstraction. This is used by
   the editor to browse through the list of dependences in the source file, 
   and by the abstractions that implement parallel transformations.
   - Vas, Sept 1987.
 
   Changed SH abstraction to the more useful LI (Loop Info) abstraction.
   Added several new routines to support the manipulation of shared and
   private variables and statement insertion/deletion.
   - Vas, May 1988.

   NOTE: As long as the dependence info used by ParaScope is that generated 
   by PSERVE, remember to convert variable names into upper case before
   making any comparisons using strcmp(). This will be unnecessary when
   ParaScope becomes completely independent of PSERVE. 
   -Vas, May 1988.
 ****************************************************************************/

#ifndef el_h
#define el_h

#include <libs/moduleAnalysis/dependence/edgeList/el_instance.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_header.h>
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/dt_info.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>



/* free the memory used by the LI_Instance
 */
EXTERN( void,	ped_li_free,	( PedInfo ped ) );


/* -----------------------------------------------------------------------
   ped_el_new_loop() - collect the list of dependences for this loop in an
   edge list structure, and return the number of dependences found.
   "node" is the ast index of the loop header statement. 
  -----------------------------------------------------------------------*/
EXTERN(  int,	ped_el_new_loop, ( PedInfo ped, AST_INDEX  node) );




#endif	/* el_h */
