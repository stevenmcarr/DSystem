/* $Id: vector_dg.C,v 1.1 1997/06/25 15:06:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	File:	dep/dg/vector_dg.c					*/
/*	Author:	Kathryn McKinley					*/
/*									*/
/*									*/
/************************************************************************/

#include <libs/moduleAnalysis/dependence/dependenceGraph/private_dg.h>

#include <libs/support/arrays/ExtensibleArray.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>

/*		Forward Declarations of Local Functions			*/

STATIC(void,	free_vmd,(DG_Instance *dg, int start, int num_vectors));
STATIC(void,	put_on_free_vmd,(DG_Instance *dg, int i, int size));
STATIC(void,	set_levels_empty,(DG_Vector *vmd, int vec, int size));

STATIC( int,	delete_from_free_vmd, 
		( DG_Instance * dg, int i, int oneback ) );


void
dg_create_level_vectors(DG_Instance *dg, int num_vectors)
{

	dg->vmd            = (DG_Vector *)   xalloc (num_vectors * 6, sizeof (DG_Vector));
	dg->free_vmd       = (DG_Free_VMD *) xalloc (num_vectors * 6, sizeof (DG_Free_VMD));
	dg->free_vmd_stack = stack_create (sizeof (int));
	free_vmd (dg, 1, num_vectors * 6);
	
	dg->num_vmd      = num_vectors * 6;
	dg->vmd[0].size  = num_vectors * 6;
	dg->vmd[0].used  = false;

	dg->first_free_vmd    = 0;
	dg->free_vmd[0].index = 0;
	dg->free_vmd[0].next_size = END_OF_LIST;
	dg->free_vmd[0].next_free = END_OF_LIST;
}

int
dg_alloc_level_vector(DG_Instance *dg, int size)
{
	int i;		/* index of a free descriptor (free_vmd) */
	int j;
	int k;
	int vec;	/* the index into the dependecy lists (vmd) */
	int oneback = END_OF_LIST;
	
	size += 2;
	
	/* If the free list is empty, we reallocate it, and set i to
	 * dg->num_vmd.
	 */	
	if ((i = dg->first_free_vmd) == END_OF_LIST)
	{
		/* Increase by a factor of 1/2 the number of vectors avaiable.*/
		j		   = dg->num_vmd;
		dg->num_vmd 	   += j/2;
		dg->vmd            = (DG_Vector *)   xrealloc ((int *)dg->vmd, dg->num_vmd);
		dg->free_vmd       = (DG_Free_VMD *) xrealloc ((int *)dg->free_vmd, dg->num_vmd);
		free_vmd (dg, j, dg->num_vmd);
		
		dg->vmd[j].size  = j/2;
		dg->vmd[j].used  = false;
		
		Generic temp;
		(void) stack_pop (dg->free_vmd_stack, &temp);
		i = (int) temp;
		dg->free_vmd[i].index     = j;
		dg->free_vmd[i].next_size = END_OF_LIST;
		dg->free_vmd[i].next_free = END_OF_LIST;
		dg->first_free_vmd = i;
	}
	/* walk the free list until a vector of appropriate size is found */

	for ( vec = dg->free_vmd[i].index; 
	     (dg->vmd[vec].size < size) && 
	     (dg->free_vmd[i].next_size != END_OF_LIST);
	     vec = dg->free_vmd[i].index)
	{ 
		oneback = i;
	     	i = dg->free_vmd[i].next_size;
	}
	/* there is one of appropriate size */
	
	if (dg->vmd[vec].size >= size)
	{ 
		if ((j = delete_from_free_vmd( dg, i, oneback)) == -1)
		{ 
			/* there is 1 item, with an exact fit */
			if (dg->vmd[vec].size == size)
			{
				(void) stack_push (dg->free_vmd_stack, (Generic *)&(dg->first_free_vmd));
				dg->first_free_vmd = END_OF_LIST;
			}
		}

		if (dg->vmd[vec].size != size)
		{/* split it, and put new free size in the appropriate place
		  */
		  	dg->vmd[vec + size].used = false;
			dg->vmd[vec + size].size = dg->vmd[vec].size - size;
		  	dg->vmd[vec].size = size;
			dg->free_vmd[i].index = dg->free_vmd[i].index + size;
			put_on_free_vmd( dg, i, dg->vmd[vec + size].size);
		}
		else 
		{/* reuse of the free descriptor */
			stack_push (dg->free_vmd_stack, (Generic *)&i);
		}
		dg->vmd[vec].used 	= true;
		dg->vmd[vec].src.next	= NOT_SET;
		dg->vmd[vec].sink.next	= NOT_SET;
		dg->num_free_vmd        -= size;
		set_levels_empty (dg->vmd, vec, size);
		return (Generic) vec;
	}
	/* we need to allocate a new chunk */
	vec = dg->num_vmd; 	/* where we're putting it */ 
		
	/* determine the size of the new chunk */
	/* set k to be the number of new things */
	if (size > dg->num_vmd/2)
	{
		dg->num_vmd      += size * MIN_LIST_HEADS;
		dg->num_free_vmd += size * MIN_LIST_HEADS;
		k		 =  size * MIN_LIST_HEADS;
	}
	else
	{
		dg->num_free_vmd += dg->num_vmd/2;
		k		  = dg->num_vmd/2;
		dg->num_vmd      += dg->num_vmd/2;
	}

	dg->vmd           = (DG_Vector *)   xrealloc ((int *)dg->vmd, dg->num_vmd);
	dg->free_vmd      = (DG_Free_VMD *) xrealloc ((int *)dg->free_vmd, dg->num_vmd);
	free_vmd( dg, vec + 1, dg->num_vmd);
	
	/* use the first size elements for the new vector */
	dg->vmd[vec].size       = size;
	dg->vmd[vec].used	= true;
	dg->vmd[vec].src.next	= NOT_SET;
	dg->vmd[vec].sink.next	= NOT_SET;
	set_levels_empty (dg->vmd, vec, size);
	
	/* put what's left on the free lists */
	dg->vmd[vec + size].size  = k - size;
	dg->vmd[vec + size].used  = false;

	/* there are always as many discriptors as entries */
	Generic temp1;
	(void) stack_pop (dg->free_vmd_stack, &temp1);
	k = (int)temp1;
	dg->free_vmd[k].index = vec + size;
	put_on_free_vmd( dg, k, dg->vmd[vec + size].size); 
	dg->num_free_vmd -= size;
	return (Generic) vec;
}

