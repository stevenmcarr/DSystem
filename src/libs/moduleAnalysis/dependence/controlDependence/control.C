/* $Id: control.C,v 1.1 1997/06/25 15:05:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/************************************************************************/
/*									*/
/*	dep/cd/control.c						*/
/*									*/
/*	control.c -- makes a control dependence graph, or extracts	*/
/*		it from the dependence graph for a program part.	*/ 
/*		This code was designed to aid in loop distribution	*/
/*	Includes the following routines:				*/
/*									*/
/*		Exported Functions					*/
/*									*/
/*		ControlDep * dstr_cd_graph();				*/
/*		AST_INDEX cd_insert_evar();				*/
/*		Boolean dstr_cd_restructure();				*/
/*		AST_INDEX dstr_cd_rebuild_tree();			*/
/*									*/
/*									*/
/*									*/
/************************************************************************/


/************************************************************************/
/*			I N C L U D E    F I L E S 			*/
/************************************************************************/

#include <stdlib.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>
#include <libs/support/stacks/xstack.h>

#include <libs/moduleAnalysis/dependence/controlDependence/cd_graph.h>
#include <libs/moduleAnalysis/dependence/controlDependence/private_cd.h>


#include <libs/support/memMgmt/mem.h>
#include <libs/frontEnd/ast/groups.h>


/************************************************************************/
/*			forward declarations				*/
/************************************************************************/

STATIC(void,      cd_delete_edge_succ,(void));
STATIC(void, 	  cd_build_guards,(ControlDep *cd, int n, cdEdge *edge, 
                                   cdEdge *pedge, AST_INDEX ev));
STATIC(AST_INDEX, cd_gen_loop_body,(ControlDep *cd, int p, AST_INDEX new_stmt_list,
                                    Stack ifstmts));
STATIC(int, 	  cd_new_node,(ControlDep *cd, int p, int old));
STATIC(cdEdges,   cd_move_edge_new_pred,(cdEdges predge, cdEdges oldedge, cdEdges
                                         lastsucc, cdNodes nodes, int n));
STATIC(void,      cd_create_ast_guard,(AST_INDEX loop, cdNodes nodes, int n,
                                       AST_INDEX true, AST_INDEX false, int t));
STATIC(AST_INDEX, cd_build_ast_guard_rvalue,(AST_INDEX ev, char *val));
STATIC(AST_INDEX, cd_move_stmt,(AST_INDEX stmt, AST_INDEX new_stmt_list, Stack
                                ifstmts, AST_INDEX ifstmt));
STATIC(void,  	  gen_declaration_evar,(AST_INDEX loop, AST_INDEX evar));

/* void 	 	 dg_print_cdg();	no definition anywhere	*/

/************************************************************************/
/*									*/
/*  dstr_cd_graph ---							*/
/*	builds a cdg for a loop and annotates it with the partition	*/
/*	information that is in adj_list					*/
/*									*/
/************************************************************************/

