/************************************************************************/
/*									*/
/*	ped_cp/pt/FDgraph.ansi.c ---			                */
/*              creates and destroys fusion & distribution problems.    */
/*              In distribution, SCC are detected and collapsed to a    */
/*		to a single node.		                	*/
/*									*/
/*		author: Kathryn S. McKinley				*/
/*									*/
/*	    E X P O R T E D					        */
/*		fdMake						        */
/*		fdBuildFusion						*/
/*		fdBuildDistribution					*/
/*		fdDestroyProblem					*/
/*									*/
/*	    I N T E R N A L						*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#include <private_pt.h>
#include <dep/dep_dt.h>         /* dg_ref_info_and_count()...   */
#include <PedExtern.h>

#include <FDgraph.h>	        /* FDGraph, FDNode, FDEdge	*/


#define FD_NONE -100

struct FDparams {
  PedInfo ped;
  int       fuse_result;
  int       level;
  int       amount;
};

/************************************************************************/
/*     I N T E R N A L   F O R W A R D     D E C L A R A T I O N S	*/
/************************************************************************/

EXTERN( FDGraph *, fdCreateGraph, (void) );
EXTERN( void,      fdFindFusionCanidates, 
                         (PedInfo ped, FDGraph *problem, AST_INDEX stmt) );
EXTERN( void,      fdCreateNode,
                         (FDGraph *problem, AST_INDEX loop,  
			  AST_INDEX perfect, AST_INDEX stmt) );
EXTERN( Boolean,   fdTypeLoops,
                         (PedInfo ped, FDGraph *problem, 
			  Boolean parallel, int depth) );
EXTERN( void,      fdAddFusionEdges,
                         (PedInfo ped, FDGraph *problem, 
			  Boolean parallel) );
EXTERN( void,      fdGreedyType, (FDGraph *problem, UtilList *roots, 
                          UtilList *rootcopy, UtilList *first, 
			  Boolean *visited, int *depth, 
			  Boolean wantAll, Boolean *all, Boolean *any) );
EXTERN( void,      fdFindStmts, (FDGraph *problem, AST_INDEX oloop) ); 
EXTERN( void,      fdFindCompound,
                         (FDGraph *problem, AST_INDEX oloop, AST_INDEX perfect,
			  AST_INDEX stmt) );
EXTERN( AST_INDEX, fdFindPerfectNest, (AST_INDEX oloop) );
EXTERN( void,      fdAddDistributionEdges, 
                         (PedInfo ped, FDGraph *problem, Boolean parallel) );
EXTERN( void,      fdCollapseSCC, (FDGraph *problem)  );

EXTERN( void,      fdAddFPEdge, (FDNode *node1, FDNode *node2));
EXTERN( void,      fdAddEdge, (FDNode *node1, FDNode *node2, 
			       int amount, Boolean directed));
EXTERN( void,      fdCreateEdge, (FDNode *node1, FDEdge *elist, 
				 FDNode *node2, int amount, 
				  Boolean fp, Boolean directed));

EXTERN( void,      fdPrintNode, (FDNode *node) );
EXTERN( void,      util_copy_list, (UtilList *copy, UtilList *original));
EXTERN( void,      addSuccsAllVisitedPreds, (FDNode *node,  Boolean *visited, 
					    int *mayFuse, UtilList *roots));
EXTERN( Boolean,   predsVisited, (FDNode *node, Boolean *visited));
EXTERN( void,      combineNodes, (FDNode *node1, FDNode *node2, FDEdge *edge));
EXTERN( int,       pt_loop_nesting, (AST_INDEX loop));

EXTERN( void,      fdPrint, (FDGraph *problem));

/**********************************************************************
 * fdBuildFusion --
 *     Creates a fusion problem from adjacent loops beginning with stmt.
 *     There must be two or more canidate fusion loops with conformable
 *     headers. 
 *
 * Still to decide: if the loops can be interspersed with movable 
 * statements.
 */

FDGraph *
fdBuildFusion(PedInfo ped, AST_INDEX stmt, Boolean parallel, int depth)
{
    FDGraph  *problem;

    if (depth <= 0)
	return NULL;
    
    problem           = fdCreateGraph ();
    problem->parallel = parallel;

    fdFindFusionCanidates (ped, problem, stmt);
    
    if (problem->size <= 1)
    {
	fdDestroyProblem (problem);
	return NULL;
    }
    if (fdTypeLoops (ped, problem, parallel, depth) == false)
    {
	fdDestroyProblem (problem);
	return NULL;
    }
    fdAddFusionEdges (ped, problem, parallel);

    return (problem);

}

/**********************************************************************
 * fdBuildDistribution --
 *     Creates a distribution problem for the statments in oloop.
 *     There must be two or scc for a problem to be formed.
 *      
 *  Add distributing by a perfect nest vs. a single loop ?????
 */

FDGraph *
fdBuildDistribution (PedInfo ped, AST_INDEX oloop, Boolean parallel)
{
    FDGraph *problem;

    if (!is_loop(oloop))
	return NULL;

    problem = fdCreateGraph ();
    fdFindStmts (problem, oloop);
    if (problem->size <= 1)
    {
	fdDestroyProblem (problem);
	return NULL;
    }
    problem->types  = loop_level (oloop);
    fdAddDistributionEdges (ped, problem, parallel);
    fdCollapseSCC (problem);
    return(problem);
}

FDGraph *
fdCreateGraph()
{
    FDGraph *problem;

    problem = (FDGraph *) get_mem(sizeof(FDGraph), "fdCreateGraph");
    problem->FDnodes = NULL;
    problem->FDedges = NULL;
    problem->size    = 0;
    problem->types   = 0;
    problem->roots   = util_list_alloc((int)problem, "fdCreateGraph");
    return problem;
}


/**********************************************************************
 * fdCreateNode --
 *    Allocates a FDnode, intializes all fields except parallel and 
 *    links it into the problem.
 */
