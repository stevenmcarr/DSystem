/* $Id: prefetch.h,v 1.3 1995/03/13 15:11:50 carr Exp $ */

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
