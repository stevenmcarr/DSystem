/* $Id: ExtendingArray.h,v 1.9 1997/03/11 14:36:29 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
	/*  f_array.h - Automatically extensible/freeable arrays */

/*
 * To allocate an array of n elements of size bytes 
 *	ptr = (type *) f_alloc ( n, sizeof(type), 
 *				"tag string", f_init_callback init );
 *
 * To access a value from an element
 *	i = ptr[j];
 *
 * To assign a value to an element
 *	ptr[j] = i;
 *
 * To free an extendable array 
 *	f_free(ptr);
 *
 * To reset an f_array to empty without clearing.
 *	f_reset(ptr);
 *
 * To reset and clear an f_array
 *	f_clear(ptr);
 *
 * To get an unused element
 *	index = f_new(&ptr);
 *
 * To free up an element
 *	f_dispose(ptr, index);
 *
 * To check if an element is freed
 *	flag = f_unused(ptr, index);
 *
 * To get number of distinct indices ever returned by f_new for this array
 *	(high-water mark, guaranteed greater than any useful index)
 *	integer = f_curr_size(ptr);
 */

#ifndef f_array_h
#define f_array_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

typedef FUNCTION_POINTER(void, f_init_callback, (void *ptr));
/*	void *ptr;	Pointer to element to initialize
 *
 * Callback function to allow client to initialize new elements.
 */

EXTERN(Generic, f_alloc, (int n, int size, char  *tag,
 f_init_callback init));
/*	int n;		Number of elements to initially allocate
 *	int size;	Size of an element of the array in bytes
 *	char *tag;	Tag for allocation, passed to get_mem
 *	f_init_callback init; Function to initialize an element (or 0)
 *
 * Returns a pointer to an extending array.  This pointer may be used like
 * a normal array address when reading and writing values which lie
 * inside the current bounds of the array.
 * The extending array gotten from this call must be freed with f_free().
 *
 * size should be a multiple of sizeof(int), or at least of
 * the minimal alignment of int.  Other sizes will result in alignment
 * errors as these routines try to store next_free indices in 
 * elements.
 * 
 * N should of course not be negative...
 */

EXTERN(void, f_free, (Generic f));
/*	void *f;	An existing extending array gotten from f_alloc().
 *
 * Frees the extending array f.
 */

EXTERN(void, f_reset, (Generic f));
/*	void *f;	An existing extending array gotten from f_alloc().
 *
 * Resets the extending array f without clearing.  O(1).
 */

EXTERN(void, f_clear, (Generic f));
/*	void *f;	An existing extending array gotten from f_alloc().
 *
 * Resets and clobbers the extending array f.  O(size).
 */

EXTERN(int, f_new, (Generic *f));
/*	void *f;	F_array to get new element in.
 *
 * Gets a new element index.
 * Extends the array if it is full and free list is empty.
 */

EXTERN(void, f_dispose, (Generic f, int id));
/*	void *f;	F_array that element is in.
 *      int  id;	index of element to get rid of
 *
 * Puts element on the free list.
 */

EXTERN(Boolean, f_unused, (Generic f, int i));
/*	void *f;	pointer to an f_array
 *	int i;		index of element
 *	
 *  Returns true if f[i] is out of range or if f[i] is on the free list.
 *  Very inefficient; each call for something in range can take linear time.
 */

EXTERN(int, f_curr_size, (Generic f));
/*	void *f;	pointer to an f_array
 *	
 *  Returns the number of elements ever used.
 *  (Some may have been freed -- essentially the high-water mark.
 *  The number actually in use plus number on free list equals this number.)
 */

#endif