void
fdCreateNode(
     FDGraph *problem, AST_INDEX loop, AST_INDEX perfect, AST_INDEX stmt)
{
    FDNode   *new, *prev;
    UtilNode *utop;

    new = (FDNode *) get_mem(sizeof(FDNode), "fdCreateNode");

    new->loop        = loop;
    new->perfect     = perfect;
    new->stmt        = stmt;
    new->dfn         = problem->size;
    new->partition   = (problem->size)++;
    new->type        = 0;
    new->depth       = 0;

    new->next              = NULL;
    new->nextNodeInThisSCC = NULL;
    new->succ              = NULL;
    new->pred              = NULL;

    /* A singley linked list of all the nodes needs to be formed.
     * The first node is set to the beginning of this list, which is stored
     * in problem->FDnodes.  The rest must be linked from the worklist, 
     * problem->roots, which is a stack of the previously encountered nodes.
     */
    if (problem->size == 1)
	problem->FDnodes = new;
    else
    {
	utop = util_head (problem->roots);
	prev = (FDNode *) util_node_atom (utop);
	prev->next = new;
    }
    utop = util_node_alloc ((Generic) new, "fdCreateNode");
    util_push (utop, problem->roots);
}


/**********************************************************************
 *  fdDestroyProblem --
 *    All memory is freed reachable from pointers in problem.
 */
void
fdDestroyProblem (FDGraph *problem)
{
    FDNode *node, *nnext;
    FDEdge *edge, *enext;

    for (node = problem->FDnodes; node != NULL; node = nnext) 
    {
	nnext = node->next;
	for (edge = node->succ; edge != NULL; edge = enext)
	{
	    enext = edge->nextSucc;
	    free_mem (edge);
	}
	free_mem (node);
    }

    util_free_nodes (problem->roots);
    util_list_free  (problem->roots);

    free_mem (problem);
}


/**********************************************************************/
/*                    F U S I O N    R O U T I N E S                  */
/**********************************************************************/



/**********************************************************************
 * fdFindFusionCanidates --
 *    Insures there are at least two loops to consider fusing.  Starts
 *    looking for loops at stmt, even if stmt is not a loop.  Loops
 *    must be contiguous (except for comments, which are ignored).
 */
void
fdFindFusionCanidates (PedInfo ped, FDGraph *problem, AST_INDEX stmt)
{
    AST_INDEX loop, loop2;

    /* Find and allocate the first pair of loops.  They may only be
     *  separated by comments.
     */
    if (is_list(stmt)) 
	stmt = list_first(stmt);
    for (loop = stmt; !is_loop(loop); loop = list_next(loop)) 
    {
	if (loop == AST_NIL) 
	{
	    problem->size = 0;
	    return;
	} 
    }
    for (loop2 = list_next(loop); is_comment(loop2); 
	 loop2 = list_next(loop2)) ;

    if (loop2 == AST_NIL || !is_loop(loop2)) 
    {
	problem->size = 0;
	return;
    }
    fdCreateNode(problem, loop,  fdFindPerfectNest(loop), 
		 gen_DO_get_stmt_LIST(loop));
    fdCreateNode(problem, loop2, fdFindPerfectNest(loop2), 
		 gen_DO_get_stmt_LIST(loop2));


    /* Find and allocate any other loops in this statement list
     */
    for (loop = list_next(loop2); loop != AST_NIL; loop = list_next(loop))
    {
	if (is_comment(loop))
	    continue;
	else if (is_loop(loop))
	    fdCreateNode(problem, loop, fdFindPerfectNest(loop), 
			 gen_DO_get_stmt_LIST(loop));
	else
	    break;
    }   
    util_free_nodes (problem->roots);

}


/**********************************************************************
 * fdTypeLoops ---
 *    Depth of 100 (FD_ALL) indicates find matching types as deeply as
 *    possible, depth of 1 to n (n << 100) indicates only try typing 
 *    and fusing at level depth or shallower.
 */

Boolean
fdTypeLoops (PedInfo ped, FDGraph *problem, Boolean parallel, int depth)
{
    FDNode    *node1, *node2, *match;        /* node1 always textually        */
                                             /* preceeds node2 in the program */
    int        i;
    Boolean    *everMatch;
    FDNode    *top        = problem->FDnodes;
    Boolean    any        = false; 
    int        depthFound = 0;
    int        deepest    = 0;
    int        types      = 0;

    everMatch = (Boolean*)malloc((problem->size + 1)*sizeof(Boolean));
    for (i = 0; i <problem->size; i++)
	everMatch[i] = false;

    top->depth     = 0;
    top->type      = 0;

    /* find conformable headers for node2 with all the previous loops  */
    for (node2 = top->next; node2 != NULL; node2 = node2->next)
    {
	match = NULL;
	depthFound = 0;
	for (node1 = top; node1 != node2; node1 = node1->next)
	{
	    any = true;
	    if (fdCompatibleHeaders (node1, node2, depth, &depthFound)
		== FUSE_OK)  		/* Found a compatible header */
	    {
		/* the first match for node2 */
		if (match == NULL)
		{
		    match = node1;
		    node2->depth = depthFound;  /* record depth of match */
		}
		/* change the match if it is deeper than a previous match 
		 * to say node a, or if the match is deeper than a previous match of 
		 * node1
		 */
		else if ((depthFound > node2->depth) && 
		         (depthFound >= node1->depth))
		{    
		    match = node1;
		    node2->depth = depthFound;
		}
	    }
	}
	/* no match, a new type */
	if (match == NULL)
	{
	    node2->type  =  ++(types);
	    node2->depth = 0;
	}
	else 
	{
	    /* If the match has not been matched before node2 gets its type
	     * and the match node gets node2's depth.
	     * Else, if the match is at the seme depth as a previous match, it 
	     * keeps the same type and node2 gets that type.
	     * Else if the match is deeper than previously found, it requires a 
	     * new type and setting of depth.
	     * Else the match is shallower than a previous one, so node2
	     * is in a singleton group with a new type.
	     */

	    i = match->dfn;
	    if (!everMatch[i]) /* first match of node1 (marked with match) */
	    {
		node2->type  = match->type;
		match->depth = node2->depth;
		everMatch[i] = true;
		everMatch[node2->dfn] = true;
	    }
	    else if (match->depth == node2->depth)
	    {
	        node2->type = match->type;
		everMatch[node2->dfn] = true;
	    }
	    else if (match->depth < node2->depth)
	    {
		match->type  = ++(types);
		match->depth = node2->depth;
		node2->type  = match->type;
		everMatch[node2->dfn] = true;
	    }
	    else if (match->depth > node2->depth)
	    {
		node2->type  =  ++(types);
		node2->depth = 0;
	    }
	}
	if (node2->depth > deepest)
	    deepest = node2->depth;
    }
    problem->types = ++types;
    problem->depth = deepest;

    free(everMatch);
    return (any);
}
/**********************************************************************
 * fdAddFusionEdges ---
 *    Given a typed set of loops compute and add dependence edges, with
 *    simple weights, and mark fusion preventing as appropriate.
 */

