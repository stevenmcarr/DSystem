/* $Id: tuple.C,v 1.1 1997/06/25 15:19:03 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdio.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/support/sets/tuple.h>

/*
 * Create a TUPLE_SET with tuples of size n.
 */
TUPLE_SET*
tuple_set_create (int n)
{
    TUPLE_SET      *newset;

    newset = (TUPLE_SET *) get_mem (sizeof (TUPLE_SET), "tuple set");
    newset->first = 0;
    newset->n = n;
    return newset;
}

/*
 * Is this TUPLE_SET empty?
 */
Boolean
tuple_set_is_empty (TUPLE_SET* set)
{
    return BOOL (set->first == 0);
}

/*
 * Create a new TUPLE_SET and add a copy of each of the tuples in 
 * the old set into the new one.
 */
TUPLE_SET*
tuple_set_copy (TUPLE_SET* oset)
{
    TUPLE_SET      *set;
    TUPLE          *oldelt;
    TUPLE          *newelt;
    TUPLE          *prevnewelt;
    int             n;

    n = oset->n;
    set = tuple_set_create (n);
    prevnewelt = 0;
    oldelt = oset->first;
    while (oldelt != 0)
    {
	newelt = tuple_copy (n, oldelt);
	if (prevnewelt == 0)
	{
	    set->first = newelt;
	    newelt->next = 0;
	    prevnewelt = newelt;
	}
	else
	{
	    prevnewelt->next = newelt;
	    newelt->next = 0;
	    prevnewelt = newelt;
	}

	oldelt = oldelt->next;
    }

    return set;
}

/*
 * Union set2 into set1. For each tuple in set2 that is not already
 * in set1, create a copy of the tuple and insert this tuple into set1.
 * Set2 is not modified.
 */
void
tuple_set_union (TUPLE_SET* set1, TUPLE_SET* set2)
{
    TUPLE          *curr2;
    TUPLE          *curr1;
    TUPLE	   *elt;
    TUPLE	   *prev;
    int             n,cmp;

    n = set2->n;
    curr1 = set1->first;
    curr2 = set2->first;
    while (curr2 != 0)
    {
	/*
	 * for this element of the second set, search the first set till we find it, or pass it, if we pass it, then
	 * backup one and insert it... 
	 */

	prev = 0;

	while (curr1 != 0)
	{
	    cmp = tuple_cmp (n, curr1, curr2);
	    /* sets...not multisets */
	    if (cmp == 0)
		break;

	    if (cmp < 0)
	    {
	        elt = tuple_copy (n, curr2);
		if (prev == 0)
		{
		    set1->first = elt;
		    elt->next = curr1;
		}
		else
		{
		    prev->next = elt;
		    elt->next = curr1;
		}

		/* grab the tail of the new list */
	        curr1 = elt;
		break;
	    }

	    /* keep advancing */
	    prev = curr1;
	    curr1 = curr1->next;
	}

	if (curr1 == 0)
	{			/* elt must be after the end of the list */
	    elt = tuple_copy (n, curr2);

	    if (prev == 0)
	    {
		set1->first = elt;
		elt->next = curr1;
	    }
	    else
	    {
		prev->next = elt;
		elt->next = curr1;
	    }
	    curr1 = elt;
	}

    /* advance to the next element in the second set */
    curr2 = curr2->next;
    }
}

/*
 * Insert a tuple into a set in reverse lexicographic order. 
 */
void
tuple_set_insert (TUPLE_SET* set, TUPLE* elt)
{
    TUPLE          *curr,
                   *prev;
    int             n,
                    cmp;

    n = set->n;

    prev = 0;
    curr = set->first;
    while (curr != 0)
    {
	cmp = tuple_cmp (n, curr, elt);
	/* sets...not multisets */
	if (cmp == 0)
	    return;

	/* if curr < elt then elt should be inserted just before curr */
	if (cmp < 0)
	{
	    if (prev == 0)
	    {
		set->first = elt;
		elt->next = curr;
	    }
	    else
	    {
		prev->next = elt;
		elt->next = curr;
	    }
	    return;
	}

	prev = curr;
	curr = curr->next;
    }

    /* elt must be after the end of the list */
    if (prev == 0)
    {
	set->first = elt;
	elt->next = curr;
    }
    else
    {
	prev->next = elt;
	elt->next = curr;
    }
}


/*
 *
 */
TUPLE*
tuple_locate (TUPLE_SET* set, TUPLE* elt)
{
    TUPLE	*t;
    int		n;

    n = set->n;
    for (t=set->first; t!=0; t=tuple_next(set,t))
    {
	if (tuple_cmp(n, t, elt) == 0)
	    break;
    }
    return t;
}

/*
 * Delete a tuple from its set.
 */
