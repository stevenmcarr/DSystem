/* $Id: cfg_utils.C,v 1.1 1997/06/25 15:03:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- cfg_utils.c
 *
 */

#include <libs/moduleAnalysis/cfg/cfg_private.h>
#include <libs/moduleAnalysis/cfg/cfg_utils.h>
#include <libs/support/stacks/xstack.h>
#include <assert.h>

static STR_INDEX
get_subprog_symbol(AST_INDEX node)
{
    AST_INDEX name;

    if (is_subroutine(node)) {
	name = gen_SUBROUTINE_get_name(node);
    }
    else if (is_function(node)) {
	name = gen_FUNCTION_get_name(node);
    }
    else if (is_program(node)) {
	name = gen_PROGRAM_get_name(node);
    }
    else if (is_block_data(node)) {
	name = gen_BLOCK_DATA_get_name(node);
    }
    else
	return NIL_STR;

    return gen_get_symbol(name);
}
    
void 
long_set(long *array, int size, long value)
{
    if (size > 0)
	while (size--)
	    *array++ = value;
}

void 
int_set(int *array, int size, int value)
{
    if (size > 0)
	while (size--)
	    *array++ = value;
}

void 
short_set(short *array, int size, int value)
{
    if (size > 0)
	while (size--)
	    *array++ = (short) value;
}

void 
Boolean_set(Boolean *array, int size, Boolean value)
{
    if (size > 0)
	while (size--)
	    *array++ = value;
}

CfgList
cfg_list_push(CfgList list, Generic item, char *who)
{
    CfgList New;
    New = (CfgList) get_mem(sizeof(*New), who);
    New->stuff = item;
    New->next  = list;
    return New;
}

CfgList
cfg_list_pop(CfgList list)
{
    CfgList retVal;

    retVal = list->next;
    free_mem((void*) list);
    return retVal;
}

Generic
cfg_list_top(CfgList list)
{
    return list->stuff;
}

void
cfg_list_kill(CfgList list)
{
    CfgList loop;

    while (list)
    {
	loop = list->next;
	free_mem((void*) list);
	list = loop;
    }
}



CfgInstance
cfg_get_inst(CfgInfo cfgGlobals, char *name)
{
    CfgInstance cfg;
    AST_INDEX root, proc;
    STR_INDEX string = NIL_STR;

    cfg = cfgGlobals->firstInst;
    if (cfg && (cfg->type == CFG_GLOBAL))
    {
	cfg = cfg->next;
    }

    for (;
         cfg; cfg = cfg->next)
    {
	if (!strcmp(string_table_get_text(cfg->name), name))
	{
	    return cfg;
	}
    }

    root = ft_Root((FortTree) cfgGlobals->ft);
    assert(is_global(root));

    proc = in(root);

    for (; proc != AST_NIL; proc = list_next(proc))
    {
      if (is_comment(proc))
	continue;
      
      string = get_subprog_symbol(proc);

      if ((string != NIL_STR) &&
	  !strcmp(string_table_get_text(string), name))
	{
	  return cfg_build_inst(cfgGlobals, proc, AST_NIL);
	}
    }

    return (CfgInstance) 0;
}



CfgInstance 
cfg_get_first_inst(CfgInfo cfgGlobals)
{
    CfgInstance cfg;
    AST_INDEX root, proc;

    cfg = cfgGlobals->firstInst;
    if (cfg && (cfg->type == CFG_GLOBAL))
	cfg = cfg->next;

    root = ft_Root((FortTree) cfgGlobals->ft);
    assert(is_global(root));

    proc = in(root);

    if (cfg && (proc == cfg->astnode))
	return cfg;
    else
    {
        for (; proc != AST_NIL;	proc = list_next(proc))
	{
	  if (new_instance(proc))
	    return cfg_build_inst(cfgGlobals, proc, AST_NIL);
	}

        return (CfgInstance) 0;
    }
}

CfgInstance 
cfg_get_next_inst(CfgInstance cfg)
{
    AST_INDEX next_by_ast;
    CfgInstance next_by_cfg;

    
    next_by_ast = cfg->astnode;
    do
    {
	next_by_ast = list_next(next_by_ast);
    }
    while (is_comment(next_by_ast));

    if (is_null_node(next_by_ast))
	return (CfgInstance) 0;

    next_by_cfg = cfg->next;

    if (next_by_cfg && (next_by_cfg->astnode == next_by_ast))
	return next_by_cfg;
    else
	return cfg_build_inst(cfg->cfgGlobals, next_by_ast, AST_NIL);
}


