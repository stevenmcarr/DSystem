/* $Id: ujam.h,v 1.4 1992/12/11 11:23:44 carr Exp $ */

#ifndef ujam_h
#define ujam_h

#ifndef Arena_h
#include <Arena.h>
#endif

#ifndef dp_h
#include <dp.h>
#endif

#ifndef fortsym_h
#include <fort/fortsym.h>
#endif

typedef struct loopinfotype {
  int     unroll_level,
          num_loops,
          num_do;
  PedInfo ped;
  SymDescriptor symtab;
  arena_type    *ar;
 } loop_info_type;

#endif 
