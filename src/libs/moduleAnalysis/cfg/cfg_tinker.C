/* $Id: cfg_tinker.C,v 1.1 1997/06/25 15:03:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *  cfg_tinker.c
 *
 *  Add extra CFG nodes and edges around loops, to simplify construction
 *  of Gated Single-Assignment form.
 *					paco, June 1992
 *
 *  Name changed from cfg_loopy to cfg_tinker.  Added code by Reinhard
 *  von Hanxleden for splitting edges.	paco, April 1993
 */

#include <libs/moduleAnalysis/cfg/cfg_private.h>
#include <libs/moduleAnalysis/cfg/cfg_ids.h>
#include <libs/moduleAnalysis/cfg/tarjan.h>
#include <libs/moduleAnalysis/cfg/dtree.h>
#include <libs/moduleAnalysis/cfg/cfg_nodes.h>
#include <libs/support/stacks/xstack.h>

STATIC(void, split_node,(CfgInstance cfg, CfgNodeId stmt, 
                         Boolean doPostBody));
STATIC(void, split_edge,(CfgInstance cfg, CfgEdgeId edge));
STATIC(void, add_edge,(CfgInstance cfg, CfgNodeId New, 
                       CfgNodeId stmt));
STATIC(void, move_next_edge,(CfgInstance cfg, CfgEdgeId prevEdge,
                             CfgNodeId newSink));
STATIC(void, enlist_loops_and_exits,(CfgInstance cfg, 
                                     Stack needPreHeader,
                                     Stack needPostBody,
                                     Stack loopExitEdges));
STATIC(void, enlist_critical_edges,(CfgInstance cfg, 
                                    Stack criticalEdges));

/*
 *  Because we'll be adding nodes without updating the Tarjan interval
 *  structure, we need to save some information about whether an
 *  edge was a backedge or not.
 *
 *  Use the "executable" flag in the CFG_edge structure.
 */
#define CFG_is_backedge(cfg, edge)	(CFG_edge(cfg, edge)->exec)

void cfg_add_loop_nodes(CfgInstance cfg)
{
    CfgNodeId stmt;
    CfgEdgeId edge, edgeCount;

    Stack needPreHeader;	/* loop headers w/ multiple forward inedges */
    Stack needPostBody;		/* loop headers w/ multiple backward inedges */
    Stack loopExitEdges;	/* edge that leaves a Tarjan interval */

    needPreHeader = stack_create(sizeof(CfgNodeId));
    needPostBody  = stack_create(sizeof(CfgNodeId));
    loopExitEdges = stack_create(sizeof(CfgEdgeId));

    /*
     *  Make the list of nodes and edges to split
     */
    enlist_loops_and_exits(cfg, needPreHeader, needPostBody, loopExitEdges);

    /*
     *  Do the real work
     */
    while (stack_pop(needPreHeader, &stmt))
    {
	split_node(cfg, stmt, /* doPostBody = */ false);
    }

    while (stack_pop(needPostBody, &stmt))
    {
	split_node(cfg, stmt, /* doPostBody = */ true);
    }

    while (stack_pop(loopExitEdges, &edge))
    {
	if (CFG_node(cfg, CFG_edge(cfg, edge)->dest)->fanIn > 1)
	    split_edge(cfg, edge);
    }

    /*
     *  Cleanup
     */
    stack_destroy(needPreHeader);
    stack_destroy(needPostBody);
    stack_destroy(loopExitEdges);

    edgeCount = f_curr_size((Generic) cfg->cfgEdges);

    for (edge = 0; edge < edgeCount; edge++)
	/*
	 *  Reset "exec" flag...
	 */
	CFG_is_backedge(cfg, edge) = false;

    /*
     *  Rebuild topological order, interval, pre and postdom trees
     */
    if (cfg->topMap)
    {
	free_mem((void*) cfg->topMap);
	cfg->topMap = NULL;
    }
    tarj_build(cfg);
    if (cfg->predom)  dom_build(cfg, /* forward = */ true);	/* predom */
    if (cfg->postdom) dom_build(cfg, /* forward = */ false);	/* postdom */
    cfg_free_all_cds(cfg);
}