CfgInstanceType 
cfg_curr_inst_type(CfgInstance cfg)
{
    return cfg->type;
}

CfgNodeId 
cfg_start_node(CfgInstance cfg)
{
    return cfg->start;
}

CfgNodeId 
cfg_end_node(CfgInstance cfg)
{
    return cfg->end;
}

CfgEdgeId 
cfg_first_from_ast(CfgInstance cfg, AST_INDEX a)
{
    return CFG_node(cfg, cfg_node_map(cfg, a))->outs;
}

CfgEdgeId 
cfg_first_from_cfg(CfgInstance cfg, CfgNodeId n)
{
    return CFG_node(cfg, n)->outs;
}

CfgEdgeId 
cfg_first_to_ast(CfgInstance cfg, AST_INDEX a)
{
    return CFG_node(cfg, cfg_node_map(cfg, a))->ins;
}

CfgEdgeId 
cfg_first_to_cfg(CfgInstance cfg, CfgNodeId n)
{
    return CFG_node(cfg, n)->ins;
}

CfgEdgeId 
cfg_next_from(CfgInstance cfg, CfgEdgeId e)
{
    return CFG_edge(cfg, e)->outNext;
}

CfgEdgeId 
cfg_next_to(CfgInstance cfg, CfgEdgeId e)
{
    return CFG_edge(cfg, e)->inNext;
}

CfgNodeId cfg_edge_src(CfgInstance cfg, CfgEdgeId e)
{
    return CFG_edge(cfg, e)->src;
}

CfgNodeId cfg_edge_dest(CfgInstance cfg, CfgEdgeId e)
{
    return CFG_edge(cfg, e)->dest;
}

int cfg_edge_label(CfgInstance cfg, CfgEdgeId e)
{
    return (int) CFG_edge(cfg, e)->label;
}

int cfg_node_fanin(CfgInstance cfg, CfgNodeId n)
{
    return (int) CFG_node(cfg, n)->fanIn;
}

int cfg_node_fanout(CfgInstance cfg, CfgNodeId n)
{
    return (int) CFG_node(cfg, n)->fanOut;
}

AST_INDEX cfg_node_to_ast(CfgInstance cfg, CfgNodeId n)
{
    return CFG_node(cfg, n)->astnode;
}

CfgNodeId cfg_node_from_ast(CfgInstance cfg, AST_INDEX n)
{
    return cfg_node_map(cfg, n);
}

CfgNodeId cfg_header_from_ast(CfgInstance cfg, AST_INDEX n)
{
    return cfg_header_map(cfg, n);
}

CfgNodeId cfg_preheader_from_ast(CfgInstance cfg, AST_INDEX n)
{
    return cfg_preheader_map(cfg, n);
}

CfgNodeId cfg_get_first_node(CfgInstance cfg)
{
    /*
     *  Assuming 0 is the lowest possible index,
     *  -1 is the previous (impossible) index, 
     *  of whether or not it is CFG_NIL.
     */
    return cfg_get_next_node(cfg, -1);
}

CfgNodeId cfg_get_next_node(CfgInstance cfg, CfgNodeId i)
{
    int size = f_curr_size((Generic) cfg->cfgNodes);

    for (i++;
	 (i < size) && (CFG_node(cfg, i)->freed);
	 i++);

    if (i == size)
	return CFG_NIL;
    else
	return i;
}

CfgNodeId
cfg_get_last_node(CfgInstance cfg)
{
    return cfg_get_prev_node(cfg, f_curr_size((Generic) cfg->cfgNodes));
}

CfgNodeId 
cfg_get_prev_node(CfgInstance cfg, CfgNodeId i)
{
    for (i--;
	 (i >= 0) && (CFG_node(cfg, i)->freed);
	 i--);

    if (i == -1)
	return CFG_NIL;
    else
	return i;
}

CfgNodeId
cfg_node_max(CfgInstance cfg)
{
    return f_curr_size((Generic) cfg->cfgNodes);
}

CfgEdgeId
cfg_edge_max(CfgInstance cfg)
{
    return f_curr_size((Generic) cfg->cfgEdges);
}

