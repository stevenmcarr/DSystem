/* $Id: el.C,v 1.1 1997/06/25 15:09:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                            		*/
/*	dep/el/el.c							*/
/*                                                            		*/
/*	el.c -- The edge-list abstraction. Used to manipulate the 	*/
/*		edges in the dependence graph.                        	*/
/*                                                            		*/
/*                                                            		*/
/*	el_create_instance() 						*/
/*	el_destroy_instance()						*/
/*	el_copy_active_edge_list_structure()				*/
/*	el_copy_edge()							*/
/*	addEdge()							*/
/*	ped_el_new_loop()						*/
/*	edge_is_active()						*/
/*	get_dependence()						*/
/*	first_dependence()						*/
/*	next_dependence()						*/
/*	prev_dependence()						*/
/*	match()								*/
/*	fixDtype()							*/
/*	el_show()							*/
/*	el_hide()							*/
/*	el_showall()							*/
/*	el_sort()							*/
/*	edgeComp()							*/
/*	idRefs()							*/
/*	el_get_dims()							*/
/*	el_get_blocks()							*/
/*	el_gen_get_text()						*/
/************************************************************************/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <libs/support/strings/rn_string.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/support/arrays/ExtensibleArray.h>

#include <include/bstring.h>

#include <libs/moduleAnalysis/dependence/dependenceTest/dt_info.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/dep_dt.h>
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#include <libs/moduleAnalysis/dependence/loopInfo/private_li.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_instance.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_header.h>
#include <libs/moduleAnalysis/dependence/edgeList/private_el.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>

#include <libs/frontEnd/ast/groups.h>

EXTERN(char, *dg_var_name,(DG_Edge *Edge));

STATIC(void,     addEdge,(DG_Instance *dg, EL_Instance *el, EDGE_INDEX Eindex,
                          Boolean shared, Slist *share_info));
STATIC(void,     walkList,(EL_Instance *el, LI_Instance *li, SideInfo *infoPtr,
                           DG_Instance *dg, AST_INDEX loop, AST_INDEX list,
                           int level, Boolean top));
STATIC(void,     walkStmt,(EL_Instance *el, LI_Instance *li, SideInfo *infoPtr,
                           DG_Instance *dg, AST_INDEX loop, AST_INDEX node, 
                           int level, Boolean top));
STATIC(Boolean,  match,(char *haystack, char *needle));
STATIC(void,     fixDtype,(char *dtypelist));
STATIC(int,      edgeComp,(Edge_List *first, Edge_List *second));
STATIC(Boolean,  idRefs,(DepType type));
STATIC(void,     dumpLinfo,(void));

typedef FUNCTION_POINTER (int, QsortCompareFunc,(const void *, const void *)) ;
/* -----------------------------------------------------------------------
   el_create_instance() - create an edge list structure.
 ----------------------------------------------------------------------- */
EL_Instance *
el_create_instance(int num_edges)
{
    EL_Instance    *el;

    el = (EL_Instance *) get_mem(sizeof(EL_Instance), "Edge_List");
    if (num_edges < 100)
	num_edges = 100;

    el->num_edges         = num_edges;
    el->edgelist          = (Edge_List *) xalloc(num_edges, sizeof(Edge_List));
    bzero( (char *)el->edgelist, num_edges*sizeof(Edge_List) );     /* zero Edge_List */
    el->current_dep_num   = -1;
    el->total_num_of_deps = 0;
    el->num_lc		  = 0;
    el->num_lc_shared	  = 0;
    el->lc		  = true;
    el->control		  = false;
    el->li		  = false;
    el->privatev	  = false;

 /* Invariant: 0 <= current_dep_num < total_num_of_deps */
 /* Invariant: 0 <= num_lc_shared <= num_lc < total_num_of_deps */

    return (EL_Instance *) el;
}

void
el_destroy_instance(EL_Instance *EL)
{
	int	i, size;		/* mullin:910718	*/

	size = ((int *)(((EL_Instance *)EL)->edgelist))[-1];
	for(i=0; i<size; i++)
	    if ( (((EL_Instance *)EL)->edgelist)[i].cblock != NULL)
		free_mem( (((EL_Instance *)EL)->edgelist)[i].cblock );
	xfree((int *)((EL_Instance *)EL)->edgelist);
	free_mem(EL);
}

