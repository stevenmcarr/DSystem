/* $Id: profit.h,v 1.4 1992/12/11 11:22:46 carr Exp $ */

#ifndef profit_h
#define profit_h

#ifndef scalar_h
#include <scalar.h>
#endif

#ifndef cgen_set_h
#include <cgen_set.h>
#endif

#ifndef block_h
#include <block.h>
#endif

#ifndef Arena_h
#include <Arena.h>
#endif

typedef struct profinfotype {
  Set              pavset;
  float            prob;
  array_table_type *array_table;
 } prof_info_type;

EXTERN(void, sr_perform_profit_analysis,(flow_graph_type flow_graph,
					 int size,
					 array_table_type *array_table,
					 arena_type *ar));

#endif
