/* $Id: dfavail.h,v 1.4 1992/12/11 11:22:37 carr Exp $ */

#ifndef dfavail_h
#define dfavail_h

#ifndef block_h
#include <block.h>
#endif

#ifndef check_h
#include <check.h>
#endif

#ifndef cgen_set_h
#include <cgen_set.h>
#endif

#ifndef dp_h
#include <dp.h>
#endif

typedef struct availinfotype {
  block_type *block;
  Set        LC_kill;
  PedInfo    ped;
 } avail_info_type;

EXTERN(void, sr_perform_avail_analysis,(flow_graph_type flow_graph,
					check_info_type check_info));
#endif