/*
 * -- cfg_top_sort
 *
 *        Create a topological sort map for the forward CFG 
 *        Precondition: 
 *               1. cfg will be used as Current instance.
 *               2. Tarjan tree must already have been built
 */
CfgNodeId* cfg_top_sort(CfgInstance cfg)
{
    CfgNodeId* map;     /* CFG map from topological order to CFG node Id */
    int* fanInLeft;        /* number of unprocessed CFG input nodes */
    Stack readyToNumber;   /* stack of CFG id's ready to get a number */
    CfgNodeId i, cfgId;        
    CfgNodeId dest;        /* successor CFG node of cfgId */
    CfgEdgeId outEdge;     /* CFG out edge */
    int cfgSize;           /* size of CFG graph */
    int topNum;            /* next topological number */

    if (cfg->topMap)
	return(cfg->topMap);

    cfgSize = f_curr_size((Generic) cfg->cfgNodes);
    map = (CfgNodeId*)get_mem(cfgSize*sizeof(CfgNodeId), 
				 "cfg_top_sort:  topsort map to CFG node id");
#ifdef LONG_POINTER
    long_set(map, cfgSize, (Generic)CFG_NIL);	/* initialize the map */
#else
    int_set(map, cfgSize, (Generic)CFG_NIL);	/* initialize the map */
#endif

    fanInLeft = (int*)get_mem(cfgSize*sizeof(int), 
			      "cfg_top_sort: number of unprocessed inputs");

    readyToNumber = stack_create(sizeof(CfgNodeId));
    

    /*
     * Initialize fanInLeft[] to contain number of forward CFG in edges
     * Initialize readyToNumber to contain nodes with no forward in edges
     */
    for (i = cfg_get_first_node(cfg);
	 i != CFG_NIL;
	 i = cfg_get_next_node(cfg, i) )
    {
	fanInLeft[i] = cfg_get_forward_fanin(cfg, i);

	if (fanInLeft[i] == 0)
	    stack_push(readyToNumber, &i); /* ready to give a Top number */

    } /* end of for each CFG node initialize */
				  
    
    /*
     * Now start issuing topological numbers
     */
    topNum = cfg_get_first_node(cfg);
    while ( stack_pop(readyToNumber, &cfgId) ) /* while stack not empty */
    {
	map[topNum] = cfgId;

	topNum = cfg_get_next_node(cfg, topNum);

	/* Decrement fanin count of cfgId's forward successors */
	outEdge = CFG_node(cfg, cfgId)->outs;
	while ( outEdge != CFG_NIL )
	{
	    if ( !cfg_is_backedge(cfg, outEdge) )
	    {
		dest = CFG_edge(cfg, outEdge)->dest;
		fanInLeft[dest]--;
		if ( fanInLeft[dest] == 0 )
		    stack_push(readyToNumber, &dest); 
	    }

	    outEdge = CFG_edge(cfg, outEdge)->outNext;
	}
    } /* end of while stack not empty */

    
    stack_destroy(readyToNumber);
    free_mem((void*) fanInLeft);

    cfg->topMap = map;
    return map;

} /* end of cfg_top_sort() */




/*************************************/
/* I added those interface functions */
/*************************************/

SymDescriptor cfg_get_inst_symtab(CfgInstance cfg)
{
    return cfg->symtab;
}

AST_INDEX cfg_get_inst_root(CfgInstance cfg)
{
    return cfg->astnode;
}

char *cfg_get_inst_name(CfgInstance cfg)
{
  return string_table_get_text(cfg->name);
}
  
CfgInfo cfg_get_globals(CfgInstance cfg)
{
    return cfg->cfgGlobals;
}

AST_INDEX cfg_get_global_root(CfgInfo cfgInfo)
{
    return ft_Root(cfgInfo->ft);
}

CfgNodeId cfg_containing_loop(CfgInstance cfg, AST_INDEX an)
{
    CfgNodeId cn = CFG_NIL;
    TarjTree intervals;
    
    for (; !is_null_node(an); an = out(an))
    {
	cn = cfg_node_from_ast(cfg, an);

	if (cn != CFG_NIL)
	    break;
    }

    intervals = cfg_get_intervals(cfg);

    for (; cn != CFG_NIL; cn = tarj_outer(intervals, cn))
    {
	TarjType tt = tarj_type(intervals, cn);

	if ((tt == TARJ_INTERVAL) || (tt == TARJ_IRREDUCIBLE))
	    break;
    }
    return cn;
}