void
fdAddFusionEdges (PedInfo ped, FDGraph *problem, Boolean parallel)
{
    Boolean    dependences, directed;
    int        fusable, amount, total, depth;
    AST_INDEX  loop1, loop2;
    FDNode    *node1, *node2;        /* node1 always textually        */
                                     /* preceeds node2 in the program */
    FDNode    *top = problem->FDnodes;


    /* ------------------------------- */
    /* Add Edges to the Fusion problem */

    /* consider each node with respect to every node which appears 
     * before it in the program. Finding ordering and fusion-preventing
     * constraints, etc. 
     */
    for (node2 = top->next; node2 != NULL; node2 = node2->next)
    {
	for (node1 = top; node1 != node2; node1 = node1->next)
	{
	    dependences = false;
	    amount = 0;
	    total  = 0;
	    loop1 = node1->loop;
	    loop2 = node2->loop;

	    /* Add a fusion-preventing edge between nodes of different type 
	     * if there is _any_ dependence between them.
	     */
	    if (node1->type != node2->type) 
	    {
		directed = false;
		depth = max (pt_loop_nesting(loop1), pt_loop_nesting(loop2));
		fusable = pt_fuse_check_dependences (ped, loop1, loop2, depth,
						 &amount, &dependences, &directed);
		if ((fusable == FUSE_DEP_ILL) || (directed == true))
		    fdAddFPEdge (node1, node2);
		continue;
	    }
	    /* Nodes are of the same type.  Check fusion for each level
	     */
	    fusable = pt_fuse_check_dependences (ped, loop1, loop2, node1->depth,
						 &amount, &dependences, &directed);

	    /* catch any edge that is fusion preventing */
	    if ((fusable == FUSE_DEP_ILL) || 
		((fusable == FUSE_OK_CARRY) && parallel))
	    {
		fdAddFPEdge (node1, node2);
		fusable = FUSE_DEP_ILL;
	    }
	    /* Add dependence edges */
	    else if (dependences == true)
		fdAddEdge (node1, node2, amount, directed);
	}
    }
}

/* fdCompatibleHeaders  ---
 *    Determine if two loop headers are of the compatible at depth
 *    or shallower.  Returns FUSE_OK if headers are compatible at any
 *    level, *depthFound records how many nesting levels deep the headers 
 *    are compatible.  "depth" is assumed to be at least 1.
 */

int
fdCompatibleHeaders (FDNode *node1, FDNode *node2, int depth, int *depthFound)
{
    int       fusable, deep;
    AST_INDEX loop1 = node1->loop;
    AST_INDEX loop2 = node2->loop;

    /* first level must be compatible for any level to be */
    if ((fusable = pt_same_bounds (loop1,  loop2)) == FUSE_OK)
    {
	*depthFound = 1;

	/* walk the nests from next outermost to innermost  */
	for (deep = 2; 
	     ((loop1 != node1->perfect) && (loop2 != node2->perfect)); 
	     deep++ )
	{
	    /* Only consider compatibility to level depth */
	    if (deep > depth)
		return FUSE_OK;
	    
	    loop1 = pt_next_inner_loop(loop1);
	    loop2 = pt_next_inner_loop(loop2);
	    
	    /* headers are compatible at least to depth deep */
	    if (pt_same_bounds (loop1, loop2) == FUSE_OK)
		*depthFound = deep;
	    
	    /* not compatible at this level */
	    else
		break;
	}
	return FUSE_OK;
    }
    /* outermost level was not compatible */
    return fusable;
}

/* fdAddFPEdge ---
 */
void
fdAddFPEdge (FDNode *node1, FDNode *node2)
{
    FDEdge *edge, *last;

    last = NULL;
    edge = NULL;
    
    for (edge = node1->succ; edge != NULL; edge = edge->nextSucc)
    {
	/* an edge already exists between these two */
	if (edge->sink == node2)
	{
	    edge->fp       = true;
	    edge->directed = true;
	    return;
	}
	last = edge;
    }
    fdCreateEdge (node1, last, node2, 0, true, true);

}


/* fdAddEdge ---
 */
void
fdAddEdge (FDNode *node1, FDNode *node2, int amount, Boolean directed)
{
    FDEdge *edge, *last;
    
    edge = NULL;
    last = NULL;

    for (edge = node1->succ; edge != NULL; edge = edge->nextSucc)
    {
	/* an edge already exists between these two */
	if (edge->sink == node2)
	{
	    if (directed)
		edge->directed = directed;
	    edge->weight += amount;
	    return;
	}
	last = edge;
    }
    fdCreateEdge (node1, last, node2, amount, false, directed);
}


/* fdCreateEdge ---
 */
