/* $Id: cfg_times.C,v 1.3 1997/03/11 14:35:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/*
 *  cfg_times.C
 *
 *  Declarations of global stopwatches and printing routines
 */
#include <stdio.h>
#include <string.h>

#include <libs/support/time/stopwatch.h>
EXTERN(void, cfg_init_times, ());
EXTERN(void, cfg_fini_times, ());

StopWatch cfg_watches[5];

Swatch cfg_build_inst_sw = &(cfg_watches[0]);    
Swatch cfg_build_nodes_sw = &(cfg_watches[1]);
Swatch cfg_build_edges_sw = &(cfg_watches[2]);
Swatch cfg_add_loop_nodes_sw = &(cfg_watches[3]);
Swatch cfg_build_cds_sw = &(cfg_watches[4]);

void cfg_init_times()
{
    memset (cfg_watches, 0, 5*sizeof(StopWatch));
}

void cfg_fini_times()
{
    printf("\tcfg_build_inst:\t%10ld\n", cfg_build_inst_sw->check());

    printf("\tcfg_build_nodes:\t%10ld\n", cfg_build_nodes_sw->check());

    printf("\tcfg_build_edges:\t%10ld\n", cfg_build_edges_sw->check());

    printf("\tcfg_add_loop_nodes:\t%10ld\n", cfg_add_loop_nodes_sw->check());

    printf("\tcfg_build_cds:\t%10ld\n", cfg_build_cds_sw->check());
}
