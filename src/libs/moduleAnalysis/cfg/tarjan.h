/* $Id: tarjan.h,v 3.4 1997/03/11 14:35:35 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * 
 * -- tarjan.h
 *  
 *         Header file for Tarjan interval finder (see tarjan.c).
 *         Modeled after dtree.c                      phh 6 Mar 91
 *
 *
 *  void tarj_build(cfg)
 *      CfgInstance cfg;
 *          construct Tarjan intervals for cfg
 *
 *  void tarj_free(cfg)
 *      CfgInstance cfg;
 *          Free the interval tree array and nullify the pointer.
 *
 *  void tarj_renumber(cfg)
 *      CfgInstance cfg;
 *		Updates the prenumbering of the tree.  Useful if some change
 *		has been made to the cfg where one knows how to update the
 *		interval tree (as when adding PREHEADER nodes).
 *
 *  void tarj_sort(cfg)
 *      CfgInstance cfg;
 *		Sorts the children of each loop header according to
 *		topological order.
 */

#ifndef tarjan_h
#define tarjan_h

/*
 *  Interval tree will consist of an array of TarjEntry's,
 *  same size as the cfgNodes table, indexed by CFG_NodeId.
 *
 *  Since we are indexing by the node name, don't need it as an explicit field.
 */
typedef struct tarjan_entry_struct {
    SmallInt  level;		/* nesting depth -- outermost loop is 1 */
    TarjType  type;		/* acyclic, interval or irreducible */
    CfgNodeId outer;		/* id of header of containing interval */
    CfgNodeId inners;		/* id of header of first nested interval*/
    CfgNodeId next;		/* id of next nested header */
    int       prenum;		/* preorder number */
    int       last;		/* number of last descendent */
    CfgNodeId last_id;		/* id of last descendent */
} TarjEntry; /* *TarjTree */

#define TARJ_outer(tarj,name)	(tarj[name].outer)
#define TARJ_inners(tarj,name)	(tarj[name].inners)
#define TARJ_next(tarj,name)	(tarj[name].next)
#define TARJ_level(tarj,name)	(tarj[name].level)
#define TARJ_type(tarj,name)	(tarj[name].type)
#define TARJ_last(tarj,name)	(tarj[name].last_id)
#define TARJ_contains(tarj,a,b)	\
    ( ( tarj[a].prenum <= tarj[b].prenum ) && \
      ( tarj[b].prenum <= tarj[tarj[a].last_id].prenum ) \
    )


EXTERN(void, tarj_build, (CfgInstance cfg));
EXTERN(void, tarj_free, (CfgInstance cfg));
EXTERN(void, tarj_renumber, (CfgInstance cfg));
EXTERN(void, tarj_sort, (CfgInstance cfg));

#endif
