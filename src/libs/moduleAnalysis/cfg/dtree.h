/* $Id: dtree.h,v 3.3 1997/03/11 14:35:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- dtree.h
 *
 *       void dom_build(cfg, forward_direction)
 *	     CfgInstance cfg;
 *	     Boolean forward_direction;
 *	   	   construct dominator tree for CfgInstance
 *		   forward? build predom tree : build postdom tree
 *
 *       void dom_free(cfg, forward_direction)
 *	     CfgInstance cfg;
 *	     Boolean forward_direction;
 *		   Free the dominator tree array and nullify the pointer.
 */


#ifndef dtree_h
#define dtree_h
/*
 *  Dominator tree will consist of an array of DomEntry's,
 *  same size as the cfgNodes table, indexed by CfgNodeId.
 *  
 *  Since we are indexing by the node name, don't need it as an explicit field.
 */
typedef struct dominator_entry_struct {
    CfgNodeId idom;		/* id of immediate dominator */
    CfgNodeId kids;		/* id of first node w/ this idom */
    CfgNodeId next;		/* id of next node with same idom */
    int prenum;			/* preorder number */
    int last;			/* preorder number of last descendent */
} DomEntry;  /* *DomTree */

#define DOM_idom(dt,name)	(dt[name].idom)
#define DOM_kids(dt,name)	(dt[name].kids)
#define DOM_next(dt,name)	(dt[name].next)
#define DOM_is_dom(dt,a,b) \
    ( ( dt[a].prenum <= dt[b].prenum ) && \
      ( dt[b].prenum <= dt[a].last ) \
    )

EXTERN(void, dom_build, (CfgInstance cfg, Boolean direction));
EXTERN(void, dom_free, (CfgInstance cfg, Boolean direction));

#endif
