#ifndef CacheAnalysis_h
#define CacheAnalysis_h

#include <general.h>
#include <mh.h>
#include <dp.h>

typedef struct CacheInfoStruct {
  model_loop *loop_data;
  int        loop;
  PedInfo    ped;
 } CacheInfoType;

#endif
