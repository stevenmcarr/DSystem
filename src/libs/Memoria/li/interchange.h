/* $Id: interchange.h,v 1.8 1997/03/27 20:25:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef interchange_h
#define interchange_h

#ifndef fortsym_h
#include <libs/frontEnd/fortTree/fortsym.h>
#endif

#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif

#ifndef mh_h
#include <libs/Memoria/include/mh.h>
#endif

#ifndef list_h
#include <libs/support/lists/list.h>
#endif

#ifndef Arena_h
#include <libs/support/memMgmt/Arena.h>
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

#endif
