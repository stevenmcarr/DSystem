/* $Id: interchange.h,v 1.3 1992/12/07 10:15:10 carr Exp $ */

#ifndef interchange_h
#define interchange_h

#include <Arena.h>

typedef struct indexinfotype{
  int           index;
  SymDescriptor symtab;
 } index_info_type;

typedef struct intinfotype {
  SymDescriptor symtab;
  PedInfo       ped;
  model_loop    *loop_data;
  UtilList      *loop_list;
 } int_info_type;

typedef struct updinfotype {
  PedInfo    ped;
  heap_type  *heap;
  model_loop *loop_data;
  int        num_loops;
 } upd_info_type;

typedef struct levelinfotype {
  int level;
  PedInfo ped;
 } level_info_type;

#define INDEX  "mh: index"
#define PRE_VAL  1
#define POST_VAL 2

typedef enum {MISS, IHIT, AHIT,REGISTER} RefType;

#endif
