#ifndef CacheAnalysis_h
#define CacheAnalysis_h

#include <general.h>
#include <mh.h>
#include <dp.h>
#include <dg.h>
#include <misc/list.h>
#include <malloc.h>
#include <UniformlyGeneratedSets.h>

typedef struct CacheInfoStruct {
  model_loop *loop_data;
  int        loop;
  int        RefNum;
  PedInfo    ped;
  SymDescriptor symtab;
  arena_type *ar;
  UniformlyGeneratedSets *UGS;
  char       **IVar;
  Boolean    HasSelfSpatial;
 } CacheInfoType;

typedef struct DepInfoStruct {
  int ReferenceNumber;
  UtilList *DependenceList;
  LocalityType Locality;
 } DepInfoType;         /* copy in a2i_lib/ai.h */

typedef struct depstruct {
  int ReferenceNumber;
  char DType;
  int Distance;
 } DepStruct; /* copy in a2i_lib/ai.h */

#define DepInfoPtr(n) \
   ((DepInfoType *)ast_get_scratch(n))

#define CreateDepInfoPtr(n) \
   ast_put_scratch(n,(SCRATCH)malloc(sizeof(DepInfoType)))

extern int aiCache;
#endif
