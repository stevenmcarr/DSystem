/* $Id: bstring.h,v 1.1 1999/09/08 17:45:14 carr Exp $ */
/*
** definitions of BSD utility routines bzero, bcopy, and bcmp
** in terms of their System V equivalents
**
** Author: John Mellor-Crummey
** Hacked by Mark Anderson for Pablo
*/

#ifndef bstring_h
#define bstring_h

#ifdef BSD

#include <string.h>

#else

#define bcopy(src_addr, dest_addr, nbytes)  \
	memmove((void*)(dest_addr), (const void*)(src_addr), (nbytes))

#define bzero(src_addr, nbytes) \
	memset((void*)(src_addr), 0, (nbytes))

#define bcmp(src1_addr, src2_addr, nbytes) \
	memcmp((void*)(src1_addr), (const void*)(src2_addr), (nbytes))

#endif /* BSD */

#endif /* bstring_h */
