/* $Id: bstring.h,v 1.6 1999/06/11 15:03:42 carr Exp $ */
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

#ifdef _BSD

#ifndef OSF1
EXTERN(void, bcopy, (const void* src_addr, void* dest_addr, int nbytes));
EXTERN(void, bzero, (void* src_addr, int nbytes));
EXTERN(int, bcmp, (const void* src_addr, const void* dest_addr, int nbytes));
EXTERN(int,atoi,(const char *str));
#endif

#else

#include <string.h>

#define bcopy(src_addr, dest_addr, nbytes)  \
	memmove((void*)(dest_addr), (const void*)(src_addr), (nbytes))

#define bzero(src_addr, nbytes) \
	memset((void*)(src_addr), 0, (nbytes))

#define bcmp(src1_addr, src2_addr, nbytes) \
	memcmp((void*)(src1_addr), (const void*)(src2_addr), (nbytes))

EXTERN(int,atoi,(const char *str));

#endif /* BSD */

#endif /* bstring_h */
