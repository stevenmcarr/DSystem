/* $Id: pick.h,v 1.4 1992/12/11 11:22:45 carr Exp $ */

#ifndef pick_h
#define pick_h

#ifndef general_h
#include <general.h>
#endif

#ifndef scalar_h
#include <scalar.h>
#endif

#ifndef block_h
#include <block.h>
#endif

#ifndef cgen_set_h
#include <cgen_set.h>
#endif

#ifndef dg_h
#include <dg.h>
#endif

#ifndef dp_h
#include <dp.h>
#endif

#ifndef ast_h
#include <ast.h>
#endif

typedef struct pickinfotype {
  block_type *block,
             *exit_block;
  Set        LI_avail,
             LI_rgen,
             LC_rgen_if_1,
             LI_pavail,
             LC_avail_if_1,
             LC_pavail_if_1;
  DG_Edge    *dg;
  Boolean    contains_cf;
  int        level;
  PedInfo    ped;
  Boolean    def;
  AST_INDEX  lhs;
 } pick_info_type;

EXTERN(void, sr_pick_possible_generators,(flow_graph_type flow_graph,
					  int level,
					  prelim_info_type *prelim_info,
					  PedInfo ped));

#endif