/*	if loop carried deps are to be shown 
 */
Boolean
el_show_loopCarried(EL_Instance *el)
{
	return	(el->lc);
}

/*	loop carried on shared variables
 */
int
el_num_lc_shared(EL_Instance *el)
{
	return	(el->num_lc_shared);
}

/* -----------------------------------------------------------------------
   el_copy_active_edge_list_structure() - Create a copy of an edge list 
   structure
   ----------------------------------------------------------------------- */

EL_Instance	*
el_copy_active_edge_list_structure(EL_Instance *oldel)
{
    EL_Instance    *el;
    char 	   *from,*to;
    int             i;
    Edge_List      *old_edge_list;

    el                    = (EL_Instance *) get_mem(sizeof(EL_Instance), "Edge_List");
    el->current_dep_num   = -1;

    /*
     * some prefix of the edges in oldel are active - how many? 
     */
    
    old_edge_list = oldel->edgelist;

    for (i = 0;
	 i < oldel->total_num_of_deps && old_edge_list[i].active;
	 i++);	  

    el->num_edges         = i; 	/* need exactly i edges */
    el->total_num_of_deps = i;

    el->edgelist          = (Edge_List *) xalloc(i, sizeof(Edge_List));

    from  = (char *) oldel->edgelist;
    to    = (char *) el->edgelist;

    bcopy((const char *)from, (char *)to, i * sizeof(Edge_List));
    return (EL_Instance *) el;
}

/*---------------------------------------------------------------------------
  el_copy_edge() - copies DG edge
 -------------------------------------------------------------------------- */

void
el_copy_edge(DG_Instance *dg, DT_info *dt, DG_Edge *edgeptr, 
             EDGE_INDEX oldedge, EDGE_INDEX newedge)
{
    DG_Edge    *New, *old;
    
    New = edgeptr + newedge;
    old = edgeptr + oldedge;
    
    bcopy((const char *)old, (char *)New, sizeof(DG_Edge));
    
    New->src_str  = old->src_str  ? 
	(char *) dt_ssave(old->src_str, dt)  : NULL;
    New->sink_str = old->sink_str ? 
	(char *) dt_ssave(old->sink_str, dt) : NULL;

    if ((old->type == dg_true) || (old->type == dg_anti) ||
	(old->type == dg_output) || (old->type == dg_input))
	dt_copy_info( dt,old,New);      /* copy the DT information */
}

/*---------------------------------------------------------------------------
   addEdge() - add another link to the edge list.

 -------------------------------------------------------------------------- */
static void
addEdge(DG_Instance *dg, EL_Instance *el, EDGE_INDEX Eindex, Boolean shared, 
        Slist *share_info)
{
    DG_Edge        *Earray;
    Edge_List      *eptr;
    
    /* 
     * make sure the src and sink of this edge involve 
     * shared variables - we no longer do this, user can see 
     * dependences on private variables as well, if they want.
     * This is all properly controlled at a higher level. 
     */
    
    Earray = dg_get_edge_structure( dg );
    
    if (el->total_num_of_deps >= el->num_edges)
    {
	int   oldSize;
	oldSize         = el->num_edges;
	el->num_edges <<= 1;
	el->edgelist    = (Edge_List *) 
	    xrealloc((int *)el->edgelist, el->num_edges);

	/* zero new edge_list */
	bzero( (char *)&(el->edgelist[oldSize]), oldSize*sizeof(Edge_List) );   
    }
    
    eptr = el->edgelist + el->total_num_of_deps++;
    
    eptr->edge_index 	= Eindex; 
    eptr->active 	= true; 

    if (Earray[Eindex].type == dg_call || 
	Earray[Eindex].type == dg_exit ||
	Earray[Eindex].type == dg_io || 
	Earray[Eindex].type == dg_control || 
	!shared)
    {
	eptr->def_before  = -1;
	eptr->use_after   = -1;
	eptr->why         = var_shared;
	eptr->user        = false;
	eptr->dims        = -1;
	eptr->cblock      = (char *)ssave ("");
    }
    else
    {
	/* this is a shared variable */
	
       	eptr->def_before  = share_info->def_before;
        eptr->use_after   = share_info->use_after;
	eptr->why         = share_info->why;
	eptr->user        = share_info->user;
	eptr->cblock      = (char *)ssave (share_info->cblock);
	eptr->dims        = share_info->dim;
    }
    
    if (Earray[Eindex].level != LOOP_INDEPENDENT)
    {
    	el->num_lc++;
    	if (shared)
	    el->num_lc_shared++;
    }
}

