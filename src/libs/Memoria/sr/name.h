/* $Id: name.h,v 1.3 1992/12/07 10:20:38 carr Exp $ */

#ifndef name_h
#define name_h

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
