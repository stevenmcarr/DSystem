/* $Id: libmem.h,v 1.6 1997/03/11 14:36:53 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * Heap Memory Management
 *
 * Author: Cliff Click, Copyright (C) 1991
 * This material is released for public use, but is not placed in the public
 * domain.  You are free to use/mangle/copy this code, but since I retain
 * ownership you cannot copyright or patent this code.	Please leave this
 * copyright notice in copies of this code, so that future programmers cannot
 * be denied using it.
 */

#ifndef _MEMORY_
#define _MEMORY_

#ifdef __GNUC__

typedef char *malloc_t;
#ifndef stdlib_h
#include <stdlib.h>
#endif	stdlib_h

#elif defined  __TURBOC__

// Alas, I need stdlib to declare malloc/calloc/realloc/free before the macros
// over-ride prior declarations.
#ifndef __STDLIB_H
#include <stdlib.h>
#endif	__STDLIB_H
#ifndef __ALLOC_H
#include <alloc.h>
#endif	__ALLOC_H
#define malloc_t     void*
#define malloc(s)    farmalloc(s)
#define calloc(n,s)  farcalloc(n,s)
#define realloc(p,s) farrealloc(p,s)
#define free(p)      farfree(p)

#elif defined __cplusplus
// AT&T's cfront

#include <stdlib.h>

#else  All other machines

#include <memory.h>

#endif

#define close_heap() (0)
#define heapstats() (0)

#endif _MEMORY_
