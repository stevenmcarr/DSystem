#ifndef insert_h
#define insert_h

typedef struct {
  Set  LI_Insert,
       LC_Insert;
 } Set_info;

EXTERN_FUNCTION(void sr_perform_insert_analysis,(flow_graph_type flow_graph,
						 int size,arena_type *ar,
						 PedInfo ped));

#endif
