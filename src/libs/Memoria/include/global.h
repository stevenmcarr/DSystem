/* $Id: global.h,v 1.5 1992/12/11 11:19:45 carr Exp $ */

/*
 *	global.h
 *
 */

#ifndef global_h
#define global_h

#define INLINE
#undef INLINE

#define get_atom(ar,n,x) ar->arena_alloc_mem(n,x)


typedef char Bool;
#define tr 1
#define fa 0


#endif