void
tuple_set_delete (TUPLE_SET* set, TUPLE* elt)
{
    TUPLE          *next,
                   *prev,
		   *temp;
    int             n;

    n = set->n;
    next = elt->next;

    if (set->first == elt)
	set->first = next;
    else
    {     /* Locate previous element */
	prev = set->first;
        for (temp=tuple_next(set,(TUPLE *)0); temp!=0; temp=tuple_next(set,temp))
	{
	    if (tuple_cmp (n, temp, elt) == 0)
		break;
	    prev = temp;
	}
	prev->next = next;
    }

    tuple_free (elt);    
}

/*
 * Free all of the tuples in this set, and free the TUPLE_SET
 * structure itself.
 */
void
tuple_set_free (TUPLE_SET* set)
{
    TUPLE	   *curr,
		   *save;

    /* free the set elements */
    curr = set->first;
    while (curr != 0)
    {
	save = curr;
        curr = curr->next;
	tuple_free(save);
    }    
    free_mem ((void*) set);
}

/*
 * Do these two sets contain identical tuples?
 */
Boolean
tuple_set_equal (TUPLE_SET* s1, TUPLE_SET* s2)
{
    TUPLE          *elt1,
                   *elt2;
    int             n,
                    cmp;

    n = s1->n;
    elt1 = s1->first;
    elt2 = s2->first;
    while (elt1 != 0 && elt2 != 0)
    {
	cmp = tuple_cmp (n, elt1, elt2);
	if (cmp != 0)
	    return false;
	elt1 = elt1->next;
	elt2 = elt2->next;
    }

    /* if both are 0, then these are equal, and the lists are the same */
    return BOOL (elt1 == elt2);
}

/*
 * Return the number of elements in this set.
 */
int
tuple_set_length (TUPLE_SET* set)
{
    TUPLE          *curr;
    register int    length;

    length = 0;
    curr = set->first;
    while (curr != 0)
    {
	length++;
	curr = curr->next;
    }

    return length;
}

/*
 * Delete the items in set2 that occur in set1. This does not 
 * modify set2.
 */
void
tuple_set_subtract (TUPLE_SET* set1, TUPLE_SET* set2)
{
    TUPLE          *curr1,
		   *curr2,
                   *next1,
                   *next2,
		   *prev;
    int		    cmp;
    int		    n;

    n = set1->n;

    prev = 0;
    curr1 = set1->first;
    curr2 = set2->first;
    while (curr2 != 0)
    {
	next2 = curr2->next;

	/* now delete away this element from set1 */
  	while (curr1 != 0)
	{
	   next1 = curr1->next;

	   cmp = tuple_cmp (n, curr2, curr1);
	   if (cmp > 0)
	      break;
	   else if (cmp == 0)
	   {  /* a match ... delete it from set1 */
	      if (prev != 0)
		prev->next = next1;
	      else
		set1->first = next1;

	      tuple_free(curr1);
	       /* prev remains the same */
	      curr1 = next1;
	      break;
	   }	   
	
           prev  = curr1;
	   curr1 = next1;
	}

	/* nothing to delete in set1 */
	if (curr1 == 0)
	   return;

	curr2 = next2;
    }
}


/* Use method similar to tuple_set_subtract to find intersection of two tuple
 * sets.  Deletes from set1 those elements not in both sets.  This does
 * not alter set2.
 */
void
tuple_set_intersect (TUPLE_SET* set1, TUPLE_SET* set2)
{
    TUPLE          *curr1,
		   *curr2,
                   *next1,
                   *next2,
		   *prev;
    int		    cmp;
    int		    n;

    n = set1->n;

    prev = 0;
    curr1 = set1->first;
    curr2 = set2->first;
    while (curr1 != 0)
    {
	next1 = curr1->next;

        if (curr2 == 0)	
	{ /* delete element of set1 */
	      if (prev != 0)
		prev->next = next1;
	      else
		set1->first = next1;

	      tuple_free(curr1);
	       /* prev remains the same */
	      curr1 = next1;	    
	}

  	while (curr2 != 0)
	{
	   next2 = curr2->next;

	   cmp = tuple_cmp (n, curr2, curr1);
	   if (cmp < 0)
	   { /* curr1 cannot be an elt of set2 */
	      if (prev != 0)
		prev->next = next1;
	      else
		set1->first = next1;

	      tuple_free(curr1);
	       /* prev remains the same */
	      curr1 = next1;
	      break;
	   }

	   else if (cmp == 0)
	   {  /* a match.  Advance ptrs for both sets. */
		prev  = curr1;
		curr1 = next1;
		curr2 = next2;
		break;
	   }	   
	
	   else 
	   { /* Advance ptr for set2 */
		curr2 = next2;
	   }
	}
    }
}


