/* $Id: cfg_local_cd.C,v 1.1 1997/06/25 15:03:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 *  -- cfg_local_cd.c
 *
 *	Build control dependence predeccessor relation into the CFG
 *
 *		-- paco, June 1992
 */

#include <libs/moduleAnalysis/cfg/cfg_private.h>
#include <libs/moduleAnalysis/cfg/dtree.h>
#include <libs/moduleAnalysis/cfg/tarjan.h>
#include <libs/moduleAnalysis/cfg/cfg_ids.h>

#include <libs/support/time/swatch.h>

extern Swatch cfg_build_cds_sw;

/*
 *  Construction of the control dependence graph, based on results in
 *
 *  Compact Representations for Control Dependence
 *  Cytron, Ferrante & Sarkar SIGPLAN PLDI '90
 *
 *  For each edge out of a node with multiple successors,
 *  we record the source and label, then walk up the postdominator
 *  tree from the sink, adding control dependences while we have not
 *  reached the postdominator of the source.          paco, June 1992
 */

STATIC(void, add_cd_edge,(CfgInstance cfg, CfgNodeId source,
                          CfgNodeId sink, int label, int level));

static void build_cfg_cds(CfgInstance cfg)
{
    DomTree	postdom;
    TarjTree	intervals;
    CfgNodeId	node, chain;
    CfgEdgeId	succ;
    CfgNode	*nodeP;
    int		cfgSize;
    int level;

    postdom = cfg_get_postdom(cfg);

    intervals = cfg_get_intervals(cfg);

    /*
     *  Add special CD_DEFAULT edges from ROOT so that the cd graph 
     *  for each instance will be rooted.
     */
    swatch_start(cfg_build_cds_sw);

    node = cfg->start;
    for (chain = dom_idom(postdom, node);
	 chain != CFG_NIL;
	 chain = dom_idom(postdom, chain))
    {
	add_cd_edge(cfg, node, chain,
		    CD_DEFAULT, LOOP_INDEPENDENT/*, CD_UNCONDITIONAL*/);
    }

    /*
     *  Use f_curr_size of the cfgNodes table (same indexing),
     *  because postdom is a regular array created with the same size.
     */
    cfgSize = f_curr_size((Generic) cfg->cfgNodes);

    for (node = 0; node < cfgSize; node++)
    {
	nodeP = CFG_node(cfg, node);
	/*
	 *  If reachable and multiple control flow successors...
	 */
	if ((nodeP->fanOut > 1) && !(nodeP->freed))

	    for (succ = nodeP->outs; succ != CFG_NIL;
		 succ = CFG_edge(cfg, succ)->outNext) {
		/*
		 *  Add all postdom ancestors until we reach the
		 *  postdominator of the branch node
		 */
		level = LOOP_INDEPENDENT;
		for (chain = CFG_edge(cfg, succ)->dest;
		     (chain != CFG_NIL) &&
		     (chain != dom_idom(postdom, node));
		     chain = dom_idom(postdom, chain)) 
		{
		    if (level == LOOP_INDEPENDENT &&
			(tarj_edge_type(intervals, node, chain)
			 == TARJ_ITERATE))
		    {
			level = tarj_level(intervals, chain);
		    }
		    add_cd_edge(cfg, node, chain,
				CFG_edge(cfg, succ)->label, level);
		}
	    }
    }
    swatch_lap(cfg_build_cds_sw);
}


static void add_cd_edge(CfgInstance cfg, CfgNodeId source, 
                        CfgNodeId sink, int label, int level)
{
    CfgEdgeId edge;

    edge = cfg_cd_edge_new_id(cfg);

    CFG_cdedge(cfg, edge)->src     = source;
    CFG_cdedge(cfg, edge)->dest    = sink;
    CFG_cdedge(cfg, edge)->label   = label;
    CFG_cdedge(cfg, edge)->level   = level;	/* note the abuse here */

    CFG_cdedge(cfg,edge)->outNext  = CFG_node(cfg,source)->cdOuts;
    CFG_node(cfg,source)->cdOuts = edge;

    CFG_cdedge(cfg,edge)->inNext   = CFG_node(cfg,sink)->cdIns;
    CFG_node(cfg,sink)->cdIns    = edge;
}

CfgCdId 
cfg_cd_first_from(CfgInstance cfg, CfgNodeId n)
{
    if (!(cfg->cfgCdEdges))
	build_cfg_cds(cfg);

    return CFG_node(cfg, n)->cdOuts;
}

CfgCdId 
cfg_cd_first_to(CfgInstance cfg, CfgNodeId n)
{
    if (!(cfg->cfgCdEdges))
	build_cfg_cds(cfg);

    return CFG_node(cfg, n)->cdIns;
}

CfgCdId 
cfg_cd_next_from(CfgInstance cfg, CfgCdId e)
{
    return CFG_cdedge(cfg, e)->outNext;
}

CfgCdId 
cfg_cd_next_to(CfgInstance cfg, CfgCdId e)
{
    return CFG_cdedge(cfg, e)->inNext;
}

CfgNodeId
cfg_cd_src(CfgInstance cfg, CfgCdId e)
{
    return CFG_cdedge(cfg, e)->src;
}

CfgNodeId
cfg_cd_dest(CfgInstance cfg, CfgCdId e)
{
    return CFG_cdedge(cfg, e)->dest;
}

int 
cfg_cd_label(CfgInstance cfg, CfgCdId e)
{
    return (int) CFG_cdedge(cfg, e)->label;
}

int 
cfg_cd_level(CfgInstance cfg, CfgCdId e)
{
    return (int) CFG_cdedge(cfg, e)->level;
}

/*
 *  CdSourceType cd_cfg_src_type(cfg, n)
 *
 *  Classifies a cfg node as to the type of control flow it can
 *  originate.
 */
CdBranchType cfg_branch_type(CfgInstance cfg, CfgNodeId n)
{
    if (cfg_node_fanout(cfg, n) > 1)
	return cd_branch_type(cfg_node_to_ast(cfg, n));
    else
	return CD_UNCONDITIONAL;
}
