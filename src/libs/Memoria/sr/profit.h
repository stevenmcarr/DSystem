/* $Id: profit.h,v 1.2 1992/10/03 15:49:29 rn Exp $ */
#ifndef profit_h
#define profit_h

#include <scalar.h>

typedef struct {
  Set              pavset;
  float            prob;
  array_table_type *array_table;
 } prof_info_type;

EXTERN(void, sr_perform_profit_analysis,(flow_graph_type flow_graph,
						 int size,
						 array_table_type *array_table,
						 arena_type *ar));

#endif
