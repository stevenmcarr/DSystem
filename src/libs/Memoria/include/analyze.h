/* $Id: analyze.h,v 1.7 1997/03/27 20:24:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef analyze_h
#define analyze_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif
#ifndef mh_h
#include <libs/Memoria/include/mh.h>
#endif
#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif
#ifndef fortsym_h
#include <libs/frontEnd/fortTree/fortsym.h>
#endif
#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

typedef struct buildinfotype {
  int        parent,
             last_loop,
             last_stack[MAX_LEVEL],
             loop_num,
             level,
             stack_top;
  model_loop *loop_data;
  PedInfo    ped;
  SymDescriptor symtab;
 } build_info_type;

typedef struct checkinfotype {
  PedInfo ped;
  Boolean illegal;
 } CheckInfoType;

EXTERN(void, ut_analyze_loop,(AST_INDEX root,model_loop *loop_data,
				      int level,PedInfo ped,
				      SymDescriptor symtab));

#define IVAR "ut: ivar"
#endif
