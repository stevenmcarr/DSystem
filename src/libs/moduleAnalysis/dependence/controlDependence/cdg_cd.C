/* $Id: cdg_cd.C,v 1.1 1997/06/25 15:05:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Id: cdg_cd.C,v 1.1 1997/06/25 15:05:47 carr Exp $
*/

/************************************************************************/
/*									*/
/*	File:	dep/cd/cdg_cd.c						*/
/*	Author:	Kathryn McKinley					*/
/*	Previous Name: ped_cp/dg/cdg_dg.c				*/
/*									*/
/*	Routines:							*/
/*	   Externally available:					*/
/*		ControlDep *dg_build_cdg ()				*/
/*		Boolean     dg_delete_cds ()				*/
/*		void        cdg_free ()					*/
/*		void        cdg_delete_edge ()				*/
/*		int         cdg_walk_nodes ()				*/
/*         Internal use:						*/
/*		static int  find_cdg_edges ()				*/
/*		Boolean     may_be_cd_sink ()  				*/
/*		void        cdg_print ()   				*/
/*									*/
/*									*/
/************************************************************************/


#include <libs/frontEnd/ast/treeutil.h>

#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/support/lists/list.h>

#include <libs/moduleAnalysis/dependence/controlDependence/cd_graph.h>
					/* ControlDep, cdEdge, cdNode	*/

#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>

#include <libs/moduleAnalysis/dependence/controlDependence/private_cd.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/frontEnd/ast/groups.h>

/* Local forward declarations 
 */
void	collect_cdg_edges();
STATIC(void,	cdg_place_edge, (ControlDep *cd, cdEdge *newedge));
STATIC(int,	find_cdg_edges, (AST_INDEX stmt, int nesting_level, Generic Parm));
STATIC(int,	delete_cd_edges, (AST_INDEX stmt, int nesting_level, Generic Parm));
STATIC( int,	cdg_walk_nodes_recur, 
		( cdNode * node, cdg_action_callback pre_action, 
		 cdg_action_callback post_action, Generic parm ) );


/* Parameter structure passed by walk_statements in find_cdg_edges &
 * delete_cd_edges.
 */
typedef struct
{
    DG_Instance	*dg;
    SideInfo	*infoPtr;
    UtilList	*elist;
    UtilList	*nlist;
    int		 e, n;
    Boolean      firstGuard;
    AST_INDEX    root;
    Boolean      check;
    Boolean      contained;
} findEdgeParms;