/* -------------- begin el_new_loop code ------------	*/




/* -----------------------------------------------------------------------
   el_new_loop() - collect the list of dependences for this loop in an 
   edge list structure, and return the number of dependences found.
   "loop" is the ast index of the loop header statement. 
  -----------------------------------------------------------------------*/

int 
el_new_loop (EL_Instance *el, LI_Instance *li, SideInfo *infoPtr, 
             DG_Instance *dg, AST_INDEX loop)
{
    int	nesting_level	  = loop_level (loop); 
   
    el->current_dep_num   = -1;
    el->total_num_of_deps = 0;
    el->num_lc	          = 0;
    el->num_lc_shared     = 0;
    el->cd		  = NULL;

    if (el_get_loop_info ( li, loop))
    	li_set_cflow( li, NOFLOW);

    /* Find all the dependences in the loop */
    walkStmt( el, li, infoPtr, dg, loop, loop, nesting_level, true);
    return (el->total_num_of_deps);
}

/*----------------------------------------------------------------------
   walkList() - walk the list of statements.
 ----------------------------------------------------------------------*/
static void
walkList(EL_Instance *el, LI_Instance *li, SideInfo *infoPtr, DG_Instance *dg, 
         AST_INDEX loop, AST_INDEX list, int level, Boolean top)
{
    AST_INDEX       curr;

    curr = list_first(list);
    while (curr != AST_NIL)
    {
	walkStmt ( el, li, infoPtr, dg, loop, curr, level, top);
	curr = list_next(curr);
    }
} /* end_walkList */


/*----------------------------------------------------------------------
   walkStmt() - walk the ast, starting at "node". For each statement node
   check if it has an associated level vector. If yes, collect all the
   dependence edges pointed to by the level vector.
   Also it sets dep-EL->cflow to true if the top statement list has a
   compound 'IF'.
 ----------------------------------------------------------------------*/
