/* $Id: ped_ipperf.C,v 1.1 1997/03/11 14:32:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Id: ped_ipperf.C,v 1.1 1997/03/11 14:32:11 carr Exp $
*/

/*
 * This is the main hook that PED uses to invoke the IP performance
 * estimator local phase. The performance estimation IP local phase used
 * to live in PED, and it now is in its own directory.
 * 
 * A stub exists for this routine, so that if you comment out the ped_cp/perf
 * line from your list of PED archives, the stub will be linked in instead
 * of this routine.
 * 
 * Author: N. McIntosh
 */

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/fort.h>
#include <libs/frontEnd/fortTree/fortsym.h>
#include <libs/support/database/newdatabase.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/perf/ip_perfdata.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_ipperf.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/perf/ip_perf.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/perf/ip_perf.h>

/* Link control. */
int ped_link_ip_perf = 1;

/* Debugging flag */
int ped_debug_ip_perf = 0;

/*
 * The following is the entry point use by PED in this module. Build
 * up some data structures for the performance estimator, then pass
 * them into the local phase. 
 */

void ped_perf_ip_local_phase(PedInfo pedp, Context module_context)
{
  Perf_data *pdata;
  FILE *debug_fp = 0;
  Boolean singlemod;
  
  /*
   * Start by reading the performance data. Currently we only have data
   * for the Sequent (I'll worry about putting in some sort of a switch
   * here later on).
   */
  pdata = perf_read_data(PE_SEQUENT_SYMMETRY);
  if (!pdata)
    return;	/* perf_read_data will already have printed an error msg */
  
  /* Open the debugging output file if printing enabled.
  */
  if (ped_debug_ip_perf) {
    debug_fp = fopen("perf.out", "w");
    (void) fprintf(debug_fp, "\n Starting Perf Est from PED\n");
  }

  /*
   * Invoke the local phase
   */
  perf_local_phase_walkcd(pdata,
			  (FortTree) PED_FT(pedp),
			  module_context,
			  (DG_Instance *) PED_DG(pedp),
			  (SideInfo *) PED_INFO(pedp),
			  debug_fp,
			  &singlemod,
			  "dummy");

  /* Dump the tree as a final step.
  */
  if (ped_debug_ip_perf) {
    FortTree ft = (FortTree) PED_FT(pedp);
    AST_INDEX root_ast = ft_Root(ft);
    fprintf(debug_fp, "\n\n\n+++ Dump of tree: +++\n");
    tree_print_to_file(root_ast, debug_fp);
    fclose(debug_fp);
  }
  
  /*
   * Free the performance data
   */
  free_Perf_data(pdata);
}
