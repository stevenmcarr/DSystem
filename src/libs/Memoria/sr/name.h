/* $Id: name.h,v 1.6 1997/03/27 20:27:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef name_h
#define name_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef list_h
#include <libs/support/lists/list.h>
#endif

#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

#ifndef dg_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#endif

#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif

#ifndef Arena_h
#include <libs/support/memMgmt/Arena.h>
#endif

typedef struct namenodetype {
  UtilList  *nlist;
  AST_INDEX gen;
  Boolean   opt_will_allocate;
  int       benefit,
            cost;
  float     ratio;
 }  name_node_type;

typedef struct nameinfotype {
  PedInfo  ped;
  DG_Edge  *dg;
  UtilList *glist;
  arena_type *ar;
 } name_info_type;

EXTERN(void, sr_find_generator,(UtilNode *lnode,DG_Edge *dg,
				PedInfo ped));
EXTERN(void, sr_generate_names,(AST_INDEX root,
				name_info_type *name_info));

#endif
