/* $Id: cfg_dump.C,v 1.1 1997/06/25 15:03:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 *  -- cfg_dump.c
 *
 *      Routines for printing out control flow graphs.
 *
 *              -- Paul Havlak
 */

#include <libs/moduleAnalysis/cfg/cfg_private.h>
#include <libs/moduleAnalysis/cfg/tarjan.h>
#include <libs/moduleAnalysis/cfg/cfg_utils.h>

#define ALL_SUBGRAPHS 0

#define printSsa false
#define printVals false

static char *instType[] = {"GLOBAL", "PROGRAM", "FUNCTION",
    "SUBROUTINE", "BLOCK DATA", "FRAGMENT"};

STATIC(void, cfg_dump_node,(CfgInstance cfg, CfgNodeId i));

/*
 *  Print out control flow graph.
 *  if printSsa,
 *	also print static single assignment form.
 *	if printVals, also print value numbers
 *  fi
 */
void cfg_dump(CfgInstance cfg)
{
    CfgNodeId i;

    printf("\nDumping CFG for %s:\n", string_table_get_text(cfg->name));

    for (i = 0;
	 (i < f_curr_size((Generic) cfg->cfgNodes)) &&
	 !(CFG_node(cfg, i)->freed);
	 i++)
    {
	cfg_dump_node(cfg, i);
    }
}


static void sorted_dump_walk(CfgInstance cfg, CfgNodeId i)
{
    CfgNodeId inner;

    cfg_dump_node(cfg, i);

    for (inner = TARJ_inners(cfg->intervals, i);
	 inner != CFG_NIL;
	 inner = TARJ_next(cfg->intervals, inner))
     {
	 sorted_dump_walk(cfg, inner);
     }
}



void cfg_sorted_dump(CfgInstance cfg)
{
    printf("\nDumping sorted CFG for %s:\n", string_table_get_text(cfg->name));

    tarj_sort(cfg);

    sorted_dump_walk(cfg, cfg->start);
}


static void cfg_dump_node(CfgInstance cfg, CfgNodeId i)
{
    CfgNode   *n;
    CfgEdgeId p;
    CfgClient client;

    printf("\nnode %d:\n", i);
    n = CFG_node(cfg, i);

    printf("    asttype %s, fanin %d, fanout %d, label %d\n",
	   get_AST_type_text(n->astnode),
	   n->fanIn, n->fanOut, n->lbldef);

    if (cfg->predom)
	printf("\tPredominator : %d\n", dom_idom(cfg->predom, i));
    if (cfg->postdom)
	printf("\tPostdominator: %d\n", dom_idom(cfg->postdom, i));
    if (cfg->intervals)
	printf("\tHeader       : %d\n", tarj_outer(cfg->intervals, i));
    if (cfg->topMap)
	printf("\tTop number   : %d\n", (cfg->topMap)[i]);
    if (cfg->cfgCdEdges)
    {
	CfgEdgeId cd;

	if (CFG_node(cfg, i)->cdIns != CFG_NIL)
	{
	    printf("\tCD preds: ");
	    for (cd = CFG_node(cfg, i)->cdIns;
		 cd != CFG_NIL;
		 cd = CFG_cdedge(cfg, cd)->inNext)
		printf("%d/%d ",
		       CFG_cdedge(cfg, cd)->src,
		       CFG_cdedge(cfg, cd)->label);
	    printf("\n");
	}
	if (CFG_node(cfg, i)->cdOuts != CFG_NIL)
	{
	    printf("\tCD succs: ");
	    for (cd = CFG_node(cfg, i)->cdOuts;
		 cd != CFG_NIL;
		 cd = CFG_cdedge(cfg, cd)->outNext)
		printf("%d/%d ",
		       CFG_cdedge(cfg, cd)->dest,
		       CFG_cdedge(cfg, cd)->label);
	    printf("\n");
	}
    }
    
    if (!n->fanOut)
	printf("\tno CFG out edges\n");
    else {
	printf("\tCFG out edges:\n");
	for (p = n->outs; p != CFG_NIL; p = CFG_edge(cfg, p)->outNext)
	{
	    printf("\t\t%d\tto %d\tlabel %d\n",
		   p, CFG_edge(cfg, p)->dest, CFG_edge(cfg, p)->label);
	    for (client = cfg->cfgGlobals->firstClient;
		 client;
		 client = client->next)
	    {
		if (client->dump_edge)
		    client->dump_edge(client->handle, cfg, p);
	    }
	}
    }

    for (client = cfg->cfgGlobals->firstClient;
	 client;
	 client = client->next)
    {
	if (client->dump_node)
	    client->dump_node(client->handle, cfg, i);
    }
}


static int cd_chase_max(CfgInstance cfg, CfgNodeId cn, 
                        CfgNodeId stopper)
{
    int maxChase = 0;
    CfgEdgeId cd;

    for (cd = CFG_node(cfg,cn)->cdIns;
	 cd != CFG_NIL;
	 cd = CFG_cdedge(cfg, cd)->inNext)
    {
	CfgNodeId src = CFG_cdedge(cfg,cd)->src;

	if ((CFG_cdedge(cfg, cd)->level > 0) ||
	    (src == stopper))
	{
	    continue;
	}

	maxChase = max(maxChase,
		       cd_chase_max(cfg,
				    CFG_cdedge(cfg,cd)->src,
				    stopper));
    }
    return (maxChase + 1);
}

