/* $Id: prefetch.h,v 1.7 1997/03/27 20:26:39 carr Exp $ */
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
  PrefetchList  *WordPrefetches;
  UniformlyGeneratedSets *UGS;
 } locality_info_type;

typedef struct spcycletype {
  int MemCycles;
  int FlopCycles;
  PedInfo ped;
 } SPCycleInfoType;

#endif
