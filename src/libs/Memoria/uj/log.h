/* $Id: log.h,v 1.5 1993/07/20 16:35:12 carr Exp $ */

#ifndef log_h
#define log_h

#include <stdio.h>

#ifndef general_h
#include <general.h>
#endif

#ifndef mh_h
#include <mh.h>
#endif

EXTERN(void, mh_log_data,(model_loop *loop_data,
			  FILE *logfile,
			  PedInfo ped,
			  SymDescriptor symtab,
			  arena_type    *ar,
			  LoopStatsType *LoopStats));

#endif