static void enlist_loops_and_exits(CfgInstance cfg, 
                                   Stack needPreHeader, 
                                   Stack needPostBody, 
                                   Stack loopExitEdges)
    /*Stack needPreHeader;	loop headers w/ multiple forward inedges */
    /*Stack needPostBody;	loop headers w/ multiple backward inedges */
    /*Stack loopExitEdges;	edge that leaves a Tarjan interval */
{    
    TarjTree intervals = cfg_get_intervals(cfg);/* the tree of natural loops */

    CfgNodeId stmt;
    CfgEdgeId edge;
    
    for (stmt = cfg_get_first_node(cfg);
	 stmt != CFG_NIL;
	 stmt = cfg_get_next_node(cfg, stmt))
    {
	if (!cfg_is_reachable(cfg, stmt) ||
	    !cfg_reaches_end(cfg, stmt))
	{
	    continue;
	}
	if (TARJ_type(intervals, stmt) == TARJ_INTERVAL)
	{
	    int fwrdIns = 0;
	    int backIns = 0;

	    for (edge = CFG_node(cfg, stmt)->ins;
		 edge != CFG_NIL;
		 edge = CFG_edge(cfg, edge)->inNext)
	    {
		CFG_is_backedge(cfg, edge) = cfg_is_backedge(cfg, edge);

		if (CFG_is_backedge(cfg, edge)) backIns++;
		else				fwrdIns++;

		if (tarj_exits(intervals,
			       CFG_edge(cfg, edge)->src,
			       CFG_edge(cfg, edge)->dest) > 0)
		{
		    stack_push(loopExitEdges, &edge);
		}
	    }
	    if (backIns > 1)
		stack_push(needPostBody, &stmt);

	    if (fwrdIns > 1)
		stack_push(needPreHeader, &stmt);
	}
	else
	{
	    for (edge = CFG_node(cfg, stmt)->ins;
		 edge != CFG_NIL;
		 edge = CFG_edge(cfg, edge)->inNext)
	    {
		if (tarj_exits(intervals,
			       CFG_edge(cfg, edge)->src,
			       CFG_edge(cfg, edge)->dest) > 0)
		{
		    stack_push(loopExitEdges, &edge);
		}
	    }
	}	
    }
}

static void split_node(CfgInstance cfg, CfgNodeId stmt, 
                       Boolean doPostBody)
{
    CfgEdgeId prevEdge, edge;
    CfgNodeId New = cfg_node_new_id(cfg);

    add_edge(cfg, New, stmt);

    prevEdge = CFG_node(cfg, stmt)->ins;	/* the edge we just added */

    while (CFG_edge(cfg, prevEdge)->inNext != CFG_NIL)
    {
	edge = CFG_edge(cfg, prevEdge)->inNext;

	if (doPostBody == BOOL(CFG_is_backedge(cfg, edge)))
	{
	    move_next_edge(cfg, prevEdge, New);
	}
	else
	    prevEdge = edge;
    }
}    

static void add_edge(CfgInstance cfg, CfgNodeId New, CfgNodeId stmt)
{
    CfgEdgeId edge;

    edge = cfg_edge_new_id(cfg);
    CFG_edge(cfg,edge)->label = CD_FALLTHROUGH;

    CFG_edge(cfg,edge)->src   = New;
    ++(CFG_node(cfg,New)->fanOut);
    CFG_edge(cfg,edge)->outNext = CFG_node(cfg,New)->outs;
    CFG_node(cfg,New)->outs = edge;

    CFG_edge(cfg,edge)->dest  = stmt;
    ++(CFG_node(cfg,stmt)->fanIn);
    CFG_edge(cfg,edge)->inNext = CFG_node(cfg,stmt)->ins;
    CFG_node(cfg,stmt)->ins = edge;
}