void 
dg_free_level_vector(DG_Instance *dg, int vec)
{
	int k;
	Generic temp;
	
	(void) stack_pop (dg->free_vmd_stack, &temp);
	k = (int)temp;
	
	dg->vmd[vec].used = false;
	put_on_free_vmd( dg, k, dg->vmd[vec].size);
	dg->num_free_vmd += 1;
}

/********************************************************/
/* All the following routines are for internal use only */
/********************************************************/

static void
free_vmd(DG_Instance *dg, int start, int num_vectors)
{
	EDGE_INDEX i;
	
	for (i = num_vectors - 1; i >= start; i--)
	{/* push em on from high numbers down, so low ones come off first! */
		stack_push (dg->free_vmd_stack, (Generic *)&i);
	}
	dg->num_free_vmd = num_vectors - start;
}

static int
delete_from_free_vmd (DG_Instance *dg, int i, int oneback)
{
	int j, k;

  	/* the next size of free objects. */
	k = dg->free_vmd[i].next_size;
	j = dg->free_vmd[i].next_free;
	
	/* If there is only one object, return */
	if ((j == END_OF_LIST) && (k == END_OF_LIST) && 
	    (dg->first_free_vmd == i))
	{
		return -1;
	}		
	if (j != END_OF_LIST)
	{/* there are more of this size */
	
		if (i != dg->first_free_vmd)
		{/* it's not the first guy in the free lists */
			dg->free_vmd[oneback].next_size = j;
			dg->free_vmd[j].next_size = k;
		}
		else
		{/* we delete the first one and reset the 
		  * index for it
		  */
			dg->first_free_vmd = j;
			dg->free_vmd[j].next_size = k;
		}
	}
	else
	{ /* only one of its size */
		if (i != dg->first_free_vmd)
		{
			dg->free_vmd[oneback].next_size = k;
		}
		else
		{
			dg->first_free_vmd = k;
		}
	}
	return 1;
}

static void
put_on_free_vmd(DG_Instance *dg, int i, int size)
{
	int k;
	int vec;
	int oneback = END_OF_LIST;
	
	/* There is only this one object on the free list */
	if ((i == dg->first_free_vmd) &&
	    (dg->free_vmd[i].next_size == END_OF_LIST) &&
	    (dg->free_vmd[i].next_free == END_OF_LIST))
	{
		return;
	}
	if (dg->first_free_vmd == END_OF_LIST)
	{
		dg->first_free_vmd = i;
		dg->free_vmd[i].next_free = END_OF_LIST;
		dg->free_vmd[i].next_size = END_OF_LIST;
		return;
	}
	k = dg->first_free_vmd;
	for ( vec = dg->free_vmd[k].index; 
	     (dg->vmd[vec].size < size) && 
	     (dg->free_vmd[k].next_size != END_OF_LIST);
	     vec = dg->free_vmd[k].index)
	{ 
		oneback = k;
	     	k = dg->free_vmd[k].next_size;
	}
	if (dg->vmd[vec].size == size)
	{/* put i on the front of the free vec list for this size */
		dg->free_vmd[i].next_size = dg->free_vmd[k].next_size;
		dg->free_vmd[i].next_free = k;
		
		/* attach to the rest of the list */
		if (k != dg->first_free_vmd)
			dg->free_vmd[oneback].next_size = i;
		else
			dg->first_free_vmd = i;
	}
	else
	{	/* we're starting a new list, put i after k */
		if (dg->vmd[vec].size < size)
		{
			dg->free_vmd[i].next_size = dg->free_vmd[k].next_size;
			dg->free_vmd[i].next_free = END_OF_LIST;
			dg->free_vmd[k].next_size = i;
		}
		/* put i before k */
		else 
		{
			dg->free_vmd[i].next_size = k;
			dg->free_vmd[i].next_free = END_OF_LIST;

			/* attach to the rest of the list */
			if (k != dg->first_free_vmd)
				dg->free_vmd[oneback].next_size = i;
			else
				dg->first_free_vmd = i;
		}
	}	
}

static void
set_levels_empty (DG_Vector *vmd, int vec, int size)
{
	int i;
	
	for (i = 0; i < size; i++)
	{
		vmd[vec +i].src.next  = NOT_SET;
		vmd[vec +i].sink.next = NOT_SET;
	}
}


