#ifndef dfrgen_h
#define dfrgen_h

#include "block.h"
#include "check.h"

typedef struct {
  block_type *block;
  Set        LC_kill;
  PedInfo    ped;
  AST_INDEX  lhs;
 } rgen_info_type;

EXTERN_FUNCTION(void sr_perform_rgen_analysis,(flow_graph_type flow_graph,
					       check_info_type check_info));

#endif
