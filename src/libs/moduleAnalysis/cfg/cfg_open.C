/* $Id: cfg_open.C,v 1.1 1997/06/25 15:03:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- cfg_main.c
 *
 */

#include <string.h>

#include <libs/moduleAnalysis/cfg/cfg_private.h>
#include <libs/moduleAnalysis/cfg/cfg_nodes.h>
#include <libs/moduleAnalysis/cfg/cfg_edges.h>
#include <libs/moduleAnalysis/cfg/dtree.h>
#include <libs/moduleAnalysis/cfg/tarjan.h>
#include <libs/moduleAnalysis/cfg/cfg_ids.h>

#include <libs/support/time/swatch.h>

extern Swatch cfg_build_inst_sw;
extern Swatch cfg_build_nodes_sw;
extern Swatch cfg_build_edges_sw;
extern Swatch cfg_add_loop_nodes_sw;

static Generic INFO_SIDE_ARRAY_INITIALS = {(Generic) CFG_NIL};

STATIC(CfgInfo, cfg_info_init,(FortTree ft));
STATIC(CfgInstance, make_instance, (CfgInfo cfgGlobals, AST_INDEX root, 
                                    AST_INDEX endAst));

/* 
 *  Driver for Control Flow Graph construction
 *
 *  Root is root of AST to analyze; 
 *  interProc indicates if interprocedural information is available; 
 *
 *  returns the CfgInfo superstructure when finished
 */
CfgInfo
cfg_Open(FortTree ft)
{
    CfgInfo	cfgGlobals;

    /* 
     *	Create permanent space for the cprop information...
     */
    cfgGlobals = cfg_info_init(ft);

    return cfgGlobals;
}

CfgInstance
cfg_build_inst(CfgInfo cfgGlobals, AST_INDEX root, AST_INDEX endAst)
{
    CfgInstance cfg = cfgGlobals->firstInst;
    Boolean	error;
    CfgClient	client;
    AST_INDEX	subprog;

    swatch_start(cfg_build_inst_sw);

    if (is_null_node(root)) return (CfgInstance) 0;

    for (subprog = root;
	 !new_instance(subprog);
	 subprog = out(subprog))
	;

    if (cfg &&
	(cfg_node_map(cfg, subprog) != CFG_NIL))
    {
	/*
	 *  Should already have an instance for this subprogram
	 */
	for (;
	     cfg->next && (cfg->astnode != subprog);
	     cfg = cfg->next)
	    ;
	if (cfg->astnode == subprog)
	{
	    if ((subprog == root) && (endAst == cfg->endAst))
		return cfg;
	    else
		/*
		 *  We have a subprogram instance but don't want one!
		 *  Destroy the subprogram instance.
		 */
		cfg_destroy_inst(cfg);
	}
	else
	{
	    fprintf(stderr, "cfg_build_inst:  instance should exist\n");
	}
    }

    cfg = make_instance(cfgGlobals, root, endAst);

    if (!is_global(root))
    {
	swatch_start(cfg_build_nodes_sw);

	error = cfg_build_nodes(cfg, root);
	/*
	 *  Add end CfgNode with no corresponding AST_INDEX
	 *  -- adding it after normal nodes is not strictly 
	 *  necessary, but less confusing.
	 *
	 *  Want an end node whether we're analyzing a procedure or
	 *  fragment, as destination for abnormal exits.
	 *
	 *					--paco, 15 Feb 93
	 */
	if (cfg->end == CFG_NIL)
	{
	    cfg->end = cfg_node_new_id(cfg);
	}

	swatch_lap(cfg_build_nodes_sw);
	swatch_start(cfg_build_edges_sw);

	error = BOOL(error || cfg_build_edges(cfg, root));

	swatch_lap(cfg_build_edges_sw);
	swatch_start(cfg_add_loop_nodes_sw);

	cfg_add_loop_nodes(cfg);

	swatch_lap(cfg_add_loop_nodes_sw);
    }
    else
	error = false;

    if (error)
	fprintf(stderr, "cfg_build_inst:  bad nodes or edges\n");

    swatch_lap(cfg_build_inst_sw);

    for (client = cfgGlobals->firstClient;
	 client;
	 client = client->next)
    {
	if (client->create_instance)
	    client->create_instance(client->handle, cfg);
    }

    return cfg;
}

static CfgInfo 
cfg_info_init(FortTree ft)
{
    CfgInfo cfgGlobals;

    cfgGlobals = (CfgInfo) get_mem(sizeof(*cfgGlobals),
				   "cfg_info_init: CFG global structure");
    cfgGlobals->ft = ft;
    cfgGlobals->sideArray = 
	(Generic *) ft_AttachSideArray(ft, 1, &INFO_SIDE_ARRAY_INITIALS);

    cfgGlobals->firstInst =
	cfgGlobals->lastInst = (CfgInstance) 0;

    cfgGlobals->firstClient  = (CfgClient) 0;

    cfgGlobals->ssaParms = (Generic) NULL;
    cfgGlobals->valParms = (Generic) NULL;

    return cfgGlobals;
}

