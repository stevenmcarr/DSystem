/* $Id: do_dist.h,v 1.4 1992/12/11 11:23:38 carr Exp $ */

#ifndef do_dist_h
#define do_dist_h

#ifndef general_h
#include <general.h>
#endif

#ifndef mh_h
#include <mh.h>
#endif

EXTERN(void, mh_do_distribution,(model_loop *loop_data,
				 int        *num_loops));
#endif