ControlDep *
dstr_cd_graph (DG_Instance *dg, LI_Instance *li, SideInfo *infoPtr, 
               Adj_List *adj_list, AST_INDEX loop)
{
    ControlDep	*cd;
    UtilNode   **search, *enode;
    UtilList   **plist;
    cdNode      *nodes;
    ldNode      *ldnodes;
    int		 i, p, nall;
    Loop_Type   *loop_list = adj_list->loop_array;
    int		 max_loop  = adj_list->max_loop + 1;

    
    /* build the control dependence graph just for a
     * statement list inside a do loop
     */
    if (!(is_loop(loop)) || (el_cflow(li) == NOFLOW))	
	return (NULL);

    cd = dg_build_cdg ( dg, infoPtr, loop, 2.0);
    if (cd == NULL)
    	return (NULL);


    /* Allocate and intialize the search lists and 
     * the partition lists of the statements.
     * 
     * The number of nodes in the cdg = number of statements in the
     * loop plus one for the header.  The loop_list contains a node
     * for each statement in the loop and they are in lexical order.
     * 		Aside:  the search list adjusts for partitions 
     * 			with non-contiguous  statement groups. 
     */
    search = (UtilNode **) get_mem (max_loop * sizeof (UtilNode), 
				    "cd_distribution_graph");
    plist  = (UtilList **) get_mem (max_loop * sizeof (UtilList), 
				    "cd_distribution_graph");
    
    for (p = 0; p < max_loop; p++)
    {
	search[p] = UTIL_HEAD(loop_list[p].stmts);
	plist[p]  = util_list_alloc (p, "cd_make_summary_array");
    }

    /* The first node in cd is the loop header, which roots this cdg */
    nodes   = cd->nodes;

    /* Add ldNode for each of the nodes in the cd
     */    
    nall    = cd->ntotal + cd->newnodes;
    ldnodes = (ldNode *) get_mem (nall * (sizeof(ldNode)), "dstr_graph");
    for (i = 0; i < nall; i++)
    {
	nodes[i].lda           = &ldnodes[i];
	nodes[i].lda->run      = NIL;
	nodes[i].lda->newStmts = AST_NIL;
    }

    /*************************
     * Give partitions numbers, map them into the cdg nodes, and
     * place lists of them in plist.
     */

    nodes->lda->tier = -1;  /* the loop header is not part of a partition */

    p = 0;
    for (i = 1; i < cd->ntotal; i++)
    {
	if (UTIL_NODE_ATOM(search[p]) != nodes[i].stmt)
	{
	    /* circle around looking at the top of each search list */
	    for (p = ++p % max_loop;
		 UTIL_NODE_ATOM(search[p]) != nodes[i].stmt;
		 p = ++p % max_loop)    ;
	}
	nodes[i].lda->tier = p;
	
	enode = util_node_alloc (i, "cd_make_summary_array");
	util_append (plist[p], enode);
	
	if (i != (cd->ntotal - 1))
	{  
	    for (search[p] = UTIL_NEXT(search[p]); 
		 search[p] == NULLNODE; p = ++p % max_loop)   ;
	}
    }
    free_mem((void *)search);

    cd->lddesc = (ldDesc *) get_mem (sizeof(ldDesc), "dstr_graph");
    cd->lddesc->plist = plist;

    return (cd);
}


static Boolean 
cd_sink_in_partition(cdNode *node)
{
    int     j;
    cdEdge *edge;
    
    for (edge = node->succ; edge != NULL; edge = edge->next_succ)
    {
	if (edge->sink->lda->tier == node->lda->tier)
	    return true;
    }
    return false;
}


