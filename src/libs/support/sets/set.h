/* $Id: set.h,v 1.8 1997/03/11 14:37:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *  Revision 3.
 * 
 *  The data structures and constants involved in the code generator
 *  set abstraction.
 */
#ifndef set_h
#define set_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#define NUMBITS	32
#define LOGBITS	 5
#define BYTES	 4

/* Universe -
 *
 *	Anytime that set 
 *	operations are done, they refer to the current universe, defined by
 *	a static pointer to the active universe.  The following entry points
 *	allow manipulation of universes:
 *
 *	  create_universe(size)  - allocates a universe with room 
 *					for "size" symbols.
 *
 *	  destroy_universe(univ) - frees all space associted with 
 *					"univ" and its sets.
 *
 *	  change_universe(univ)  - makes "univ" the active universe.
 * 
 *	To maintain all this, a universe needs the following structure:
 */


typedef struct set {
    struct set *next;
    int words;		/* the number of words allocated	*/
    int word[1];	/* the actual storage			*/
} SetBase, *Set;


typedef struct {
    int	words;		/* number of words to hold set elt's	*/
    Set	sets; 		/* list of all sets in the universe	*/
} UniverseBase, *Universe;



/*
 *  The externally visible interface
 */

EXTERN(Universe, create_universe, (int size));
EXTERN(void, destroy_universe, (Universe goner));
EXTERN(void, change_universe, (Universe new_universe));
EXTERN(Set, create_set, (void));
EXTERN(Set, copy_set, (Set set));
EXTERN(Set, copy_and_complement, (Set set));
EXTERN(void, destroy_set, (Set set));
EXTERN(Boolean, set_is_empty, (Set s));
EXTERN(int, set_next_member, (Set set, int index));
EXTERN(Boolean, sets_differ, (Set s1, Set s2));
EXTERN(void, print_set, (char *string, Set set, Universe universe));



#define add_number(s,n) \
    s->word[n>>LOGBITS] |= 1<<(n & (NUMBITS-1))

#define delete_number(s,n) \
    s->word[n>>LOGBITS] &= ~(1<<(n & (NUMBITS-1)))

#define member_number(s,n) \
    (s->word[n>>LOGBITS] & (1<<(n & (NUMBITS-1))))

#define union121(s1,s2) \
    {	register int ppb; \
	for (ppb=0;ppb<s1->words;ppb++) \
	    s1->word[ppb] |= s2->word[ppb]; \
    }

#define intersect121(s1,s2) \
    {	register int ppb; \
	for (ppb=0;ppb<s1->words;ppb++) \
	    s1->word[ppb] &= s2->word[ppb]; \
    }
	
#define difference121(s1,s2) \
    {	register int ppb; \
	for (ppb=0;ppb<s1->words;ppb++) \
	    s1->word[ppb] &= ~s2->word[ppb]; \
    }

#define difference122(s1,s2) \
    {	register int ppb; \
	for (ppb=0;ppb<s1->words;ppb++) \
	    s1->word[ppb] = s2->word[ppb] & ~s1->word[ppb]; \
    }

#define complement(s) \
    {	register int ppb; \
	for (ppb=0;ppb<s->words;ppb++) \
	    s->word[ppb] ^= ~0; \
    }

#define clear_set(s) \
    { 	register int ppb; \
	for (ppb=0;ppb<s->words;ppb++) \
	    s->word[ppb] = 0; \
    }

#define copy12(s1,s2) \
    {	register int ppb; \
	for (ppb=0;ppb<s1->words;ppb++) \
	    s1->word[ppb] = s2->word[ppb]; \
    }

#define forall(i,s) \
    for (i=0; i<(s->words<<LOGBITS); i++) if (member_number(s,i))

#define fforall(i,s,x,y) \
    for (x=0; x<s->words; x++) \
	if (y=s->word[x]) \
	    for (i=(x<<LOGBITS); y; i++ && (y=y>>1)) \
		if (y & 1)


/* a common step in a dataflow algorithm 	*/
/*   computes s1 union (s2 intersect s3),	*/
/*   leaving the result in s1			*/

#define set1u2i3(s1,s2,s3) \
    {	register int ppb; \
	for (ppb=0;ppb<s1->words;ppb++) \
	    s1->word[ppb] |= s2->word[ppb] & s3->word[ppb]; \
    }

#endif