void
fdCreateEdge (FDNode *node1, FDEdge *elist, FDNode *node2, 
	      int amount, Boolean fp, Boolean directed)
{
    FDEdge *edge, *tedge;

    /* allocate and initialize the edge 
     */
    edge = (FDEdge *) get_mem(sizeof(FDEdge), "fdCreateEdge");

    edge->src       = node1;
    edge->sink      = node2;
    edge->nextSucc  = NULL;
    edge->prevSucc  = NULL;
    edge->nextPred  = NULL;
    edge->prevPred  = NULL;
    edge->weight    = amount;
    edge->fp        = fp;
    edge->directed  = directed;
    
    /* connect the doubly linked edge structure in program order 
     */
    if (node1->succ == NULL)
	node1->succ = edge;
    else
    {
	edge->prevSucc  = elist;
	elist->nextSucc = edge;
    }

    if (node2->pred == NULL)
	node2->pred = edge;
    else
    {
	for (tedge = node2->pred; 
	     tedge->nextPred != NULL; tedge = tedge->nextPred )
	    ;
	edge->prevPred  = tedge;
	tedge->nextPred = edge;
    }	
}

/**********************************************************************
 * fdGreedyFusion --- 
 *     Partitions the fusion graph based on dependences and type.  Returns 
 * true if all loops are fused into one.
 */
void
fdGreedyFusion (PedInfo ped, FDGraph *problem, 
		Boolean wantAll, Boolean *all, Boolean *any) 
{
    int        i, shallowest, deepest, depth;
    Boolean   *visited;
    UtilNode  *unode, *tnode;
    FDNode    *node, *gnode, *pred, *fnode;
    FDEdge    *edge;
    FDNode    *top   = problem->FDnodes;
    UtilList  *roots, *rootcopy, *first;

    visited = (Boolean *) get_mem(sizeof(Boolean) * (problem->size) + 1, 
				  "fdGreedyFusion");
    /*  the first node (lexigraphically) of a given type */
    first = util_list_alloc((int)top, "fdGreedyFusion");

    /*  list of the nodes with no incoming edges */ 
    roots    = util_list_alloc((int) problem, "fdGreedyFusion");
    rootcopy = util_list_alloc((int) roots, "fdGreedyFusion");

    /* use visited to fill in the first list */
    for (i = 0; i < problem->types;  i++)
	visited[i] = false;

    /*--------------------------------------------------------
     * Set up the roots, first lists, and find the type with 
     * the deepest fusions
     */
    unode = util_node_alloc ((Generic) top, "fdGreedyFusion");
    util_push (unode, roots);
    
    unode = util_node_alloc ((Generic) top, "fdGreedyFusion");
    util_push (unode, first);
    visited[top->type] = true;
    shallowest = deepest = top->depth;
    
    for (node = top->next; node != NULL; node = node->next)
    {
	node->partition = node->dfn;
	/* roots have no predecessors */
	if (node->pred == NULL)
	{
	    unode = util_node_alloc ((Generic) node, "fdCreateNode");
	    util_append (roots, unode);
	}
	/* The first node (lexigraphically) of a type? */
	if (visited[node->type] == false)
	{
	    /* order the list from the type with the deepest fusion first
	     * to the shallowest
	     */
	    visited[node->type] = true;
	    unode = util_node_alloc ((Generic) node, "fdCreateNode");

	    depth = node->depth;
	    if (depth >= deepest)
		util_push (unode, first);
	    else if (depth <= shallowest)
		util_append (first, unode);
	    else 
	    {   /* Insert into the middle of the list somewhere */
		/* tnode should never get to be NULLNODE */
		for (tnode = util_next(util_head(first)); tnode != NULLNODE; 
		     tnode = util_next(tnode))
		{
		    gnode = (FDNode *) util_node_atom(tnode);
		    if (depth < gnode->depth)
		    {
			util_insert_before(unode, tnode);
			break;
		    }
		}
	    }
	    if (node->depth > deepest)
		deepest = node->depth;
	    else if (node->depth < shallowest)
		shallowest = node->depth;
	}
    }
    /*--------------------------------------------------------
     * Greedy Fusion - start with a type with depth = deepest 
     */
    
    fdGreedyType (problem, roots, rootcopy, first, visited, &depth, 
		  wantAll, all, any); 

    problem->depth = deepest;

    util_free_nodes (roots);
    util_list_free  (roots);

    util_free_nodes (rootcopy);
    util_list_free  (rootcopy);

    util_free_nodes (first);
    util_list_free  (first);

    free_mem (visited);
}

/**********************************************************************
 * fdGreedyType --- 
 *     Fuses nodes of same type greedily
 */
