/* $Id: edges_dg.C,v 1.1 1997/06/25 15:06:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	File:	dep/dg/edges_dg.c					*/
/*	Author:	Kathryn McKinley					*/
/*									*/
/************************************************************************/


/************************************************************************/
/*			Include Files					*/
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/private_dg.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/dep_dt.h>

#include <libs/support/arrays/ExtensibleArray.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>

#ifdef SOLARIS
EXTERN(void,bzero,(char *s, size_t n));
#endif

/************************************************************************/
/*	Forward Declarations of Local Functions		*/
/************************************************************************/
STATIC(void,	set_all_free,( ));
STATIC(void,	dg_add_stmt_dep,( ));
STATIC(void,	verify_edgeptr,(DG_Edge *a, DG_Edge *b));
STATIC(void,	set_all_free,(DG_Instance *dg, int start, 
                              int num_edges));
STATIC(EDGE_INDEX,	get_free_edge,(DG_Instance *dg));
STATIC(void,	clear_edge,(DG_Instance *dg, EDGE_INDEX i));
STATIC(Boolean,	check_indexes,(DG_Instance *dg, EDGE_INDEX edge,
                               int src_vector, int sink_vector,
                               int src_ref, int sink_ref, 
                               char *routine));
STATIC(Boolean,	check_indexes2,(DG_Instance *dg, EDGE_INDEX edge,
                                int src_vector, int sink_vector,
                                char *routine ));
STATIC(DG_Edge *, dg_get_edge_ptr, (DG_Instance * DG));
/* static	DG_Edge *dg_get_edge_ptr( ); */


/*---------------------------------------------------------------------*/
/* (DG_Edge *) dg_create_edge_structure(dg, num_edges)
 *
 *	This returns a pointer to the edge array for the DG.  Num_edges
 *	is the number of edges which should initially be allocated.
 *	None of the edges are considered used at this time.
 */

DG_Edge        *
dg_create_edge_structure(DG_Instance *dg, int num_edges)
{
  if (num_edges < MIN_EDGES)
    num_edges = MIN_EDGES;

  dg->edgeptr = (DG_Edge *) xalloc(num_edges, sizeof(DG_Edge));
  bzero( (char *)dg->edgeptr, num_edges*sizeof(DG_Edge) ); /* zero edgeArray */
  dg->ref_list = (DG_List *) xalloc(num_edges, sizeof(DG_List));
  bzero( (char *)dg->ref_list, num_edges*sizeof(DG_List) ); /* zero ref_list */
  dg->vec_list = (DG_List *) xalloc(num_edges, sizeof(DG_List));
  bzero( (char *)dg->vec_list, num_edges*sizeof(DG_List) ); /* zero vec_list */

  dg->fstack = stack_create(sizeof(EDGE_INDEX));
  dg->num_edges = num_edges;

  /* Invariant:  0 <= num_edges - num_free <= num_edges (total allocated)
   * (num_free set in set_all_free) */
  set_all_free(dg, 0, num_edges);

  /* HACK */
  dg_create_level_vectors(dg, num_edges); /* was 2, 4000 */
  dg_create_ref_lists(dg, num_edges);

  return (dg->edgeptr);
}

/*---------------------------------------------------------------------*/

DG_Edge        *
dg_get_edge_structure(DG_Instance *dg)
{
  return (dg->edgeptr);
}

/* EDGE_INDEX dg_alloc_edge( dg, edgeptr)
 *
 *	Returns an index into the edge array, edgeptr, of an edge not currently
 *	in use.  The edge is marked as used, and all its other fields are
 *	initialized to be an unset value.
 */

/*---------------------------------------------------------------------*/