/************************************************************************/
/*									*/
/* dg_build_cdg -- copies the control dependences from the dependence   */
/*		graph, forming and returning a rooted, ordered control 	*/
/*		dependence graph.					*/
/*									*/
/************************************************************************/
ControlDep *
dg_build_cdg(DG_Instance *dg, SideInfo *infoPtr, AST_INDEX root, double extra)
{
    findEdgeParms  parm;
    cdEdge	  *edges, *edge, *tedge;
    cdNode	  *nodes;
    ControlDep    *cd;
    UtilNode	  *enode, *nnode;
    AST_INDEX	   stmt;
    int		   i, j, d, sinknode, nesting;
    DG_Edge	  *dgEdge  = dg_get_edge_structure(dg);

    /* initialization */
    parm.elist	= util_list_alloc (root, "collect_cdg_edges");
    parm.nlist	= util_list_alloc (root, "collect_cdg_edges");
    parm.dg	= dg;
    parm.infoPtr= infoPtr;
    parm.e	= 0;
    parm.n	= 0;
    parm.firstGuard = true;

    if ((int)extra <= 0)
	extra = 1.0;

    /**************************** 	
     * Walk all the statements in a module, starting at the root 
     * Coping the control edges out of the dependence graph. 
     */

    nesting = loop_level(root);
	
    walk_statements( root, nesting, find_cdg_edges,
		     NOFUNC, (Generic) &parm );
    if (parm.e == 0)
    { 
	printf ("No control dep edges? in dg_build_cdg\n");
	return (NULL);
    }
    /****************************
     * Allocate and initialize the control dependence structures 
     */
    edges = (cdEdge *) get_mem ((int)(parm.e * extra) * sizeof(cdEdge),  
				"dg_build_cdg - edges");
    nodes = (cdNode *) get_mem ((int)(parm.n * extra) * sizeof(cdNode),  
				"dg_build_cdg - nodes");
    cd    = (ControlDep *) get_mem (sizeof(ControlDep), "dg_build_cdg - cd");

    /****************************
     * Put the graph together using the newly formed edge and node lists.
     */

    /* initialize the nodes */
    i = 0;
    for (nnode = UTIL_HEAD(parm.nlist); nnode != NULLNODE; 
	 nnode = UTIL_NEXT(nnode))
    {
	stmt = UTIL_NODE_ATOM(nnode);
	
	nodes[i].index = i;
	nodes[i].pred  = NULL;
	nodes[i].succ  = NULL;
	nodes[i].stmt  = stmt;

	/* this field is used in loop distribution */
	nodes[i].lda   = NULL;

	/* map from the ast to the node number, so we can find them from
	 * the edges directly.
	 */
	dg_put_info( infoPtr, stmt, type_cd_map, i);
	
	i++;
    }

    /* add the edges */
    enode  = UTIL_HEAD(parm.elist);
    d      = UTIL_NODE_ATOM(enode);
    j      = 0;
    i      = 0;
    for (nnode = UTIL_HEAD(parm.nlist); nnode != NULLNODE; 
	 nnode = UTIL_NEXT(nnode))
    {
	stmt = UTIL_NODE_ATOM(nnode);
	
	/* Find all the control dependence successors for this node */
	for ( ; (j < parm.e) && (dgEdge[d].src == stmt); d = UTIL_NODE_ATOM(enode))
	{ 
	    sinknode = dg_get_info( infoPtr, dgEdge[d].sink, type_cd_map);

	    /* create the new edge */
	    edges[j].index  = j;
	    edges[j].src    = &(nodes[i]);
	    edges[j].sink   = &(nodes[sinknode]);
	    edges[j].level  = dgEdge[d].level;
	    edges[j].label  = dgEdge[d].cdlabel;
	    edges[j].dgedge = d;

	    /*
	     * Avoid the situation where you have an ALTERNATE_ENTRY edge
	     * from a SUBROUTINE node to itself. Treat this edge as a
	     * special case and don't add it to the graph.  -NM [6/18/92]
	     */
	    if (dgEdge[d].cdlabel != CD_ALTERNATE_ENTRY ||
		edges[j].src != edges[j].sink) {
	      
	      /* put the edge in the cd graph */
	      cdg_place_edge (cd, &(edges[j]));

	    } else {
	      /* fprintf(stderr, "removing CD_ALTERNATE_ENTRY edge\n"); */
	      parm.e--;     /* remove this edge */
	    }

	    j++;
	    if ((enode = UTIL_NEXT(enode)) == NULLNODE)
		break;
	}
	i++;
    }
    /* cdg fields */
    cd->top       = root;
    cd->nodes     = nodes;
    cd->edges     = edges;
    cd->ntotal    = parm.n;
    cd->noriginal = parm.n;
    cd->etotal    = parm.e;
    cd->newedges  = extra * parm.e - parm.e;
    cd->newnodes  = extra * parm.n - parm.n;

    /* other pointer fields */
    cd->lddesc = NULL;
    
    /*	dg_print_cdg(cd);      */
    return (cd);    
}



/************************************************************************/
/*									*/
/* cdg_place_edge---							*/
/*	places newedge, which has already been allocated and		*/
/*	initialized, into the cd graph.	 The succ node lists are 	*/
/*	ordered by lexical number and the pred lists are not.		*/
/*									*/
/************************************************************************/

