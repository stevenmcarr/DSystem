/* $Id: moderate.h,v 1.3 1992/12/07 10:20:36 carr Exp $ */

#ifndef moderate_h
#define moderate_h

#include <scalar.h>

#define GREEDY   0

typedef struct regelement {
  int   value;
  Set   in_pack;
 } reg_element;

typedef struct gennodetype {
  int cost,
      ratio;
 } gen_node_type;

typedef struct heaptype {
  int index;
  name_node_type *name;
  UtilNode       *lnode;
 }heap_type;

EXTERN(void, sr_moderate_pressure,(PedInfo ped,UtilList *glist,
					   int free_regs,Boolean *red,
					   array_table_type *array_table,
					   FILE *logfile,arena_type *ar));

#endif
