/* $Id: ssa_ids.C,v 3.5 1997/03/11 14:36:12 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- ssa_ids.c
 *
 *        This file contains functions to create/delete SSA nodes.
 *
 */

#include <libs/support/tables/IntegerHashTable.h>
#include <libs/moduleAnalysis/ssa/ssa_private.h>
#include <libs/moduleAnalysis/ssa/ssa_ids.h>

#define SSA_node_map(cfg) (((SsaStuff)cfg->ssaStuff)->ssaNodeMap)

SsaNodeId ssa_node_new_id(CfgInstance cfg, SsaType type)
{
    SsaNodeId newId;

    if (!SSA_nodes(cfg))
	SSA_nodes(cfg) = area_create(sizeof(SsaNode), (AREA_INIT_FN) 0,
				     "SSA Dataflow Graph Nodes");
    
    newId = area_new(&(SSA_nodes(cfg)));

    SSA_node(cfg, newId)->type		= type;
    SSA_node(cfg, newId)->refAst	= AST_NIL;
    SSA_node(cfg, newId)->name		= SSA_NIL_NAME;
    SSA_node(cfg, newId)->cfgParent	= CFG_NIL;
    SSA_node(cfg, newId)->nextCfgSib	= CFG_NIL;
    SSA_node(cfg, newId)->ssaParent	= SSA_NIL;
    SSA_node(cfg, newId)->nextSsaSib	= SSA_NIL;
    SSA_node(cfg, newId)->subUses	= SSA_NIL;
    SSA_node(cfg, newId)->defsIn	= SSA_NIL;
    SSA_node(cfg, newId)->unuseful	= false;
    SSA_node(cfg, newId)->fanIn		= 0;

    if (ssa_is_def(cfg, newId)) /* kill or mod */
    {
	SSA_def(cfg, newId)->defKillOuts	= SSA_NIL;
	SSA_def(cfg, newId)->defModOuts		= SSA_NIL;
	SSA_def(cfg, newId)->defUseOuts		= SSA_NIL;
	SSA_def(cfg, newId)->nextDef		= SSA_NIL;
    }
    else /* use */
    {
	SSA_use(cfg, newId)->subDefs	= SSA_NIL;
    }

    return newId;
}

SsaEdgeId ssa_edge_new_id(CfgInstance cfg)
{
    SsaEdgeId newId;

    if (!SSA_edges(cfg))
	SSA_edges(cfg) = area_create(sizeof(SsaEdge), (AREA_INIT_FN) 0,
				     "SSA Dataflow Graph Edges");
    
    newId = area_new(&(SSA_edges(cfg)));

    SSA_edge(cfg, newId)->source	= SSA_NIL;
    SSA_edge(cfg, newId)->sink		= SSA_NIL;
    SSA_edge(cfg, newId)->pathEdge	= CFG_NIL;
    SSA_edge(cfg, newId)->inNext	= SSA_NIL;
    SSA_edge(cfg, newId)->outNext	= SSA_NIL;

    return newId;
}

void ssa_node_free(CfgInstance cfg, SsaNodeId id)
{
    SsaEdgeId ed;
    SsaNodeId kid;
    SsaNodeId *ptr;

    /*
     *  Support deletion of a single node from the SSA kids of a CFG node.
     */
    for (ptr = &(CFG_node(cfg, SSA_node(cfg, id)->cfgParent)->ssaKids);
	 (*ptr != id) && (*ptr != SSA_NIL);
	 ptr = &(SSA_node(cfg, *ptr)->nextCfgSib));

    if (*ptr == id) *ptr = SSA_node(cfg, id)->nextCfgSib;

    /*
     *  If doDefDef is set, every SSA node can have incoming edges from
     *  definitions.
     */
    for (ed = SSA_node(cfg, id)->defsIn; ed != SSA_NIL;
	 ed = SSA_node(cfg, id)->defsIn)

	ssa_edge_free(cfg, ed);

    /*
     *  Most SSA nodes can have subordinate uses, e.g. from array
     *  subscripts.  SSA_GAMMA has subUse field used for something else...
     */
    if (SSA_node(cfg, id)->type != SSA_GAMMA)
    {
	for (kid = SSA_node(cfg, id)->subUses; kid != SSA_NIL;
	     kid = SSA_node(cfg, id)->subUses)

	    ssa_node_free(cfg, kid);
    }

    if (ssa_is_def(cfg, id)) /* kill or mod */
    {
	for (ed = SSA_def(cfg, id)->defUseOuts; ed != SSA_NIL;
	     ed = SSA_def(cfg, id)->defUseOuts)

	    ssa_edge_free(cfg, ed);

	for (ed = SSA_def(cfg, id)->defKillOuts; ed != SSA_NIL;
	     ed = SSA_def(cfg, id)->defKillOuts)

	    ssa_edge_free(cfg, ed);

	for (ed = SSA_def(cfg, id)->defModOuts; ed != SSA_NIL;
	     ed = SSA_def(cfg, id)->defModOuts)

	    ssa_edge_free(cfg, ed);

	if (ssa_is_use(cfg, SSA_node(cfg, id)->ssaParent)) 
	{
	    /*
	     *  Support deletion of a single subDef of a CALL
	     */
	    for (ptr = &(SSA_use(cfg,
				 SSA_node(cfg, id)->ssaParent)->subDefs);
		 (*ptr != id) && (*ptr != SSA_NIL);
		 ptr = &(SSA_node(cfg, *ptr)->nextSsaSib));
	}
	if (*ptr == id) *ptr = SSA_node(cfg, id)->nextSsaSib;
    }
    else /* pure use */
    {
	/*
	 *  Free subordinate definitions and uses -- really exist only
	 *  for SSA_CALL
	 */
	for (kid = SSA_use(cfg, id)->subDefs; kid != SSA_NIL;
	     kid = SSA_use(cfg, id)->subDefs)

	    ssa_node_free(cfg, kid);

	/*
	 *  Support deletion of a single subUse
	 */
	if (SSA_node(cfg, id)->ssaParent != SSA_NIL)
	{
	    SSA_node(cfg, SSA_node(cfg, id)->ssaParent)->fanIn--;

	    for (ptr = &(SSA_node(cfg, SSA_node(cfg, id)->ssaParent)->subUses);
		 (*ptr != id) && (*ptr != SSA_NIL);
		 ptr = &(SSA_node(cfg, *ptr)->nextSsaSib));

	    if (*ptr == id) *ptr = SSA_node(cfg, id)->nextCfgSib;
	}
    }

    if (!is_null_node(SSA_node(cfg, id)->refAst))
	ssa_node_zap_map(cfg, SSA_node(cfg, id)->refAst);

    area_free(SSA_nodes(cfg), &id);
}

