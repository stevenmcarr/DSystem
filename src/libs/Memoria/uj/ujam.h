/* $Id: ujam.h,v 1.2 1992/10/03 15:50:14 rn Exp $ */
#ifndef ujam_h
#define ujam_h

#include <Arena.h>

typedef struct {
  int     unroll_level,
          num_loops,
          num_do;
  PedInfo ped;
  SymDescriptor symtab;
  arena_type    *ar;
 } loop_info_type;

#endif 
