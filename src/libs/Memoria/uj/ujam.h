/* $Id: ujam.h,v 1.3 1992/12/07 10:23:09 carr Exp $ */

#ifndef ujam_h
#define ujam_h

#include <Arena.h>

typedef struct loopinfotype {
  int     unroll_level,
          num_loops,
          num_do;
  PedInfo ped;
  SymDescriptor symtab;
  arena_type    *ar;
 } loop_info_type;

#endif 