AST_INDEX
cd_insert_evar (DG_Instance *dg, SideInfo *infoPtr, ControlDep *cd, 
                int p, int first, int n, AST_INDEX loop)
    /* p: partition */
    /* first: the first node in this partition */
    /* n: cd node */
{
    int        e, v;
    AST_INDEX  init, ev, induct, ivar, minusOne, guard, newtest;
    char       evar[10];
    cdNode    *nodes   = cd->nodes;
    DG_Edge   *edgeptr = dg_get_edge_structure( dg );
    
    /* Create an Execution variable with the name $ev<n>[i]
       make it an array with the induction variable from the
       loop header.  Insert the initialization, if needed.
       Insert the assignment.
       */
    
    /* get the induction variable, i */
    induct = gen_DO_get_control(loop);
    ivar  = gen_INDUCTIVE_get_name(induct);
    ivar = list_create(tree_copy(ivar));
    
    /* create the exectution variable, $ev<n>[i] */
    sprintf(evar, "$ev%d", n);
    init = gen_IDENTIFIER();
    gen_put_text (init, evar, STR_IDENTIFIER);
    ev = gen_SUBSCRIPT(init, ivar);

    /* if this node is control dependent on something other
     * than the header
     */
    if (nodes[n].pred->src->stmt != loop)
    {
	/*  build assignment $ev<n>[i] = -1, the initialization of the evar
	 */
	minusOne = gen_CONSTANT();
	gen_put_text(minusOne, "-1", STR_CONSTANT_INTEGER);
	init = gen_ASSIGNMENT(AST_NIL, tree_copy(ev), minusOne);
	
	if (nodes[first].lda->newStmts == AST_NIL)
	    nodes[first].lda->newStmts = list_create(init);
	else
	    list_insert_before(list_first(nodes[first].lda->newStmts), init);

	/* make it control dependent on loop in the dg, it will be corrected
	 * a new loop header exists.
	 */
	e = dg_alloc_edge( dg, &edgeptr);
	v = dg_alloc_level_vector( dg, MAXLOOP);
	dg_put_info ( infoPtr, init, type_levelv, v);
	    
	edgeptr[e].sink     = init;
	edgeptr[e].sink_vec = v;
	edgeptr[e].src      = loop;
	edgeptr[e].src_vec  = dg_get_info ( infoPtr, loop, type_levelv);
	edgeptr[e].type     = dg_control;
	edgeptr[e].level    = LOOP_INDEPENDENT;
	edgeptr[e].cdtype   = CD_DO_LOOP;
	edgeptr[e].cdlabel  = CD_ENTER;	

	dg_add_edge( dg, e);
    }
    /* get the test for this node, build:  $ev<n>[i] = <test> */
    /* replace if <test> with "$ev<n>[i] .eq. .true." */
    
    if (is_if(nodes[n].stmt))
	guard = list_first(gen_IF_get_guard_LIST(nodes[n].stmt));
    else if (is_guard(nodes[n].stmt))
	guard = nodes[n].stmt;
    else
    {
	printf("What do we need a evar for %d for??\n", nodes[n].stmt);
	return (AST_NIL);
    }
    guard = gen_GUARD_get_rvalue(guard); 	/* guard = <test> */

    /* If there are any edges from n to nodes in this partition,
     * replace the test in the original guard.  Otherwise, delete
     * the original guard.
     */
    if (cd_sink_in_partition (&nodes[n]))
    {
	/* $ev<n>[i] .eq. .true. */
	newtest = cd_build_ast_guard_rvalue (ev, "1");
	tree_replace (guard, newtest);		
    }
    else
    {
	list_remove_node(nodes[n].stmt);
	nodes[n].stmt = AST_NIL;
	guard = tree_copy(guard);
    }
    /* create & insert the assignment of evar, $ev<n>[i] = <test>
     */
    guard = gen_ASSIGNMENT(AST_NIL, tree_copy(ev), guard); 
    if (nodes[n].lda->newStmts == AST_NIL)
	nodes[n].lda->newStmts = list_create(guard);
    else
	list_insert_after(list_last(nodes[n].lda->newStmts), guard);

    return ev;
}

/* An exectution variable has already been created and assigned.
   Insert the guards for all the edges in different partitions.
   Group the ones with the same label together, try to put
   if-then-else's when possible.
 */
static void
cd_build_guards(ControlDep *cd, int n, cdEdge *edge, cdEdge *pedge, AST_INDEX ev)
{
    cdNode    *node;
    cdEdge    *tedge, *cedge, *lastsucc, *back, *next;
    UtilNode  *pnode, *newnode;
    int	      i, t, f, first;
    int	      into;
    AST_INDEX guard;
    AST_INDEX true;
    AST_INDEX false;
    UtilList **plist = cd->lddesc->plist;
    AST_INDEX loop   = cd->top;
    cdNode    *nodes = cd->nodes;
    cdEdge    *edges = cd->edges;

    
    /* look at all the successors */
    for (tedge = edge; tedge != NULL; tedge = next)
    {	
	next = tedge->next_succ;
	/* ignore edges to nodes in the same partition */

	if (tedge->sink->lda->tier == nodes[n].lda->tier)
	{
	    pedge = tedge;
	    continue;
	}
	/* Make a new control cd node for cd succs
	 * in succ's partition, keep plist ordered */
	into  = tedge->sink->lda->tier;
	t = cd_new_node (cd, into, n);
	i = UTIL_NODE_ATOM(UTIL_HEAD(plist[into]));
	for (pnode = UTIL_HEAD(plist[into]); 
	     nodes[i].index < n; pnode = UTIL_NEXT(pnode))
	{
	    i = UTIL_NODE_ATOM(pnode);
	}
	newnode = util_node_alloc (t, "cd_build_guards");
	util_insert_before (newnode, pnode);
	
	true  = AST_NIL;
	false = AST_NIL;
	first = tedge->sink->lda->tier;
	for ( ; (tedge != NULL) && (tedge->sink->lda->tier == first); 
	     tedge = next)
	{
	    next = tedge->next_succ;
	    /* create $ev<n>[i] .eq. 1 */
	    if ((tedge->label == CD_TRUE) && (true == AST_NIL))
		true = cd_build_ast_guard_rvalue (ev, "1");
	    
	    else if ((tedge->label == CD_FALSE) && (false == AST_NIL)) 
	    {
		/* create $ev<n>[i] .eq. 0 */
		false = cd_build_ast_guard_rvalue (ev, "0");
		
		if ((true != AST_NIL) && (nodes[n].pred->src->stmt != loop))
		{/* a node is needed for each of the true and 
		  * false branch, pnode has the correct place for t */
		    t = cd_new_node (cd, tedge->sink->lda->tier, n);
		    newnode = util_node_alloc (t, "cd_build_guards");
		    util_insert_before (newnode, pnode);
		}
	    }
	    lastsucc = cd_move_edge_new_pred (pedge, tedge, lastsucc, nodes, t);
	}
	cd_create_ast_guard (loop, nodes, n, true, false, t);
    }
}


