/* $Id: prune.h,v 1.3 1992/12/07 10:20:42 carr Exp $ */

#ifndef prune_h
#define prune_h

typedef struct geninfotype {
  block_type       *entry;
  int              level;
  PedInfo          ped;
  array_table_type *array_table;
 } gen_info_type;

EXTERN(void, sr_prune_graph,(AST_INDEX root,int level,
				     gen_info_type *gen_info));

#endif
