/* $Id: ssa_dump.C,v 1.1 1997/06/25 15:10:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- dump.c
 *
 */

#include <libs/moduleAnalysis/ssa/ssa_private.h>

STATIC(void, ssa_dump_node,(CfgInstance cfg, SsaNodeId ssaId, int indent));
STATIC(void, ssa_dump_node_list,(CfgInstance cfg, SsaNodeId ssaId, int indent,
                                 char *comment));
STATIC(void, ssa_dump_edge_list,(CfgInstance cfg, SsaEdgeId edgeId, int indent,
                                 char *comment));
STATIC(void, ssa_dump_edge,(CfgInstance cfg, SsaEdgeId edgeId, int indent));

static char* ssaNodeType[] = {	"SSA_EDGE",

    /*
     *  guard expressions
     */
    "GUARD_LOGICAL",		"GUARD_INTEGER",	"GUARD_INDUCTIVE",
    "GUARD_ALT_RETURN",		"GUARD_ALT_ENTRY",	"GUARD_CASE", 

    /*
     *  other use nodes
     */
    "SSA_IP_REF",               "SSA_ACTUAL",		"SSA_SUBSCRIPT",
    "SSA_ALT_RETURN",           "SSA_USE",

    "SSA_LOOP_INIT",            "SSA_LOOP_BOUND",  	"SSA_LOOP_STEP", 

    "SSA_CALL",

    /*
     *  def nodes
     */
    "SSA_IP_IN",		"SSA_INDUCTIVE",	"SSA_DEF",

    "SSA_IP_KILL",

    /*
     *  mod nodes (redefs and ambiguous defs, treated as use and kill)
     */
    "SSA_PHI",			"SSA_ETA",		"SSA_GAMMA",
    "SSA_IP_MOD",

    "SSA_NO_TYPE"
};

#define INDENT_FACTOR 2

static void indentN(int n)
{
    static char format[20];

    sprintf(format, "%%%d.0s", n*INDENT_FACTOR);
    printf(format, " ");
}

/*
 *  Print out ssa kids for a particular Cfg node.
 */
void ssa_DumpKids(Generic junk, CfgInstance cfg, CfgNodeId cfgNode)
{
    SsaNodeId kid;
    int indent = 2;

    printf("\n");
    if (CFG_node(cfg, cfgNode)->ssaKids != SSA_NIL)
    {
	indentN(indent);
	printf("SSA kids in execution order: ");
	for (kid = (SsaNodeId) CFG_node(cfg, cfgNode)->ssaKids;
	     kid != SSA_NIL;
	     kid = SSA_node(cfg, kid)->nextCfgSib)
	{
	    printf("%d ", kid);
	}
	printf("\n");
	indentN(indent);
	printf("SSA kids in hierarchy:\n\n");
	for (kid = (SsaNodeId) CFG_node(cfg, cfgNode)->ssaKids;
	     kid != SSA_NIL;
	     kid = SSA_node(cfg, kid)->nextCfgSib)
	{
	    if (SSA_node(cfg, kid)->ssaParent == SSA_NIL)
		ssa_dump_node(cfg, kid, indent);
	}
    }

    return;
}