void 
cfg_Close(CfgInfo cfgGlobals)
{
    CfgInstance cfg, follow;

    /*
     *	Free symbol tables?  Depends...
     */
    ft_DetachSideArray((FortTree) cfgGlobals->ft,
		       (Generic) cfgGlobals->sideArray);
    cfgGlobals->sideArray = (Generic *) 0;

    cfg = cfgGlobals->firstInst;

    if (cfg)
    {
	for (follow = cfg->next;
	     follow;
	     cfg = follow, follow = cfg->next) cfg_destroy_inst(cfg);

	cfg_destroy_inst(cfg);
    }

    free_mem((void*) cfgGlobals);
}


void cfg_destroy_inst(CfgInstance cfg)
{
    CfgClient client;
    CfgNodeId node;

    for (client = cfg->cfgGlobals->firstClient;
	 client;
	 client = client->next)
    {
	if (client->destroy_instance)
	    client->destroy_instance(client->handle, cfg);
    }

    /*
     *  Clear out the map if it still exists
     */
    if (cfg->cfgGlobals->sideArray)
    {
	for (node = cfg_get_first_node(cfg);
	     node != CFG_NIL;
	     node = cfg_get_next_node(cfg, node))
	{
	    cfg_node_put_map(cfg, CFG_node(cfg, node)->astnode, CFG_NIL);
	}
    }

    if (cfg->cfgNodes)	f_free((Generic) cfg->cfgNodes);
    if (cfg->cfgEdges)	f_free((Generic) cfg->cfgEdges);
    if (cfg->cfgCdEdges) f_free((Generic) cfg->cfgCdEdges);
    if (cfg->topMap)	free_mem((void*) cfg->topMap);

    if (cfg->predom)	dom_free(cfg, /* forward = */ true);
    if (cfg->postdom)	dom_free(cfg, /* forward = */ false);
    if (cfg->intervals)	tarj_free(cfg);
    if (cfg->lblMap)	cfg_kill_label_map(cfg->lblMap);

    if (cfg->comnAstList)
	f_free((Generic) cfg->comnAstList);

    if (cfg->prev)
	cfg->prev->next = cfg->next;
    else
	cfg->cfgGlobals->firstInst = cfg->next;

    if (cfg->next)
	cfg->next->prev = cfg->prev;
    else
	cfg->cfgGlobals->lastInst = cfg->prev;

    free_mem((void*) cfg);
}

void cfg_Register(CfgInfo cfgGlobals, Generic handle,
		  CFG_INSTANCE_FN create_instance, 
                  CFG_INSTANCE_FN destroy_instance, 
                  CFG_INSTANCE_FN dump_instance,
		  CFG_NODE_FN create_node, 
                  CFG_NODE_FN destroy_node,     
                  CFG_NODE_FN dump_node,
		  CFG_EDGE_FN create_edge,     
                  CFG_EDGE_FN destroy_edge,     
                  CFG_EDGE_FN dump_edge)

/*  Generic handle;	must match on Register/Deregister */
/*  CFG_INSTANCE_FN 	create_instance;
    CFG_INSTANCE_FN 	destroy_instance;
    CFG_INSTANCE_FN 	dump_instance; */
    /*
     *  Create and destroy functions for nodes and edges are *not* called
     *  when whole CfgInstances are created and destroyed -- only when there
     *  are update operations on individual nodes and edges.
     */
/*  CFG_NODE_FN 	create_node;
    CFG_NODE_FN 	destroy_node;
    CFG_NODE_FN 	dump_node;
    CFG_EDGE_FN 	create_edge;
    CFG_EDGE_FN 	destroy_edge;
    CFG_EDGE_FN 	dump_edge; */
{
    CfgClient New;
    CfgInstance cfg;

    New = (CfgClient) get_mem(sizeof(*New), "cfg_Register");

    New->handle 		= handle;
    New->create_instance 	= create_instance;
    New->destroy_instance 	= destroy_instance;
    New->dump_instance 		= dump_instance;
    New->create_node 		= create_node;
    New->destroy_node 		= destroy_node;
    New->dump_node 		= dump_node;
    New->create_edge 		= create_edge;
    New->destroy_edge 		= destroy_edge;
    New->dump_edge 		= dump_edge;

    New->next = cfgGlobals->firstClient;
    cfgGlobals->firstClient = New;

    if (create_instance) 
	for (cfg = cfgGlobals->firstInst;
	     cfg;
	     cfg = cfg->next)
	{
	    if (cfg->cfgNodes && cfg->cfgEdges)
		create_instance(handle, cfg);
	}
}