static void
walkStmt(EL_Instance *el, LI_Instance *li, SideInfo *infoPtr, DG_Instance *dg, 
         AST_INDEX loop, AST_INDEX node, int level, Boolean top)
{
    EDGE_INDEX      Eindex;
    int             Lvec;
    AST_INDEX       rvnode;
    Boolean         shared;
    Slist          *share_info;
    DG_Edge        *Earray;
	char			*name;
	int				i;

    if (!is_statement(node))
	return;

    if (is_private(node))
       return;

    Earray = dg_get_edge_structure( dg );
    
    if ((Lvec = dg_get_info( infoPtr, node, type_levelv)) != -1)
    {
	/* edges on this level */
	if (el->lc)
	{
	    for (Eindex  = dg_first_src_stmt( dg, Lvec, level); 
		 Eindex != -1;
		 Eindex  = dg_next_src_stmt( dg, Eindex))
	    {
		if (Earray[Eindex].type == dg_control)
		{
		    if (el->control)
			addEdge( dg, el, Eindex, true, NULL);
		}
		else
		{
			if (name = dg_var_name(Earray + Eindex))
			{
				shared = check_if_shared( li, Earray[Eindex].type,
					name, &share_info); 
				sfree(name);
			}
			else  /* can't find name of var for dependence edge */
			{
				shared = true;
				share_info = NULL;
			}

		    if ((!shared) && el->privatev)
			addEdge( dg, el, Eindex, shared, share_info);
		    else if (shared)
			addEdge( dg, el, Eindex, shared, share_info);
		}
	    }
	}
	
	if (el->li)
	{/* loop independent edges */

	    for (Eindex = dg_first_src_stmt( dg, Lvec, -1); 
		 Eindex != -1;
		 Eindex = dg_next_src_stmt( dg, Eindex))
	    {
		if (Earray[Eindex].type == dg_control)
		{
		    if (el->control)
			addEdge( dg, el, Eindex, true, NULL);
		}
		else
		{
			if (name = dg_var_name(Earray + Eindex))
			{
				shared = check_if_shared( li, Earray[Eindex].type,
					name, &share_info); 
				sfree(name);
			}
			else  /* can't find name of var for dependence edge */
			{
				shared = true;
				share_info = NULL;
			}

		    if ((!shared) && el->privatev)
			addEdge( dg, el, Eindex, shared, share_info);
		    else if (shared)
			addEdge( dg, el, Eindex, shared, share_info);
		}
	    }
	}
	
	/*** does this do anything???*/
        /* scalar dependences */
	for (Eindex = dg_first_src_stmt( dg, Lvec, 0); 
	     Eindex != -1;
	     Eindex = dg_next_src_stmt( dg, Eindex))
	{
	    printf("There is a scalar edge in here\n");
               /*  previously this function did not have enough  */
               /*  parameters being passed in.  shared and       */
               /*  share_info were added, but I am not sure what */
               /*  the net effect will be.   --curetonk 09/09/93 */
	    addEdge( dg, el, Eindex, shared, share_info);
	}
    }
    
    /* If the node has a label it is a potential source of a jump to 
     *
     * unfortunately, we'll have to ignore for now since
     * do <label> ... continue loops are labeled as UNSTRUCT
     *
     * if (! (gen_get_label(node) == AST_NIL))
     *   li_max_cflow( li, UNSTRUCT);
     */
    
    if (is_compound(node))
    {				       /* IF or GUARD or DO */
	if (is_if(node))
	{
	    if (top)
		li_max_cflow( li, STRUCT);
	    walkList( el, li, infoPtr, dg, loop, 
		     gen_IF_get_guard_LIST(node), level, false);
	}
	
	else if (is_arithmetic_if(node))
	{
	    /* FIX this so that logical and arithmetic ifs don't constitute cflow */
	    if (top)
	    {  rvnode = gen_ARITHMETIC_IF_get_rvalue(node);
	       if ( is_goto(rvnode) || is_assigned_goto(rvnode) ||
		   is_computed_goto(rvnode) )
		   li_max_cflow( li, UNSTRUCT);
	       else
		   li_max_cflow( li, STRUCT);
	   }
	}
	
	else if (is_logical_if(node))
	{
	    if (top)
		li_max_cflow( li, STRUCT);
	    walkList ( el, li, infoPtr, dg, loop, 
		      gen_LOGICAL_IF_get_stmt_LIST(node), level, false);
	}
	
	else if (is_guard(node))
	{
	    if (top)
		li_max_cflow( li, STRUCT);
	    walkList ( el, li, infoPtr, dg, loop, 
		      gen_GUARD_get_stmt_LIST(node), level, false);
	}	
	
	else if (is_loop(node))
	{
	    if (node == loop)
		walkList ( el, li, infoPtr, dg, loop, 
			  gen_DO_get_stmt_LIST(node), level, true);
	    else
		walkList ( el, li, infoPtr, dg, loop,
			  gen_DO_get_stmt_LIST(node), level, false);
	}
	
    } /* end if (is_compound ... */
    else 
    { 
	if (is_goto(node))
	    li_max_cflow( li, UNSTRUCT);
	
	else if (is_assigned_goto(node))
	    li_max_cflow( li, UNSTRUCT);
	
	else if (is_computed_goto(node))
	    li_max_cflow( li, UNSTRUCT);
	
    } /* end else ... */
} /* end_walkStmt */

/* -------------- end of el_new_loop code ------------	*/

int 
el_total_num_deps(EL_Instance *el)
{
    return el->total_num_of_deps;
}
   

Boolean
edge_is_active(EL_Instance *el, int i)
{
    if (i < 0 || i >= el->total_num_of_deps)
        return false;

    return (el->edgelist[i].active);
}

/*----------------------------------------------------------------------
   get_dependence() - get the index of this edge in the dep edge list if
   		 	the dep edge is active.
  ----------------------------------------------------------------------*/
EDGE_INDEX 
get_dependence(EL_Instance *el, int i)
{
    if (i < 0 || i >= el->total_num_of_deps)
	return -1;

    if (!el->edgelist[i].active)
	return -1;

    return el->edgelist[i].edge_index;
}

/*----------------------------------------------------------------------
   first_dependence() - get the first dependence edge, and return its
   edge_index.
  ----------------------------------------------------------------------*/
EDGE_INDEX 
first_dependence(EL_Instance *el)
{
    if (el->total_num_of_deps == 0)     /* no edges in the filtered list */
	return -1;

    if (!el->edgelist[0].active)
    {
        el->current_dep_num = -1;
	return -1;
    }
    
    el->current_dep_num = 0;
    return (el->edgelist[el->current_dep_num].edge_index);
}

