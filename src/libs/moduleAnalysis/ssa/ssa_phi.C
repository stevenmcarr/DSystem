/* $Id: ssa_phi.C,v 1.1 1997/06/25 15:10:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- ssa_phi.c
 *
 *       This file contains functions for SSA phi construction combining the
 *       algorithms given in:
 *
 *          "An Efficient Method of Computing Static Single Assignment Form"
 *           Cytron, Ferrante, Rosen, Wegman, Zadeck,  1989
 *
 *	    and
 *
 *	    "Compact Representations for Control Dependence"
 *	    Cytron, Ferrante & Sarkar, SIGPLAN '90 PLDI 
 */



#include <libs/moduleAnalysis/ssa/gstack.h>
#include <libs/moduleAnalysis/ssa/ssa_private.h>
#include <libs/moduleAnalysis/cfg/dtree.h>
#include <libs/moduleAnalysis/cfg/tarjan.h>
#include <libs/moduleAnalysis/ssa/ssa_ids.h>
#include <libs/support/time/swatch.h>

extern Swatch ssa_list_loop_exits_sw;
extern Swatch ssa_build_dom_frontier_sw;
extern Swatch ssa_process_var_sw;
extern Swatch ssa_search_sw;

STATIC(void, initialize,(CfgInstance cfg));
STATIC(void, cleanup,(CfgInstance cfg));

STATIC(void, build_dom_frontier,(CfgInstance cfg));
STATIC(void, add_df_elem,(CfgNodeId elem, CfgNodeId node));
STATIC(void, init_wklist,());
STATIC(void, process_var,(SymDescriptor symtab, fst_index_t var, Generic passCfg));
STATIC(void, add_phi,(CfgInstance cfg, fst_index_t var, CfgNodeId cfgId));
STATIC(void, search,(CfgInstance cfg, CfgNodeId root));
STATIC(void, add_ssa_edge,(CfgInstance cfg, SsaNodeId sink));
STATIC(void, add_phi_op,(CfgInstance cfg, SsaNodeId phi, CfgEdgeId pathEdge));
STATIC(SsaNodeId, top_def,(CfgInstance cfg, fst_index_t var));
STATIC(void, push_def,(CfgInstance cfg, SsaNodeId ssaId));
STATIC(void, pop_def,(CfgInstance cfg, fst_index_t var));
STATIC(void, list_loop_exits,(CfgInstance cfg));

#define INIT_PHI_WORK_SIZE     64  /* initial worklist size */

static GStack ssaWkList;
static Universe cfgNodeSets;
static CfgList *df;
static Area dfSpace;
static CfgNodeId *exitLists;

/* 
 * -- ssa_build_phis
 *
 *           Build the SSA phi node and its edges
 *           The instance is gotten from cfgInfo->currInst
 */
void ssa_build_phis(CfgInstance cfg)
{
    initialize(cfg);

    swatch_start(ssa_list_loop_exits_sw);

    list_loop_exits(cfg);

    swatch_lap(ssa_list_loop_exits_sw);
    swatch_start(ssa_build_dom_frontier_sw);

    /*
     * Build phi nodes
     */
    build_dom_frontier(cfg);

    swatch_lap(ssa_build_dom_frontier_sw);
    swatch_start(ssa_process_var_sw);

    fst_ForAll(cfg->symtab, process_var, (Generic) cfg);

    swatch_lap(ssa_process_var_sw);
    swatch_start(ssa_search_sw);

    search(cfg, cfg->start);

    swatch_lap(ssa_search_sw);

    cleanup(cfg);

} /* end of ssa_build_phis() */



static void initialize(CfgInstance cfg)
{
    int cfgSize;	/* number of cfg nodes */
    int i;

    ssaWkList = gstack_create(INIT_PHI_WORK_SIZE, sizeof(CfgNodeId));

    cfgSize = f_curr_size((Generic) cfg->cfgNodes);
    df = (CfgList *) get_mem(cfgSize * sizeof(CfgList *),
			     "dom_frontier");

    for (i=0; i < cfgSize; i++)
	df[i] = NULL;

    dfSpace = area_create(sizeof(struct cfg_generic_list_struct), NULL,
			  "ssa_phi dominance frontiers");

    cfgNodeSets = create_universe(cfgSize);
    change_universe(cfgNodeSets);

    exitLists = (CfgNodeId *) get_mem(cfgSize*sizeof(CfgNodeId),
				      "ssa_phi: loop exits");

    /*
     *  Make sure Tarjan intervals are built...
     */
    (void) cfg_get_intervals(cfg);
}



