#ifndef gavail_h
#define gavail_h

typedef struct {
  block_type *block,
             *exit_block;
  Set        LI_avail,
             LI_pavail;
  PedInfo    ped;
 } gavail_info_type;

EXTERN_FUNCTION(void sr_redo_gen_avail,(flow_graph_type flow_graph,int size,
					PedInfo ped,arena_type *ar));

#endif