static void
cdg_place_edge (ControlDep *cd, cdEdge *newedge)
{
    cdEdge *edge, *tedge;
    
    cdNode *src  = newedge->src;	/* the src  node of the edge */
    cdNode *sink = newedge->sink;	/* the sink node of the edge */

    /* plop it on the top of the pred list */
    newedge->next_pred = sink->pred; 
    sink->pred 	       = newedge;
    
    /* Order the edges in src's successor list based on node number 
     * (lexical graphic ordering).
     */

    if ((src->succ == NULL) || (sink->index < src->succ->sink->index))
    {
	/* the new element is the top one */
	newedge->next_succ = src->succ;
	src->succ          = newedge;
    }
    else
    {
	/* somewhere in the middle, or at the end */
	tedge = src->succ;
	for (edge = tedge->next_succ;   ;edge = edge->next_succ)
	{
	    if ((edge == NULL) || (sink->index < edge->sink->index))
	    {
		/* insert newedge after tedge and possibly before edge */
		newedge->next_succ = edge;
		tedge->next_succ   = newedge;
		break;
	    }
	    tedge = edge;
	}
    }
}

/************************************************************************/
/*									*/
/* dg_delete_cds -- Starts at root and deletes control dependences 	*/
/*	for every statement in root's scope.  If check is true, it  	*/
/*	returns false if there are edges into or out of the scope that 	*/
/*	are not deleted, otherwise it returns true.			*/
/*									*/
/************************************************************************/
Boolean
dg_delete_cds(DG_Instance *dg, SideInfo *infoPtr, AST_INDEX root, Boolean check)
{
    findEdgeParms  parm;

    parm.dg	= dg;
    parm.infoPtr= infoPtr;
    parm.check	= check;
    parm.root	= root;
    parm.contained = true;
    
    walk_statements( root, loop_level(root), delete_cd_edges, NOFUNC,
		    (Generic) &parm );

    return (parm.contained);
}


/************************************************************************/
/*									*/
/* delete_cd_edges -- for each statement that may be a source or sink  	*/
/*     of a control dependence find and delete any cds.  If check,	*/
/*     determine if there are edges all have source and sink within	*/
/*     the scope of the root, if not abort the walk.			*/
/*									*/
/************************************************************************/

static int
delete_cd_edges (AST_INDEX stmt, int nesting_level, Generic Parm)
{
    DG_Edge	  *edges;
    Generic	   vector;
    int	    	   l, d;
    AST_INDEX      src, sink;
    int		   rtn  = WALK_CONTINUE;
    findEdgeParms *parm = (findEdgeParms *) Parm;
    SideInfo	  *infoPtr = parm->infoPtr;
    DG_Instance	  *dg	= parm->dg;

    /* Are there any edges here? */
    if (vector = dg_get_info ( infoPtr, stmt, type_levelv) == -1)
	return rtn;

    edges  = dg_get_edge_structure( dg );

    /* Delete the cd with src at stmt
     */
    for (l = LOOP_INDEPENDENT; l <= nesting_level; l++)
    {
	for (d = dg_first_src_stmt( dg, vector, l); d != NIL; 
	     d = dg_next_src_stmt( dg, d))
	{
	    if (edges[d].type == dg_control)
	    {
		/* Determine if the sink is in the scope of root
		 */
		if (parm->check)
		{
		    src = edges[d].src;
		    for (sink = edges[d].sink; (sink != src) &&
			 (sink != AST_NIL) && (sink != parm->root);
			 sink = out(sink) )  ;
		    if (sink == AST_NIL)
		    {
			parm->contained = false;
			rtn = WALK_ABORT;
		    }
		}
		/* Delete and free the edge */
		dg_delete_free_edge( dg, d);
	    }
	}

	if (parm->check && parm->contained)
	{
	    /* Determine if stmt is the sink of uncontained cd's
	     */ 
	    for (d = dg_first_sink_stmt( dg, vector, l); d != NIL; 
		 d = dg_next_sink_stmt( dg, d))
	    {
		if (edges[d].type == dg_control)
		{
		    sink = edges[d].sink;
		    for (src = edges[d].src; (sink != src) &&
			 (src != AST_NIL) && (src != parm->root);
			 src = out(src) )  ;
		    if (src == AST_NIL)
		    {
			parm->contained = false;
			rtn = WALK_ABORT;
		    }
		}
	    }
	}
    }    
}

/************************************************************************/
/*									*/
/* cdg_free -- frees all the structures in cd.				*/
/*									*/
/************************************************************************/