EDGE_INDEX
dg_alloc_edge(DG_Instance *dg, DG_Edge **edgeptrptr)
     /* DG_Edge       **edgeptrptr;	existing array of edges */
{
  int             i;
  int             oldI;		/* last position not needing to be zeroed */
  EDGE_INDEX      edge;

  verify_edgeptr(dg->edgeptr, *edgeptrptr);

  if (dg->num_free <= 0)
    {
      /* Manually reallocate the xarray to be 1 and
       * 1/2 times its current size.  The first
       * num_edges elements are untouched, the last
       * contiguous (num_edges/2) are put on the
       * fstack */

      /* reallocate the array of edges, adding some */   
      /* initialization added, 910507, mpal */

      i    = dg->num_edges;
      oldI = i;
      i   += i >> 1;		/* increment by ~50% */

      *edgeptrptr = (DG_Edge *) xrealloc((int *) *edgeptrptr, i);
      bzero( (char *)&((*edgeptrptr)[oldI]), (i-oldI)*sizeof(DG_Edge) ); /* zero new edgeArray */
      dg->ref_list = (DG_List *) xrealloc((int *) dg->ref_list, i);
      bzero( (char *)&(dg->ref_list[oldI]), (i-oldI)*sizeof(DG_List) );	/* zero new ref_list */
      dg->vec_list = (DG_List *) xrealloc((int *) dg->vec_list, i);
      bzero( (char *)&(dg->vec_list[oldI]), (i-oldI)*sizeof(DG_List) );	/* zero new vec_list */

      dg->edgeptr = *edgeptrptr;

      /* put the new ones on the fstack */
      set_all_free( dg, dg->num_edges, i);
      dg->num_edges = i;
    }

  /* get an edge off the free list, guaranteed to be at least of size 1 */
  edge = get_free_edge(dg);
  clear_edge( dg, edge);
  return edge;
}

/*---------------------------------------------------------------------*/
/*  (void) dg_free_edge( dg, edgeArray, edgeIndex)
 *
 *	Puts edge on the free stack.
 */
void
dg_free_edge(DG_Instance *dg, DG_Edge *edgeptr, EDGE_INDEX edge)
{
  verify_edgeptr(edgeptr, dg->edgeptr);
  dg->edgeptr[edge].used = false;
  stack_push(dg->fstack, (Generic *) & edge);
  (dg->num_free)++;
}

/*---------------------------------------------------------------------*/
static void
dg_add_stmt_dep(DG_Instance *dg, EDGE_INDEX edge, int src_vec, 
                int sink_vec)
{
  int         level         = dg->edgeptr[edge].level + 1;
  DG_List    *vec_list      = &(dg->vec_list[edge]);
  EDGE_INDEX *vmd_src_next  = &(dg->vmd[src_vec + level].src.next);
  EDGE_INDEX *vmd_sink_next = &(dg->vmd[sink_vec + level].sink.next);

  /* We always insert at the top of the list, so there is no previous pointer. */
  vec_list->src.prev  = NOT_SET;
  vec_list->sink.prev = NOT_SET;

  /* copy the vector's next pointer into the new edge list's vector */
  vec_list->src.next  = *vmd_src_next;
  vec_list->sink.next = *vmd_sink_next;
  
  /* If there was at least one edge in the list, set its previous
   * pointer to the new edge. */
  if (*vmd_src_next != NOT_SET)
    {
      dg->vec_list[*vmd_src_next].src.prev = edge;
    }
  if (*vmd_sink_next != NOT_SET)
    {
      dg->vec_list[*vmd_sink_next].sink.prev = edge;
    }

  /* Now point the level vector header at the first edge in its list. */
  *vmd_src_next = edge;
  *vmd_sink_next = edge;
}

/*---------------------------------------------------------------------*/
/* (void) dg_add_edge( dg, edge)
 *
 *	Takes the edge and inserts it into all the appropriate lists.
 */
void
dg_add_edge(DG_Instance *dg, EDGE_INDEX edge)
{
  DG_List    *ref_list      = &(dg->ref_list[edge]);
  DG_Edge    *edgeptr       = &(dg->edgeptr[edge]);
  int         src_vec       = edgeptr->src_vec;
  int         sink_vec      = edgeptr->sink_vec;
  int         src_ref       = edgeptr->src_ref;
  int         sink_ref      = edgeptr->sink_ref;
  EDGE_INDEX *src_ref_next  = &(dg->ref[src_ref].src.next);     
  EDGE_INDEX *sink_ref_next = &(dg->ref[sink_ref].sink.next);     


  /* special handling for statement only dependences	*/

  if ((edgeptr->type == dg_exit) ||
      (edgeptr->type == dg_io) ||
      (edgeptr->type == dg_control) ||
      (edgeptr->type == dg_call))
    {
      if (!check_indexes2(dg, edge, src_vec, sink_vec, "dg_add_edge"))
	dg_add_stmt_dep(dg, edge, src_vec, sink_vec);
      return;
    }

  if (check_indexes(dg, edge, src_vec, sink_vec, 
		    src_ref, sink_ref, "dg_add_edge"))
    return;

  /* Starting with reference lists */

  /* We always insert at the top of the list, so there is no previous
   * pointer. */
  ref_list->src.prev  = NOT_SET;
  ref_list->sink.prev = NOT_SET;

  /* copy the reference's next pointer into the new edge list's ref */
  ref_list->src.next  = *src_ref_next;
  ref_list->sink.next = *sink_ref_next;

  /* If there was at least one edge in the list, set its previous
   * pointer to the new edge. */
  if (*src_ref_next != NOT_SET)
    {
      dg->ref_list[*src_ref_next].src.prev = edge;
    }
  if (*sink_ref_next != NOT_SET)
    {
      dg->ref_list[*sink_ref_next].sink.prev = edge;
    }

  /* Now point the reference header at the first edge in its list. */
  *src_ref_next  = edge;
  *sink_ref_next = edge;

  /* Now level vectors. */
  dg_add_stmt_dep( dg, edge, src_vec, sink_vec);
}