void ssa_edge_free(CfgInstance cfg, SsaEdgeId id)
{
    SsaEdgeId *ptr;
    SsaNodeId sink;

    sink = SSA_edge(cfg, id)->sink;

    SSA_node(cfg, sink)->fanIn--;

    for (ptr = &(SSA_node(cfg, sink)->defsIn);
	 (*ptr != id) && (*ptr != SSA_NIL);
	 ptr = &(SSA_edge(cfg, *ptr)->inNext));

    if (*ptr == id) *ptr = SSA_edge(cfg, id)->inNext;

    /*
     *  Source can be nil if deleting a temporary edge
     */
    if (SSA_edge(cfg, id)->source != SSA_NIL)
    {
	if (ssa_is_mod(cfg, sink))
	{
	    for (ptr = &(SSA_def(cfg, SSA_edge(cfg, id)->source)->defModOuts);
		 (*ptr != id) && (*ptr != SSA_NIL);
		 ptr = &(SSA_edge(cfg, *ptr)->outNext));
	}
	else if (ssa_is_kill(cfg, sink))
	{
	    for (ptr = &(SSA_def(cfg, SSA_edge(cfg, id)->source)->defKillOuts);
		 (*ptr != id) && (*ptr != SSA_NIL);
		 ptr = &(SSA_edge(cfg, *ptr)->outNext));
	}
	else if (ssa_is_use(cfg, sink))
	{
	    for (ptr = &(SSA_def(cfg, SSA_edge(cfg, id)->source)->defUseOuts);
		 (*ptr != id) && (*ptr != SSA_NIL);
		 ptr = &(SSA_edge(cfg, *ptr)->outNext));
	}
	else
	{
	    /* sink is SSA_NIL -- is this ever true? */
	}
	if (*ptr == id) *ptr = SSA_edge(cfg, id)->outNext;
    }

    area_free(SSA_edges(cfg), &id);
}

void ssa_KillKids(Generic junk, CfgInstance cfg, CfgNodeId cfgNode)
{
    SsaNodeId kid;

    for (kid = (SsaNodeId) CFG_node(cfg, cfgNode)->ssaKids; kid != SSA_NIL;
	 kid = (SsaNodeId) CFG_node(cfg, cfgNode)->ssaKids)

	ssa_node_free(cfg, kid);
}

void ssa_init_maps(CfgInstance cfg)
{
    SSA_node_map(cfg) = (void *) new IntHashTable;
}

void ssa_kill_maps(CfgInstance cfg)
{
    delete (IntHashTable *) SSA_node_map(cfg);
}

SsaNodeId ssa_node_map(CfgInstance cfg, AST_INDEX n)
{
    return ((IntHashTable *) SSA_node_map(cfg))->query_entry((uint) n);
}

void ssa_node_put_map(CfgInstance cfg, AST_INDEX ast, SsaNodeId id)
{
    ((IntHashTable *) SSA_node_map(cfg))->add_entry((uint) ast, id);
}

void ssa_node_zap_map(CfgInstance cfg, AST_INDEX ast)
{
    ((IntHashTable *) SSA_node_map(cfg))->delete_entry((uint) ast);
}