void cfg_Deregister(CfgInfo cfgGlobals, Generic handle)
    /*Generic	handle;	 Must match for Register/Deregister*/
{
    CfgClient *currPtr;	/* address of pointer to deleted client */
    CfgClient follow;	/* next client after deleted client */

    for (currPtr = &(cfgGlobals->firstClient);
	 *currPtr;
	 currPtr = &((*currPtr)->next))
    {
	if (handle == (*currPtr)->handle)
	{
	    follow = (*currPtr)->next;
	    free_mem((void*) *currPtr);
	    *currPtr = follow;
	    break;
	}  
    }
}


/* make and add new instance for inst */
static CfgInstance
make_instance(CfgInfo cfgGlobals, AST_INDEX root, AST_INDEX endAst)
{
    CfgInstance cfg;		/* new instance */
    AST_INDEX name;		/* new instance AST name node */
    CfgInstance prevCfg;	/* stepping through subprogram instance list */
    AST_INDEX subprog;		/* stepping through list of subprogram ASTs */
    int i;

    cfg = (CfgInstance) get_mem(sizeof(*cfg), "CFG instance");

    cfg->cfgGlobals	= cfgGlobals;
    cfg->astnode	= root;
    cfg->endAst		= endAst;

    /*
     *  If we have a subprogram instance, add to the list
     */
    if (new_instance(root))
    {
	if (!cfgGlobals->firstInst)
	{
	    cfgGlobals->firstInst = cfgGlobals->lastInst = cfg;
	    cfg->prev = cfg->next = (CfgInstance) 0;
	}
	else
	{
	    /*
	     *  Insert in right place in list
	     *  Assume it's most efficient to start at end of list,
	     *  as when we're adding subprogram instances in order.
	     */
	    prevCfg = cfgGlobals->lastInst;

	    for (subprog = list_last(ft_Root((FortTree) cfgGlobals->ft));
		 !is_null_node(subprog);
		 subprog = list_prev(subprog))
	    {
		if (subprog == root)
		    /*  New instance needs to go after prevCfg  */
		    break;
		else if (cfg && (subprog == cfg->astnode))
		    /*  New instance must go before this prevCfg  */
		    prevCfg = prevCfg->prev;
	    }

	    cfg->prev = prevCfg;

	    if (prevCfg)		/* insert after */
	    {
		cfg->next = prevCfg->next;
		prevCfg->next = cfg;
	    }
	    else			/* insert at beginning */
	    {
		cfg->next = cfgGlobals->firstInst;
		cfgGlobals->firstInst = cfg;
	    }
	    if (cfg->next)
		cfg->next->prev = cfg;
	    else
		cfgGlobals->lastInst = cfg;
	}
    }


    if (is_subroutine(root)) {
	name = gen_SUBROUTINE_get_name(root);
	cfg->type = CFG_SUBROUTINE;
    }

    else if (is_function(root)) {
	name = gen_FUNCTION_get_name(root);
	cfg->type = CFG_FUNCTION;
    }

    else if (is_program(root)) {
	name = gen_PROGRAM_get_name(root);
	cfg->type = CFG_PROGRAM;
    }

    else if (is_block_data(root)) {
	name = gen_BLOCK_DATA_get_name(root);
	cfg->type = CFG_BLOCK_DATA;
    }

    else if (is_global(root)) {
	name = AST_NIL;
	cfg->type = CFG_GLOBAL;
    }

    else /* instance for non-procedural rooted ASTs */ {
	name = AST_NIL;
	cfg->type = CFG_FRAGMENT;
    }

    if (is_null_node(name))
    {
	cfg->name = NIL_STR;
    }
    else
    {
	cfg->name = gen_get_symbol(name);
    }

    cfg->cfgNodes	= (CfgNode *) 0;
    cfg->cfgEdges	= (CfgEdge *) 0;
    cfg->cfgCdEdges	= (CfgCdEdge *) 0;
    /*
     *  Both start and end cfg nodes are distinct in having no corresponding
     *  ast node.  End will be added later (less confusing that way).
     */
    cfg->start		= cfg_node_new_id(cfg);
    cfg->end		= CFG_NIL;

    cfg->topMap		= (CfgNodeId *) 0;
    cfg->predom		= (DomTree) 0;
    cfg->postdom	= (DomTree) 0;
    cfg->intervals	= (TarjTree) 0;

    cfg->symtab		= (SymDescriptor) 0;

    cfg->lblMap		= (LabelMap) 0;
    cfg->comnAstList	= (AST_INDEX *) 0;

    for (i = 0; i < SSA_WORK_SLOTS; i++) cfg->ssaStuff[i] = 0;

    for (i = 0; i < VAL_WORK_SLOTS; i++) cfg->valStuff[i] = 0;

    return cfg;
}
