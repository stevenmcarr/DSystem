/* $Id: ssa_gated.C,v 1.1 1997/06/25 15:10:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- ssa_gated.c
 *
 *	Code to build dags of Gamma nodes to replace Phi nodes,
 *	as described in a paper I'm working on.
 *
 *	Also adds builds loop-termination predicates and inserts
 *	edges to SSA_ETA nodes.
 *
 *	After this pass, any remaining SSA_PHI nodes can be considered
 *	the same as Mu nodes (loop-entry merge).
 *
 *						-- paco, June 1992
 *
 *	Each SSA_PHI is replaced with an SSA_GAMMA, which has inputs:
 *
 *	* the SSA_GUARD_* node controlling the merge (in the subUses field)
 *
 *	* value inputs (as inedges), including
 *
 *		-- other SSA_GAMMAs created at the same merge point
 *			(only if multiple predicates control the merge)
 *
 *		-- SSA nodes (possibly including SSA_GAMMAs) 
 *			at other CFG nodes 
 */

#include <include/bstring.h>

#include <libs/moduleAnalysis/ssa/gstack.h>
#include <libs/moduleAnalysis/ssa/ssa_private.h>
#include <libs/moduleAnalysis/cfg/dtree.h>
#include <libs/moduleAnalysis/cfg/tarjan.h>
#include <libs/moduleAnalysis/ssa/ssa_ids.h>
#include <libs/moduleAnalysis/cfg/cfg_utils.h>

STATIC(void, add_gamma_op,(CfgInstance cfg, SsaNodeId sink, int label,
                           SsaNodeId source));
STATIC(void, build_gamma_dag,(CfgInstance cfg, SsaNodeId phi));
STATIC(SsaNodeId, build_gamma,(CfgInstance cfg, CfgNodeId node));
STATIC(void, build_selectors,(CfgInstance cfg, SsaNodeId phi));
STATIC(void, do_prewalk,(CfgInstance cfg));
STATIC(void, do_postwalk,());
STATIC(void, do_loop_walk,(CfgInstance cfg, CfgNodeId node));
STATIC(void, finalize,(void));
STATIC(void, initialize,(CfgInstance cfg));
STATIC(void, process_choice,(CfgInstance cfg, Generic choice, Boolean isLeaf,
                             int label, CfgNodeId node));

static CfgNodeId current;	/* CFG merge (or end-loop) being processed */
static CfgNodeId predom;	/* predominator of current */
static fst_index_t var;		/* the variable we're building a dag for */
static Universe cfgNodeBits;	/* set universe for selectors */
static Set selectors;		/* branch nodes that help decide this merge */
static short * visits;		/* number of visits to a branch node */
static SsaNodeId *dags;		/* gamma dag built for this node, if any */
static Generic * choices;	/* values or branches that branches decide on */
static Generic ** myChoices;	/* pointers to the choices for each branch */
static Universe choiceBits;	/* set universe for choiceIsLeaf */
static Set choiceIsLeaf;	/* if true, corresponding choice is a value */
static Generic * firstChoice;	/* first choice registered for a branch */
static Set firstIsLeaf;		/* first choice is a leaf or not? */
static SsaNodeId *myGuard;	/* guard, if CFG node is a branch */

#define BITNUM(node, label)	((myChoices[node] - choices) + label)

