/* $Id: dfavail.h,v 1.3 1992/12/07 10:20:29 carr Exp $ */

#ifndef dfavail_h
#define dfavail_h

#include <block.h>
#include <check.h>

typedef struct availinfotype {
  block_type *block;
  Set        LC_kill;
  PedInfo    ped;
 } avail_info_type;

EXTERN(void, sr_perform_avail_analysis,(flow_graph_type flow_graph,
						check_info_type check_info));
#endif