static void ssa_dump_node(CfgInstance cfg, SsaNodeId ssaId, int indent)
{
    SsaShared *ptr = SSA_node(cfg, ssaId);

    char *name;

    if (ptr->name == SSA_NIL_NAME)
    {
        if (ptr->type == SSA_CALL)
            name = gen_get_text(gen_INVOCATION_get_name(ptr->refAst));
        else
            name = "NONE";
    }
    else
    {
        name = (char *) fst_GetFieldByIndex(cfg->symtab,
                                            ptr->name, SYMTAB_NAME);
    }

    indentN(indent);
    printf("%s %d, var %s (%d), ast %d, val %d,  ",
           ssaNodeType[ptr->type], ssaId, name,
           ptr->name,
           ptr->refAst,
           ptr->value);
    if (ssa_is_def(cfg, ssaId))
        printf("nextDef %d, ", ((SsaDef *)ptr)->nextDef);

    printf("fanIn %d\n", ptr->fanIn);

    indentN(indent +1);
    printf("cfgParent %d, sib %d; ssaParent %d, sib %d\n",
	   ptr->cfgParent, ptr->nextCfgSib, ptr->ssaParent, ptr->nextSsaSib);

    printf("\n");

    if (ptr->type == SSA_GAMMA)
    {
	indentN(indent +1);
	printf("Controlling guard: %d\n", ptr->subUses);
    }
    else
	ssa_dump_node_list(cfg, ptr->subUses,
			   indent +1, "Subordinate Uses:\n");

    if (ssa_is_use(cfg, ssaId))
    {
	ssa_dump_node_list(cfg, ((SsaUse *)ptr)->subDefs,
			   indent +1, "Subordinate Defs:\n");
    }

    ssa_dump_edge_list(cfg, ptr->defsIn,
		       indent +1, "Incoming Defs:\n");

    if (ssa_is_def(cfg, ssaId))
    {
	ssa_dump_edge_list(cfg, ((SsaDef *)ptr)->defKillOuts,
			   indent +1, "Outedges to Kills:\n");

	ssa_dump_edge_list(cfg, ((SsaDef *)ptr)->defModOuts,
			   indent +1, "Outedges to Mods:\n");

	ssa_dump_edge_list(cfg, ((SsaDef *)ptr)->defUseOuts,
			   indent +1, "Outedges to Uses:\n");
    }
}

static void ssa_dump_node_list(CfgInstance cfg, SsaNodeId ssaId, int indent, 
                               char *comment)
{
    if (ssaId == SSA_NIL) return;
    
    indentN(indent);
    printf(comment);
    for (; ssaId != SSA_NIL; ssaId = SSA_node(cfg, ssaId)->nextSsaSib)
	ssa_dump_node(cfg, ssaId, indent +1);

    return;
}

static void ssa_dump_edge_list(CfgInstance cfg, SsaEdgeId edgeId, int indent, 
                               char *comment)
{
    if (edgeId == SSA_NIL) return;

    indentN(indent);
    printf(comment);

    if (strcmp(comment, "Incoming Defs:\n"))
    {
	/*
	 *  Dealing with outedges, *not* inedges
	 */
	for (; edgeId != SSA_NIL; edgeId = SSA_edge(cfg, edgeId)->outNext)
	    ssa_dump_edge(cfg, edgeId, indent +1);
    }
    else
    {
	/*
	 *  dealing with inedges
	 */
	for (; edgeId != SSA_NIL; edgeId = SSA_edge(cfg, edgeId)->inNext)
	    ssa_dump_edge(cfg, edgeId, indent +1);
    }
    printf("\n");
    return;
}


static void ssa_dump_edge(CfgInstance cfg, SsaEdgeId edgeId, int indent)
{
    SsaEdge *ptr = SSA_edge(cfg, edgeId);
    SsaType sinkType;
    char *pathEdgeMeaning;
    
    indentN(indent);

    sinkType = SSA_node(cfg, ptr->sink)->type;
    if (sinkType == SSA_PHI)		pathEdgeMeaning = "pathEdge";
    else if (sinkType == SSA_GAMMA)	pathEdgeMeaning = "guard";
    else 				pathEdgeMeaning = NULL;

    /*
     *  If this is a temporary edge before things get fully linked
     *  up in SSA construction, source may really be the variable name
     */
    if (pathEdgeMeaning)
	printf("edgeId %d, %s %d, source %d, sink %d, \n",
	       edgeId, pathEdgeMeaning, ptr->pathEdge, ptr->source, ptr->sink);
    else
	printf("edgeId %d, source %d, sink %d, \n",
	       edgeId, ptr->source, ptr->sink);

    indentN(indent +1);
    printf("inNext %d, outNext %d\n", ptr->inNext, ptr->outNext);
}



static SsaUse *ssa_use_addr(CfgInstance cfg, SsaNodeId ssaId)
{
    return SSA_use(cfg, ssaId);
}

static SsaDef *ssa_def_addr(CfgInstance cfg, SsaNodeId ssaId)
{
    return SSA_def(cfg, ssaId);
}

