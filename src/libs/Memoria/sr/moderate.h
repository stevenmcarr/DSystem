/* $Id: moderate.h,v 1.7 1997/03/27 20:27:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef moderate_h
#define moderate_h

#include <stdio.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef scalar_h
#include <libs/Memoria/sr/scalar.h>
#endif

#ifndef cgen_set_h
#include <libs/Memoria/include/cgen_set.h>
#endif

#ifndef list_h
#include <libs/support/lists/list.h>
#endif

#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif

#ifndef Arena_h
#include <libs/support/memMgmt/Arena.h>
#endif

#ifndef name_h
#include <libs/Memoria/sr/name.h>
#endif

#include <libs/Memoria/include/LoopStats.h>

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
				   FILE *logfile,arena_type *ar,
				   LoopStatsType *LoopStats));

#endif
