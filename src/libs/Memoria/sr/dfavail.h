/* $Id: dfavail.h,v 1.2 1992/10/03 15:49:11 rn Exp $ */
#ifndef dfavail_h
#define dfavail_h

#include <block.h>
#include <check.h>

typedef struct {
  block_type *block;
  Set        LC_kill;
  PedInfo    ped;
 } avail_info_type;

EXTERN(void, sr_perform_avail_analysis,(flow_graph_type flow_graph,
						check_info_type check_info));
#endif
