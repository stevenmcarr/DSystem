/* $Id: set.C,v 1.1 1997/06/25 15:19:03 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *  An implementation of sets as bit-vectors
 *	(part of the Rn Module Compiler code generator)
 */

/*
 *  The plan:
 *
 * 	This implementation of sets is based on a bit vector paradigm.
 *	It supports the following operations:
 *
 *	  create_set()			allocates
 * 	  destroy_set()			frees
 *
 *	  create_universe()		allocates new set space
 * 	  destroy_universe()		frees set space
 *	  change_universe()		changes set space
 *
 *	  set_is_empty( set )		true if set has no members
 *	  sets_differ( set1, set2 )	true if set1 != set2
 *
 *
 */

#include <stdio.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/sets/set.h>

/*	As discussed in the comments in sets.h, there is a single active
 *	universe.  To ensure its sanctity, it is declared as a static 
 *	variable, specific to this file, making it accessible only to code
 *	compiled in this file.  
 */
static Universe set_base;

/* create_set()
 *
 *	allocates a set, links it into the universe, returns it!
 */
Set create_set()
{
    register Set ts;

  /* allocate it */
    ts = (Set) get_mem(BYTES*(set_base->words+2),"SET HEADER" );

    ts->words = set_base->words;

  /* initialize it to the empty set */
    clear_set( ts );

  /* add it to the list of active sets maintained by the universe */
    ts->next = set_base->sets;
    set_base->sets = ts;

    return ts;
}


/* destroy_set()
 *
 *	unlinks a set from the universe and discards it.
 */
void destroy_set(Set set)
{
    register Set p;

    if (set == NULL) {
	(void) fprintf( stderr, "destroy_set: Freeing NULL set!\n" );
	return;
    }

    if (set_base->sets == set) {
	set_base->sets = set->next;
    }
    else {
	p = set_base->sets;
	while (p != NULL && p->next != set) {
	    p = p->next;
	}
	if (p == NULL) {
	    (void) fprintf( stderr, "destroy_set: set not from this universe!\n" );
	    return;
	}
	p->next = set->next;
    }
    free_mem((void*) set );
}


/* create_universe() 
 *
 * 	returns a pointer to a newly allocated universe, suitable
 * 	for passing to change_universe() and destroy_universe();
 */
Universe create_universe(int size)
{
    register Universe result;

    result = (Universe) get_mem(sizeof(UniverseBase), "IDFA SET UNIVERSE");
    result->words = (size-1)/NUMBITS + 1;
    result->sets  = NULL;		/* list used for reallocation	*/
    return result;
}


/* destroy_universe()
 *
 *	deallocates the universe pointed to by its argument.
 */
void destroy_universe(Universe goner)
{
    register Set temp;		/* used with the set list	*/

    while (goner->sets != NULL) {
	temp = goner->sets;
	goner->sets = temp->next;
	free_mem((void*) temp );
    }

    free_mem((void*) goner );	/* free the universe */
}


/* change_universe()
 *
 *	takes one argument, a universe, and makes it the default 
 *	for future set operations.
 */
void change_universe(Universe new_universe)
{
    set_base = new_universe;
}


Boolean set_is_empty(Set s)
{
    register int i;

    for (i=0; i<s->words; i++)
	if (s->word[i] != 0)
	    return false;

    return true;
}


Boolean sets_differ(Set s1, Set s2)
{
    register int i;

    for (i=0; i<s1->words; i++)
	if (s1->word[i] != s2->word[i])
	    return true;

    return false;
}

Set copy_set (Set set)
{
	register Set newSet;
	register int i;
	
	newSet = create_set();
	for (i = 0; i < newSet->words; i++)
	{
		newSet->word[i] = set->word[i];
	}
	return newSet;
}

/*ARGSUSED*/
void print_set (char *string, Set set, Universe universe)
{
	register int i;
	
	(void) fprintf (stdout, "%s {", string);
	forall (i,set) (void) fprintf (stdout, "%d, ", i);
	(void) fprintf (stdout, "}\n");
}

Set copy_and_complement (Set set)
{
	register Set newSet;
	
	newSet = copy_set (set);
	complement(newSet);
	return newSet;
}


int set_next_member(Set set, int index)
{
  register int i;	/* loop counter */
  register int j;	/* loop counter */
  register int bits;

  bits  = index % 32; /* index REM 32 */

  for (i=index >> 5 /* index DIV 32 */; i < set->words; i++)
  {
    if (set->word[i] != 0)
       for (j=bits; j < 32; j++)
       {
	 if ((1<<j) & set->word[i])
	    return (i<<5)+j;
       }
    bits = 0;
  }
  return -1; /* indicates no next member */
}