/*---------------------------------------------------------------------*/
/* (void) dg_delete_edge( dg, edge)
 *
 *	Deletes the edge from all lists.  Does not free storage.
 *	Edge should be reinserted later.	910603, mpal
 */

void
dg_delete_edge(DG_Instance *dg, EDGE_INDEX edge)
{
  int             level;
  int             i;
  DG_Edge        *edgeptr  = &(dg->edgeptr[edge]);
  int             src_vec  = edgeptr->src_vec;
  int             sink_vec = edgeptr->sink_vec;
  int             src_ref;
  int             sink_ref;
  Boolean         statement_only;

  /*----------------------------------------------------*/
  /* special handling for statement only dependences	*/

  statement_only = BOOL((edgeptr->type == dg_exit) ||
			(edgeptr->type == dg_io) ||
			(edgeptr->type == dg_control) ||
			(edgeptr->type == dg_call));

  if (statement_only)		/* statement level dependence	*/
    {
      if (check_indexes2(dg, edge, src_vec, sink_vec, "dg_delete_edge"))
	return;
    }
  else				/* reference level dependence	*/
    {
      src_ref = edgeptr->src_ref;
      sink_ref = edgeptr->sink_ref;

      if (check_indexes( dg, edge, src_vec, sink_vec, 
			src_ref, sink_ref, "dg_delete_edge"))
	return;
    }

  level = edgeptr->level + 1;

  /*----------------------------------------------------------------*/
  /* first the vec lists	 */

  /* Delete the edge in the vector src lists. */

  if (edge == dg->vmd[src_vec + level].src.next)
    {				/* it's the first one on the src list */
      dg->vmd[src_vec + level].src.next = dg->vec_list[edge].src.next;
      if (dg->vmd[src_vec + level].src.next != NOT_SET)
	{
	  /* there is more than one on the list, detach
	   * the edge */
	  i = dg->vmd[src_vec + level].src.next;
	  dg->vec_list[i].src.prev = NOT_SET;
	}
    }

  /* it's either in the middle or last in a list of at least 2 edges */
  else if (dg->vec_list[edge].src.next == NOT_SET)
    {			
      /* it's last */
      /* detach whoever has edge as its next src */
      i = dg->vec_list[edge].src.prev;
      if (i != NOT_SET)
	dg->vec_list[i].src.next = NOT_SET;
    }

  else				/* it's in the middle */
    {
      /* copy the edges next and prev pointers to its comrade */
      i = dg->vec_list[edge].src.next;
      if (i != NOT_SET)
	dg->vec_list[i].src.prev = dg->vec_list[edge].src.prev;
      i = dg->vec_list[edge].src.prev;
      if (i != NOT_SET)
	dg->vec_list[i].src.next = dg->vec_list[edge].src.next;
    }

  /* Delete the edge in the vector sink lists. */

  if (edge == dg->vmd[sink_vec + level].sink.next)
    {
      /* it's the first one on the sink list */
      dg->vmd[sink_vec + level].sink.next = dg->vec_list[edge].sink.next;
      if (dg->vmd[sink_vec + level].sink.next != NOT_SET)
	{
	  /* there is more than one on the list, detach
	   * the edge */
	  i = dg->vmd[sink_vec + level].sink.next;
	  dg->vec_list[i].sink.prev = NOT_SET;
	}
    }

  /* it's either in the middle or last in a list of at least 2 edges */
  else if (dg->vec_list[edge].sink.next == NOT_SET)
    {
      /* it's last */
      /* detach whoever has edge as its next sink */
      i = dg->vec_list[edge].sink.prev;
      if (i != NOT_SET)
	dg->vec_list[i].sink.next = NOT_SET;
    }

  else				/* it's in the middle */
    {
      /* copy the edges next and prev pointers to its comrade */
      i = dg->vec_list[edge].sink.next;
      if (i != NOT_SET)
	dg->vec_list[i].sink.prev = dg->vec_list[edge].sink.prev;
      i = dg->vec_list[edge].sink.prev;
      if (i != NOT_SET)
	dg->vec_list[i].sink.next = dg->vec_list[edge].sink.next;
    }

  /* clear the edge's next and prev pointers */

  dg->vec_list[edge].src.next = NOT_SET;
  dg->vec_list[edge].src.prev = NOT_SET;
  dg->vec_list[edge].sink.next = NOT_SET;
  dg->vec_list[edge].sink.prev = NOT_SET;

  /*----------------------------------------------------------------*/
  /* now the ref lists	 */

  if (statement_only)		/* no need to look at ref lists	 */
    return;

  /* Delete the edge in the reference src lists. */

  if (edge == dg->ref[src_ref].src.next)
    {	
      /* its the first one on the src list */
      dg->ref[src_ref].src.next = dg->ref_list[edge].src.next;
      if (dg->ref[src_ref].src.next != NOT_SET)
	{
	  /* there is more than one on the list, detach
	   * the edge */
	  i = dg->ref[src_ref].src.next;
	  dg->ref_list[i].src.prev = NOT_SET;
	}
    }

  /* it's either in the middle or last in a list of at least 2 edges */
  else if (dg->ref_list[edge].src.next == NOT_SET)
    {		
      /* it's last */
      /* detach whoever has edge as its next src */
      i = dg->ref_list[edge].src.prev;
      if (i != NOT_SET)
	dg->ref_list[i].src.next = NOT_SET;
    }

  else				/* it's in the middle */
    {
      /* copy the edges next and prev pointers to its comrade */
      i = dg->ref_list[edge].src.next;
      if (i != NOT_SET)
	dg->ref_list[i].src.prev = dg->ref_list[edge].src.prev;
      i = dg->ref_list[edge].src.prev;
      if (i != NOT_SET)
	dg->ref_list[i].src.next = dg->ref_list[edge].src.next;
    }


  /* Delete the edge in the reference sink lists. */

  if (edge == dg->ref[sink_ref].sink.next)
    {	
      /* it's the first one on the sink list */
      dg->ref[sink_ref].sink.next = dg->ref_list[edge].sink.next;
      if (dg->ref[sink_ref].sink.next != NOT_SET)
	{			/* there is more than one on the list, detach the edge */
	  i = dg->ref[sink_ref].sink.next;
	  dg->ref_list[i].sink.prev = NOT_SET;
	}
    }

  /* it's either in the middle or last in a list of at least 2 edges */
  else if (dg->ref_list[edge].sink.next == NOT_SET)
    {	
      /* it's last */
      /* detach whoever has edge as its next sink */
      i = dg->ref_list[edge].sink.prev;
      if (i != NOT_SET)
	dg->ref_list[i].sink.next = NOT_SET;
    }

  else				/* it's in the middle */
    {
      /* copy the edges next and prev pointers to its comrade */
      i = dg->ref_list[edge].sink.next;
      if (i != NOT_SET)
	dg->ref_list[i].sink.prev = dg->ref_list[edge].sink.prev;
      i = dg->ref_list[edge].sink.prev;
      if (i != NOT_SET)
	dg->ref_list[i].sink.next = dg->ref_list[edge].sink.next;
    }

  /* clear the edge's next and prev pointers */

  dg->ref_list[edge].src.next = NOT_SET;
  dg->ref_list[edge].src.prev = NOT_SET;
  dg->ref_list[edge].sink.next = NOT_SET;
  dg->ref_list[edge].sink.prev = NOT_SET;
}