/*----------------------------------------------------------------------
   next_dependence() - get the next dependence edge, and return its
   edge_index. 
  ----------------------------------------------------------------------*/
EDGE_INDEX 
next_dependence(EL_Instance *el)
{
    if (el->current_dep_num == -1)     /* no edge selected */
	return -1;

    if (el->current_dep_num == el->total_num_of_deps - 1)   /* no next */
	return -1;

    if (!el->edgelist[el->current_dep_num+1].active)
	return -1;

    el->current_dep_num++;
    return (el->edgelist[el->current_dep_num].edge_index);
}

/*-----------------------------------------------------------------------
   prev_dependence() - get the prev dependence edge, and return its
   edge_index. 
  ----------------------------------------------------------------------*/
EDGE_INDEX 
prev_dependence(EL_Instance *el)
{
    if (el->current_dep_num == -1)  /* no edge selected */
	return (-1);

    if (el->current_dep_num ==  0)  /* no prev */
	return -1;

    if (!el->edgelist[el->current_dep_num-1].active)
	return -1;

    el->current_dep_num--;
    return (el->edgelist[el->current_dep_num].edge_index);
}


/*------------------------------------------------------------------------
  match() - find needle in haystack. `needle' is a char string that has to
            be located in the char string `haystack'. Return true if found.
 ------------------------------------------------------------------------*/
static Boolean 
match(char *haystack, char *needle)
{
    char           *p,
                    buf[100];
    int             i;

    if (strlen(haystack) == 0)
	return true;  /* VAS */

    for (p = haystack; *p != '\0'; p++)
    {
        /* get next word from haystack */
	i = 0;
	while (*p != '\0' && *p != ' ' && i <= 100)
	{
	    buf[i++] = *p;
	    p++;
	}
	buf[i] = '\0';

	if (strcmp(needle, buf) == 0)
	    return true;

	if (*p == '\0')
	    break;
    }

    return false;
}


/*------------------------------------------------------------------------
  fixDtype() - convert the string of dep types to the string of integers
  		that they correspond to.
 ------------------------------------------------------------------------*/
static void
fixDtype(char *dtypelist)
{
    DepType	type;
    int         i;
    int         len;

    len = strlen(dtypelist);
    for (i = 0; i < len; i++)
    {
	if (isalpha(dtypelist[i]))
	{
	    type = get_dtype(dtypelist[i]);
	    dtypelist[i] = ((int) type) + '0';
	}
    }
}


/*--------------------------------------------------------------------------
   These routines are for the Dependence Filter Facility. They are called
   from the /dp abstraction. -Vas, Sept. 1987

   el_show()   - show all deps that match this query. When a blank query is
   		 given, its effect is the same as showall().
   el_hide()   - hide all deps that match this query.
   el_showall() - show all deps that match query on top of stack. If there is
   		  no query on top of stack, then show all deps.

   Modified Sept. 1988 to make the dep filter facility behave like PTOOL's
   filter facility. Added the following routine:
   
   el_remove() - permanently remove deps matching this query from the dep 
		 graph. ** NOTE: the routine that calls this must call
		 "forcePedUpdate()" immediately on return, otherwise
		 something bad will happen. **

   Vas, Sept. 1988.
 ---------------------------------------------------------------------------*/ 

void
el_show(EL_Instance *el, DG_Instance *dg, char *list1, char *list2, 
        AST_INDEX ind1, AST_INDEX ind2, char *list3, char *list4)
{
    DG_Edge        *Earray = dg_get_edge_structure(dg);
    int             n;
    char            s[20];
    EDGE_INDEX      k;
    Boolean 	   rc1, rc2, rc3, rc4, rc5, rc6;
    
    fixDtype(list1);
    
    if(strlen(list1)==0 && strlen(list2)==0 && strlen(list3)==0 && 
       strlen(list4)==0 && ind1==AST_NIL && ind2==AST_NIL){
	el_showall(el);
	return;
    }
    for (n = 0; n < el->total_num_of_deps; n++)
    {
	k = el->edgelist[n].edge_index;
	sprintf(s, "%d", Earray[k].type);
	rc1 = BOOL(match(list1, s));
	rc2 = BOOL(match(list2, gen_get_text(Earray[k].src)));
	/*  IFs replaced by mpal   */ /* 910503  */
	if(ind1 == AST_NIL || ind1 == Earray[k].src)   
	    rc3 = true;
	else rc3 = false;
	if(ind2 == AST_NIL || ind2 == Earray[k].sink)  
	    rc4 = true;
	else rc4 = false;
	sprintf(s, "%d", el->edgelist[n].dims);
	rc5 = BOOL(match(list3, s));
	rc6 = BOOL(match(list4, el->edgelist[n].cblock));
	
	el->edgelist[n].active = BOOL(rc1 && rc2 && rc3 && rc4 && rc5 && rc6);
    }
}

