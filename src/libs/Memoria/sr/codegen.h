/* $Id: codegen.h,v 1.2 1992/10/03 15:49:05 rn Exp $ */
#ifndef codegen_h
#define codegen_h

#include <scalar.h>

typedef struct {
  int         iteration,
              copy;
  AST_INDEX   stmt,
              do_stmt;
  char        *target;
  Boolean     post_stores,
              load_scalar;
  UtilList    *glist;
  SymDescriptor symtab;
 } code_info_type;

#define NUM_REGS  "sr:num_regs"

/* EXTERN(AST_INDEX, sr_change_logical_to_block_if,(AST_INDEX stmt)); */
EXTERN(void, sr_generate_code,(AST_INDEX root,PedInfo ped,int level,
				       flow_graph_type flow_graph,
				       array_table_type *array_table,
				       SymDescriptor symtab,
				       UtilList *glist,
				       FILE *logfile,
				       arena_type *ar));

#endif
