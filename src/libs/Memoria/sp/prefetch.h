/* $Id: prefetch.h,v 1.2 1993/06/21 13:48:32 carr Exp $ */

#ifndef prefetch_h
#define prefetch_h

#include <misc/Arena.h>
#include <PrefetchList.h>

typedef struct intinfotype {
  SymDescriptor symtab;
  PedInfo       ped;
  int           loop;
  model_loop    *loop_data;
  PrefetchList  *SPLinePrefetches;
  PrefetchList  *DPLinePrefetches;
  PrefetchList  *WordPrefetches;
 } locality_info_type;

typedef struct cycletype {
  int MemCycles;
  int FlopCycles;
  PedInfo ped;
 } CycleInfoType;

#endif
