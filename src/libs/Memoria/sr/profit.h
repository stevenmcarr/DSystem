/* $Id: profit.h,v 1.6 1997/03/27 20:27:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef profit_h
#define profit_h

#ifndef scalar_h
#include <libs/Memoria/sr/scalar.h>
#endif

#ifndef cgen_set_h
#include <libs/Memoria/include/cgen_set.h>
#endif

#ifndef block_h
#include <libs/Memoria/include/block.h>
#endif

#ifndef Arena_h
#include <libs/support/memMgmt/Arena.h>
#endif

typedef struct profinfotype {
  Set              pavset;
  float            prob;
  array_table_type *array_table;
 } prof_info_type;

EXTERN(void, sr_perform_profit_analysis,(flow_graph_type flow_graph,
					 int size,
					 array_table_type *array_table,
					 arena_type *ar));

#endif