void cfg_stats(CfgInstance cfg)
{
    CfgNodeId cn;
    int branchCount = 0;
    int branchMax   = 0;
    int branchSum   = 0;
    int branchSumSq = 0;
    int cuMax   = 0;
    int cuSum   = 0;
    int cuSumSq = 0;
    int mgcbMax = 0;
    int mgcbSum = 0;
    int mergeCount = 0;
    int loopCount  = 0;
    int cxclMax    = 0;
    int cxclSum    = 0;
    int scbcxMax   = 0;
    int scbcxSum   = 0;

    int *loopExits  = NULL;
    int *loopBranches = NULL;
    TarjTree intervals = cfg_get_intervals(cfg);
    DomTree  postdom   = cfg_get_postdom(cfg);
    DomTree  predom    = cfg_get_predom(cfg);
    int csize = f_curr_size((Generic) cfg->cfgNodes);
    int oldCfgSize = 0, newCfgSize = 0;

    if (!(cfg->cfgNodes)) return;

    for (cn = cfg_get_first_node(cfg);
	 cn != CFG_NIL;
	 cn = cfg_get_next_node(cfg, cn))
    {
	newCfgSize++;
	if (!is_null_node(CFG_node(cfg, cn)->astnode))
	    oldCfgSize++;
    }
    printf("\tCFG.nodes.old:\t%10d\n", oldCfgSize);
    printf("\tCFG.nodes.new:\t%10d\n", newCfgSize);
    printf("\tCFG.edges:\t%10d\n", f_curr_size((Generic) cfg->cfgEdges));

    loopExits = (int *) get_mem(csize*sizeof(int),
				"cfg_stats: loopExits");
    int_set(loopExits, csize, 0);
    loopBranches = (int *) get_mem(csize*sizeof(int),
				   "cfg_stats: loopBranches");
    int_set(loopBranches, csize, 0);

    for (cn = cfg_get_first_node(cfg);
	 cn != CFG_NIL;
	 cn = cfg_get_next_node(cfg,cn))
    {
	int arity = CFG_node(cfg,cn)->fanOut;
	int inarity = CFG_node(cfg,cn)->fanIn;
	int edge;

	if (!cfg_is_reachable(cfg,cn))
	    continue;

	if (arity > 1) /* branch */
	{
	    CfgNodeId loop = tarj_outer(intervals, cn);

	    branchCount++;
	    branchMax = max(branchMax, arity);
	    branchSum += arity;
	    branchSumSq += arity * arity;

	    if ((loop != CFG_NIL) && (loop != cfg->start))
	    {
		if (!dom_is_dom(postdom, loop, cn))
		{
		    loopBranches[loop] += arity;
		}
	    }
	}
	if (inarity > 1) /* merge */
	{
	    /*
	     *  mgcb a measure of merges cross branching
	     */
	    int mgcb = inarity * CFG_node(cfg, dom_idom(predom, cn))->fanOut;
	    /*
	     *  cu a measure of unstructuredness
	     */
	    int cuMerge = 0;
	    CfgEdgeId edge;

	    mergeCount++;
	    mgcbMax = max(mgcbMax, mgcb);
	    mgcbSum += mgcb;

	    for (edge = CFG_node(cfg,cn)->ins;
		 edge != CFG_NIL;
		 edge = CFG_edge(cfg,edge)->inNext)
	    {
		cuMerge = max(cuMerge,
			      cd_chase_max(cfg, CFG_edge(cfg, edge)->src,
					   dom_idom(predom, cn)));
	    }
	    cuMax = max(cuMax, cuMerge);
	    cuSum += cuMerge;
	    cuSumSq += cuMerge * cuMerge;
	}
	for (edge = CFG_node(cfg,cn)->outs;
	     edge != CFG_NIL;
	     edge = CFG_edge(cfg,edge)->outNext)
	{
	    if (tarj_exits(intervals, cn, CFG_edge(cfg,edge)->dest))
	    {
		loopExits[tarj_outer(intervals, cn)]++;
	    }
	}
    }
    for (cn = cfg_get_first_node(cfg);
	 cn != CFG_NIL;
	 cn = cfg_get_next_node(cfg,cn))
    {
	if (!cfg_is_reachable(cfg,cn))
	    continue;

	if ((tarj_type(intervals, cn) == TARJ_INTERVAL) ||
	    (tarj_type(intervals, cn) == TARJ_IRREDUCIBLE))
	{
	    int cxcl = loopExits[cn] * tarj_level(intervals, cn);
	    int scbcx = loopExits[cn] * loopBranches[cn];

	    loopCount++;
	    cxclMax = max(cxclMax, cxcl);
	    cxclSum += cxcl;

	    scbcxMax = max(scbcxMax, scbcx);
	    scbcxSum += scbcx;
	}
    }

    printf("\tCFG.branches:\t%10d\n", branchCount);
    printf("\tCFG.branchMax:\t%10d\n", branchMax);
    printf("\tCFG.branchSum:\t%10d\n", branchSum);
    printf("\tCFG.branchSumSq:\t%10d\n", branchSumSq);

    printf("\tCFG.merges:\t%10d\n", mergeCount);
    printf("\tCFG.mgcbMax:\t%10d\n", mgcbMax);
    printf("\tCFG.mgcbSum:\t%10d\n", mgcbSum);
    printf("\tCFG.cuMax:\t%10d\n", cuMax);
    printf("\tCFG.cuSum:\t%10d\n", cuSum);
    printf("\tCFG.cuSumSq:\t%10d\n", cuSumSq);

    printf("\tCFG.loops:\t%10d\n", loopCount);
    printf("\tCFG.cxclMax:\t%10d\n", cxclMax);
    printf("\tCFG.cxclSum:\t%10d\n", cxclSum);
    printf("\tCFG.scbcxMax:\t%10d\n", scbcxMax);
    printf("\tCFG.scbcxSum:\t%10d\n", scbcxSum);

    free_mem((void*) loopExits);
    free_mem((void*) loopBranches);
}
