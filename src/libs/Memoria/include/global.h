/* $Id: global.h,v 1.6 1997/03/20 15:49:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


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
