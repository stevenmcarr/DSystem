/* $Id: block.h,v 1.8 1997/03/27 20:24:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



#ifndef block_h
#define block_h

#ifndef Arena_h
#include <libs/support/memMgmt/Arena.h>
#endif
#ifndef cgen_set_h
#include <libs/Memoria/include/cgen_set.h>
#endif
#ifndef general_h
#include <libs/support/misc/general.h>
#endif
#ifndef ast_h          
#include <libs/frontEnd/ast/ast.h>
#endif
#ifndef fortsym_h
#include <libs/frontEnd/fortTree/fortsym.h>
#endif

#include <libs/Memoria/include/LoopStats.h>

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
                      Insert,
                      Transp,
                      Antloc;
}; 

typedef struct {
  block_type  *entry,
              *exit;
 } flow_graph_type;

EXTERN(void, sr_build_flow_graph,(flow_graph_type *flow_graph,
				  AST_INDEX stmt_list,
				  SymDescriptor symtab,
				  arena_type *ar,
				  LoopStatsType *loopstat));
EXTERN(block_type *, sr_insert_block_on_edge,(arena_type *ar,
					      edge_type *edge,
					      SymDescriptor symtab));
EXTERN(void, sr_free_flow_graph,(flow_graph_type flow_graph));
EXTERN(void, debug_print_graph,(flow_graph_type flow_graph));
#endif
