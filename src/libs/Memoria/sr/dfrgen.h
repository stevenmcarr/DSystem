/* $Id: dfrgen.h,v 1.4 1992/12/11 11:22:38 carr Exp $ */

#ifndef dfrgen_h
#define dfrgen_h

#ifndef block_h
#include <block.h>
#endif

#ifndef check_h
#include <check.h>
#endif

#ifndef ast_h
#include <ast.h>
#endif

#ifndef dp_h
#include <dp.h>
#endif

#ifndef cgen_set_h
#include <cgen_set.h>
#endif

typedef struct rgeninfotype {
  block_type *block;
  Set        LC_kill;
  PedInfo    ped;
  AST_INDEX  lhs;
 } rgen_info_type;

EXTERN(void, sr_perform_rgen_analysis,(flow_graph_type flow_graph,
				       check_info_type check_info));

#endif