static SsaEdge *ssa_edge_addr(CfgInstance cfg, SsaEdgeId ssaId)
{
    return SSA_edge(cfg, ssaId);
}

void ssa_stats(CfgInstance cfg)
{
    int counts[SSA_NO_TYPE], hwm =0, used =0;
    int ssaUses = 0, ssaIP = 0, ssaDefs = 0, ssaPseudo = 0;
    char prefix;

    SsaNodeId sn;
    CfgNodeId cn;
    int type;

    if (!SSA_nodes(cfg)) return;

    prefix = SSA_doGated(cfg)? 'g' : 's';

    for (type = SSA_EDGE;
	 type <= SSA_NO_TYPE;
	 type++)
    {
	counts[type] = 0;
    }
    
    for (sn = ssa_get_first_node(cfg);
	 sn != SSA_NIL;
	 sn = ssa_get_next_node(cfg, sn))
    {
	type = SSA_node(cfg, sn)->type;

	if (type != SSA_ETA)
	    counts[type]++;
	else
	{
	    /*
	     *  Really should count one ETA node per loop exited,
	     *  when we're building GSA form.
	     */
	    CfgNodeId here = SSA_node(cfg, sn)->cfgParent;
	    CfgNodeId loop = CFG_edge(cfg, CFG_node(cfg, here)->ins)->src;
	    TarjTree loops = cfg_get_intervals(cfg);

	    if (SSA_doGated(cfg))
		counts[SSA_ETA] +=
		    max(1, tarj_exits(loops, loop, here));
	    else
		counts[SSA_ETA]++;

	    /*
	     *  But we only want to count each ETA once in determining
	     *  used vs. wasted nodes
	     */
	    used++;
	}
    }
    if (SSA_edges(cfg))
	counts[SSA_EDGE] = area_size(SSA_edges(cfg));

    for (type = SSA_EDGE;
	 type != SSA_NO_TYPE;
	 type++)
    {
	printf("\t%c%s:\t%10d\n", prefix, ssaNodeType[type], counts[type]);

	/*
	 *  Edges and ETAs are counted elsewhere
	 */
	if ((type != SSA_EDGE) && (type != SSA_ETA))
	    used += counts[type];
    }

    hwm = area_size(SSA_nodes(cfg));

    printf("\t%cSSA.nodes.hwm:\t%10d\n", prefix, hwm);
    printf("\t%cSSA.nodes.used:\t%10d\n", prefix, used);

    for (type = SSA_EDGE;
	 type <= SSA_NO_TYPE;
	 type++)
    {
	switch (type)
	{
	  case SSA_GUARD_LOGICAL:
	  case SSA_GUARD_INTEGER:
	  /* case SSA_GUARD_INDUCTIVE: just count SSA_INDUCTIVE */
	  case SSA_GUARD_ALT_RETURN:
	  case SSA_GUARD_ALT_ENTRY:
	  case SSA_GUARD_CASE:
	  case SSA_IP_REF:
	  case SSA_ACTUAL:
	  case SSA_SUBSCRIPT:
	  case SSA_ALT_RETURN:
	  case SSA_USE:
	  case SSA_LOOP_INIT:
	  case SSA_LOOP_BOUND:
	  case SSA_LOOP_STEP:
	    ssaUses += counts[type];
	    break;

	  case SSA_CALL:
	  case SSA_IP_KILL:
	  case SSA_IP_MOD:
	    ssaIP += counts[type];
	    break;

	  case SSA_IP_IN:
	  case SSA_INDUCTIVE:
	  case SSA_DEF:
	    ssaDefs += counts[type];
	    break;

	  case SSA_PHI:
	  case SSA_ETA:
	  case SSA_GAMMA:
	    ssaPseudo += counts[type];
	    break;

	  default:
	    break;
	}	    
    }
    printf("\t%cSSA.uses:\t%10d\n", prefix, ssaUses);
    printf("\t%cSSA.defs:\t%10d\n", prefix, ssaDefs);
    printf("\t%cSSA.IP:\t%10d\n", prefix, ssaIP);
    printf("\t%cSSA.pseudo:\t%10d\n", prefix, ssaPseudo);
}
