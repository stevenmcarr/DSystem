#ifndef block_h
#define block_h

#include <Arena.h>
#include <sr.h>

typedef struct block_struct block_type;

struct edge_struct {
  block_type         *from,
                     *to;
  struct edge_struct *prev_pred,
                     *prev_succ,
                     *next_pred,
                     *next_succ;
  Set                Insert;
  AST_INDEX          lbl_ref;  /* for computed gotos */
  float              probability;  
}; 

typedef struct edge_struct edge_type;

struct block_struct {
  int                 block_num,
                      num_preds,
                      num_succs;
  Boolean             mark;
  AST_INDEX           first,
                      last,
                      join;
  struct block_struct *next;
  edge_type           *pred,
                      *succ;
  Set                 gen,
                      LC_gen,
                      kill,
                      LI_avail_in,
                      LI_avail_out,
                      LI_rgen_in,
                      LI_rgen_out,
                      LI_pavail_in,
                      LI_pavail_out,
                      LI_antic_in,
                      LI_antic_out,
                      LC_avail_in,
                      LC_avail_out,
                      LC_rgen_in,
                      LC_rgen_out,
                      LC_rgen_in_if_1,
                      LC_rgen_out_if_1,
                      LC_pavail_in,
                      LC_pavail_out,
                      LC_avail_in_if_1,
                      LC_avail_out_if_1,
                      LC_pavail_in_if_1,
                      LC_pavail_out_if_1,
                      LC_antic_in,
                      LC_antic_out,
                      PP_in,
                      PP_out,
                      Insert;
}; 

typedef struct {
  block_type  *entry,
              *exit;
 } flow_graph_type;

EXTERN_FUNCTION(void sr_build_flow_graph,(flow_graph_type *flow_graph,
					  AST_INDEX stmt_list,
					  SymDescriptor symtab,
					  arena_type *ar));
EXTERN_FUNCTION(block_type *sr_insert_block_on_edge,(arena_type *ar,
						     edge_type *edge,
						     SymDescriptor symtab));
EXTERN_FUNCTION(void sr_free_flow_graph,(flow_graph_type flow_graph));
EXTERN_FUNCTION(void debug_print_graph,(flow_graph_type flow_graph));
#endif
