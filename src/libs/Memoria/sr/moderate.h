/* $Id: moderate.h,v 1.2 1992/10/03 15:49:21 rn Exp $ */
#ifndef moderate_h
#define moderate_h

#include <scalar.h>

#define GREEDY   0

typedef struct {
  int   value;
  Set   in_pack;
 } reg_element;

typedef struct {
  int cost,
      ratio;
 } gen_node_type;

typedef struct {
  int index;
  name_node_type *name;
  UtilNode       *lnode;
 }heap_type;

EXTERN(void, sr_moderate_pressure,(PedInfo ped,UtilList *glist,
					   int free_regs,Boolean *red,
					   array_table_type *array_table,
					   FILE *logfile,arena_type *ar));

#endif
