#ifndef CacheAnalysis_h
#define CacheAnalysis_h

#include <general.h>
#include <mh.h>
#include <dp.h>
#include <dg.h>
#include <misc/list.h>
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
