#ifndef DepGraphStat_h
#define DepGraphStat_h


#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#include <libs/support/lists/list.h>
#include <malloc.h>

typedef struct DependenceInfoStruct {
  model_loop *loop_data;
  int        loop;
  PedInfo    ped;
  SymDescriptor symtab;
  arena_type *ar;
  LoopStatsType *LoopStats; 
 } DependenceInfoType;

#endif