static void cleanup(CfgInstance cfg)
{
    destroy_universe(cfgNodeSets);

    gstack_destroy(ssaWkList);
    area_destroy(&dfSpace);
    
    free_mem((void*) df);
    free_mem((void*) exitLists);
}


/*
 *  --  list_loop_exits
 *
 *	List the CfgNodeIds that are loop exits with only a single inEdge.
 *	These "exit nodes" are listed under the loop exited (loops headers
 *	be such exit nodes, since they have at least two inEdges).
 *
 *	An edge may exit multiple loops at once -- so we only list the node
 *	under the outermost loop exited.  This works well, as it also keeps
 *	us from inserting duplicate eta-nodes for inner loops -- any variable
 *	with a phi on an inner loop gets one for an outer loop as well.
 *
 *	This is intended to support Gated Single Assignment form construction
 *	without too much disruption of other users.
 *
 *			-- paco, June 1992
 */
static void list_loop_exits(CfgInstance cfg)
{
    CfgNodeId node, source, loop;
    int cfgSize;

    cfgSize = f_curr_size((Generic) cfg->cfgNodes);

    for (node = 0; node < cfgSize; node++)
	exitLists[node] = CFG_NIL;

    for (node = cfg_get_first_node(cfg);
	 node != CFG_NIL;
	 node = cfg_get_next_node(cfg, node))
    {
	if (CFG_node(cfg, node)->fanIn == 1)
	{
	    source = CFG_edge(cfg, CFG_node(cfg, node)->ins)->src;
	    loop = tarj_loop_exited(cfg_get_intervals(cfg),
				    source, node);
	    if (loop != CFG_NIL)
	    {
		exitLists[node] = exitLists[loop];
		exitLists[loop] = node;
	    }
	}
    }
}


/* 
 *  -- build_dom_frontier
 * 
 *	Build dominance frontiers for all nodes using method described in
 *
 *	Compact Representations for Control Dependence
 *	Cytron, Ferrante & Sarkar SIGPLAN PLDI '90
 *
 *	For each node with multiple predecessors, for each inedge, 
 *	we record the sink, then walk up the predominator tree from 
 *	the source, adding to dominance frontiers while we have not
 *	reached the predominator of the source.          Paul Havlak 8/28/91
 */
static void build_dom_frontier(CfgInstance cfg)
{
    CfgNodeId node;	/* node being added to dominance frontiers */
    CfgNodeId chain;    /* node walking up the predom tree */
    CfgEdgeId pred;     /* predecessor cfg edge */
    DomTree predom;

    /*
     * Build all the dominance frontiers in bottom up traversal of dom tree
     */
    predom = cfg_get_predom(cfg);

    for (node = cfg_get_first_node(cfg); node != SSA_NIL;
	 node = cfg_get_next_node(cfg, node))
    {
	if (CFG_node(cfg, node)->fanIn > 1)
	{
	    for (pred = CFG_node(cfg, node)->ins; pred != SSA_NIL;
		 pred = CFG_edge(cfg, pred)->inNext)
	    {
		/*
		 *  Add all predom ancestors until we reach the predominator 
		 *  "node".
		 */
		for (chain = CFG_edge(cfg, pred)->src;
		     (chain != CFG_NIL) &&
		     (chain != DOM_idom(predom, node));
		     chain = DOM_idom(predom, chain))
		 {
		     add_df_elem(node, chain);
		 }
	    }
	}
    }
} /* end of build_dom_frontier() */



/* 
 * -- add_df_elem
 *
 *	Add "elem" to dominance frontier for "node".
 *	No longer need to check for prior membership due to 
 *	"Compact Representations" method of construction
 *	(we never try twice to add an elem to a df).
 */
static void add_df_elem(CfgNodeId elem, CfgNodeId node)
{
    CfgList dfElem;
    int temp;

    /*
     *  add to the beginning of the link list
     */
    temp = area_new(&dfSpace);
    dfElem = (CfgList) area_addr(dfSpace, temp);
    dfElem->stuff = (Generic) elem;
    dfElem->next = df[node];
    df[node] = dfElem;

}  /* end of add_df_elem() */
 



/* 
 * -- init_wklist
 *  
 *       initialize DFP, work, and worklist (contain all non-Phi defs of var)
 *       for phi placement 
 */
static void init_wklist(CfgInstance cfg, SsaDefVar *defVar)
{
    CfgNodeId node;     /* cfg node defining var */
    SsaNodeId tmp;      /* loop through all defs for var */

    defVar->work = create_set();
    defVar->DFP  = create_set();

    /* for each X in A(V) do */
    for (tmp = defVar->defList; 
	 tmp != SSA_NIL; 
	 tmp = SSA_def(cfg, tmp)->nextDef) 
    {
        node = SSA_node(cfg, tmp)->cfgParent;

        /*
	 *  Work(X) <=1;  W <= W union {X}
	 */
        gstack_push(ssaWkList, &node);
        add_number(defVar->work, node);
    }
} /* end of init_wklist() */





