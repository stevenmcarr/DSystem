/* $Id: prune.h,v 1.4 1992/12/11 11:22:48 carr Exp $ */

#ifndef prune_h
#define prune_h

#ifndef block_h
#include <block.h>
#endif

#ifndef dp_h
#include <dp.h>
#endif

#ifndef scalar_h
#include <scalar.h>
#endif

#ifndef ast_h
#include <ast.h>
#endif

typedef struct geninfotype {
  block_type       *entry;
  int              level;
  PedInfo          ped;
  array_table_type *array_table;
 } gen_info_type;

EXTERN(void, sr_prune_graph,(AST_INDEX root,int level,
			     gen_info_type *gen_info));

#endif
