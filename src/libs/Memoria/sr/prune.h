/* $Id: prune.h,v 1.5 1997/03/27 20:27:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef prune_h
#define prune_h

#ifndef block_h
#include <libs/Memoria/include/block.h>
#endif

#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif

#ifndef scalar_h
#include <libs/Memoria/sr/scalar.h>
#endif

#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

typedef struct geninfotype {
  block_type       *entry;
  int              level;
  PedInfo          ped;
  array_table_type *array_table;
 } gen_info_type;

EXTERN(void, sr_prune_graph,(AST_INDEX root,int level,
			     gen_info_type *gen_info));

#endif