void
el_hide(EL_Instance *el, DG_Instance *dg, char *list1, char *list2, 
        AST_INDEX ind1, AST_INDEX ind2, char *list3, char *list4)
{
    DG_Edge        *Earray = dg_get_edge_structure(dg);
    int             n;
    char            s[20];
    EDGE_INDEX      k;
    Boolean 	   rc1, rc2, rc3, rc4, rc5, rc6;
    
    fixDtype(list1);
    
    for (n = 0; n < el->total_num_of_deps; n++)
    {
	if (!el->edgelist[n].active)
	{   /* already inactive */
	    continue;
	}
	
	k = el->edgelist[n].edge_index;
	sprintf(s, "%d", Earray[k].type);
	rc1 = BOOL(match(list1, s));
	rc2 = BOOL(match(list2, gen_get_text(Earray[k].src)));
	/*  IFs replaced by mpal   */ /* 910503  */
	if(ind1 == AST_NIL || ind1 == Earray[k].src)   
	    rc3 = true;
	else rc3 = false;
	if(ind2 == AST_NIL || ind2 == Earray[k].sink)  
	    rc4 = true;
	else rc4 = false;
	sprintf(s, "%d", el->edgelist[n].dims);
	rc5 = BOOL(match(list3, s));
	rc6 = BOOL(match(list4, el->edgelist[n].cblock));
	
	el->edgelist[n].active = NOT(BOOL(rc1 && rc2 && rc3 && rc4 && rc5 && rc6));
    }
}

void
el_showall(EL_Instance *el)
{
    int             n;
    
    for (n = 0; n < el->total_num_of_deps; n++)
    {
	el->edgelist[n].active = true;
    }
}

void
el_remove(EL_Instance *el, DG_Instance *dg, char *list1, char *list2, 
          AST_INDEX ind1, AST_INDEX ind2, char *list3, char *list4)
{
    DG_Edge        *Earray = dg_get_edge_structure(dg);
    int             n;
    char            s[20];
    EDGE_INDEX      k;
    Boolean 	    rc1, rc2, rc3, rc4, rc5, rc6;
    
    fixDtype(list1);
    
    for (n = 0; n < el->total_num_of_deps; n++)
    {
	k = el->edgelist[n].edge_index;
	sprintf(s, "%d", Earray[k].type);
	rc1 = BOOL(match(list1, s));
	rc2 = BOOL(match(list2, gen_get_text(Earray[k].src)));
	if(ind1 != AST_NIL && ind1 != Earray[k].src)
	    rc3 = false;
	else rc3 = true;
	if(ind2 != AST_NIL && ind2 != Earray[k].sink)
	    rc4 = false;
	else rc4 = true;
	sprintf(s, "%d", el->edgelist[n].dims);
	rc5 = BOOL(match(list3, s));
	rc6 = BOOL(match(list4, el->edgelist[n].cblock));
	
	el->edgelist[n].active = BOOL(rc1 && rc2 && rc3 && rc4 && rc5 && rc6);
	if(el->edgelist[n].active){
	    /* this guy is history */
	    dg_delete_free_edge( dg, k);
	}
    }
    
}


/*	el_query_convert( EL_Instance * )
 *	   This routine converts the dependence type information into
 *	a text format.
 */
