/* $Id: moderate.h,v 1.5 1993/06/21 13:47:03 carr Exp $ */

#ifndef moderate_h
#define moderate_h

#include <stdio.h>

#ifndef general_h
#include <general.h>
#endif

#ifndef scalar_h
#include <scalar.h>
#endif

#ifndef cgen_set_h
#include <cgen_set.h>
#endif

#ifndef list_h
#include <misc/list.h>
#endif

#ifndef dp_h
#include <dp.h>
#endif

#ifndef Arena_h
#include <misc/Arena.h>
#endif

#ifndef name_h
#include <name.h>
#endif

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