void
cdg_free (ControlDep *cd)
{
    int i;

    if (cd == NULL)
	return;
    
    if (cd->nodes != NULL)
    {
	if (cd->nodes[0].lda != NULL)
	    free_mem ((void *)cd->nodes[0].lda);
	free_mem ((void *)cd->nodes);
    }

    if (cd->edges != NULL)
	free_mem ((void *)cd->edges);

    if (cd->lddesc != NULL)
    {
	if (cd->lddesc->plist != NULL)
	{
	    for (i = 0; i < cd->ntotal; i++)
	    {
		util_free_nodes (cd->lddesc->plist[i]);
		util_list_free  (cd->lddesc->plist[i]);
	    }
	    free_mem ((void *)cd->lddesc->plist);
	}
	
	if (cd->lddesc->tlist != NULL)
	{
	    for (i = 0; i < cd->ntotal; i++)
	    {
		util_free_nodes (cd->lddesc->tlist[i]);
		util_list_free  (cd->lddesc->tlist[i]);
	    }
	    free_mem ((void *)cd->lddesc->tlist);
	}
	free_mem ((void *)cd->lddesc);
    }
    free_mem ((void *)cd);
}


/************************************************************************/
/*									*/
/* find_cdg_edges -- for each statement that can be a source or sink  	*/
/*     of a control dependence a node is created.  The edges are picked	*/
/*     off at their source.  The nodes and edges are counted and put in	*/
/*     in lists. 							*/
/*									*/
/************************************************************************/

static int
find_cdg_edges (AST_INDEX stmt, int nesting_level, Generic Parm)
{
    DG_Edge	  *edges;
    EDGE_INDEX     d;
    UtilNode  	  *unode;
    Generic	   vector;
    int	    	   level;
    findEdgeParms *parm = (findEdgeParms *) Parm;
    DG_Instance	  *dg	= parm->dg;
    SideInfo	  *infoPtr = parm->infoPtr;


    /* Ignore empty guards */
    if (gen_get_node_type(stmt) == GEN_GUARD)
	if (gen_GUARD_get_rvalue(stmt) == AST_NIL)
	    return WALK_CONTINUE;

    /* The control dependences for a block if get connected to the
     * if rather than the guard, so the first guard is ignored.
     * The cd for the else if blah branches get connected to the guard.
     */
    if (gen_get_node_type(stmt) == GEN_IF)
	parm->firstGuard = true;

    else if ((gen_get_node_type(stmt) == GEN_GUARD) && (parm->firstGuard))
    {
	parm->firstGuard = false;
	return WALK_CONTINUE;
    }    
    /* No control dependences have their source here, but
     * if stmt is executable, a sink node needs to be created.
     */
    if (cd_branch_type(stmt) == CD_UNCONDITIONAL)
    {
	if (is_executable(stmt))
	{
	    (parm->n)++;
	    unode = util_node_alloc (stmt, "pt_make_loop_array");
	    util_append (parm->nlist, unode);
	}
	return WALK_CONTINUE;
    }

    /* stmt may be the source of control dependences, find 'em
     */
    (parm->n)++;
    unode = util_node_alloc (stmt, "pt_make_loop_array");
    util_append (parm->nlist, unode);

    edges  = dg_get_edge_structure( dg );
    vector = dg_get_info ( infoPtr, stmt, type_levelv);

    for (level = LOOP_INDEPENDENT; level <= nesting_level; level++)
    {
	for (d = dg_first_src_stmt( dg, vector, level); d != NIL; 
	     d = dg_next_src_stmt( dg, d))
	{
	    if (edges[d].type == dg_control)
	    {
		/* put this edge on the edgelist */
		(parm->e)++;
		edges[d].src_ref = parm->n;
		unode = util_node_alloc (d, "pt_make_loop_array");
		util_append (parm->elist, unode);
	    }
	}
    }
    return WALK_CONTINUE;
}


/************************************************************************/
/*									*/
/* dg_delete_cdg_edge ---						*/
/*     Deletes edge out of a control dep graph.	 Uses prev edge from	*/
/*     the successor list to track.					*/
/*									*/
/************************************************************************/

