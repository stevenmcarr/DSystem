/* $Id: gavail.h,v 1.4 1992/12/11 11:22:39 carr Exp $ */

#ifndef gavail_h
#define gavail_h

#ifndef block_h
#include <block.h>
#endif

#ifndef cgen_set_h
#include <cgen_set.h>
#endif

#ifndef dp_h
#include <dp.h>
#endif

#ifndef Arena_h
#include <Arena.h>
#endif

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
