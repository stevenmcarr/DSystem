/* $Id: dfantic.h,v 1.6 1997/03/27 20:27:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef dfantic_h
#define  dfantic_h

#ifndef block_h
#include <libs/Memoria/include/block.h>
#endif

#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif

#ifndef Arena_h
#include <libs/support/memMgmt/Arena.h>
#endif


EXTERN(void, sr_perform_antic_analysis,(flow_graph_type flow_graph,
					int size,PedInfo ped,
					arena_type *ar));

EXTERN(void, sr_calc_local_antic,(block_type  *block,
				  int         size,
				  PedInfo     ped));

#endif