void 
cdg_delete_edge (cdEdge *edge, cdEdge *prev)
{
    cdEdge *pred, *back;
    cdNode *src   = edge->src;
    cdNode *sink  = edge->sink;
    
    /* delete edge the on the successor list of the source */
    if (prev == NULL)
	src->succ = edge->next_succ;
    else
	prev->next_succ = edge->next_succ;
    
    /* find & delete edge on the predecessors list of the sink */
    back = NULL;
    for (pred = sink->pred; pred != edge; pred = pred->next_pred)  
	back = pred;
    if (back == NULL)	
	sink->pred = edge->next_pred;
    else
	back->next_pred = edge->next_pred;
}



/*
 * -- cdg_walk_nodes --
 * 
 * walks a control dependence graph starting at node in depth first order,
 * applying pre_action before visiting a node and post_action after
 * visiting a node.  Both are called with parm. Modelled after
 * walk_statements in el/walk.c. The 'visited' flag at each node
 * is used to insure that this is a true depth-first search.
 */

int
cdg_walk_nodes(ControlDep *cdg, cdNode *node, cdg_action_callback pre_action, 
               cdg_action_callback post_action, Generic parm)
{
  int i, total = cdg->ntotal;
  cdNode *cn;
  
  /*
   * Zero all of the 'visited' flags for the nodes in the graph, then call
   * the recursive walk routine.
   */
  for (cn = cdg->nodes, i = 0; i < total; i++, cn++)
    cn->visited = 0;
  
  return cdg_walk_nodes_recur(node, pre_action, post_action, parm);
}

static int
cdg_walk_nodes_recur(cdNode *node, cdg_action_callback pre_action, 
                     cdg_action_callback post_action, Generic parm)
{
    cdEdge     *edge;

    /*
     * Mark this node as visited.
     */
    node->visited = 1;

    if (pre_action != NOFUNC)
    {
	if ((*pre_action) (node, parm) == WALK_ABORT)
	    return WALK_ABORT;
    }

    /* Look at all the successors - sinks of cd edges from this node
     */
    for (edge = node->succ; edge != NULL; edge = edge->next_succ)
    {
	/* Data dependences, if present, can be ignored because the
	 * control dependences will fully connect the graph
	 */
	if (edge->label == CD_INVALID)
	    continue;

	/* Back edge? Don't visit.
	 */
	if (edge->sink->visited)
	  continue;

	if (cdg_walk_nodes_recur(edge->sink, pre_action, post_action, parm)
	    == WALK_ABORT)
	    return WALK_ABORT;
    }

    if (post_action != NOFUNC)
    {
	if ((*post_action) (node, parm) == WALK_ABORT)
	    return WALK_ABORT;
    }

    return WALK_CONTINUE;
}

/************************************************************************/
/*									*/
/*	cdg_print ---							*/
/*		prints a list of the edges and nodes in a cdg		*/
/*									*/
/************************************************************************/

void
cdg_print (ControlDep *cd)
{
    int i, j;
    cdEdge *edges   = cd->edges;
    cdNode *nodes   = cd->nodes;
    int    e        = cd->etotal;
    int    n        = cd->ntotal;
    
    printf("Node Info\n");
    
    for (i = 0; i < n; i++)
    {
	printf("%d", i);
	if (nodes[i].lda->run == 1)
	    printf("(p)");
	else printf("(s)");
	printf(":  Ast: %d, ",  nodes[i].stmt);
	if (nodes[i].pred != NULL)
	    printf("pred: %d,",nodes[i].pred->index);
	if (nodes[i].succ != NULL)
	    printf(" succ: %d,", nodes[i].succ->index); 
	printf(" part: %d\n", nodes[i].lda->scr );
    }
    
    printf("Edge Info\n");
    
    for (i = 0; i < e; i++)
    {
	printf ("%d:  ", i);
	if (edges[i].src != NULL)
	    printf("(%d -> ", edges[i].src->index);
	if (edges[i].sink != NULL)
	    printf("%d)  ", edges[i].sink->index);
	printf("level: %d ", edges[i].level);

	if (edges[i].next_succ != NULL)
	    printf("next_succ: %d, ", edges[i].next_succ->index);
	if (edges[i].next_pred != NULL)
	    printf("next_pred: %d, \n", edges[i].next_pred->index);
	else printf("\n");
    }
}
