/* $Id: prune.h,v 1.2 1992/10/03 15:49:31 rn Exp $ */
#ifndef prune_h
#define prune_h

typedef struct {
  block_type       *entry;
  int              level;
  PedInfo          ped;
  array_table_type *array_table;
 } gen_info_type;

EXTERN(void, sr_prune_graph,(AST_INDEX root,int level,
				     gen_info_type *gen_info));

#endif
