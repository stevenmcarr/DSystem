/* $Id: name.h,v 1.2 1992/10/03 15:49:24 rn Exp $ */
#ifndef name_h
#define name_h

typedef struct {
  UtilList  *nlist;
  AST_INDEX gen;
  Boolean   opt_will_allocate;
  int       benefit,
            cost;
  float     ratio;
 }  name_node_type;

typedef struct {
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
