/* $Id: codegen.h,v 1.6 1997/03/27 20:27:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef codegen_h
#define codegen_h

#include <stdio.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

#ifndef scalar_h
#include <libs/Memoria/sr/scalar.h>
#endif

#ifndef Arena_h
#include <libs/support/memMgmt/Arena.h>
#endif

#ifndef list_h
#include <libs/support/lists/list.h>
#endif

#ifndef fortsym_h
#include <libs/frontEnd/fortTree/fortsym.h>
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