/*---------------------------------------------------------------------*/
/* (void) dg_delete_free_edge( dg, edge)
 *
 *	Deletes the edge from all lists.
 *	Then frees the memory for reuse.		910603, mpal
 */

void
dg_delete_free_edge(DG_Instance *dg, EDGE_INDEX edge)
{
  DG_Edge	*Elist;

  dg_delete_edge( dg, edge );
  Elist = dg_get_edge_structure( dg );
  dg_free_edge( dg, Elist, edge );

}


/********************************************************/
/* All the following routines are for internal use only */
/********************************************************/

/*---------------------------------------------------------------------*/
static void
verify_edgeptr(DG_Edge *a, DG_Edge *b)
{
  if (a != b)
    {
      (void) fprintf(stdout,
		     "Edge table pointer and instance variable are inconsistent\n");
      exit(-1);
    }
}

/*---------------------------------------------------------------------*/
/* This routine assumes that there is enough room in the free list for
 *	(num_edges - start) edges.  All edges between start and num_edges
 *	are put into an empty free list;
 */

static void 
set_all_free(DG_Instance *dg, int start, int num_edges)
{
  EDGE_INDEX      i;

  for (i = (EDGE_INDEX) start; i < (EDGE_INDEX) num_edges; i++)
    {
      stack_push(dg->fstack, (Generic *) & i);
    }
  dg->num_free = num_edges - start;
}

