/* $Id: bstring.h,v 1.3 1997/03/11 14:27:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
** definitions of BSD utility routines bzero, bcopy, and bcmp
** in terms of their System V equivalents
**
** Author: John Mellor-Crummey
*/

#ifndef bstring_h
#define bstring_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifdef BSD

EXTERN(void, bcopy, (const void* src_addr, void* dest_addr, int nbytes));
EXTERN(void, bzero, (void* src_addr, int nbytes));
EXTERN(int, bcmp, (const void* src_addr, const void* dest_addr, int nbytes));

#else

#include <string.h>

#define bcopy(src_addr, dest_addr, nbytes)  \
	memmove((void*)(dest_addr), (const void*)(src_addr), (nbytes))

#define bzero(src_addr, nbytes) \
	memset((void*)(src_addr), 0, (nbytes))

#define bcmp(src1_addr, src2_addr, nbytes) \
	memcmp((void*)(src1_addr), (const void*)(src2_addr), (nbytes))

#endif /* BSD */

#endif /* bstring_h */
