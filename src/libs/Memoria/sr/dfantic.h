/* $Id: dfantic.h,v 1.3 1992/12/07 10:20:27 carr Exp $ */

#ifndef dfantic_h
#define  dfantic_h

EXTERN(void, sr_perform_antic_analysis,(flow_graph_type flow_graph,
						int size,PedInfo ped,
						arena_type *ar));

EXTERN(void, sr_calc_local_antic,(block_type  *block,
					  int         size,
					  PedInfo     ped));

#endif