/*---------------------------------------------------------------------*/

static EDGE_INDEX
get_free_edge(DG_Instance *dg)
{
  EDGE_INDEX      i;

  (void) stack_pop(dg->fstack, &i);
  (dg->num_free)--;
  return i;
}

/*---------------------------------------------------------------------*/
static void
clear_edge(DG_Instance *dg, EDGE_INDEX i)
{
  DG_Edge *edgeptr  = &(dg->edgeptr[i]);
  DG_List *ref_list = &(dg->ref_list[i]);
  DG_List *vec_list = &(dg->vec_list[i]);

  edgeptr->src     = NOT_SET;
  edgeptr->sink    = NOT_SET;
  edgeptr->level   = NOT_SET;
  edgeptr->type    = dg_unknown;
  edgeptr->used    = true;
  edgeptr->dt_type = DT_UNKNOWN;

  ref_list->src.next  = NOT_SET;
  ref_list->sink.next = NOT_SET;
  ref_list->src.prev  = NOT_SET;
  ref_list->sink.prev = NOT_SET;
	
  vec_list->src.next  = NOT_SET;
  vec_list->sink.next = NOT_SET;
  vec_list->src.prev  = NOT_SET;
  vec_list->sink.prev = NOT_SET;
}

/*---------------------------------------------------------------------*/
static Boolean
check_indexes(DG_Instance *dg, EDGE_INDEX edge, int src_vector, 
              int sink_vector, int src_ref, int sink_ref, 
              char *routine)
{
  /* Are they at least positive? */
  if ((edge < 0) || (src_vector < 0) || (sink_vector < 0) ||
      (src_ref < 0) || (sink_ref < 0))
    {
      fprintf(stdout, "Negative index from %s\n", routine);
      return true;
    }
  if (!(dg->edgeptr[edge].used && dg->vmd[src_vector].used &&
	dg->vmd[sink_vector].used && dg->ref[src_ref].used &&
	dg->ref[sink_ref].used))
    {
      fprintf(stdout, "Referencing an unused index, from %s\n", routine);
      return true;
    }
  if ((dg->vmd[sink_vector].size < dg->edgeptr[edge].level) ||
      (dg->vmd[src_vector].size < dg->edgeptr[edge].level))
    {
      fprintf(stdout, "The level of the dependence is deeper than its associated level vectors , from %s\n", routine);
      return true;
    }
  return false;
}

/*---------------------------------------------------------------------*/
/* like check_indexes, but for statement only dependences	*/
static Boolean
check_indexes2(DG_Instance *dg, EDGE_INDEX edge, int src_vector, 
               int sink_vector, char *routine)
{
  /* Are they at least positive? */
  if ((edge < 0) || (src_vector < 0) || (sink_vector < 0))
    {
      fprintf(stdout, "Negative index from %s\n", routine);
      return true;
    }
  if (!(dg->edgeptr[edge].used && dg->vmd[src_vector].used &&
	dg->vmd[sink_vector].used ))
    {
      fprintf(stdout, "Referencing an unused index, from %s\n", routine);
      return true;
    }
  if ((dg->vmd[sink_vector].size < dg->edgeptr[edge].level) ||
      (dg->vmd[src_vector].size < dg->edgeptr[edge].level))
    {
      fprintf(stdout, "The level of the dependence is deeper than its associated level vectors , from %s\n", routine);
      return true;
    }
  return false;
}


/*---------------------------------------------------------------------*/

static DG_Edge	*
dg_get_edge_ptr(DG_Instance *dg)
{
  return (dg->edgeptr);
}



