/* $Id: cfg_dgcd.C,v 1.1 1997/06/25 15:05:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 *  -- cfg_dgcd.c
 *
 *	Build control dependences into the dependence graph.
 *
 *		-- Paul Havlak
 *
 *	3 Feb 1993 (paco):
 *		cleaned up interface to cfg and moved to dep/cd/cfg_dgcd.c
 */

#include <libs/moduleAnalysis/dependence/controlDependence/cd_graph.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>
/*
 *  Construction of the control dependence graph, based on results in
 *
 *  Compact Representations for Control Dependence
 *  Cytron, Ferrante & Sarkar SIGPLAN PLDI '90
 *
 *  For each edge out of a node with multiple successors,
 *  we record the source and label, then walk up the postdominator
 *  tree from the sink, adding control dependences while we have not
 *  reached the postdominator of the source.          Paul Havlak 3/27/91
 */

#define lvector_size    10

STATIC(void, add_cd_edge, (DG_Instance *dg, SideInfo *infoPtr, CfgInstance cfg,
                           CfgNodeId source, CfgNodeId sink, int label, int level,
                           CdBranchType sourceType));

/* 
 *  build Control Dependence graph
 *	-- One DG instance per CfgInfo structure
 *	-- Therefore one cdg for multiple subroutine cfg instances
 */
void dg_build_module_cds(DG_Instance *dg, SideInfo *infoPtr, CfgInfo cfgGlobals)
{
    CfgInstance cfg;
    Boolean global;

    if (!cfgGlobals) {
	fprintf(stderr, "cdg_build:  Null Control Flow Graph passed in.\n");
	return;
    }
    if (!dg) {
	fprintf(stderr, "cdg_build:  Null DependenceGraph object passed in.\n");
	return;
    }
    if (!infoPtr) {
	fprintf(stderr, "cdg_build:  Null SideInfo object passed in.\n");
	return;
    }

    /*
     *  build control dependences for each procedure
     */
    for (cfg = cfg_get_first_inst(cfgGlobals);
	 cfg;
	 cfg = cfg_get_next_inst(cfg)) 
    {
	/* 
	 *  Put this code back to get a module-wide rooted cdg.
	 *  It adds edges from the global node -- root of the 
	 *  module AST -- to the root (main entry) of each procedure.
	 *
	 *  if (cfg_get_inst_root(cfg) != cfg_get_global_root(cfgGlobals))
	 *  {
	 *  add_cd_edge(dg, infoPtr, cfg, CFG_NIL,
	 *  cfg_node_from_ast(cfg, cfg_get_inst_root(cfg)),
	 *  CD_DEFAULT, LOOP_INDEPENDENT, CD_UNCONDITIONAL);
	 *  }
	 */
	/*
	 *  Copy CFG control dependences in DG dependence graph.
	 */
	dg_build_subprogram_cds(dg, infoPtr, cfg);
    }
}




void dg_build_subprogram_cds(DG_Instance *dg, SideInfo *infoPtr, CfgInstance cfg)
{
    CfgNodeId	node, chain;
    CfgCdId	cd;
    DomTree     postdom = cfg_get_postdom(cfg);

    /*
     *  CFG routines have CD_DEFAULT edges from root to unconditionally
     *  executed nodes so that the cd graph for each procedure will 
     *  be rooted.
     */

    for (node = cfg_get_first_node(cfg);
	 node != CFG_NIL;
	 node = cfg_get_next_node(cfg, node))
    {
	for (cd = cfg_cd_first_from(cfg, node);
	     cd != CFG_NIL;
	     cd = cfg_cd_next_from(cfg, cd))
	{
	    add_cd_edge(dg, infoPtr, cfg, node, 
			cfg_cd_dest(cfg, cd),
			cfg_cd_label(cfg, cd),
			cfg_cd_level(cfg, cd),
			cfg_branch_type(cfg, node));
	}
    }
}


static void add_cd_edge(DG_Instance *dg, SideInfo *infoPtr, CfgInstance cfg, 
                        CfgNodeId source, CfgNodeId sink, int label, int level, 
                        CdBranchType sourceType)
{
    DG_Edge *Edge_array;
    int e;
    int lvector;
    AST_INDEX sinkAst, srcAst, newSrc;

    sinkAst = cfg_node_to_ast(cfg, sink);
    if (is_null_node(sinkAst) || (is_label_def(sinkAst)/*preheader for do*/) ||
	((is_guard(sinkAst)) &&
	 (is_null_node(gen_GUARD_get_rvalue(sinkAst)))))
    {
	/*
	 *  avoid adding edges to null ast node
	 *  or multiple edges to loop header
	 *  ... or edges to ELSE (guard with null condition) phh 2 Sep 91
	 */
	return;
    }

    Edge_array = dg_get_edge_structure( dg );
    e = dg_alloc_edge( dg, &Edge_array);

    Edge_array[e].type    = dg_control;
    Edge_array[e].cdtype  = sourceType;
    Edge_array[e].cdlabel = label;
    Edge_array[e].level   = level;
    Edge_array[e].ic_sensitive = false;
    Edge_array[e].ic_preventing = false;

    if (source == CFG_NIL)
    {
       /*
	*  Hack -- NIL source means adding edge from the global instance
	*/
	Edge_array[e].src = cfg_get_global_root(cfg_get_globals(cfg));
    }
    else if (source == cfg_start_node(cfg))
    {
	/*
	 *  cfg->start now has no astnode, so make these are sourced
	 *  at the cfg ast root
	 *
	 *  This prompts the above change to avoid cycles on
	 *  main subprogram entry point.   -- paco 27 May 92
	 */
	Edge_array[e].src = cfg_get_inst_root(cfg);
    }
    else
    {
	/*
	 *  for consistency with PFC, cds with their src on the
	 *  first guard are moved to be on the IF
	 */
	srcAst = cfg_node_to_ast(cfg, source);
	if (is_guard(srcAst))
	{
	    newSrc = out(srcAst);
	    if (srcAst == list_first(gen_IF_get_guard_LIST(newSrc)))
		srcAst = newSrc;
	}
	Edge_array[e].src = srcAst;
    }
    /*
     *  for consistency with PFC, also
     *  Need to move edges to first guard to go to the IF.
     */
    if (is_guard(sinkAst))
    {
	newSrc = out(sinkAst);
	if (sinkAst == list_first(gen_IF_get_guard_LIST(newSrc)))
	    sinkAst = newSrc;
    }
    Edge_array[e].sink = sinkAst;

    Edge_array[e].src_ref  = -1;
    Edge_array[e].sink_ref = -1;

    /* do these stmts have level vectors? if not, allocate them */
    if ((lvector = dg_get_info ( infoPtr, Edge_array[e].src, type_levelv)) == -1)
    {
        lvector = dg_alloc_level_vector( dg, lvector_size);
        dg_put_info ( infoPtr, Edge_array[e].src, type_levelv, lvector);
    }
    Edge_array[e].src_vec = lvector;

    if ((lvector = dg_get_info ( infoPtr, Edge_array[e].sink, type_levelv)) == -1)
    {
        lvector = dg_alloc_level_vector( dg, lvector_size);
        dg_put_info ( infoPtr, Edge_array[e].sink, type_levelv, lvector);
    }
    Edge_array[e].sink_vec = lvector;

    dg_add_edge( dg, e);
}