/*
 * Dump the contents of this set to stdout.
 */
void
tuple_set_print (TUPLE_SET* set)
{
    TUPLE          *curr;
    int             n;

    curr = set->first;
    n = set->n;
    while (curr != 0)
    {
	tuple_print (n, curr);
	curr = curr->next;
	if (curr != 0)
	    (void) printf (" | ");
    }
    (void) printf ("\n");
}

/*
 * Is elt a member of set?
 */
Boolean
tuple_set_is_member (TUPLE_SET* set, TUPLE* elt)
{
    TUPLE          *curr;
    int             n,
                    cmp;

    curr = set->first;
    n = set->n;
    while (curr != 0)
    {
	cmp = tuple_cmp (n, elt, curr);
	/* the one we want */
	if (cmp == 0)
	    return true;

	/* did we pass it...only if elt > curr */
	if (cmp > 0)
	    return false;

	curr = curr->next;
    }

    return false;
}

/*
 * Step through a TUPLE_SET, tuple by tuple. If elt is 0 then return
 * the first tuple in the set, otherwise return the tuple following
 * elt.
 */
TUPLE*
tuple_next (TUPLE_SET* set, TUPLE* elt)
{
    if (elt == 0)
	return set->first;

    return elt->next;
}

/*
 *  Copy a tuple.
 */
TUPLE          *
tuple_copy (int n, TUPLE *elt)
{
    TUPLE          *newelt;
    int            *tnums,
                   *onums;
    register int    i;

    newelt = tuple_create (n);
    tnums = &newelt->num;
    onums = &elt->num;
    for (i = 0; i < n; i++)
	tnums[i] = onums[i];

    return newelt;
}

/*
 * Free a tuple. Elt points to the TUPLE structure and to the 
 * extra integers tacked on.
 */
void
tuple_free (TUPLE *elt)
{
    free_mem ((void*) elt);
}

/*
 * Create a tuple of size n by allocating a TUPLE + n-1 tacked on
 * integers.
 */
TUPLE          *
tuple_create (int n)
{
    TUPLE          *newelt;

    newelt = (TUPLE *) get_mem (sizeof (TUPLE) + (n - 1) * sizeof (int), "tuple");
    return newelt;
}

/*
 * Dump the contents of a tuple to stdout.
 */
void
tuple_print (int n, TUPLE *elt)
{
    int             i,
                   *n1;

    n1 = &elt->num;
    for (i = 0; i < n; i++)
	(void) printf (" %d ", n1[i]);
}

/*
 *  Return the contents of the left-most slot in an n-tuple. 
 */
int
tuple_get_one (TUPLE *tuple)
{
    int            *tnums;

    tnums = &tuple->num;
    return tnums[0];
}


/*
 *  Return the contents of the nth slot in an n-tuple. 
 */
int
tuple_get_nth (TUPLE *tuple, int n)
{
    int            *tnums;

    tnums = &tuple->num;
    return tnums[n-1];
}

/*
 *  Create a 1-TUPLE with value a.
 */
TUPLE          *
tuple_create_1 (int a)
{
    TUPLE          *newelt;
    int            *nums;

    newelt = (TUPLE *) get_mem (sizeof (TUPLE), "tuple");
    nums = &newelt->num;
    nums[0] = a;
    return newelt;
}

/*
 *  Create a 2-TUPLE with values a and b.
 */
TUPLE          *
tuple_create_2 (int a, int b)
{
    TUPLE          *newelt;
    int            *nums;

    newelt = (TUPLE *) get_mem (sizeof (TUPLE) + sizeof (int), "tuple");
    nums = &newelt->num;
    nums[0] = a;
    nums[1] = b;
    return newelt;
}

/*
 *  Create a 3-TUPLE with values a,b,c.
 */
TUPLE          *
tuple_create_3 (int a, int b, int c)
{
    TUPLE          *newelt;
    int            *nums;

    newelt = (TUPLE *) get_mem (sizeof (TUPLE) + sizeof (int) + sizeof (int), "tuple");
    nums = &newelt->num;
    nums[0] = a;
    nums[1] = b;
    nums[2] = c;
    return newelt;
}

/*
 * Compare two n-tuples. Return -1 if elt1 < elt2, 1 if elt1 > elt2
 * and 0 if elt1 == elt2. (lexicographically)
 */
int
tuple_cmp (int n, TUPLE *elt1, TUPLE *elt2)
{
    register int    i;
    int            *n1,
                   *n2;

    n1 = &elt1->num;
    n2 = &elt2->num;

    for (i = 0; i < n; i++)
    {
	if (n1[i] == n2[i])
	    continue;
	else
	if (n1[i] < n2[i])
	    return -1;
	else
	    return 1;
    }

    return 0;
}

