/* $Id: interchange.h,v 1.5 1993/06/15 14:04:33 carr Exp $ */

#ifndef interchange_h
#define interchange_h

#ifndef fortsym_h
#include <fort/fortsym.h>
#endif

#ifndef dp_h
#include <dp.h>
#endif

#ifndef mh_h
#include <mh.h>
#endif

#ifndef list_h
#include <misc/list.h>
#endif

#ifndef Arena_h
#include <Arena.h>
#endif

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
