/* $Id: main_dg.h,v 1.5 1997/03/11 14:31:18 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef _main_dg_h_
#define _main_dg_h_

/**********************************************************************
 * main_dg.h
 * author: John Mellor-Crummey
 *
 * provide an interface for building/reading and freeing a dependence 
 * graph for a module. 
 *********************************************************************/


#include <libs/support/misc/general.h>
#include <libs/support/database/context.h>
#include <libs/support/database/newdatabase.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>

/* read in or build a dependence graph for a module (among other things).
 * if a non NULL program context is supplied, interprocedural summary
 * information is used to build dependence edges for callsites
 */
EXTERN(Generic, pedInitialize, 
		(Generic ed_handle, 
		 FortTextTree ftt, FortTree ft,
		 Context module_context, 
		 Context mod_in_prog_context, 
		 Context pgm_context,
		 Boolean has_errors,
		 Boolean InputDep));


/* free storage for all structures allocated by pedInitialize */
EXTERN(void, pedFinalize, (PedInfo ped));


/* invoke batch dependence analysis of a module in the context of a 
 * program and save the resulting dependence graph. the dependence 
 * analyzer uses interprocedural summary information to construct
 * dependences involving callsites
 */
EXTERN(void, perform_batch_dependence_analysis, 
		(Context module_context, Context program_context));


#endif