void
fdGreedyType (FDGraph *problem, UtilList *roots, UtilList *rootcopy, 
	      UtilList *first, Boolean *visited, int *depth, 
	      Boolean wantAll, Boolean *all, Boolean *any) 
{
    int        fnum, dfn, t, i, nfuse;
    Boolean    fused;
    UtilNode  *unode, *tnode;
    FDNode    *node, *gnode, *pred, **fnode, *node1, *node2;
    FDEdge    *edge;
    int       *mayFuse;

    fnode = (FDNode **)malloc((problem->size + 1)*sizeof(FDNode *));
    mayFuse = (int *) get_mem(sizeof(int) * (problem->size + 1), 
				  "fdGreedyType");
    /* No partitions */
    fnum   = 0;

    /* make a copy of "roots" to find the next set of potential roots */
    util_copy_list (rootcopy, roots);

    /* first list has the nodes in order by depth to consider for fusion
     * We want to fuse the type with the deepest fusions first since it
     * hopefully offers the most opportunities for reuse and interchange.
     */
    for (unode = util_pop(first); unode != NULLNODE; unode = util_pop(first))
    {
	node = (FDNode *) util_node_atom(unode);
	util_free_node(unode);

	for (i = 0; i < problem->size; i++)
	{
	    visited[i] = false;
	    mayFuse[i] = FD_NONE;
	    fnode[i]  = NULL;
	}
	/* the type being fused */
	t = node->type;
	
	/* visit all the roots, and pop them off the stack
	 */ 
	for (tnode = util_pop(roots); tnode != NULLNODE; 
	     tnode = util_pop(roots))
	{
	    node = (FDNode *) util_node_atom(tnode);
	    util_free_node(tnode);
	    
	    dfn          = node->dfn;
	    visited[dfn] = true;
	    
	    /* all roots with the same type go into an initial partition 
	     */
	    if (node->type == t) 
	    {
		if (fnode[0] == NULL)
		{
		    node->partition = dfn;
		    fnode[0]        = node;
		}
		else
		{
		    if (wantAll)
		    {
			node->partition = fnode[0]->partition;
			combineNodes (fnode[0], node, (FDEdge *) NULL);
		    }
		    else
		    {
			node->partition = fnum;
			fnode[fnum]     = node;
			mayFuse[dfn]    = fnum++;
		    }
		}
		mayFuse[dfn] = fnum;
	    }
	    
	} 	/* The root list is now empty */

	/* One node of the type has been found, increment fnum to one. */
	if (fnode[0] != NULL)
	    fnum++;

	/* visit all the successors of the roots (use rootcopy), and put 
	 * them in roots if all their predecessors have been visited
	 */ 
	for (tnode = util_head(rootcopy); tnode != NULLNODE; 
	     tnode = util_next(tnode))
	{
	    node = (FDNode *) util_node_atom(tnode);
	    addSuccsAllVisitedPreds (node, visited, mayFuse, roots);
	}
	/* roots contains nodes whose predessors have been visited and
	 * now we visit them.  This is in breadth first order, such that
	 * we are really looking at nodes all at the same level.  So if
	 * a node does not fuse with a predecessor, it can fuse with a 
	 * "sibling" (a node at the same level) or a node that is not a 
         * predecessor.
	 */
	
	/* loop through all the nodes at this level whose preds have
	 * all been visited.
	 */
	for (tnode = util_pop(roots); tnode != NULLNODE; 
	     tnode = util_pop(roots)) 
	{
	    node = (FDNode *) util_node_atom(tnode);
	    util_free_node(tnode);
	    
	    dfn          = node->dfn;
	    visited[dfn] = true;
	    
	    if (node->type == t)          /* only fuse nodes of type t */
	    {
		/* the first node of this type */
		if (fnode[0] == NULL)
		{
		    fnode[0]        = node;
		    node->partition = dfn;
		    mayFuse[dfn]    = fnum++;
		    addSuccsAllVisitedPreds (node, visited, mayFuse, roots);
		    continue;
		}
		fused    = false;
		for (edge = node->pred; edge != NULL; edge = edge->nextPred)
		{
		    /* try to fuse with a predecessors of type t to get reuse */
		    pred = edge->src;
		    if (pred->type == t)
		    {
			if ((edge->fp) || (mayFuse[dfn] > mayFuse[pred->dfn]))
			    continue;
			
			/* FUSION! */	
			if (wantAll || (edge->weight > 0) )
			{  
			    node->partition = pred->partition;
			    mayFuse[dfn]    = mayFuse[pred->dfn];
			    combineNodes (pred, node, edge);
			    addSuccsAllVisitedPreds (pred, visited, mayFuse, roots);
			    fused = true;
			    break;
			}
		    }
		}
		/* Cannot fuse with a predecessor, if (all) find fusion with
		 * nodes that do not provide reuse
		 */
		if (!fused && wantAll)
		{			
		    /* No ancestors of type t, fuse with fnode[0]   */
		    if (mayFuse[dfn] == FD_NONE)
		    {
			/* FUSION! */
			node->partition   = fnode[0]->partition;
			mayFuse[dfn]      = mayFuse[fnode[0]->dfn];
			combineNodes (fnode[0], node, NULL);
			addSuccsAllVisitedPreds (fnode[0], visited, mayFuse, roots);
			fused  = true;
		    }
		    /* can fuse with the next higher numbered fusion 
		     * partition, if one previously exists 
		     */
		    else if ((i = mayFuse[dfn] + 1) < fnum)
		    {
			/* FUSION! */
			node->partition   = fnode[i]->partition;
			mayFuse[dfn]      = mayFuse[fnode[i]->dfn];
			combineNodes (fnode[i], node, NULL);
			addSuccsAllVisitedPreds (fnode[i], visited, mayFuse, roots);
			fused  = true;
		    }
		}
		if (!fused)
		{
		    /* A new partition must be created */
		    node->partition = dfn;
		    fnode[fnum]     = node;
		    mayFuse[dfn]    = fnum++;
		    addSuccsAllVisitedPreds (node, visited, mayFuse, roots);
		}
	    }
	    else
		addSuccsAllVisitedPreds (node, visited, mayFuse, roots);
	} 
	/* roots is empty, restore the dag roots from root copy */
	util_copy_list (roots, rootcopy);
    } 
    /* Determine if all or any of the loops were fused and how deep */
    for (i = 0; i < problem->size; i++)
	visited[i] = false;
    
    *all   = true;
    *any   = false;
    *depth = 0;
    
    node1 = problem->FDnodes;
    visited[node1->dfn] = true;
    nfuse = 0;
    for (node2 = node1->nextNodeInThisSCC; node2 != NULL; 
	 node2 = node2->nextNodeInThisSCC) 
    {
	nfuse++;
	visited[node2->dfn] = true;
	*any = true;
	*depth = max(*depth, node1->depth);
    }
    
    for (node1 = node1->next; node1 != NULL; node1 = node1->next)
    {
	if (!visited[node1->dfn])
	{
	    *all = false;
	    visited[node1->dfn] = true;
	    for (node2 = node1->nextNodeInThisSCC; node2 != NULL; 
		 node2 = node2->nextNodeInThisSCC) 
	    {
		nfuse++;
		visited[node2->dfn] = true;
		*any = true;
		*depth = max(*depth, node1->depth);
	    }
	}
    }
    problem->types = nfuse;
    free(fnode);
}


/* Performs the fusion on problem and records the depth of the deepest 
 * fusion in problem->depth.
 */
