/* $Id: log.h,v 1.6 1997/03/27 20:28:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef log_h
#define log_h

#include <stdio.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef mh_h
#include <libs/Memoria/include/mh.h>
#endif

EXTERN(void, mh_log_data,(model_loop *loop_data,
			  FILE *logfile,
			  PedInfo ped,
			  SymDescriptor symtab,
			  arena_type    *ar,
			  LoopStatsType *LoopStats));

#endif
