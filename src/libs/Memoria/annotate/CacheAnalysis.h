#ifndef CacheAnalysis_h
#define CacheAnalysis_h

#include <general.h>
#include <mh.h>
#include <dp.h>
#include <misc/list.h>
#include <malloc.h>

typedef struct CacheInfoStruct {
  model_loop *loop_data;
  int        loop;
  int        RefNum;
  PedInfo    ped;
 } CacheInfoType;

typedef struct DepInfoStruct {
  int ReferenceNumber;
  UtilList *DependenceList;
  LocalityType Locality;
 } DepInfoType;         /* copy in a2i_lib/ai.h */

#define DepInfoPtr(n) \
   ((DepInfoType *)ast_get_scratch(n))

#define CreateDepInfoPtr(n) \
   ast_put_scratch(n,malloc(sizeof(DepInfoType)))

#endif