void ssa_convert_phis(CfgInstance cfg)
  // CfgInstance cfg;
{
    /*
     *  Just in case we're not called from ssa_Open...
     */
    SSA_doGated(cfg) = true;

    /*
     *  Make sure this information is around...
     */
    if (!cfg->cfgCdEdges)
	(void) cfg_cd_first_from(cfg, cfg->start);

    (void) cfg_get_predom(cfg);
    (void) cfg_get_postdom(cfg);
    (void) cfg_get_intervals(cfg);

    initialize(cfg);

    for (current = cfg_get_first_node(cfg);
	 current != CFG_NIL;
	 current = cfg_get_next_node(cfg, current))
    {
	/*
	 *  Don't bother with non-merges and loop merges
	 *  (loop merges have forward fanin of 1 since preheaders were added)
	 */
	if (cfg_get_forward_fanin(cfg, current) > 1)
	{
	    SsaNodeId phi;
	    SsaNodeId *follow;

	    predom = DOM_idom(cfg->predom, current);

	    /*
	     *  Phis come at the beginning of the kid list
	     *  (except at loop headers, which we're not looking at
	     */
	    for (phi = CFG_node(cfg, current)->ssaKids;
		 (phi != SSA_NIL) && (SSA_node(cfg, phi)->type == SSA_PHI);
		 phi = SSA_node(cfg, phi)->nextCfgSib)
	    {
		var = SSA_node(cfg, phi)->name;

		do_prewalk(cfg/*, current*/);

		/*
		 *  selectors set is initially empty
		 */
		build_selectors(cfg, phi);

		build_gamma_dag(cfg, phi);
#ifdef DEBUG		
		do_postwalk(cfg, current);
#endif
	    }
	    /*
	     *  Removing the PHIs is a little tricky...
	     *  (do not move this code where it can hit loop PHIs)
	     */
	    for (follow = (SsaNodeId *)&(CFG_node(cfg, current)->ssaKids);
		 *follow != SSA_NIL;
		 )
	    {
		if (SSA_node(cfg, *follow)->type == SSA_PHI)
		    ssa_node_free(cfg, *follow);
		else
		    follow = &(SSA_node(cfg, *follow)->nextCfgSib);
	    }
	}
	/*
	 *  Having trouble with eta-construction for now...
	 */
	/* continue; */
	
	if ((CFG_node(cfg, current)->fanOut == 1) &&
	    (CFG_edge(cfg, CFG_node(cfg, current)->outs)->dest ==
	     TARJ_outer(cfg->intervals, current)))
	{
	    CfgEdgeId loopEdge;
	    Boolean simpleExit = true;

	    /*
	     *  Node is a postbody, last node in a loop iteration
	     *
	     *  Build the loop-termination predicate
	     */
	    CfgNodeId loop = TARJ_outer(cfg->intervals, current);

	    for (predom = loop;
		 DOM_is_dom(cfg->predom,
			    DOM_idom(cfg->postdom, predom),
			    current);
		 predom = DOM_idom(cfg->postdom, predom))
		;

	    clear_set(selectors);
	    
	    do_loop_walk(cfg, current);

	    var = SYM_INVALID_INDEX;

	    for (loopEdge = CFG_node(cfg, predom)->outs;
		 loopEdge != CFG_NIL;
		 loopEdge = CFG_edge(cfg, loopEdge)->outNext)
	    {
		int label = CFG_edge(cfg, loopEdge)->label;
		int bitnum = BITNUM(predom, label);

		if (!member_number(choiceIsLeaf, bitnum))
		{
		    simpleExit = false;
		    break;
		}
	    }

	    if (simpleExit)
		SSA_loopPredicates(cfg)[loop] = myGuard[predom];
	    else
		SSA_loopPredicates(cfg)[loop] = build_gamma(cfg, predom);
	}
    }
    finalize();
}


static void do_loop_walk(CfgInstance cfg, CfgNodeId node)
      // CfgInstance cfg;
      // CfgNodeId node;
{
    CfgEdgeId cd;

    for (cd = CFG_node(cfg, node)->cdIns;
	 cd != CFG_NIL;
	 cd = CFG_cdedge(cfg, cd)->inNext)
    {
	CfgNodeId source = CFG_cdedge(cfg, cd)->src;
	int cdLabel = CFG_cdedge(cfg, cd)->label;

	if (CFG_cdedge(cfg, cd)->level != LOOP_INDEPENDENT)
	    continue;

	if (DOM_is_dom(cfg->predom, predom, source))
	{
	    if (!member_number(selectors, source))
	    {
		CfgEdgeId cfEdge;

		for (cfEdge = CFG_node(cfg, source)->outs;
		     cfEdge != CFG_NIL;
		     cfEdge = CFG_edge(cfg, cfEdge)->outNext)
		{
		    int label = CFG_edge(cfg, cfEdge)->label;

		    myChoices[source][label] = SSA_LOOP_FALSE;
		    
		    add_number(choiceIsLeaf, BITNUM(source, label));
		}
		dags[source] = SSA_NIL;
		add_number(selectors, source);
		do_loop_walk(cfg, source);
	    }

	    if (node == current) myChoices[source][cdLabel] = SSA_LOOP_TRUE;
	    else
	    {
		myChoices[source][cdLabel] = node;
		delete_number(choiceIsLeaf, BITNUM(source, cdLabel));
	    }
	}
    }
}