void
fdDoFusion (PedInfo ped, FDGraph *problem)
{
    int       i;
    FDNode   *node1, *node2;
    Boolean   *visited;
    
    visited = (Boolean *)malloc(sizeof(Boolean)*(problem->size +1));
    for (i = 0; i < problem->size; i++)
	visited[i] = false;
    
    for (node1 = problem->FDnodes; node1 != NULL; node1 = node1->next)
    {
	if (!visited[node1->dfn])
	{
	    visited[node1->dfn] = true;
	    for (node2 = node1->nextNodeInThisSCC; node2 != NULL; 
		 node2 = node2->nextNodeInThisSCC) 
	    {
		pt_fuse_pair(ped, node1->loop, node2->loop, node1->depth);
		visited[node2->dfn] = true;
	    }
	} 
    }
   free(visited);
}
/**********************************************************************/
/*           D I S T R I B U T I O N    R O U T I N E S               */
/**********************************************************************/


/**********************************************************************
 * fdFindStmts ---
 *     Creates nodes for statements nested inside oloop.  Statements and 
 *     non-perfectly nested loops in oloop are a single node.
 *     If a oloop encompases a perfect nest, the nest loops are
 *     distributed.  For example, in
 *    
 *         do i
 *    ->      do j 
 *               do k 
 *                   S1
 *                   S2
 *    
 *     oloop = j, then j,k is the perfect nest and distribution is tested
 *     for the j,k nest.  S1 or S2 may be loops.
 */
void
fdFindStmts (FDGraph *problem, AST_INDEX oloop)
{
    AST_INDEX  stmt, perfect;
    
    /* Use the statement list under the innermost perfectly nested 
     * loop, may be oloop.
     */
    perfect = fdFindPerfectNest (oloop);
    for (stmt = list_first(gen_DO_get_stmt_LIST(perfect)); 
	 stmt != AST_NIL; stmt = list_next(stmt))
    {
	if (!is_executable(stmt))
	    continue;
	
	if (is_compound(stmt))
	    fdFindCompound(problem, oloop, perfect, stmt);
	else
	    fdCreateNode(problem, oloop, perfect, stmt);
    } 
}

void
    fdFindCompound(FDGraph *problem, AST_INDEX oloop, AST_INDEX perfect, 
		   AST_INDEX stmt)
{
    AST_INDEX tstmt;
    
    fdCreateNode(problem, oloop, perfect, stmt);
    
    if (gen_get_node_type(stmt) == GEN_IF)
    {
	for (stmt = list_first(gen_IF_get_guard_LIST(stmt)); 
	     stmt != AST_NIL; 
	     stmt = list_next(stmt))
	{
	    for (tstmt = list_first(gen_GUARD_get_stmt_LIST(stmt)); 
		 tstmt != AST_NIL; 
		 tstmt = list_next(tstmt))
	    {
		if (!is_executable(tstmt))
		    continue;
		
		fdFindCompound(problem, oloop, perfect, tstmt);
	    }
	}
    }
}


/* fdFindPerfectNest --- Find perfect nests to distribute. Returns 
 *       the AST_INDEX of the innermost loop in the distribution 
 *       nest (at worst, oloop).  Other loops can be non-perfectly
 *       nested inside the found perfect part.
 */
AST_INDEX
fdFindPerfectNest (AST_INDEX oloop)
{
    AST_INDEX stmt, next;
    
    for (stmt = list_first(gen_DO_get_stmt_LIST(oloop)); 
	 stmt != AST_NIL; stmt = list_next(stmt))
    {
	/* skip comments between headers */
	if (!is_executable(stmt))
	    continue;
	
	if (is_loop(stmt))
	{
	    /* skip comments between footers */
	    for (next = list_next(stmt); next != AST_NIL; 
		 next = list_next(next))
		if (is_executable(next))
		    break;
	    
	    if (next == AST_NIL)
		return (fdFindPerfectNest(stmt));
	}
	else
	    return (oloop);	
    }
}
void
fdAddDistributionEdges (PedInfo ped, FDGraph *problem, Boolean parallel) 
{
    FDNode    *src, *sink;
    AST_INDEX  stmt;
    
    for (src = problem->FDnodes; src != NULL; src = src->next)
    {
	stmt = src->stmt;
    }
}

FDNode *
fdFindNode (FDGraph *problem, AST_INDEX stmt)
{
    FDNode *node;
    
    for (node = problem->FDnodes; node != NULL; node = node->next)
    {
    }    
}

