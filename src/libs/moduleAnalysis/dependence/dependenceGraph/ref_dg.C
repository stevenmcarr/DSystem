/* $Id: ref_dg.C,v 1.1 1997/06/25 15:06:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	File:	dep/dg/ref_dg.c	 					*/
/*	Author:	Kathryn McKinley					*/
/*									*/
/*									*/
/************************************************************************/

#include <libs/support/arrays/ExtensibleArray.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/private_dg.h>

void dg_create_ref_lists (DG_Instance *dg, int num_refs)
{
	dg->ref_fstack = stack_create (sizeof(int));
	dg->ref = (DG_Single_List *) 
				xalloc (num_refs, sizeof (DG_Single_List));
	set_all_ref_free( dg, 0, num_refs);
	dg->num_ref = num_refs;
}
/*	Returns a handle for a reference list.
 */
int 
dg_alloc_ref_list(DG_Instance *dg)
{
	int i;
	
	if (dg->num_free_ref <= 0)
	{
		i = dg->num_ref + (dg->num_ref/2);
		dg->ref = (DG_Single_List *)xrealloc ((int *)dg->ref, i);
		set_all_ref_free( dg, dg->num_ref, i);
		dg->num_ref = i;
	}
	Generic temp = i;
	(void) stack_pop (dg->ref_fstack, &temp);
	i = (int)temp;
	dg->num_free_ref--;
	dg->ref[i].used = true;
	dg->ref[i].src.next  = NOT_SET;
	dg->ref[i].sink.next = NOT_SET;
	return i;
}

/* (void) dg_free_ref_list(dg, ref)
 *
 *	Put ref on the reference free list.
 */
void
dg_free_ref_list(DG_Instance *dg, int ref)
{

	stack_push (dg->ref_fstack, (Generic *)&ref);
	dg->ref[ref].used = false;
	(dg->num_free_ref)++;
}

void
set_all_ref_free(DG_Instance *dg, int start, int num_refs)
{
	EDGE_INDEX i;
	
	for (i = start; i < num_refs; i++)
	{
		stack_push (dg->ref_fstack, (Generic *)&i);
		dg->ref[i].used = false;
	}
	dg->num_free_ref = num_refs - start;
}
