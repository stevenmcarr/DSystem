/* $Id: do_dist.h,v 1.5 1997/03/27 20:28:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef do_dist_h
#define do_dist_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef mh_h
#include <libs/Memoria/include/mh.h>
#endif

EXTERN(void, mh_do_distribution,(model_loop *loop_data,
				 int        *num_loops));
#endif
