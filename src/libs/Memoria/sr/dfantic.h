#ifndef dfantic_h
#define  dfantic_h

EXTERN_FUNCTION(void sr_perform_antic_analysis,(flow_graph_type flow_graph,
						int size,PedInfo ped,
						arena_type *ar));

EXTERN_FUNCTION(void sr_calc_local_antic,(block_type  *block,
					  int         size,
					  PedInfo     ped));

#endif
