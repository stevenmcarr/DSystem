/* $Id: prefetch.h,v 1.5 1995/08/18 10:20:56 trsuchyt Exp $ */

#ifndef prefetch_h
#define prefetch_h

#include <misc/Arena.h>
#include <PrefetchList.h>
#include <UniformlyGeneratedSets.h>

typedef struct intinfotype {
  SymDescriptor symtab;
  PedInfo       ped;
  int           loop;
  model_loop    *loop_data;
  PrefetchList  *SPLinePrefetches;
  PrefetchList  *DPLinePrefetches;
  PrefetchList  *WordPrefetches;
  UniformlyGeneratedSets *UGS;
 } locality_info_type;

typedef struct cycletype {
  int MemCycles;
  int FlopCycles;
  PedInfo ped;
 } CycleInfoType;

#endif
