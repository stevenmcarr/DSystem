/* $Id: port.h,v 1.16 2003/02/28 22:26:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef port_h
#define port_h

/*
 * Typedefs for portable compiling
 *
 * Author: Cliff Click, Copyright (C) 1991, 1992
 * This material is released for public use, but is not placed in the public
 * domain.  You are free to use/mangle/copy this code, but since I retain
 * ownership you cannot copyright or patent this code.	Please leave this
 * copyright notice in copies of this code, so that future programmers cannot
 * be denied using it.
 */

#ifdef __GNUC__

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#define touch(x)
#define huge
/*
extern "C" void bzero(void *b, int len);
extern "C" void bcopy(const void *s, void *d, int len);
extern "C" int bcmp(const void *s, const void *t, int len);
*/
#include <include/bstring.h>
//class ostream;
//ostream &operator << (ostream &os, void *ptr);

#elif defined  __TURBOC__

#define volatile
#define touch(x) ((x)=(x));
#include <libs/support/memMgmt/mem.h>
// #define bcopy(s,d,l) memmove(d,s,l)
// #define bzero(p,l) memset(p,0,l)
// #define bcmp(s,d,l) memcmp(s,d,l)
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MAX(a,b) (((a) < (b)) ? (a) : (b))
inline long abs( long x ) { return x < 0 ? -x : x; }

#elif defined __cplusplus
// AT&T's cfront

#define volatile
#define huge

#ifndef SOLARIS
#define signed
#endif

#define touch(x)
#include <include/bstring.h>
#ifdef general_h
#  ifndef min
      inline int min(int a, int b) { return a < b ? a : b; }
#  endif
#  ifndef max
      inline int max(int a, int b) { return a > b ? a : b; }
#  endif
#endif

#else 

#define volatile
#define huge

#ifndef SOLARIS
#define signed
#endif

#define touch(x)
/*extern "C" void bcopy(void *b1, void *b2, int len);*/
EXTERN(void, bcopy, (void *b1, void *b2, int len));

#endif

typedef signed char int8;
typedef unsigned char uint8;
typedef unsigned char byte;
typedef signed short int16;   /* Exactly 16bits signed */
typedef unsigned short uint16;
typedef unsigned int uint;    /* When you need a fast >=16bit unsigned value */
/*typedef int int; */	      /* When you need a fast >=16bit value */
typedef signed long int32;    /* Exactly 32bits signed */
typedef unsigned long uint32;

#define MININT32 (1L<<(32-1))
#define MAXINT32 (~(1L<<(32-1)))
int32 gcd( int32 x, int32 y );


#endif
