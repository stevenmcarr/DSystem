/* $Id: cfg_ids.C,v 1.1 1997/06/25 15:03:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- cfg_ids.c
 *
 */

#include <libs/moduleAnalysis/cfg/cfg_private.h>
#include <libs/moduleAnalysis/cfg/cfg_ids.h>

CfgNodeId cfg_node_new_id(CfgInstance cfg)
{
    CfgNodeId New;

    if (!(cfg->cfgNodes))
	cfg->cfgNodes = (CfgNode *) f_alloc(CFG_INIT_SIZE, sizeof(CfgNode), 
					    "Control Flow Graph Nodes",
					    (f_init_callback) 0);

    New = f_new((Generic *) &(cfg->cfgNodes));

    CFG_node(cfg, New)->astnode	= AST_NIL;
    CFG_node(cfg, New)->lbldef	= CFG_NO_LABEL;
    CFG_node(cfg, New)->ins	= CFG_NIL;
    CFG_node(cfg, New)->outs	= CFG_NIL;
    CFG_node(cfg, New)->cdIns	= CFG_NIL;
    CFG_node(cfg, New)->cdOuts	= CFG_NIL;
    CFG_node(cfg, New)->ssaKids	= (Generic) CFG_NIL;
    CFG_node(cfg, New)->fanIn	= 0;
    CFG_node(cfg, New)->fanOut	= 0;
    CFG_node(cfg, New)->freed	= (TinyBool) false;

    return New;
}

CfgEdgeId cfg_edge_new_id(CfgInstance cfg)
{
    CfgEdgeId New;

    if (!(cfg->cfgEdges))
	cfg->cfgEdges = (CfgEdge *)f_alloc(CFG_INIT_SIZE, sizeof(CfgEdge),
					   "Control Flow Graph Edges",
					   (f_init_callback) 0);
    New = f_new((Generic *) &(cfg->cfgEdges));

    CFG_edge(cfg, New)->src		= CFG_NIL;
    CFG_edge(cfg, New)->dest		= CFG_NIL;
    CFG_edge(cfg, New)->inNext		= CFG_NIL;
    CFG_edge(cfg, New)->outNext		= CFG_NIL;
    CFG_edge(cfg, New)->label		= CFG_NIL;
    /*
     *	CFG_edge(cfg, New)->join	= -1;
     */
    CFG_edge(cfg, New)->exec		= (TinyBool) false;
    CFG_edge(cfg, New)->freed		= (TinyBool) false;

    return New;
}

CfgEdgeId cfg_cd_edge_new_id(CfgInstance cfg)
{
    CfgEdgeId New;

    if (!(cfg->cfgCdEdges))
	cfg->cfgCdEdges = (CfgCdEdge *)f_alloc(CFG_INIT_SIZE, sizeof(CfgCdEdge),
					       "Control Dependence Edges",
					       (f_init_callback) 0);
    New = f_new((Generic *) &(cfg->cfgCdEdges));

    CFG_cdedge(cfg, New)->src		= CFG_NIL;
    CFG_cdedge(cfg, New)->dest		= CFG_NIL;
    CFG_cdedge(cfg, New)->inNext	= CFG_NIL;
    CFG_cdedge(cfg, New)->outNext	= CFG_NIL;
    CFG_cdedge(cfg, New)->label		= CFG_NIL;
    CFG_cdedge(cfg, New)->level		= CFG_NIL;

    CFG_cdedge(cfg, New)->freed		= (TinyBool) false;

    return New;
}

void cfg_node_free(CfgInstance cfg, CfgNodeId id)
{
    CfgEdgeId ed;
    CfgClient client;
    /*
     *	SsaNodeId kid;
     */

    for (client = cfg->cfgGlobals->firstClient;
	 client;
	 client = client->next)
    {
	if (client->destroy_node)
	    client->destroy_node(client->handle, cfg, id);
    }

    for (ed = CFG_node(cfg, id)->ins; ed != CFG_NIL;
	 ed = CFG_edge(cfg, ed)->inNext)

	cfg_edge_free(cfg, ed);

    for (ed = CFG_node(cfg, id)->outs; ed != CFG_NIL;
	 ed = CFG_edge(cfg, ed)->outNext)

	cfg_edge_free(cfg, ed);

    CFG_node(cfg, id)->freed	 = (TinyBool) true;
    f_dispose((Generic) cfg->cfgNodes, id);
}


void cfg_edge_free(CfgInstance cfg, CfgEdgeId id)
{
    CfgEdgeId *ptr;
    CfgClient client;

    for (client = cfg->cfgGlobals->firstClient;
	 client;
	 client = client->next)
    {
	if (client->destroy_edge)
	    client->destroy_edge(client->handle, cfg, id);
    }

    for (ptr = &(CFG_node(cfg, CFG_edge(cfg, id)->src)->outs);
	 (*ptr != id) && (*ptr != CFG_NIL);
	 ptr = &(CFG_edge(cfg, *ptr)->outNext));

    if (*ptr == id) *ptr = CFG_edge(cfg, id)->outNext;

    for (ptr = &(CFG_node(cfg, CFG_edge(cfg, id)->dest)->ins);
	 (*ptr != id) && (*ptr != CFG_NIL);
	 ptr = &(CFG_edge(cfg, *ptr)->inNext));

    if (*ptr == id) *ptr = CFG_edge(cfg, id)->inNext;

    CFG_edge(cfg, id)->freed	 = (TinyBool) true;
    f_dispose((Generic) cfg->cfgEdges, id);
}


void cfg_cd_edge_free(CfgInstance cfg, CfgEdgeId id)
{
    CfgEdgeId *ptr;

    for (ptr = &(CFG_node(cfg, CFG_cdedge(cfg, id)->src)->cdOuts);
	 (*ptr != id) && (*ptr != CFG_NIL);
	 ptr = &(CFG_cdedge(cfg, *ptr)->outNext));

    if (*ptr == id) *ptr = CFG_cdedge(cfg, id)->outNext;

    for (ptr = &(CFG_node(cfg, CFG_cdedge(cfg, id)->dest)->cdIns);
	 (*ptr != id) && (*ptr != CFG_NIL);
	 ptr = &(CFG_cdedge(cfg, *ptr)->inNext));

    if (*ptr == id) *ptr = CFG_cdedge(cfg, id)->inNext;

    CFG_cdedge(cfg, id)->freed	 = (TinyBool) true;
    f_dispose((Generic) cfg->cfgCdEdges, id);
}

void cfg_free_all_cds(CfgInstance cfg)
{
    CfgNodeId stmt;

    if (cfg->cfgCdEdges)
    {
	f_free((Generic) cfg->cfgCdEdges);
	for (stmt = cfg_get_first_node(cfg);
	     stmt != CFG_NIL;
	     stmt = cfg_get_next_node(cfg, stmt))
	{
	    CFG_node(cfg, stmt)->cdIns  = CFG_NIL;
	    CFG_node(cfg, stmt)->cdOuts = CFG_NIL;
	}
    }
}