static AST_INDEX
cd_build_ast_guard_rvalue (AST_INDEX ev, char *val)
{
    AST_INDEX guard = gen_CONSTANT();
    
    gen_put_text(guard, val, STR_CONSTANT_INTEGER);
    guard = gen_BINARY_EQ(tree_copy(ev), guard);
    return (guard);
}
			
static void
cd_create_ast_guard (AST_INDEX loop, cdNodes nodes, int n, 
                     AST_INDEX true, AST_INDEX false, int t)
{
    AST_INDEX guard;
    if (nodes[n].pred->src->stmt == loop)
    {
	if (true != AST_NIL)
	{/* if-then-else, or if-then for original true branch */
	    guard =  gen_GUARD(AST_NIL, true, AST_NIL);
	    guard = list_create(guard);
	    if (false != AST_NIL)
		list_insert_last(guard, gen_GUARD(AST_NIL, false, AST_NIL));
	    
	}
	else
	{ /* if-then for original false branch */
	    guard = gen_GUARD(AST_NIL, false, AST_NIL);
	    guard = list_create(guard);
	}
	nodes[t].stmt = guard;
    }
    else
    {
	if (true == AST_NIL)
	{ /* if-then for original false branch */
	    guard = gen_GUARD(AST_NIL, false, AST_NIL);
	    guard = list_create(guard);
	    nodes[t].stmt = guard;
	}
	else if (false != AST_NIL) 
	{ /* if-then for original true and false branch */
	    guard = gen_GUARD(AST_NIL, true, AST_NIL);
	    guard = list_create(guard);
	    nodes[t - 1].stmt = guard;
	    
	    guard = gen_GUARD(AST_NIL, false, AST_NIL);
	    guard = list_create(guard);
	    nodes[t].stmt = guard;
	}
    }

}


Boolean
dstr_cd_restructure (DG_Instance *dg, LI_Instance *li, SideInfo *infoPtr, 
                     ControlDep *cd, Adj_List *adj_list, AST_INDEX loop)
{
    int       i;
    int       succ;
    int       p; 				/* partition */
    int	      first;
    cdNode    nnode;
    cdEdge    *edge, *pedge;
    UtilNode  *unode, *snode, *fnode;
    ldDesc    *ld         = cd->lddesc;
    cdNode    *nodes      = cd->nodes;
    Boolean   crossing    = false;
    AST_INDEX evar        = AST_NIL;
    Loop_Type *loop_array = adj_list->loop_array;
    
    for (p = 0; p <= adj_list->max_loop; p++)
    { /* all the paritions, and each node in the partition */
	fnode = UTIL_HEAD(ld->plist[p]);
	first = UTIL_NODE_ATOM(fnode);
	for (unode = UTIL_HEAD(ld->plist[p]); 
	     unode != NULLNODE; 
	     unode = UTIL_NEXT(unode))
	{ 
	    i = UTIL_NODE_ATOM(unode);
	    if (i >= cd->noriginal)
		continue;	/* ignore newly added cd nodes */
	    pedge = NULL;
	    for (edge =  nodes[i].succ; 
		 edge != NULL ;
		 edge =  edge->next_succ)
	    {
		if (nodes[i].lda->tier != edge->sink->lda->tier)
		{	
		    crossing = true;
		    evar = cd_insert_evar(dg, infoPtr, cd, p, first, i, loop);
		    cd_build_guards (cd, i, edge, pedge, evar);
		    
		    /* add evar to shared variable list */
		    el_force_add_shared_var( li, loop, 
		        el_create_new_node( gen_get_text(gen_SUBSCRIPT_get_name(evar)), AST_NIL, AST_NIL, var_shared, "", 1));
 
		    /* generate a declaration statement */
		    gen_declaration_evar ( loop, evar);
		    break;
		}
		pedge = edge;
	    }
	} 
    }	
    return (crossing);
}

