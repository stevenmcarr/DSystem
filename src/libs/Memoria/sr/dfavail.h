/* $Id: dfavail.h,v 1.5 1997/03/27 20:27:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef dfavail_h
#define dfavail_h

#ifndef block_h
#include <libs/Memoria/include/block.h>
#endif

#ifndef check_h
#include <libs/Memoria/sr/check.h>
#endif

#ifndef cgen_set_h
#include <libs/Memoria/include/cgen_set.h>
#endif

#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif

typedef struct availinfotype {
  block_type *block;
  Set        LC_kill;
  PedInfo    ped;
 } avail_info_type;

EXTERN(void, sr_perform_avail_analysis,(flow_graph_type flow_graph,
					check_info_type check_info));
#endif