static void build_selectors(CfgInstance cfg, SsaNodeId phi)
  // CfgInstance cfg;
  // SsaNodeId phi;
{
    CfgEdgeId cfEdge;
    SsaEdgeId dfEdge;
    SsaNodeId def;

    for (dfEdge = SSA_node(cfg, phi)->defsIn;
	 dfEdge != CFG_NIL;
	 dfEdge = SSA_edge(cfg, dfEdge)->inNext)
    {
	/*
	 *  Note that because of the addition of preheader nodes
	 *  and our skipping of loop headers, this edge cannot be
	 *  loop-carried.
	 */
	cfEdge = SSA_edge(cfg, dfEdge)->pathEdge;
	def = SSA_edge(cfg, dfEdge)->source;

	process_choice(cfg, (Generic) def, /* isLeaf = */ true,
		       (int) CFG_edge(cfg, cfEdge)->label,
		       CFG_edge(cfg, cfEdge)->src);
    }
}


static void process_choice(CfgInstance cfg, Generic choice, Boolean isLeaf, 
			   int label, CfgNodeId node)
  // CfgInstance cfg;
  // Generic choice;
  // Boolean isLeaf;
  // int label;
  // CfgNodeId node;
{
    Generic newChoice = choice;
    Boolean newChoiceIsLeaf = isLeaf;
    CfgEdgeId cd;

    if (CFG_node(cfg, node)->fanOut > 1)
    {
	myChoices[node][label] = choice;
	if (isLeaf)
	    add_number(choiceIsLeaf, BITNUM(node, label));
	else
	    delete_number(choiceIsLeaf, BITNUM(node, label));

	visits[node]--;
	if (firstChoice[node] == CFG_NIL)
	{
	    firstChoice[node] = choice;
	    if (isLeaf)
		add_number(firstIsLeaf, node);
	}
	else
	{
	    if ((firstChoice[node] != choice) ||
		BOOL(member_number(firstIsLeaf, node)) != BOOL(isLeaf))
	    {
		add_number(selectors, node);
	    }
	}
	if (visits[node] || (node == predom)) return;
	if (member_number(selectors, node)) 
	{
	    newChoice = (Generic) node;
	    newChoiceIsLeaf = false;
	}
    }

    for (cd = CFG_node(cfg, node)->cdIns;
	 cd != CFG_NIL;
	 cd = CFG_cdedge(cfg, cd)->inNext)
    {
	if (CFG_cdedge(cfg, cd)->level == LOOP_INDEPENDENT)

	    process_choice(cfg, newChoice, newChoiceIsLeaf,
			   (int) CFG_cdedge(cfg, cd)->label,
			   CFG_cdedge(cfg, cd)->src);
    }
}



static SsaNodeId build_gamma(CfgInstance cfg, CfgNodeId node)
  // CfgInstance cfg;
  // CfgNodeId node;
{
    CfgEdgeId edge;

    if (node == CFG_NIL)
	return SSA_NIL;

    if (dags[node] != SSA_NIL) return dags[node];

    /*
     *  Add the new gamma first (before its inputs) so that
     *  it will be positioned after its inputs in the ssaKids list.
     */
    dags[node] = ssa_init_node(cfg, current, SSA_NIL, AST_NIL, SSA_GAMMA, var);

    for (edge = CFG_node(cfg, node)->outs;
	 edge != CFG_NIL;
	 edge = CFG_edge(cfg, edge)->outNext)
    {
	int label = CFG_edge(cfg, edge)->label;
	int bitnum = BITNUM(node, label);

	if (!member_number(choiceIsLeaf, bitnum))
	{
	    myChoices[node][label] = build_gamma(cfg, myChoices[node][label]);
	}
    }

    for (edge = CFG_node(cfg, node)->outs;
	 edge != CFG_NIL;
	 edge = CFG_edge(cfg, edge)->outNext)
    {
	int label = CFG_edge(cfg, edge)->label;

	add_gamma_op(cfg, dags[node], label, myChoices[node][label]);
    }

    SSA_node(cfg, dags[node])->subUses = myGuard[node];

    return dags[node];
}


