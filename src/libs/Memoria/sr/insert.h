/* $Id: insert.h,v 1.6 1997/03/27 20:27:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef insert_h
#define insert_h

#ifndef cgen_set_h
#include <libs/Memoria/include/cgen_set.h>
#endif

#ifndef block_h
#include <libs/Memoria/include/block.h>
#endif

#ifndef Arena_h
#include <libs/support/memMgmt/Arena.h>
#endif

#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif

typedef struct setinfo{
  Set  LI_Insert,
       LC_Insert;
 } Set_info;

EXTERN(void, sr_perform_insert_analysis,(flow_graph_type flow_graph,
					 int size,arena_type *ar,
					 PedInfo ped));

#endif
