/* $Id: set.C,v 1.7 1996/02/14 11:02:55 carr Exp $ */
/*
 *  An implementation of sets as bit-vectors
 *	(part of the Rn Module Compiler code generator)
 *   Copied from code developed by Preston Brigs.
 */

#include <general.h>
#include <cgen_set.h>
#include <memory.h>
#include <stdio.h>
#include <strings.h>


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_create_set                                           */
/*                                                                          */
/*    Input:        ar - arena for memory allocation                        */
/*                  n  - which arena to allocate in                         */
/*                  size - size of the set                                  */
/*                                                                          */
/*    Description:  Create a bit vector with "size" elements                */
/*                                                                          */
/****************************************************************************/


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


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_set_is_empty                                         */
/*                                                                          */
/*    Input:        s - bit vector set                                      */
/*                                                                          */
/*    Description:  Determine if the set has no elements                    */
/*                                                                          */
/****************************************************************************/


Bool ut_set_is_empty(Set	s)
    {	int *p = &s->word[0];
	int *e = p + s->words;
	do {
	    if (*p) return fa;
	    p++;
	} while (p < e);
	return tr;
    }


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_union121                                             */
/*                                                                          */
/*    Input:        s1,s2 - bit vector sets                                 */
/*                                                                          */
/*    Description:  Unions s1 and s2 and store result in s1.                */
/*                                                                          */
/****************************************************************************/


void ut_union121(Set s1, 
		 Set s2)
    {	int *ppb1 = &s1->word[0];
	int *ppb2 = &s2->word[0];
	int *ppb3 = ppb1 + s1->words;
	do {*ppb1 |= *ppb2;
	    ppb1++; ppb2++;
	} while (ppb1 < ppb3);
    }


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_intersect121                                         */
/*                                                                          */
/*    Input:        s1,s2 - bit vector sets                                 */
/*                                                                          */
/*    Description:  Intersect s1 and s2 and store result in s1              */
/*                                                                          */
/****************************************************************************/


void ut_intersect121(Set s1, 
		     Set s2)
    {	int *ppb1 = &s1->word[0];
	int *ppb2 = &s2->word[0];
	int *ppb3 = ppb1 + s1->words;
	do {*ppb1 &= *ppb2;
	    ppb1++; ppb2++;
	} while (ppb1 < ppb3);
    }


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_difference121                                        */
/*                                                                          */
/*    Input:        s1,s2 - bit vector sets                                 */
/*                                                                          */
/*    Description:  Compute the difference of s1 and s2 and store result in */
/*                  s1.                                                     */
/*                                                                          */
/****************************************************************************/

	
void ut_difference121(Set s1,
		      Set s2)
    {	int *ppb1 = &s1->word[0];
	int *ppb2 = &s2->word[0];
	int *ppb3 = ppb1 + s1->words;
	do {*ppb1 &= ~*ppb2;
	    ppb1++; ppb2++;
	} while (ppb1 < ppb3);
    }


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_complement                                           */
/*                                                                          */
/*    Input:        s - bit vector set                                      */
/*                                                                          */
/*    Description:  Complement the set s.                                   */
/*                                                                          */
/****************************************************************************/


void ut_complement(Set s)
    {	int *ppb1 = &s->word[0];
	int *ppb2 = ppb1 + s->words;
	do {*ppb1 ^= ~0;
	    ppb1++;
	} while (ppb1 < ppb2);
    }


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_set1u2i3                                             */
/*                                                                          */
/*    Input:        s1,s2,s3 - bit vector sets                              */
/*                                                                          */
/*    Description:  Union s1 with the intersection of s2 and s3 and store   */
/*                  the result in s1.                                       */
/*                                                                          */
/****************************************************************************/


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


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_add_number                                           */
/*                                                                          */
/*    Input:        s - bit vector set                                      */
/*                  n - element index                                       */
/*                                                                          */
/*    Description:  Add the element indexed by "n" to "s".                  */
/*                                                                          */
/****************************************************************************/


void ut_add_number(Set s,
		   int n)
    {
if (n < 0) fprintf(stderr, "AddNumber: negative argument");
if (n > s->words*32) fprintf(stderr, "AddNumber: large argument");
	s->word[n>>LOGBITS] |= 1<<(n & (NBITS-1));
    }


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_delete_number                                        */
/*                                                                          */
/*    Input:        s - bit vector set                                      */
/*                  n - element index                                       */
/*                                                                          */
/*    Description:  Delete the element indexed by "n" from "s".             */
/*                                                                          */
/****************************************************************************/


void ut_delete_number(Set s,
		      int n)
    {
if (n < 0) fprintf(stderr, "DeleteNumber: negative argument");
if (n > s->words*32) fprintf(stderr, "DeleteNumber: large argument");
	s->word[n>>LOGBITS] &= ~(1<<(n & (NBITS-1)));
    }


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_member_number                                        */
/*                                                                          */
/*    Input:        s - bit vector set                                      */
/*                  n - element index                                       */
/*                                                                          */
/*    Description:  Determine if the element indexed by "n" is in "s".      */
/*                                                                          */
/****************************************************************************/


Bool ut_member_number(Set s,
		      int n)
    {
if (n < 0) fprintf(stderr, "MemberNumber: negative argument");
if (n > s->words*32) fprintf(stderr, "MemberNumber: large argument");
	return s->word[n>>LOGBITS] & 1<<(n & (NBITS-1)) ? tr : fa;
    }


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_clear_set                                            */
/*                                                                          */
/*    Input:        s - bit vector set                                      */
/*                                                                          */
/*    Description:  Make a set empty                                        */
/*                                                                          */
/****************************************************************************/


void ut_clear_set(Set s)
    {	memset((char *)&s->word[0], 0, s->words*BYTES);
    }


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_copy12                                               */
/*                                                                          */
/*    Input:        s1,s2 - bit vector sets                                 */
/*                                                                          */
/*    Description:  Copy s2 into s1.                                        */
/*                                                                          */
/****************************************************************************/


void ut_copy12(Set s1, 
	       Set s2)
    {	memcpy((const char *)&s1->word[0], (char *)&s2->word[0], s1->words*BYTES);
    }


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_sets_differ                                          */
/*                                                                          */
/*    Input:        s1,s2 - bit vector sets                                 */
/*                                                                          */
/*    Description:  Determine if s1 and s2 are different.                   */
/*                                                                          */
/****************************************************************************/


Bool ut_sets_differ(Set s1, 
		    Set s2)
    {	return memcmp((const char *)&s1->word[0], (const char *)&s2->word[0], s1->words*BYTES) ? tr : fa;
    }

#endif
