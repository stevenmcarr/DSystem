/* $Id: insert.h,v 1.3 1992/12/07 10:20:34 carr Exp $ */

#ifndef insert_h
#define insert_h

typedef struct setinfo{
  Set  LI_Insert,
       LC_Insert;
 } Set_info;

EXTERN(void, sr_perform_insert_analysis,(flow_graph_type flow_graph,
						 int size,arena_type *ar,
						 PedInfo ped));

#endif