/* Completely rebuild the tree for the new loops that result
 * due to distribution.  It adjust for new control flow
 * introduced due to exectution variable insertion to correct
 * control flow.
 */
AST_INDEX 
dstr_cd_rebuild_tree(LI_Instance *li, ControlDep *cd, Adj_List *adj_list, 
                     AST_INDEX old_do, int level)
{
    AST_INDEX    return_node;
    AST_INDEX    new_do;
    AST_INDEX    prev_loop;
    AST_INDEX    new_lbl_def;
    AST_INDEX    new_lbl_ref;
    AST_INDEX    new_control;
    AST_INDEX    new_close_lbl_def;
    AST_INDEX    new_stmt_list;
    
    AST_INDEX    old_stmt_list;
    AST_INDEX    stmt;
    int          new_loop;
    Loop_Type    *loop_array;
    META_TYPE    meta_type;
    
    UtilNode     *unode;
    Stack	 ifstmts;
    AST_INDEX    ifstmt;
    int	     	 n;
    int	     	 first;
    
    loop_array    = adj_list -> loop_array;				
    prev_loop     = old_do;
    meta_type     = ast_get_meta_type(old_do);
    old_stmt_list = gen_DO_get_stmt_LIST(old_do);				
    
    ifstmts = stack_create (sizeof(AST_INDEX));
    /* Create a loop and stick it just after the last added old loop */
    for (new_loop = 0; new_loop <= adj_list -> max_loop ; new_loop++)
    {
	/* delete the label, if any on the statement  */
	new_lbl_def = AST_NIL;
	
	/* generate a new label reference              */
	new_lbl_ref = AST_NIL;
	new_control = tree_copy(gen_DO_get_control(old_do)); 
	new_close_lbl_def = AST_NIL;
	
	/* find and copy the stmt subtree corresponding to the new loop */
	new_stmt_list = list_create(AST_NIL);
	ifstmt = AST_NIL;
	stack_push(ifstmts, (Generic*)&ifstmt);
	
	ifstmt = cd_gen_loop_body (cd, new_loop, new_stmt_list, ifstmts);
	
	for ( ; ifstmt != AST_NIL; stack_pop(ifstmts, (Generic*)&ifstmt))
	    ;		
	new_do = gen_DO(new_lbl_def, new_close_lbl_def, new_lbl_ref, new_control, new_stmt_list);   
	ast_put_meta_type (new_do, meta_type);
	el_add_loop( li, prev_loop, new_do, level);		      
	el_copy_shared_list( li, old_do, new_do);
	el_copy_private_list( li, old_do, new_do);  
	prev_loop = new_do;
	if (new_loop == 0)  
	    return_node = new_do;		      
	list_insert_before (old_do, new_do);     
    }
    stack_destroy(ifstmts);
    el_remove_loop( li, old_do); 
    list_remove_node(old_do);    
    return(return_node);
}

/* Only for structured code today.
 */