static void build_gamma_dag(CfgInstance cfg, SsaNodeId phi)
      // CfgInstance cfg;
      // SsaNodeId phi;
{
    SsaNodeId root;	/* the SSA_GAMMA that anchors a multi-gamma dag */
    SsaEdgeId edge;

    /*
     *  This builds the gamma dag and hooks it up to its inputs
     */
    root = build_gamma(cfg, predom);

    /*
     *  Still need to attach the root of the gamma dag to the phi outputs
     */
    SSA_def(cfg, root)->defKillOuts = SSA_def(cfg, phi)->defKillOuts;

    for (edge = SSA_def(cfg, phi)->defKillOuts;
	 edge != SSA_NIL;
	 edge = SSA_edge(cfg, edge)->outNext)
    {
	SSA_edge(cfg, edge)->source = root;
    }
    SSA_def(cfg, phi)->defKillOuts = SSA_NIL;

    SSA_def(cfg, root)->defModOuts = SSA_def(cfg, phi)->defModOuts;

    for (edge = SSA_def(cfg, phi)->defModOuts;
	 edge != SSA_NIL;
	 edge = SSA_edge(cfg, edge)->outNext)
    {
	SSA_edge(cfg, edge)->source = root;
    }
    SSA_def(cfg, phi)->defModOuts = SSA_NIL;

    SSA_def(cfg, root)->defUseOuts = SSA_def(cfg, phi)->defUseOuts;

    for (edge = SSA_def(cfg, phi)->defUseOuts;
	 edge != SSA_NIL;
	 edge = SSA_edge(cfg, edge)->outNext)
    {
	SSA_edge(cfg, edge)->source = root;
    }
    SSA_def(cfg, phi)->defUseOuts = SSA_NIL;
}



static void prewalk(CfgInstance cfg, CfgNodeId source)
  //CfgInstance cfg;
  //CfgNodeId source;
{
    CfgEdgeId cd;
    CfgEdgeId cfgEdge;

    if (CFG_node(cfg, source)->fanOut > 1)
    {
	visits[source]++;
	if (visits[source] == 1)
	{
	    delete_number(selectors, source);
	    dags[source] = SSA_NIL;

	    for (cfgEdge = CFG_node(cfg, source)->outs;
		 cfgEdge != CFG_NIL;
		 cfgEdge = CFG_edge(cfg, cfgEdge)->outNext)
	    {
		myChoices[source][CFG_edge(cfg, cfgEdge)->label] = CFG_NIL;
	    }
	    firstChoice[source] = CFG_NIL;
	    delete_number(firstIsLeaf, source);

	    if (source == predom) return;
	}
	else
	    return;
    }
    for (cd = CFG_node(cfg, source)->cdIns;
	 cd != CFG_NIL;
	 cd = CFG_cdedge(cfg, cd)->inNext)
    {
	if (CFG_cdedge(cfg, cd)->level == LOOP_INDEPENDENT)

	    prewalk(cfg, CFG_cdedge(cfg, cd)->src);
    }
}


static void do_prewalk(CfgInstance cfg)
  // CfgInstance cfg;
{
    CfgEdgeId inEdge;

    for (inEdge = CFG_node(cfg, current)->ins;
	 inEdge != CFG_NIL;
	 inEdge = CFG_edge(cfg, inEdge)->inNext)
    {
	prewalk(cfg, CFG_edge(cfg, inEdge)->src);
    }
}


static void postwalk(CfgInstance cfg, CfgNodeId source)
  //CfgInstance cfg;
  //CfgNodeId source;
{
    CfgEdgeId cd;

    if (CFG_node(cfg, source)->fanOut > 1)
    {
	if (visits[source] != 0)
	    fprintf(stderr, "ssa_gated.c:postwalk -- nonzero visits at %d\n",
		    source);

	if (source == predom) return;
    }

    for (cd = CFG_node(cfg, source)->cdIns;
	 cd != CFG_NIL;
	 cd = CFG_cdedge(cfg, cd)->inNext)
    {
	if (CFG_cdedge(cfg, cd)->level == LOOP_INDEPENDENT)

	    postwalk(cfg, CFG_cdedge(cfg, cd)->src);
    }
}

    

static void do_postwalk(CfgInstance cfg)
  //CfgInstance cfg;
{
    CfgEdgeId inEdge;

    for (inEdge = CFG_node(cfg, current)->ins;
	 inEdge != CFG_NIL;
	 inEdge = CFG_edge(cfg, inEdge)->inNext)
    {
	postwalk(cfg, CFG_edge(cfg, inEdge)->src);
    }
}
 

static void add_gamma_op(CfgInstance cfg, SsaNodeId sink, int label, SsaNodeId source)
//     CfgInstance cfg;
//     SsaNodeId sink;
//     int label;
//     SsaNodeId source;
{
    SsaEdgeId New;
    SsaEdge  *newPtr;

    New = ssa_edge_new_id(cfg);
    newPtr = SSA_edge(cfg, New);

    newPtr->source   = source;
    newPtr->sink     = sink;
    newPtr->pathEdge = label;

    /*
     *  edge in 2 lists, ins list of use and outs list of def
     */
    newPtr->inNext = SSA_node(cfg, sink)->defsIn;
    SSA_node(cfg, sink)->defsIn = New;

    SSA_node(cfg, sink)->fanIn++;

    if ((source != SSA_LOOP_FALSE) &&
	(source != SSA_LOOP_TRUE) &&
	(source != SSA_NIL))
    {
	newPtr->outNext = SSA_def(cfg, source)->defModOuts;
	SSA_def(cfg, source)->defModOuts = New;
    }
}


