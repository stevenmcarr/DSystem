/*$Id: ped_ipperf.h,v 1.1 1997/10/30 15:33:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * External declaration for the PED portion of the PEDinterprocedural
 * local phase of the performance estimator.
 */

#ifndef IP_PERF_H_INCLUDED
#define IP_PERF_H_INCLUDED

extern int ped_link_ip_perf;

#include <libs/support/database/context.h>

EXTERN(void, ped_perf_ip_local_phase,
		(PedInfo gped, Context pedcontext));

#endif
