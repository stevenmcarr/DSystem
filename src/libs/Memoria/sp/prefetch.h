/* $Id: prefetch.h,v 1.4 1995/06/29 12:35:55 carr Exp $ */

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

#endif
