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
 *
 *	  set_is_empty(set)		true if set has no members
 *
 */

#include <general.h>
#include <Arena.h>
#include <global.h>
#include <libc.h>


Set ut_create_set(arena_type *ar,
		  int n,
		  int size)
    {	Set ts;
	size = ((size-1) >> LOGBITS) + 1;
	ts = (Set) get_atom(ar,n,BYTES*(size+1));
	ts->words = size;
	ut_clear_set(ts);
	return ts;
    }


Bool ut_set_is_empty(Set	s)
    {	int *p = &s->word[0];
	int *e = p + s->words;
	do {
	    if (*p) return fa;
	    p++;
	} while (p < e);
	return tr;
    }


void ut_union121(Set s1, 
		 Set s2)
    {	int *ppb1 = &s1->word[0];
	int *ppb2 = &s2->word[0];
	int *ppb3 = ppb1 + s1->words;
	do {*ppb1 |= *ppb2;
	    ppb1++; ppb2++;
	} while (ppb1 < ppb3);
    }


void ut_intersect121(Set s1, 
		     Set s2)
    {	int *ppb1 = &s1->word[0];
	int *ppb2 = &s2->word[0];
	int *ppb3 = ppb1 + s1->words;
	do {*ppb1 &= *ppb2;
	    ppb1++; ppb2++;
	} while (ppb1 < ppb3);
    }

	
void ut_difference121(Set s1,
		      Set s2)
    {	int *ppb1 = &s1->word[0];
	int *ppb2 = &s2->word[0];
	int *ppb3 = ppb1 + s1->words;
	do {*ppb1 &= ~*ppb2;
	    ppb1++; ppb2++;
	} while (ppb1 < ppb3);
    }


void ut_complement(Set s)
    {	int *ppb1 = &s->word[0];
	int *ppb2 = ppb1 + s->words;
	do {*ppb1 ^= ~0;
	    ppb1++;
	} while (ppb1 < ppb2);
    }


void ut_set1u2i3(Set s1, 
		 Set s2, 
		 Set s3)
    {	int *ppb1 = &s1->word[0];
	int *ppb2 = &s2->word[0];
	int *ppb3 = &s3->word[0];
	int *ppb4 = ppb1 + s1->words;
	do {*ppb1 |= *ppb2 & *ppb3;
	    ppb1++; ppb2++; ppb3++;
	} while (ppb1 < ppb4);
    }


#ifndef INLINE

void ut_add_number(Set s,
		   int n)
    {
if (n < 0) fprintf(stderr, "AddNumber: negative argument");
if (n > s->words*32) fprintf(stderr, "AddNumber: large argument");
	s->word[n>>LOGBITS] |= 1<<(n & (NBITS-1));
    }


void ut_delete_number(Set s,
		      int n)
    {
if (n < 0) fprintf(stderr, "DeleteNumber: negative argument");
if (n > s->words*32) fprintf(stderr, "DeleteNumber: large argument");
	s->word[n>>LOGBITS] &= ~(1<<(n & (NBITS-1)));
    }


Bool ut_member_number(Set s,
		      int n)
    {
if (n < 0) fprintf(stderr, "MemberNumber: negative argument");
if (n > s->words*32) fprintf(stderr, "MemberNumber: large argument");
	return s->word[n>>LOGBITS] & 1<<(n & (NBITS-1)) ? tr : fa;
    }




void ut_clear_set(Set s)
    {	bzero(&s->word[0], s->words*BYTES);
    }


void ut_copy12(Set s1, 
	       Set s2)
    {	bcopy(&s2->word[0], &s1->word[0], s1->words*BYTES);
    }


Bool ut_sets_differ(Set s1, 
		    Set s2)
    {	return bcmp(&s1->word[0], &s2->word[0], s1->words*BYTES) ? tr : fa;
    }

#endif
