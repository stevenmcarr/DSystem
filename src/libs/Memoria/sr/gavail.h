/* $Id: gavail.h,v 1.3 1992/12/07 10:20:32 carr Exp $ */

#ifndef gavail_h
#define gavail_h

typedef struct gavailinfotype {
  block_type *block,
             *exit_block;
  Set        LI_avail,
             LI_pavail;
  PedInfo    ped;
 } gavail_info_type;

EXTERN(void, sr_redo_gen_avail,(flow_graph_type flow_graph,int size,
					PedInfo ped,arena_type *ar));

#endif