void
fdCollapseSCC (FDGraph *problem)
{
    
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/

/* Local utility routines
 */

void
util_copy_list (UtilList *copy, UtilList *original)
{
    UtilNode *unode, *tnode;
    Generic  id;

    for (unode = util_tail(original); unode != NULLNODE; 
	 unode = util_prev(unode))
    {
	id = util_node_atom(unode);
	tnode = util_node_alloc (id, "util_copy_list");
	util_push (tnode, copy);
    }
    
}


Boolean
predsVisited(FDNode *node, Boolean *visited)
{
    FDNode *pred;
    FDEdge *edge;

    for (edge = node->pred; edge != NULL; edge = edge->nextPred)
    {
	pred = edge->src;
	if (visited[pred->dfn] == false)
	    return false;
    }
    return true;
}

void
addSuccsAllVisitedPreds (FDNode *node,  Boolean *visited, int *mayFuse,
			 UtilList *roots)
{
    UtilNode  *unode; 
    FDNode    *sink;
    FDEdge    *edge;

    for (edge = node->succ; edge != NULL; edge = edge->nextSucc)
    {
	sink = edge->sink;
	if (edge->fp)
	    mayFuse[sink->dfn] = 
		max (mayFuse[sink->dfn] + 1, mayFuse[node->dfn]);
	else
	    mayFuse[sink->dfn]  = max (mayFuse[sink->dfn], mayFuse[node->dfn]);

	if (predsVisited(sink, visited))
	{
	    unode = util_node_alloc ((Generic) sink, "addSuccsAllVisitedPreds");
	    util_append(roots, unode);
	}
    }
}


/* delete _edge_ (node1, node2) from the successors and predecessor 
 * lists.  
 */
void 
deleteEdgeAndFree(FDNode *node1, FDNode *node2, FDEdge *edge)
{
    FDEdge *tmp1, *pedge, *nedge; 

    if (edge != NULL)
    {
	/* --------------- */
	/* Successor List  */

	/* beginning of list */
	if (node1->succ == edge)
	{
	    nedge           = edge->nextSucc;
	    node1->succ     = nedge;
	    if (nedge != NULL)
		nedge->prevSucc = NULL;
	}
	/* end of list */
	else if (edge->nextSucc == NULL)
	{
	    pedge           = edge->prevSucc;
	    pedge->nextSucc = NULL;
	}
	else /* middle of the list */
	{
	    pedge           = edge->prevSucc;
	    nedge           = edge->nextSucc;
	    pedge->nextSucc = nedge;
	    nedge->prevSucc = pedge;
	}

	/* ----------------- */
	/* Predecessor List  */

	/* beginning of list */
	if (node2->pred == edge)
	{
	    nedge           = edge->nextPred;
	    node2->pred     = nedge;
	    if (nedge != NULL)
		nedge->prevPred = NULL;
	}
	/* end of list */
	else if (edge->nextPred == NULL)
	{
	    pedge           = edge->prevPred;
	    pedge->nextPred = NULL;
	}
	else /* middle of the list */
	{
	    pedge           = edge->prevPred;
	    nedge           = edge->nextPred;
	    pedge->nextPred = nedge;
	    nedge->prevPred = pedge;
	}
	free_mem (edge);
    }
}

/* Eliminates successor edges from node2 and node1 with same sink
 * and predecessors with the same src
 */
void
findDeleteDuplicates (FDNode *node1, FDNode *node2)
{
    FDEdge *tmp1, *tmp2, *pedge1, *pedge2;

    /* --------------- */
    /* Successor lists */

    tmp1  = node1->succ;
    tmp2  = node2->succ;
    for ( ; tmp1 != NULL && tmp2 != NULL; )
    {
	pedge1 = tmp1->prevSucc;
	pedge2 = tmp2->prevSucc;

	/* duplicate */
	if (tmp1->sink == tmp2->sink)
	{
	    /* combine the edge characteristics conservatively */
	    if (tmp2->fp)
		tmp1->fp = true;
	    if (tmp2->directed)
		tmp1->directed = true;
	    tmp1->weight = tmp1->weight + tmp2->weight;
	    
	    /* eliminate edge, tmp2 */
	    deleteEdgeAndFree (tmp2->src, tmp2->sink, tmp2);
	    
	    /* Find the next edge on both lists */
	    tmp1  = tmp1->nextSucc;
	    if (pedge2 == NULL)
		tmp2 = node2->succ;
	    else 
		tmp2 = pedge2->nextSucc;
	}
	/* since the lists are ordered, take the edges with the lower
	 * numbered sink and increment it
	 */
	else  if (tmp1->sink->dfn < tmp2->sink->dfn)
	{
	    for (tmp1 = tmp1->nextSucc ; tmp1 != NULL ; tmp1 = tmp1->nextSucc)
		if (tmp1->sink->dfn >= tmp2->sink->dfn)
		    break;
	}
	else /* (tmp1->sink > tmp2->sink) */
	{
	    for (tmp2 = tmp2->nextSucc ; tmp2 != NULL ; tmp2 = tmp2->nextSucc)
		if (tmp2->sink->dfn >= tmp1->sink->dfn)
		    break;
	}
    }

    /* ----------------- */
    /* Predecessor lists */

    tmp1  = node1->pred;
    tmp2  = node2->pred;
    for ( ; tmp1 != NULL && tmp2 != NULL; )
    {
	pedge1 = tmp1->prevPred;
	pedge2 = tmp2->prevPred;

	/* duplicate */
	if (tmp1->src == tmp2->src)
	{
	    /* combine the edge characteristics conservatively */
	    if (tmp2->fp)
		tmp1->fp = true;
	    if (tmp2->directed)
		tmp1->directed = true;
	    tmp1->weight = tmp1->weight + tmp2->weight;
	    
	    /* eliminate edge, tmp2 */
	    deleteEdgeAndFree (tmp2->src, tmp2->sink, tmp2);
	    
	    /* Find the next edge on both lists */
	    tmp1  = tmp1->nextPred;
	    if (pedge2 == NULL)
		tmp2 = node2->pred;
	    else 
		tmp2 = pedge2->nextPred;
	}
	/* since the lists are ordered, take the edges with the lower
	 * numbered sink and increment it
	 */
	else  if (tmp1->src->dfn < tmp2->src->dfn)
	{
	    for (tmp1 = tmp1->nextPred ; tmp1 != NULL ; tmp1 = tmp1->nextPred)
		if (tmp1->src->dfn >= tmp2->src->dfn)
		    break;
	}
	else /* (tmp1->src > tmp2->src) */
	{
	    for (tmp2 = tmp2->nextPred ; tmp2 != NULL ; tmp2 = tmp2->nextPred)
		if (tmp2->src->dfn >= tmp1->src->dfn)
		    break;
	}
    }
}

/* combineNodes  -- Eliminates node2 from the graph. All incoming 
 *                  and outgoing edges on node2 move to node1
 *                  Invariant:  duplicate edges are removed.
 */
void
combineNodes (FDNode *node1, FDNode *node2, FDEdge *edge)
{
    FDEdge *pedge1, *nedge2, *tmp1, *tmp2;
    FDNode *tnode;

    /* ------------------------ */
    /* Delete edge from the graph
     */
    deleteEdgeAndFree (node1, node2, edge);

    /* ----------------------------------------------- */
    /* Find and remove duplicates, keep lists in order 
     */
    findDeleteDuplicates (node1, node2);

    /* ---------------------------------------------------------- */
    /* Combine succ lists by putting node2's successors on node1's
     * successor list.  No two sinks will be the same.  Keep them 
     * in order!
     */
    tmp1 = node1->succ;
    for (tmp2 = node2->succ; tmp2 != NULL; tmp2 = nedge2)
    {
	nedge2 = tmp2->nextSucc;
	for ( ; ((tmp1 != NULL) && (tmp1->nextSucc != NULL) &&
		 (tmp1->sink->dfn < tmp2->sink->dfn));  tmp1 = tmp1->nextSucc)
	    ;

	/* Change the source to node1 */
	tmp2->src = node1;

	/* node1's list is empty, make a singleton list */
	if (tmp1 == NULL)
	{
	    tmp2->nextSucc = NULL;
	    tmp2->prevSucc = NULL;
	    node1->succ    = tmp2;
	}
	/* Put tmp2 in front of tmp1 */
	else if (tmp1->sink->dfn > tmp2->sink->dfn)
	{
	    tmp2->nextSucc = tmp1;
	    pedge1         = tmp1->prevSucc;
	    tmp1->prevSucc = tmp2;
	    tmp2->prevSucc = pedge1;
	    
	    /* first in list? */
	    if (pedge1 != NULL)
		pedge1->nextSucc = tmp2;
	}
	/* put tmp2 at the end of tmp1's list */
	else if (tmp1->nextSucc == NULL)
	{
	    tmp2->nextSucc = NULL;
	    tmp2->prevSucc = tmp1;
	    tmp1->nextSucc = tmp2;
	}
    }

    /* ---------------------------------------------------------- */
    /* Combine pred lists by putting node2's predecessors on node1's
     * predecessor list.  No two srcs will be the same.
     */
    tmp1 = node1->pred;
    for (tmp2 = node2->pred; tmp2 != NULL; tmp2 = nedge2)
    {
	nedge2 = tmp2->nextPred;
	for ( ; ((tmp1 != NULL) && (tmp1->nextPred != NULL) &&
		 (tmp1->src->dfn < tmp2->src->dfn));  tmp1 = tmp1->nextPred)
	    ;

	/* Change the source to node1 */
	tmp2->sink = node1;

	/* node1's list is empty, make a singleton list */
	if (tmp1 == NULL)
	{
	    tmp2->nextPred = NULL;
	    tmp2->prevPred = NULL;
	    node1->pred    = tmp2;
	}
	/* Put tmp2 in front of tmp1 */
	else if (tmp1->src->dfn > tmp2->src->dfn)
	{
	    tmp2->nextPred = tmp1;
	    pedge1         = tmp1->prevPred;
	    tmp1->prevPred = tmp2;
	    tmp2->prevPred = pedge1;
	    
	    /* first in list? */
	    if (pedge1 != NULL)
		pedge1->nextPred = tmp2;
	}
	/* put tmp2 at the end of tmp1's list */
	else if (tmp1->nextPred == NULL)
	{
	    tmp2->nextPred = NULL;
	    tmp2->prevPred = tmp1;
	    tmp1->nextPred = tmp2;
	}
    }

    /*----------------------------------------- */
    /* Put node2 at the end of node1's SCC list */

    /* the first node1 combination */
    if (node1->nextNodeInThisSCC == NULL)
	node1->nextNodeInThisSCC = node2;
    else
    { 
	for (tnode = node1->nextNodeInThisSCC; tnode != NULL;
	     tnode = tnode->nextNodeInThisSCC)
	{
	    if (tnode->nextNodeInThisSCC == NULL)
	    {
		tnode->nextNodeInThisSCC = node2;
		break;
	    }
	}
    }
    node2->succ = NULL;
    node2->pred = NULL;
}

int 
pt_loop_nesting (AST_INDEX loop)
{
    AST_INDEX stmt;
    int i = 1;

    for (stmt = pt_next_inner_loop(loop); 
	 stmt != AST_NIL; stmt = pt_next_inner_loop(stmt) )
	i++;
    return i;
}

/**********************************************************************/
/*            P R I N T I N G   A N D   D E B U G G I N G             */
/**********************************************************************/


void
fdPrint(FDGraph *problem)
{
    int     i;
    FDNode *node, *tnode;
    Boolean *visited;
    
    visited = (Boolean *)malloc((problem->size +1)*sizeof(Boolean));
    printf ("           FD Problem: size = %d , types = %d \n",
	    problem->size, problem->types);
    
    for (node = problem->FDnodes; node != NULL; node = node->next)
	fdPrintNode(node);

    for (i = 0; i < problem->size; i++)
	visited[i] = false;    

    for (node = problem->FDnodes; node != NULL; node = node->next)
    {
	if (visited[node->dfn] == true)
	    continue;
	else
	{
	    printf ("Partition:  ");
	    for (tnode = node; tnode != NULL; tnode = tnode->nextNodeInThisSCC )
	    {
		printf(" %d", tnode->dfn);
		visited[tnode->dfn] = true;
	    }
	    printf ("\n");
	}
    }
    free(visited);
}

void
fdPrintNode(FDNode *node)
{
    FDEdge   *edge;
    AST_INDEX stmt = node->stmt;

    printf ("Node:  %d (dfn),  partition: %d,  type: %d,  depth: %d \n   ", 
	    node->dfn, node->partition, node->type, node->depth);

/*  Don't print ast for now
    for ( ; is_list(stmt); stmt = list_first(stmt)) 
	if (is_list(stmt))
	    printf ("stmt list");

    if (is_loop(stmt))
	stmt = gen_DO_get_control(stmt);
    else if (gen_get_node_type(stmt) == GEN_IF)
	stmt = list_first(gen_IF_get_guard_LIST(
                          list_first(gen_GUARD_get_stmt_LIST(stmt))));
    tree_print(stmt);
    printf ("DO:  ", node->loop);
    tree_print (gen_DO_get_control(node->loop));
    
*/
    if (node->succ != NULL)
	printf("Succ Edges (src -> sink): \n");
    for (edge = node->succ; edge != NULL; edge = edge->nextSucc) 
    {
	printf ("    %d -> %d   weight %d", edge->src->dfn, 
		                            edge->sink->dfn, edge->weight);
	if (edge->fp)
	    printf("  fusion preventing");
	printf ("\n");
    }
    if (node->pred != NULL)
	printf("Pred Edges (src -> sink): \n");
    for (edge = node->pred; edge != NULL; edge = edge->nextPred) 
	printf ("    %d -> %d   weight %d\n", edge->src->dfn, 
		                              edge->sink->dfn, edge->weight);
}