/* 
 *   -- process_var
 *
 *       Process this variable. (place phis for this variable)
 *       construct DFP (dominance frontier +) set for var 
 */
static void process_var(SymDescriptor symtab, fst_index_t var, Generic passCfg)
{
    CfgInstance cfg = (CfgInstance) passCfg;
    SsaDefVar *defVar;
    CfgNodeId X, Y;
    CfgList dfElem;  /* elmt of dominance frontier of node being processed */

    /*
     *  Only handle one representative of each set of overlapping 
     *  equivalenced variables
     */
    if (var != ssa_var_rep(cfg, var)) return;

    defVar = ssa_find_ref_slot(cfg, var);
    if (!defVar || (defVar->defList == SSA_NIL)) return;

    init_wklist(cfg, defVar);

    while (gstack_pop(ssaWkList, &X) )   /* while worklist is not empty */
    {
        /*
	 *  taken X from worklist, for each Y in DF(X) do
	 */
        for (dfElem = df[X];
	     dfElem;
	     dfElem = dfElem->next)
	{
            Y = (CfgNodeId) dfElem->stuff;

            /*
	     *  if DomFronPlus(Y) == false
	     */
            if (!member_number(defVar->DFP, Y))
	    {
                /*
		 *  add phi-function for var to Y;  DomFronPlus(Y) <= 1
		 */
                add_phi(cfg, defVar->var, Y);
                add_number(defVar->DFP, Y);

                /*
		 *  if Work(Y) == false
		 */
                if (!member_number(defVar->work, Y))
		{
                    /*
		     *  Work(Y) <= true;  W <= W union {Y}
		     */
		    add_number(defVar->work, Y);
                    gstack_push(ssaWkList, &Y);
                }


		/*
		 *  add SSA eta-nodes to CFG loop-exit nodes
		 */
		if (TARJ_type(cfg->intervals, Y) == TARJ_INTERVAL)
		{
		    CfgNodeId anExit;
		    SsaNodeId eta;

		    for (anExit = exitLists[Y]; anExit != CFG_NIL;
			 anExit = exitLists[anExit])
		    {
			eta = ssa_init_node(cfg, anExit, SSA_NIL, AST_NIL,
					    SSA_ETA, defVar->var);

			/*
			 *  if Work(anExit) == false
			 */
			if (!member_number(defVar->work, anExit))
			{
			    /*
			     *  inWork(anExit) <= true;  W <= W union {anExit}
			     */
			    add_number(defVar->work, anExit);
			    gstack_push(ssaWkList, &anExit);
			}
		    }
		}
            }
        }
    } /* end of while work list not empty */

} /* end of process_var() */






/* 
 * -- add_phi
 *
 *           add expression for phi function for var to node cfgId
 */
static void add_phi(CfgInstance cfg, fst_index_t var, CfgNodeId cfgId)
{
    SsaNodeId New;

    /* 
     *  could try to do something about not adding expr 
     *  if var already def'd at node, unless 
     *         1) var def'd recursively 
     *         2) other expr (RESULT, etc.) uses var at node
     *      or 3) def is "inferred"     (#2 is the time-consuming one :-) 
     */

    /* 
     *  don't add phi for induction var at an inductive do loop.  
     *  there are other times not to add it, but this is an easy one to catch 
     *  and avoids problem of looking at SSA_INDUCTIVE and SSA_PHI both
     *  to reason about value in and after loop
     */

    if (ssa_get_ind_var(cfg, cfgId) == var)
        return;

    /*
     *  ssa_init_node will take care of putting PHI after any INDUCTIVEs.
     *  This is important so that PHIs follows loop bound references,
     *  which should get values on entry, not loop-variant values.
     *
     *  It's possible because we insert a separate preheader node before 
     *  a DO loop header to be the site of any forward-control-flow 
     *  PHIs on loop entry.
     *
     *  Eh, we don't always insert preheaders.  Bug!
     *  --- fixed now that we always add loop nodes in cfg. phh July 92
     */
    New = ssa_init_node(cfg, cfgId, SSA_NIL, AST_NIL, SSA_PHI, var);

} /* end of add_phi() */




/*
 * -- search
 *
 *         build ssa graph based on dom subtree starting from 'root',
 *         adding phi function operands 
 */
