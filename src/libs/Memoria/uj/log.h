/* $Id: log.h,v 1.4 1992/12/11 11:23:43 carr Exp $ */

#ifndef log_h
#define log_h

#include <stdio.h>

#ifndef general_h
#include <general.h>
#endif

#ifndef mh_h
#include <mh.h>
#endif

EXTERN(void, mh_log_data,(model_loop *loop_data,FILE *logfile));

#endif
