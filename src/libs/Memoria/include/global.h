/*
 *	global.h
 *
 *	supposed to be included by everything
 */

#ifndef global_h
#define global_h

#define INLINE
#undef INLINE

#define get_atom(ar,n,x) ar->arena_alloc_mem(n,x)
#define free_mem(x)	{free(x); x=NULL;}


#include <stdio.h>

typedef char Bool;
#define tr 1
#define fa 0


#include <set.h>

#endif
