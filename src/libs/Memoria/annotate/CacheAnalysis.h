/* $Id: CacheAnalysis.h,v 1.18 1999/07/22 18:08:52 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef CacheAnalysis_h
#define CacheAnalysis_h

#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#include <libs/support/lists/list.h>
#include <libs/Memoria/include/la.h>
#include <libs/Memoria/annotate/AddressEquivalenceClassSet.h>
#include <malloc.h>

typedef struct CacheInfoStruct {
  model_loop *loop_data;
  int        loop;
  int        RefNum;
  PedInfo    ped;
  SymDescriptor symtab;
  arena_type *ar;
  char       **IVar;
  DataReuseModel *ReuseModel;
  AddressEquivalenceClassSet *AECS;
 } CacheInfoType;

typedef struct DepInfoStruct {
  int ReferenceNumber;
  UtilList *DependenceList;
  LocalityType Locality;
  Boolean UsePrefetchingLoad;
  AST_INDEX AddressLeader;
  AST_INDEX FirstInLoop;
  int       Offset;
  int       StmtNumber;
  int       PrefetchDistance;
  AST_INDEX PrefetchOffsetAST;
 } DepInfoType;         /* copy in a2i_lib/ai.h */

typedef struct depstruct {
  AST_INDEX Reference;
  int ReferenceNumber;
  char DType;
  int Distance;
 } DepStruct; /* copy in a2i_lib/ai.h */

typedef struct cachecycleinfostruct {
    int             IntCycles;
    int             FPCycles;
    PedInfo         ped;
    DataReuseModel *ReuseModel;
} CacheCycleInfoType;

typedef struct prefetchdatatypestruct {
   int LoopCycles;
   int PrefetchLatency;
   int LineSize;
} PrefetchDataType;

#define DepInfoPtr(n) \
   ((DepInfoType *)ast_get_scratch(n))

#define CreateDepInfoPtr(n) \
   ast_put_scratch(n,(SCRATCH)malloc(sizeof(DepInfoType)))

extern int aiCache;
extern int aiSpecialCache;
#endif