static void search(CfgInstance cfg, CfgNodeId root)
{
    DomTree tree;
    SsaNodeId kid;	     	/* for each assignment loop */
    SsaShared* kidPtr; 		/* pointer to ssa node for assn */
    CfgEdgeId succ;     	/* for each Y in Succ(X) loop */
    CfgEdge* succPtr;	  	/* pointer to CFG edge structure of succ */
    CfgNodeId child;    	/* for each Y in Children(X) loop */

    if (root == SSA_NIL) return;

    tree = cfg_get_predom(cfg);

    /* 
     *  for each assignment A in node do 
     */
    for (kid = CFG_node(cfg, root)->ssaKids;
	 kid != SSA_NIL;
	 kid = SSA_node(cfg, kid)->nextCfgSib)
    {
	kidPtr = SSA_node(cfg, kid);

	/*
	 *  Here we treat uses of vars with no local definition
	 *  as uses of DUMMY_GLOBAL.
	 */

        /*
	 *  if A is ordinary assignment
	 */
        if ((kidPtr->type != SSA_PHI) &&
	    (kidPtr->type != SSA_IP_IN) &&
	    (kidPtr->name != SSA_NIL_NAME))
	{
            /*
	     *  add the edge from the (unique) reaching def
	     */
	    add_ssa_edge(cfg, kid);
        }

        /*
	 *  if a definition, push A onto def stack
	 */
        if (ssa_is_def(cfg, kid) && !ssa_ignored(cfg, kid))
	{
	    push_def(cfg, kid);
	}
	
    } /* for ssaKids */

    /*
     *  This loop hooks up top-of-stack defs with control flow successors
     *  which are not predominated by the current node.
     *  In other words, this is where the phis get their inputs.
     *
     *  for each Y in Succ(root) do  
     *  Note: Succ() refers to control flow graph 
     */
    for (succ = CFG_node(cfg, root)->outs;
	 succ != CFG_NIL;
	 succ = CFG_edge(cfg, succ)->outNext)
    {
	Boolean pastInductive;

	succPtr = CFG_edge(cfg, succ);

	/*
	 *  Note: since all nodes but PHIs were added earlier,
	 *  PHIs are the first in the list -- except for loop induction
	 *  variable expressions.
	 *
	 *  pastInductive tells us if we need to look for an
	 *  SSA_INDUCTIVE node before giving up on any more SSA_PHIs...
	 *  if pastInductive and we encounter a non-PHI node, we're done.
	 *
	 *  only need to worry if destination cfg node is the header
	 *  of an inductive DO loop.
	 */
	pastInductive = BOOL(ssa_get_ind_var(cfg, succPtr->dest) ==
			     SSA_NIL_NAME);

        /*
	 *  WhichPred(Y, root) is represented by pathEdge
	 */
        /*
	 *  for each phi-function F in Y do
	 */
	for (kid = CFG_node(cfg, succPtr->dest)->ssaKids;
	     kid != SSA_NIL;
	     kid = SSA_node(cfg, kid)->nextCfgSib)
	{
            /* replace correct operand V of F by current def for V */

            if (SSA_node(cfg, kid)->type == SSA_PHI) 
	    {
                add_phi_op(cfg, kid, succ);
	    }
	    else if (pastInductive)
		break;  /* done with all SSA_PHIs.  See Note above */
	    else 
		pastInductive = BOOL(SSA_node(cfg, kid)->type == SSA_INDUCTIVE);

        } /* while not processed all the JOINs */

    } /* while there are more successor to the CFG node */


    /* 
     * for each Y in Children(root) do  
     * Note: Children() refers to dominator tree 
     */
    child = DOM_kids(tree, root);
    while (child != SSA_NIL) 
    {
        search(cfg, child);
	child = DOM_next(tree, child);
    }

    /* for each assignment A in node do */
    for (kid = CFG_node(cfg, root)->ssaKids;
	 kid != SSA_NIL;
	 kid = SSA_node(cfg, kid)->nextCfgSib)
    {
	/*
	 *  (if was pushed to start with)  pop
	 *
	 *  It's not important that we pop the same def we're looking at --
	 *  this just makes sure that the number of pops matches the number
	 *  of pushes earlier.
	 */
	if (ssa_is_def(cfg, kid) && !ssa_ignored(cfg, kid))
	    pop_def(cfg, SSA_node(cfg, kid)->name);
    }

} /* end of search() */






/*
 * -- add_ssa_edge
 * 
 *      add SSA edge from unique reaching definition
 */
