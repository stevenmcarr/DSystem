/* $Id: gavail.h,v 1.6 1997/03/27 20:27:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef gavail_h
#define gavail_h

#ifndef block_h
#include <libs/Memoria/include/block.h>
#endif

#ifndef cgen_set_h
#include <libs/Memoria/include/cgen_set.h>
#endif

#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif

#ifndef Arena_h
#include <libs/support/memMgmt/Arena.h>
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