void
el_query_convert(EL_Instance *el)
{
   int  i, num, strLength;
   char	*buf;
   char	*str;

   str	= el->query.type;
   
   /* convert this string of digits into a string of dependence type
      characters. -vas 
      rewritten by mpal 920922 
      */
      
   strLength	= strlen(str);
   buf	= (char *)get_mem( 2*strLength + 1, "convert" );
   
   for(i = 0; i < strLength; i++) {
      if(isdigit(str[i])) {
         num = atoi(&str[i]);
	 switch(num) {
	     case dg_true: 	strcat(buf, "t ");
	     			break;
	     case dg_anti: 	strcat(buf, "a ");
	     			break;
	     case dg_output: 	strcat(buf, "o ");
	     			break;
	     case dg_input: 	strcat(buf, "n ");
	     			break;
	     case dg_inductive: strcat(buf, "i ");
	     			break;
	     case dg_exit: 	strcat(buf, "x ");
	     			break;
	     case dg_io: 	strcat(buf, "r ");
	     			break;
	     case dg_call: 	strcat(buf, "p ");
	     			break;
	     default:		break;
	 }
      }
   }
   sfree(str);
   el->query.type	= (char *)ssave(buf);
   free_mem(buf);
}


/*	el_query_init( EL_Instance )
 *	   This routine initializes the fields in the el->query structure.
 */
void
el_query_init(EL_Instance *el, char *type, char *text_name, AST_INDEX src_ast, 
              AST_INDEX sink_ast, char *text_dims, char *text_block)
{
	Query	*query;

	query->type = type;
	query->name = text_name;
	query->src  = src_ast;
	query->sink = sink_ast;
	query->dims = text_dims;
	query->block = text_block;
}

/*	el_query_free( EL_Instance )
 *	   free the data fields inside the query structure
 */
void
el_query_free(EL_Instance *el)
{
	sfree(el->query.type);
	sfree(el->query.name);
	sfree(el->query.dims);
	sfree(el->query.block);
}



/*
 *  el_sort() - The following sort routines are by ACW -vas.
 *
 *     Applies a string of "sorting commands" to a dependence edgelist.
 *  The string is taken apart into single commands and placed into a
 *  global structure g_sort[]. Qsort is then invoked with a comparator
 *  which will apply each of the sorting commands in g_sort to each edge
 *  in the dependence list.
 *
 *  Qsort will call edgeComp() on whichever edgelist element pairs
 *  it needs to in order to sort the edgelist array. The global g_Earray
 *  is a pointer to the current edge array and is needed by the comparator
 *  in order to access the fields of each dependence edge.
 */

static char     g_sort[10];
static DG_Edge *g_Earray;

/*
 *  The contents of g_sort[].
 */
 
#define SORT_SRC	1
#define SORT_SINK	2
#define SORT_TYPE	3
#define SORT_DIM	4
#define SORT_BLOCK	5
#define SORT_RSRC	6
#define SORT_RSINK	7
#define SORT_RTYPE	8
#define SORT_RDIM	9
#define SORT_RBLOCK	10
#define SORT_DONE	11

/*
 * The main sorting entrypoint. Sortlist is a string consisting of the
 * following seperated by blanks:
 *
 *   src  sink  type  dim  block    for ascending order
 *   rsrc rsink rtype rdim rblock   for decending order
 *
 */ 
void
el_sort(EL_Instance *el, DG_Instance *dg, char *sortlist)
{
    int		    i,slot;
    char	    *p;
    char	     buf[10];
    
    slot = 0;
    
    for (p = sortlist; *p != '\0'; p++)
    {
	i = 0;
	while (*p != '\0' && *p != ' ')
	{
	    if (isupper(*p))
		buf[i++] = tolower(*p);
	    else
		buf[i++] = *p;
	    p++;
	}
	buf[i] = '\0';
	
	if      (strcmp(buf,"src") == 0)
	    g_sort[slot++] = SORT_SRC;
	else if (strcmp(buf,"sink") == 0)
	    g_sort[slot++] = SORT_SINK;
	else if (strcmp(buf,"type") == 0)
	    g_sort[slot++] = SORT_TYPE;
	else if (strcmp(buf,"dim") == 0)
	    g_sort[slot++] = SORT_DIM;
	else if (strcmp(buf,"block") == 0)
	    g_sort[slot++] = SORT_BLOCK;
	else if (strcmp(buf,"rsrc") == 0)
	    g_sort[slot++] = SORT_RSRC;
	else if (strcmp(buf,"rsink") == 0)
	    g_sort[slot++] = SORT_RSINK;
	else if (strcmp(buf,"rtype") == 0)
	    g_sort[slot++] = SORT_RTYPE;
	else if (strcmp(buf,"rdim") == 0)
	    g_sort[slot++] = SORT_RDIM;
	else if (strcmp(buf,"rblock") == 0)
	    g_sort[slot++] = SORT_RBLOCK;
    }
    
    g_sort[slot] = SORT_DONE;
    g_Earray = dg_get_edge_structure(dg);    
    
    qsort((char *) el->edgelist,
	  el->total_num_of_deps,
	  sizeof(Edge_List),
	  (QsortCompareFunc)edgeComp);
}

