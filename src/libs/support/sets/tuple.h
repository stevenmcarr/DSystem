/* $Id: tuple.h,v 1.5 1997/03/11 14:37:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * This is the generic tuple code for sets. A set of n-tuples is 
 * represented as a sorted singly linked list of n-tuples. Each n-tuple
 * is consists of n integers.
 *
 * A TUPLE_SET contains a pointer to the first tuple in the set, and 
 * an integer indicating how big each tuple is (in integers).
 *
 * A TUPLE contains a pointer to the next tuple in the set, and a 
 * single integer field for the left-most slot in the n-tuple. If
 * n > 1, then additional integers are allocated just after the TUPLE
 * for storing the 2 - nth slots in the n-tuple. 
 */

#ifndef tuple_h
#define tuple_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

struct tuple_set
{
    struct tuple *first;	
    int		  n;		/* number of elements in each tuple */
};
typedef struct tuple_set TUPLE_SET;

struct tuple
{
    struct tuple  *next;	
    int		   num;		/* array of integers containing tuple elements */
};
typedef struct tuple     TUPLE;

/*
 * Create a TUPLE_SET with tuples of size n.
 */
EXTERN(TUPLE_SET *, tuple_set_create, (int n));

/*
 * Is this TUPLE_SET empty?
 */
EXTERN(Boolean, tuple_set_is_empty, (TUPLE_SET *set));

/*
 * Create a new TUPLE_SET and add a copy of each of the tuples in 
 * the old set into the new one.
 */
EXTERN(TUPLE_SET *, tuple_set_copy, (TUPLE_SET *oset));

/*
 * Union set2 into set1. For each tuple in set2 that is not already
 * in set1, create a copy of the tuple and insert this tuple into set1.
 * Set2 is not modified.
 */
EXTERN(void, tuple_set_union, (TUPLE_SET *set1, TUPLE_SET *set2));

 
/*
 * Insert a tuple into a set in lexicographic order. 
 */
EXTERN(void, tuple_set_insert, (TUPLE_SET *set, TUPLE *elt));


/*
 * Delete a tuple from its set.
 */
EXTERN(void, tuple_set_delete, (TUPLE_SET *set, TUPLE *elt));


/*
 * Return point to tuple elt in set if equal to elt
 */
EXTERN(TUPLE *, tuple_locate, (TUPLE_SET *set, TUPLE *elt));


/*
 * Free all of the tuples in this set, and free the TUPLE_SET
 * structure itself.
 */
EXTERN(void, tuple_set_free, (TUPLE_SET *set));

/*
 * Do these two sets contain identical tuples?
 */
EXTERN(Boolean, tuple_set_equal, (TUPLE_SET *s1, TUPLE_SET *s2));

/*
 * Return the number of elements in this set.
 */
EXTERN(int, tuple_set_length, (TUPLE_SET *set));

/*
 * Delete the items from set1 that occur in set2. This does not 
 * modify set2.
 */
EXTERN(void, tuple_set_subtract, (TUPLE_SET *set1, TUPLE_SET *set2));


/* Use method similar to tuple_set_subtract to find intersection of two tuple
 * sets.  Deletes from set1 those elements not in both sets.  This does
 * not alter set2.
 */
EXTERN(void, tuple_set_intersect, (TUPLE_SET *set1, TUPLE_SET *set2));


/*
 * Dump the contents of this set to stdout.
 */
EXTERN(void, tuple_set_print, (TUPLE_SET *set));

/*
 * Is elt a member of set?
 */
EXTERN(Boolean, tuple_set_is_member, (TUPLE_SET *set, TUPLE *elt));

/*
 * Step through a TUPLE_SET, tuple by tuple. If elt is 0 then return
 * the first tuple in the set, otherwise return the tuple following
 * elt.
 */
EXTERN(TUPLE *, tuple_next, (TUPLE_SET *set, TUPLE *elt));

/*
 *  Copy a tuple.
 */
EXTERN(TUPLE *, tuple_copy, (int n, TUPLE *elt));

/*
 * Free a tuple. Elt points to the TUPLE structure and to the 
 * extra integers tacked on.
 */
EXTERN(void, tuple_free, (TUPLE *elt));

/*
 * Create a tuple of size n by allocating a TUPLE + n-1 tacked on
 * integers.
 */
EXTERN(TUPLE *, tuple_create, (int n));

/*
 * Dump the contents of a tuple to stdout.
 */
EXTERN(void, tuple_print, (int n, TUPLE *elt));

/*
 * Return the contents of the left-most slot in an n-tuple. 
 */
EXTERN(int, tuple_get_one, (TUPLE *tuple));

/*
 * Return the contents of the nth slot in an n-tuple. 
 */
EXTERN(int, tuple_get_nth, (TUPLE *tuple, int n));

/*
 * Create a 1-TUPLE with value a.
 */
EXTERN(TUPLE *, tuple_create_1, (int a));

/*
 *  Create a 2-TUPLE with values a and b.
 */
EXTERN(TUPLE *, tuple_create_2, (int a, int b));

/*
 * Create a 3-TUPLE with values a,b,c.
 */
EXTERN(TUPLE *, tuple_create_3, (int a, int b, int c));

/*
 * Compare two n-tuples. Return -1 if elt1 < elt2, 1 if elt1 > elt2
 * and 0 if elt1 == elt2. (lexicographically)
 */
EXTERN(int, tuple_cmp, (int n, TUPLE *elt1, TUPLE *elt2));


#endif
