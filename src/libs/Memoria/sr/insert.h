/* $Id: insert.h,v 1.4 1992/12/11 11:22:41 carr Exp $ */

#ifndef insert_h
#define insert_h

#ifndef cgen_set_h
#include <cgen_set.h>
#endif

#ifndef block_h
#include <block.h>
#endif

#ifndef Arena_h
#include <Arena.h>
#endif

#ifndef dp_h
#include <dp.h>
#endif

typedef struct setinfo{
  Set  LI_Insert,
       LC_Insert;
 } Set_info;

EXTERN(void, sr_perform_insert_analysis,(flow_graph_type flow_graph,
					 int size,arena_type *ar,
					 PedInfo ped));

#endif