static AST_INDEX
cd_gen_guard (UtilList *plist, cdNodes node, int p, AST_INDEX new_stmt_list, 
              Stack ifstmts, AST_INDEX ifstmt)
{
    AST_INDEX newif, guard, slist, s;
    int       label;
    cdNode   *sink;
    cdEdge   *sedge;
    UtilNode *unode;
    Boolean   first = true;
    
    /* gen, and insert the new if, it may be an if-then-else */
    newif = gen_IF (AST_NIL, AST_NIL, node->stmt);
    list_insert_last(new_stmt_list, newif);
    
    /* get the statement list for the first branch */
    guard  = list_first(node->stmt);
    if ((slist  = gen_GUARD_get_stmt_LIST(guard)) == AST_NIL)
    {
	slist = list_create(AST_NIL);
	gen_GUARD_put_stmt_LIST(guard, slist);
    }
    /* put all the successors on this label on the guard stmts
     * list for this label.
     */
    for (sedge = node->succ; sedge != NULL; sedge = sedge->next_succ)
    {
	sink = sedge->sink;
	if (sink->lda->tier != p)	/* only deal with this partition */
	    continue;
	
	/* find the label for the first successor */
	if (first)
	{
	    first = false;
	    label = sedge->label;
	}
	if (sedge->label != label)
	{ /* into a different guard on this if now */
	    label = sedge->label;
	    guard = list_next(guard);
	    if ((slist  = gen_GUARD_get_stmt_LIST(guard)) == AST_NIL)
	    {
		slist = list_create(AST_NIL);
		gen_GUARD_put_stmt_LIST(guard, slist);
	    }
	}
	if (sink->lda->newStmts != AST_NIL)
	{
	    for (s = list_first(sink->lda->newStmts); 
		 s != AST_NIL; s = list_next(s))
	    {
		ifstmt = cd_move_stmt(s, slist, ifstmts, AST_NIL);
	    }
	}
	ifstmt = cd_move_stmt(sink->stmt, slist, ifstmts, AST_NIL);
	
	/* find and delete this successor in the partition list */
	for (unode = UTIL_HEAD(plist); 
	     (unode != NULLNODE) && (sink->index != UTIL_NODE_ATOM(unode));
	     unode = UTIL_NEXT(unode))
	    ;
	util_pluck(unode);
	util_free_node(unode);
    }
    return (ifstmt);
}

static AST_INDEX
cd_gen_loop_body (ControlDep *cd, int p, AST_INDEX new_stmt_list, Stack ifstmts)
{
    int        n;
    UtilNode  *unode;
    AST_INDEX  s;
    AST_INDEX  ifstmt  = AST_NIL;
    ldDesc    *ld      = cd->lddesc;
    cdNodes    nodes   = cd->nodes;

    unode = UTIL_HEAD(ld->plist[p]);
    
    for ( ; unode != NULLNODE; unode = UTIL_HEAD(ld->plist[p])) 
    {
	n = UTIL_NODE_ATOM(unode);
	util_pluck(unode);
	util_free_node(unode);
	
	if (nodes[n].lda->newStmts != AST_NIL)
	{
	    for (s = list_first(nodes[n].lda->newStmts); 
		 s != AST_NIL; s = list_next(s))
	    {
		ifstmt = cd_move_stmt(s, new_stmt_list, ifstmts, AST_NIL);
	    }
	}
	if (n >= cd->noriginal)
	{ /* this is a new guard node */
	    ifstmt = cd_gen_guard (ld->plist[p], &(nodes[n]), 
				   nodes[n].lda->tier, new_stmt_list, 
				   ifstmts, ifstmt);
	}
	else
	    ifstmt = cd_move_stmt(nodes[n].stmt, new_stmt_list, ifstmts, ifstmt);
    }
    return ifstmt;
}
/* Moves stmt from old loop into the new statement list (new_stmt_list)
 * for the new loop.  If's take their children with them, so  
 * kids that stay are not moved a second time.  All for loop distrubution
 * with control flow.
 */

