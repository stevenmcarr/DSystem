/* $Id: CacheAnalysis.h,v 1.12 1997/11/10 21:20:42 carr Exp $ */
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
#include <malloc.h>

typedef struct CacheInfoStruct {
  model_loop *loop_data;
  int        loop;
  int        RefNum;
  PedInfo    ped;
  SymDescriptor symtab;
  arena_type *ar;
  char       **IVar;
  Boolean    HasSelfSpatial;
  DataReuseModel *ReuseModel;
 } CacheInfoType;

typedef struct DepInfoStruct {
  int ReferenceNumber;
  UtilList *DependenceList;
  LocalityType Locality;
  Boolean IsGroupSpatialTrailer;
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
extern int aiSpecialCache;
#endif