static void add_ssa_edge(CfgInstance cfg, SsaNodeId sink)
{
    fst_index_t var = ssa_var_rep(cfg, SSA_node(cfg, sink)->name);
    SsaEdgeId New;
    SsaEdge *np;
    SsaNodeId def;

    if (ssa_is_kill(cfg, sink) && !SSA_doDefKill(cfg)) return;

    def = top_def(cfg, var);

    if (def == SSA_NIL)
    {
	if (!ssa_is_kill(cfg, sink))
	{
	    /*
	     *  No reaching definition -- hook this up with DUMMY_GLOBAL
	     */
	    def = top_def(cfg, DUMMY_GLOBAL(cfg));
	}
	else
	{
	    /*
	     *  Doesn't make much sense to do that for kills
	     */
	    return;
	}
    }

    New = ssa_edge_new_id(cfg);
    np  = SSA_edge(cfg, New);

    np->source   = def;
    np->sink     = sink;
    SSA_node(cfg, sink)->fanIn++;
    np->pathEdge = CFG_NIL;		/* doesn't matter for non-phi sink */

    /*
     *  edge in 2 lists, ins list of use and outs list of def
     */
    np->inNext = SSA_node(cfg, sink)->defsIn;
    SSA_node(cfg, sink)->defsIn = New;

    /*
     *  Put in the right list of outedges from def.
     */
    if (ssa_is_use(cfg, sink))
    {
	SSA_edge(cfg, New)->outNext = SSA_def(cfg, def)->defUseOuts;
	SSA_def(cfg, def)->defUseOuts = New;
    }
    else if (ssa_is_kill(cfg, sink))
    {
	SSA_edge(cfg, New)->outNext = SSA_def(cfg, def)->defKillOuts;
	SSA_def(cfg, def)->defKillOuts = New;
    }
    else /* ssa_is_mod(cfg, sink) */
    {
	SSA_edge(cfg, New)->outNext = SSA_def(cfg, def)->defModOuts;
	SSA_def(cfg, def)->defModOuts = New;
    }
} /* end of add_ssa_edge() */






/*
 * -- add_phi_op
 *   
 *        add current def for var in expr as #arg operand in phi-fctn expr 
 */
static void add_phi_op(CfgInstance cfg, SsaNodeId phi, CfgEdgeId pathEdge)
    /* CfgEdgeId pathEdge;  last CFG edge to phi */
{
    SsaShared *phiPtr;
    SsaNodeId def;
    SsaEdgeId New;
    SsaEdge *newPtr;

    phiPtr = SSA_node(cfg, phi);

    /*
     *  Name of var in phi node is always representative of its
     *  set of equivalenced vars (or else no equivs to this var)
     */
    def = top_def(cfg, phiPtr->name);

    if (def == SSA_NIL) def = top_def(cfg, DUMMY_GLOBAL(cfg));

    New = ssa_edge_new_id(cfg);
    newPtr = SSA_edge(cfg, New);

    newPtr->source   = def;
    newPtr->sink     = phi;
    newPtr->pathEdge = pathEdge;

    /*
     *  edge in 2 lists, ins list of use and outs list of def
     */
    newPtr->inNext = SSA_node(cfg, phi)->defsIn;
    SSA_node(cfg, phi)->defsIn = New;

    newPtr->outNext = SSA_def(cfg, def)->defModOuts;
    SSA_def(cfg, def)->defModOuts = New;

    phiPtr->fanIn++;

} /* end of add_phi_op() */





/*
 * -- top_def
 *
 *      return current def for var 
 */
static SsaNodeId top_def(CfgInstance cfg, fst_index_t var)
{
    SsaDefVar *defVar;      /* info for var */
    SsaNodeId retVal;

    defVar = ssa_find_ref_slot(cfg, var);

    if (defVar && 
	defVar->stack &&
	stack_get(defVar->stack, &retVal, 1))

        return retVal;
    else
	return(SSA_NIL);

} /* end of top_def() */





/* 
 * -- push_def
 *
 *          add SSA node as current def for var it defines 
 */
static void push_def(CfgInstance cfg, SsaNodeId ssaId)
{
    SsaDefVar *defVar;      /* info for var def'd in expr */

    defVar = ssa_find_ref_slot(cfg, SSA_node(cfg, ssaId)->name);
    stack_push(defVar->stack, &ssaId);

} /* end of function push_def() */




/* 
 * -- pop_def
 *
 *     remove current def for var 
 */
static void pop_def(CfgInstance cfg, fst_index_t var)
{
    SsaDefVar *tmp;          /* info for var */
    SsaNodeId top;

    tmp = ssa_find_ref_slot(cfg, var);
    (void) stack_pop(tmp->stack, &top);

} /* end of pop_def() */