/*
 * A reversing macro -1 -> 1 and 1 -> -1
 */
#define RR(value)  (rev ? -(value) : value)

static int
edgeComp(Edge_List *first, Edge_List *second)
{
    int         first_edge,
    second_edge;
    Boolean     first_active,
    second_active;
    DepType	first_type,
    second_type;
    int         i, ret;
    Boolean     rev;
    
    /* stash away some needed values */
    first_edge    = first->edge_index;
    second_edge   = second->edge_index;
    
    first_type    = g_Earray[first_edge].type;
    second_type   = g_Earray[second_edge].type;
    
    /* move all inactive edges to the end of this list */
    first_active  = first->active;
    second_active = second->active;
    
    if (first_active && !second_active)
 	return -1;
    if (!first_active && second_active)
	return 1;
    
    for (i = 0; g_sort[i] != SORT_DONE; i++)
    {
	rev = false;	/* don't reverse sort unless requested */
	
	switch (g_sort[i])
	{
	    
	    /* This does not sort by the name of the type, since this would
	     * place several of the "trash" dependencies (call, exit) at the
	     * top of the dependence list. Instead we will just group like
	     * edges together.
	     */
	case SORT_RTYPE:
	    rev = true;
	case SORT_TYPE:
	    if ((int) first_type < (int) second_type)
		return RR(-1);
	    else if ((int)first_type > (int)second_type)
		return RR(1);
	    break;
	    
	    /* If this dependence edge has a src and sink that are both
	       identifiers, then this will order the edges by the
	       names of the identifiers. */
	case SORT_RSRC:
	    rev = true;
	case SORT_SRC:
	    if (idRefs(first_type))
	    {
		if (idRefs(second_type))
		{		       /* compare them */
		    ret = strcmp(gen_get_text(g_Earray[first_edge].src),
				 gen_get_text(g_Earray[second_edge].src));
		    if (ret != 0)
			return RR(ret);
		}
		else
		    return RR(-1);
	    }
	    else
	    {
		if (idRefs(second_type))
		    return RR(1);
	    }
	    break;
	case SORT_RSINK:
	    rev = true;
	case SORT_SINK:
	    if (idRefs(first_type))
	    {
		if (idRefs(second_type))
		{		       /* compare them */
		    ret = strcmp(gen_get_text(g_Earray[first_edge].sink),
				 gen_get_text(g_Earray[second_edge].sink));
		    if (ret != 0)
			return RR(ret);
		}
		else
		    return RR(-1);
	    }
	    else
	    {
		if (idRefs(second_type))
		    return RR(1);
	    }
	    break;
	    
	    /* NYI */
	case SORT_RDIM:
	    rev = true;
	case SORT_DIM:
	    break;
	    
	case SORT_RBLOCK:
	    rev = true;
	case SORT_BLOCK:
	    break;
	}
    }
    return 0;
}

/*
 * Return true iff this dependence edge type goes from an identifier
 * to an identifier. An exit edge is incident on an entire statement,
 * not a pair of identifiers.
 */

static Boolean
idRefs(DepType type)
{
    switch (type)
    {
    case dg_true:
    case dg_anti:
    case dg_output:
    case dg_input:
    case dg_inductive:
	return true;
    case dg_exit:
    case dg_io:
    case dg_call:
    case dg_unknown:
    default:
	return false;
    }
}


/* misc. routines used by the dp abstraction */

int
el_get_dims(EL_Instance *el, int num)
{
    return(el->edgelist[num].dims);
}

char *
el_get_block(EL_Instance *el, int num)  
{
    return(el->edgelist[num].cblock);
}

char *
el_gen_get_text (AST_INDEX index)
{
    if (index == AST_NIL) 
	return (char *)0;
    return gen_get_text (index);
}
