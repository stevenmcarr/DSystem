/* $Id: ex_sets.C,v 1.1 1997/06/25 15:19:03 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <string.h>

#include <libs/graphicInterface/oldMonitor/include/mon/cp_def.h>
#include <libs/support/sets/ex_sets.h>

STATIC(int, locate, (set S, char *key));
STATIC(int, hash,   (char *key));
STATIC(int, delta,  (char *key));

/*
 *  initialize S to be {}
 */
void ex_set_create (set *Sp)
{
    int i;

    *Sp = (set) get_mem(sizeof(struct set_elt_str) * HASHTABSIZE, "ex_set");

    for ( i = 0; i < HASHTABSIZE; i++ )
	(*Sp)[i].key = (char *)NULL;
}


/*
 *  free memory for set S
 */
void ex_set_destroy (set S)
{
    free_mem((void*)S);
}


/*
 *  if not already there, insert key in the set S
 *    -- record val in the map field in any case
 */
void ex_set_insert (set S, char *key, Generic val)
{
    int p;

    /* locate where key is (or where it goes) in S's hashtable */
	p = locate(S, key);

    /* insert key, val at S[p] */
	if ( S[p].key == (char *)NULL ) S[p].key = key;
	S[p].val = val;
}


/*
 *  if key is in S, return its set elt
 *  if not there then return NULL
 */
set_elt ex_set_member (set S, char *key)
{
    int p;

    p = locate(S, key);
    if ( p < 0  ||  S[p].key == (char *)NULL )	return (set_elt)NULL;
    else					return(&S[p]);
}


static int locate (set S, char *key)
{
    int p, d, count;

    /* locate where key is, or where it should be */
	p = hash(key);
	d = delta(key);
	count = 0;
	while ( S[p].key && strcmp(S[p].key, key) && count < HASHTABSIZE ) {
	    count++;
	    p += d;
	    p %= HASHTABSIZE;
	}

	if ( count >= HASHTABSIZE ) {
	    /* error -- hash table full 
	    error_box("exmon hash table is full"); */
	    return -1;
	} else return p;
}


static int hash (char *key)
{
    int sum = 0;

    while ( *key )
	sum += *key++;

    return ( sum % HASHTABSIZE );
}


static int delta (char *key)
{
    int len;

    len = strlen(key);
    if ( len < 2 ) len = 0;
    else           len = len - 2;

    return ( *(key+len) % (HASHTABSIZE-1) + 1 );
}
