/* $Id: ujam.h,v 1.6 1993/07/20 16:35:17 carr Exp $ */

#ifndef ujam_h
#define ujam_h

#ifndef Arena_h
#include <misc/Arena.h>
#endif

#ifndef dp_h
#include <dp.h>
#endif

#ifndef fortsym_h
#include <fort/fortsym.h>
#endif

#include <LoopStats.h>

typedef struct loopinfotype {
  int     unroll_level,
          num_loops,
          num_do;
  PedInfo ped;
  SymDescriptor symtab;
  arena_type    *ar;
  LoopStatsType *LoopStats;
 } loop_info_type;

#endif 