static void initialize(CfgInstance cfg)
  // CfgInstance cfg;
{
    int nNodes = f_curr_size((Generic) cfg->cfgNodes);
    int nChoices;
    CfgNodeId node;
    CfgNodeId *tChoices;

    var = SYM_INVALID_INDEX;
    current = predom = CFG_NIL;

    /*
     *  per CfgNodeId
     */
    cfgNodeBits = create_universe(nNodes);
    change_universe(cfgNodeBits);
    selectors   = create_set();

    visits = (short *) get_mem(sizeof(short)*nNodes,
			       "ssa_gated.c: visits count array");
    bzero((char *) visits, sizeof(short)*nNodes);

    dags = (SsaNodeId *) get_mem(sizeof(SsaNodeId)*nNodes,
				 "ssa_gated.c: dags array");

    firstChoice = (CfgNodeId *) get_mem(sizeof(CfgNodeId)*nNodes,
					"ssa_gated.c: firstChoice array");

    change_universe(cfgNodeBits);
    firstIsLeaf = create_set();

    myChoices = (Generic **) get_mem(sizeof(Generic *)*nNodes,
				     "ssa_gated.c: myChoices array");
    bzero((char *) myChoices, sizeof(Generic *)*nNodes);

    myGuard = (SsaNodeId *) get_mem(sizeof(SsaNodeId)*nNodes,
				    "ssa_gated.c: myGuard array");

    /*
     *  per Choice (almost per CfgEdgeId)
     *
     *  Allow a little slop because n edge labels can range [-1:n-1], 
     *  [0:n], or [1:n+1] depending on the source type.  Easiest solution 
     *  is to allow space for n+2 choices per node.
     *
     *  Note that we never initialize choice entries that we aren't
     *  going to use, so the slop doesn't cost us much.
     *
     *	No, go ahead and initialize, to prevent Heisenbugs. --paco
     */
    nChoices = f_curr_size((Generic) cfg->cfgEdges) + 2*nNodes;

    choiceBits = create_universe(nChoices);
    change_universe(choiceBits);
    choiceIsLeaf = create_set();

    choices = (Generic *) get_mem(sizeof(Generic)*nChoices,
				  "ssa_geted.c: choices array");
#ifdef LONG_POINTER
    long_set(choices, nChoices, (long)SSA_NIL);
#else
    int_set(choices, nChoices, (long)SSA_NIL);
#endif

    /*
     *  initialize myChoices pointers
     */
    tChoices = choices +1;

    for (node = cfg_get_first_node(cfg);
	 node != CFG_NIL;
	 node = cfg_get_next_node(cfg, node))
    {
	SsaNodeId kid;

	if (CFG_node(cfg, node)->fanOut > 1)
	{
	    myChoices[node] = tChoices;
	    tChoices += (CFG_node(cfg, node)->fanOut + 2);

	    for (kid = CFG_node(cfg, node)->ssaKids;
		 kid != SSA_NIL;
		 kid = SSA_node(cfg, kid)->nextCfgSib)
	    {
		if (ssa_is_guard(cfg, kid))
		{
		    myGuard[node] = kid;
		    break;
		}
	    }
	}
    }

    /*
     *  Initialize loop predicate information
     */
    if (SSA_loopPredicates(cfg))
	free_mem((void*) SSA_loopPredicates(cfg));

    
    SSA_loopPredicates(cfg) = (SsaNodeId *) get_mem(sizeof(SsaNodeId *)*nNodes,
					    "ssa_gated.c: loop predicates");
#ifdef LONG_POINTER
    long_set(SSA_loopPredicates(cfg), nNodes, (long)SSA_NIL);
#else
    int_set(SSA_loopPredicates(cfg), nNodes, (long)SSA_NIL);
#endif
}


static void finalize()
{
    destroy_universe(cfgNodeBits);
    destroy_universe(choiceBits);

    free_mem((void*) visits);
    free_mem((void*) dags);
    free_mem((void*) firstChoice);
    free_mem((void*) myChoices);
    free_mem((void*) choices);
    free_mem((void*) myGuard);
}
