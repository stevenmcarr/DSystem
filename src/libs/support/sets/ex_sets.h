/* $Id: ex_sets.h,v 1.3 1997/03/11 14:37:22 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *  exmon's set abstraction--currently uses a closed hashing scheme
 */

#ifndef ex_sets_h
#define ex_sets_h

# define	HASHTABSIZE	251	/* a prime number */

# ifndef	NULL
#    define	NULL	0
# endif

struct set_elt_str {
    char               *key;
    Generic             val;
    /* struct set_elt_str *next_in_bucket; */
};

typedef struct set_elt_str *set_elt;
typedef struct set_elt_str *set;

EXTERN(void, ex_set_create, (set *Sp));
EXTERN(void, ex_set_destroy, (set S));
EXTERN(void, ex_set_insert, (set S, char *key, Generic val));
EXTERN(set_elt, ex_set_member, (set S, char *key));

#endif
