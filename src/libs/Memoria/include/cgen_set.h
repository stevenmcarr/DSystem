/* $Id: cgen_set.h,v 1.5 1992/12/11 11:19:44 carr Exp $ */

/*
 *  Revision 4
 * 
 *  The data structures and constants involved in the code generator
 *  set abstraction.
 */

#ifndef cgen_set_h
#define cgen_set_h

#ifndef general_h
#include <general.h>       /* for EXTERN */
#endif
#ifndef Arena_h
#include <Arena.h>    /* for arena_type */
#endif
#ifndef global_h
#include <global.h>   /* for Bool */
#endif

#define NBITS	32
#define LOGBITS	 5
#define BYTES	 4

typedef struct set {
    int words;		/* the number of words allocated	*/
    int word[1];	/* the actual storage			*/
} SetBase, *Set;


EXTERN(Set, ut_create_set,(arena_type *ar,int n,int size));
EXTERN(Bool, ut_set_is_empty,(Set s));
EXTERN(void, ut_union121,(Set s1,Set s2));
EXTERN(void, ut_intersect121,(Set s1,Set s2));
EXTERN(void, ut_difference121,(Set s1,Set s2));
EXTERN(void, ut_complement,(Set s));
EXTERN(void, ut_set1u2i3,(Set s1, Set s2, Set s3));


#ifdef INLINE

#define ut_add_number(s,n) \
    s->word[n>>LOGBITS] |= 1<<(n & (NBITS-1))

#define ut_delete_number(s,n) \
    s->word[n>>LOGBITS] &= ~(1<<(n & (NBITS-1)))

#define ut_member_number(s,n) \
    ((s->word[n>>LOGBITS] & (1<<(n & (NBITS-1))))?true:false)

#define ut_clear_set(s) \
	bzero(&s->word[0], s->words*BYTES)

#define ut_copy12(s1,s2) \
	bcopy(&s2->word[0], &s1->word[0], s1->words*BYTES)

#define ut_sets_differ(s1, s2) \
	bcmp(&s1->word[0], &s2->word[0], s1->words*BYTES)

#else

EXTERN(void, ut_add_number,(Set s, int n));
EXTERN(void, ut_delete_number,(Set s, int n));
EXTERN(Bool, ut_member_number,(Set s, int n));
EXTERN(void, ut_clear_set,(Set s));
EXTERN(void, ut_copy12,(Set s1, Set s2));
EXTERN(Bool, ut_sets_differ,(Set s1, Set s2));

#endif

#define ut_forall(i,s) \
    for (i=0; i<(s->words<<LOGBITS); i++) if (ut_member_number(s,i))

#define ut_fforall(i,s,x,y) \
    for (x=0; x<s->words; x++) \
	if (y=s->word[x]) \
	    for (i=(x<<LOGBITS); y; (y=((y>>1) & 0x7FFFFFFF)) && i++) \
		if (y & 1)

#endif /* cgen_set_h */
