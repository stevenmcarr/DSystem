/* $Id: dfantic.h,v 1.5 1993/06/21 13:46:50 carr Exp $ */

#ifndef dfantic_h
#define  dfantic_h

#ifndef block_h
#include <block.h>
#endif

#ifndef dp_h
#include <dp.h>
#endif

#ifndef Arena_h
#include <misc/Arena.h>
#endif


EXTERN(void, sr_perform_antic_analysis,(flow_graph_type flow_graph,
					int size,PedInfo ped,
					arena_type *ar));

EXTERN(void, sr_calc_local_antic,(block_type  *block,
				  int         size,
				  PedInfo     ped));

#endif