static AST_INDEX
cd_move_stmt(AST_INDEX stmt, AST_INDEX new_stmt_list, Stack ifstmts, AST_INDEX ifstmt)
{
    if (stmt == AST_NIL)
	return (ifstmt);
    
    if (ifstmt == AST_NIL)
    {
	list_remove_node(stmt);
	list_insert_last(new_stmt_list,stmt);
    }
    else if (ifstmt != out(out(stmt)))
    {
	if (ifstmt != out(stmt))
	{
	    for ( stack_pop(ifstmts, (Generic*)&ifstmt); 
		 (ifstmt != AST_NIL) && (ifstmt != out(out(stmt)));
		 stack_pop(ifstmts, (Generic*)&ifstmt) )
	    {
		if (ifstmt == out(stmt))
		    break;
	    }
	}
	if (ifstmt == AST_NIL)
	{
	    list_remove_node(stmt);
	    list_insert_last(new_stmt_list,stmt);
	}
    }
    if (is_if(stmt))
    {
	ifstmt = stmt;
	stack_push(ifstmts, (Generic*)&ifstmt);
    }
    return ifstmt;
}

static int
cd_new_node (ControlDep *cd, int p, int old)
{
    int  i = (cd->ntotal)++;

    if (--(cd->newnodes) <= 0)
    {
	printf ("not enough cd nodes in --cd_new_node");
	exit(-1);
    }
    cd->nodes[i].index 	       = old;
    cd->nodes[i].pred  	       = NULL;
    cd->nodes[i].succ  	       = NULL;
    cd->nodes[i].stmt  	       = AST_NIL;
    cd->nodes[i].lda->newStmts = AST_NIL;
    cd->nodes[i].lda->tier     = p;
    return i;
}

/* detach old edge, attach with new node, n, as pred
 * leave succ the same, and label the same
 */
static cdEdges
cd_move_edge_new_pred (cdEdges pedge, cdEdges oldedge, cdEdges lastsucc, 
                       cdNodes nodes, int n)
{
    cdNodes sinkn = oldedge->sink;
    
    cdg_delete_edge (oldedge, pedge);
    /* oldedge->sink, index, and label stay the same */
    
    /* insert in top of pred list */
    oldedge->src        = &(nodes[n]);
    oldedge->next_pred  = sinkn->pred;
    sinkn->pred	        = oldedge;
    oldedge->next_succ  = NULL;
    
    if (nodes[n].succ == NULL)
    { /* add successors to the end of the list */
	nodes[n].succ = oldedge;
	lastsucc = oldedge;
    }
    else
    {
	lastsucc->next_succ = oldedge;
	lastsucc = oldedge;
    }
    return (lastsucc);
}

static void
gen_declaration_evar (AST_INDEX loop, AST_INDEX evar)
{
    AST_INDEX zero;
    AST_INDEX inductive;
    AST_INDEX ub_expr;
    AST_INDEX dim;
    AST_INDEX decl_len;
    AST_INDEX decllist;
    AST_INDEX declpos;
    AST_INDEX newdecl;
    AST_INDEX scope = find_scope (loop);
    
    zero = gen_CONSTANT();
    gen_put_text(zero, "0", STR_CONSTANT_INTEGER);
    
    inductive = gen_DO_get_control(loop);
    ub_expr = tree_copy( gen_INDUCTIVE_get_rvalue2(inductive) );
    
    /* generate declaration statement */ 
    dim = gen_DIM(tree_copy(zero), tree_copy(ub_expr));
    decl_len = gen_ARRAY_DECL_LEN(tree_copy(gen_SUBSCRIPT_get_name(evar)), AST_NIL, dim, AST_NIL );
    decl_len = list_insert_first( gen_LIST_OF_NODES(), decl_len );	/* mpal:910719 */
    newdecl = gen_DIMENSION( AST_NIL, decl_len ); 
    
    switch (gen_get_node_type (scope))
    {
    case  GEN_PROGRAM:
	decllist = gen_PROGRAM_get_stmt_LIST( scope );            
	declpos = list_first( decllist );
	list_insert_before(declpos, tree_copy( newdecl ) );
	break;
    case  GEN_SUBROUTINE:
	decllist = gen_SUBROUTINE_get_stmt_LIST( scope );            
	declpos = list_first( decllist );
	list_insert_before(declpos, tree_copy( newdecl ) );
	break;
    case  GEN_FUNCTION:
	decllist = gen_FUNCTION_get_stmt_LIST( scope );            
	declpos = list_first( decllist );
	list_insert_before(declpos, tree_copy( newdecl ) );
	break;
    default:
	printf("cannot handle program scope\n");
	break;
    }
}
