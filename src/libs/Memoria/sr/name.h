/* $Id: name.h,v 1.5 1993/06/21 13:47:09 carr Exp $ */

#ifndef name_h
#define name_h

#ifndef general_h
#include <general.h>
#endif

#ifndef list_h
#include <misc/list.h>
#endif

#ifndef ast_h
#include <ast.h>
#endif

#ifndef dg_h
#include <dg.h>
#endif

#ifndef dp_h
#include <dp.h>
#endif

#ifndef Arena_h
#include <misc/Arena.h>
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
