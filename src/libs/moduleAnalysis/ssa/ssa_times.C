/* $Id: ssa_times.C,v 1.3 1997/03/11 14:36:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/*
 *  ssa_times.C
 *
 *  Declarations of global stopwatches and printing routines
 */
#include <stdio.h>
#include <string.h>

#include <libs/support/time/stopwatch.h>

EXTERN(void, ssa_init_times, ());
EXTERN(void, ssa_fini_times, ());

StopWatch ssa_watches[9];

Swatch ssa_OpenOne_sw = &(ssa_watches[0]);
Swatch ssa_build_ref_nodes_sw = &(ssa_watches[1]);
Swatch ssa_add_ip_refs_sw = &(ssa_watches[2]);
Swatch ssa_build_phis_sw = &(ssa_watches[3]);
Swatch ssa_convert_phis_sw = &(ssa_watches[4]);
Swatch ssa_list_loop_exits_sw = &(ssa_watches[5]);
Swatch ssa_build_dom_frontier_sw = &(ssa_watches[6]);
Swatch ssa_process_var_sw = &(ssa_watches[7]);
Swatch ssa_search_sw = &(ssa_watches[8]);

void ssa_init_times()
{
    memset (ssa_watches, 0, 9*sizeof(StopWatch));
}

void ssa_fini_times()
{
    printf("\tssa_OpenOne:\t%10ld\n",
	   ssa_OpenOne_sw->check());

    printf("\tssa_build_ref_nodes:\t%10ld\n",
	   ssa_build_ref_nodes_sw->check());

    printf("\tssa_add_ip_refs:\t%10ld\n",
	   ssa_add_ip_refs_sw->check());

    printf("\tssa_build_phis:\t%10ld\n",
	   ssa_build_phis_sw->check());

    printf("\tssa_convert_phis:\t%10ld\n",
	   ssa_convert_phis_sw->check());

    printf("\tssa_list_loop_exits:\t%10ld\n",
	   ssa_list_loop_exits_sw->check());

    printf("\tssa_build_dom_frontier:\t%10ld\n", 
	   ssa_build_dom_frontier_sw->check());

    printf("\tssa_process_var:\t%10ld\n",
	   ssa_process_var_sw->check());

    printf("\tssa_search:\t%10ld\n",
	   ssa_search_sw->check());
}
