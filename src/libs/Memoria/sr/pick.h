/* $Id: pick.h,v 1.2 1992/10/03 15:49:36 rn Exp $ */
#ifndef pick_h
#define pick_h

typedef struct {
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
