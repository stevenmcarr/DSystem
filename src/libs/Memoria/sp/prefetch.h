/* $Id: prefetch.h,v 1.9 1997/11/19 14:45:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef prefetch_h
#define prefetch_h

#include <libs/support/memMgmt/Arena.h>
#include <libs/Memoria/include/PrefetchList.h>
#include <libs/Memoria/include/UniformlyGeneratedSets.h>

typedef struct intinfotype {
  SymDescriptor symtab;
  PedInfo       ped;
  int           loop;
  model_loop    *loop_data;
  PrefetchList  *SPLinePrefetches;
  PrefetchList  *DPLinePrefetches;
  PrefetchList  *NoLocality;
  PrefetchList  *TempLocality;
  PrefetchList  *SpatLocality;
  PrefetchList  *WordPrefetches;
  UniformlyGeneratedSets *UGS;
 } locality_info_type;

typedef struct spcycletype {
  int MemCycles;
  int FlopCycles;
  PedInfo ped;
 } SPCycleInfoType;

typedef struct prefetchinfotype {
  PedInfo ped;
  SymDescriptor symtab;
} PrefetchInfoType;

#endif
