/* $Id: ujam.h,v 1.7 1997/03/27 20:28:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef ujam_h
#define ujam_h

#ifndef Arena_h
#include <libs/support/memMgmt/Arena.h>
#endif

#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif

#ifndef fortsym_h
#include <libs/frontEnd/fortTree/fortsym.h>
#endif

#include <libs/Memoria/include/LoopStats.h>

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