static void move_next_edge(CfgInstance cfg, CfgEdgeId prevEdge, 
                           CfgNodeId newSink)
{
    CfgNodeId oldSink;
    CfgEdgeId edge = CFG_edge(cfg, prevEdge)->inNext;

    oldSink = CFG_edge(cfg, edge)->dest;
    --(CFG_node(cfg, oldSink)->fanIn);
    CFG_edge(cfg, prevEdge)->inNext = CFG_edge(cfg, edge)->inNext;

    CFG_edge(cfg,edge)->dest = newSink;
    ++(CFG_node(cfg,newSink)->fanIn);
    CFG_edge(cfg,edge)->inNext = CFG_node(cfg,newSink)->ins;
    CFG_node(cfg,newSink)->ins = edge;
}

static void split_edge(CfgInstance cfg, CfgEdgeId edge)
{
    CfgEdgeId prevEdge;
    CfgNodeId sink = CFG_edge(cfg, edge)->dest;
    CfgNodeId anExit = cfg_node_new_id(cfg);

    add_edge(cfg, anExit, sink);

    for (prevEdge = CFG_node(cfg, sink)->ins;	/* the edge just inserted */
	 CFG_edge(cfg, prevEdge)->inNext != edge;
	 prevEdge = CFG_edge(cfg, prevEdge)->inNext)
	;
    move_next_edge(cfg, prevEdge, anExit);
}


/*
 *  Reinhard's code
 *
 *  Add extra CFG nodes and edges to eliminate "critical edges".
 *
 *  Critical edges lead from nodes w/ multiple successors to nodes with
 *  multiple predecessors.  This can result, for example, from GOTO's,
 *  or even from just an IF without an ELSE.
 *
 *  4/15/93 RvH: code is patterned after cfg_loopy.c
 */
/*static void enlist_critical_edges();*/

void cfg_split_critical_edges(CfgInstance cfg)
{
    CfgNodeId stmt;
    CfgEdgeId edge, edgeCount;
  
    Stack criticalEdges;
  
    criticalEdges = stack_create(sizeof(CfgEdgeId));
  
    /*
     *  Make the list of edges to split
     */
    enlist_critical_edges(cfg, criticalEdges);
  
    /*
     *  Do the real work
     */
    while (stack_pop(criticalEdges, &edge))
    {
	if (CFG_node(cfg, CFG_edge(cfg, edge)->dest)->fanIn > 1)
	    split_edge(cfg, edge);
    }
  
    /*
     *  Cleanup
     */
    stack_destroy(criticalEdges);

    edgeCount = f_curr_size((Generic) cfg->cfgEdges);

    /*
     *  Don't need to reset the is_backedge/exec flag because we
     *  didn't change it...
     */

    /*
     *  Rebuild topological order, interval, pre and postdom trees
     */
    if (cfg->topMap)
    {
	free_mem((void*) cfg->topMap);
	cfg->topMap = NULL;
    }
    tarj_build(cfg);
    if (cfg->predom)  dom_build(cfg, /* forward = */ true);	/* predom */
    if (cfg->postdom) dom_build(cfg, /* forward = */ false);	/* postdom */
    cfg_free_all_cds(cfg);
}


static void enlist_critical_edges(CfgInstance cfg, 
                                  Stack criticalEdges)
{    
    CfgNodeId stmt;
    CfgEdgeId edge;
  
    for (stmt = cfg_get_first_node(cfg);
	 stmt != CFG_NIL;
	 stmt = cfg_get_next_node(cfg, stmt))
    {
	for (edge = CFG_node(cfg, stmt)->ins;
	     edge != CFG_NIL;
	     edge = CFG_edge(cfg, edge)->inNext)
	{
	    if ((cfg_node_fanout(cfg, CFG_edge(cfg, edge)->src) > 1) &&
		(cfg_node_fanin(cfg, CFG_edge(cfg, edge)->dest) > 1))
	    {
		stack_push(criticalEdges, &edge);
	    }
	}
    }
}
