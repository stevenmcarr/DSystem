/* $Id: codegen.h,v 1.5 1993/06/21 13:46:49 carr Exp $ */

#ifndef codegen_h
#define codegen_h

#include <stdio.h>

#ifndef general_h
#include <general.h>
#endif

#ifndef ast_h
#include <ast.h>
#endif

#ifndef scalar_h
#include <scalar.h>
#endif

#ifndef Arena_h
#include <misc/Arena.h>
#endif

#ifndef list_h
#include <misc/list.h>
#endif

#ifndef fortsym_h
#include <fort/fortsym.h>
#endif

typedef struct codeinfotype {
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
