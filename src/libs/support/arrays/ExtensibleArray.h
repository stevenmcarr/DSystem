/* $Id: ExtensibleArray.h,v 1.4 1997/03/11 14:36:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/*  xarray.h - Automatically extending arrays */


/*
 * To allocate an extendable array of n elements of size bytes 
 *	ptr = (type *)xalloc ( n, sizeof(type) );
 *
 * To manually change the size of the array to hold n elements
 *	ptr = xrealloc(ptr, n )
 *
 * To retrieve a value from this extendable array
 *	i = ptr[j];
 *
 * To retrieve a value which may be beyond the end of the array
 *	ptr = xget(ptr,&i,j)
 *
 * To set a value
 *	ptr = xput(ptr,&i,j);
 *
 * To free an extendable array 
 *	xfree(ptr) 
 */

#ifndef xarray_h
#define xarray_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

EXTERN(int *, xalloc, (int n, int size));
/*	int n;		Number of elements to initially allocate
 *	int elt_size;	Size of an element of the array
 *
 * Returns a pointer to an extending array.  This pointer may be used like
 * a normal array address when reading and writing values which lie
 * inside the current bounds of the array.  Current bounds range from
 * 0 up to the maximum index that has been accessed via xput()
 * or xget() since the array was allocated, and the amount initially
 * allocated via xalloc().  Also, xrealloc() can be used to
 * increase or reduce the current maximum index of the array.
 * The extending array gotten from this call must be freed with xfree().
 *
 * elt_size probably should be 1, 2, or a multiple of four.  Attempts
 * to pack elements tighter than that (eg. saying that a
 * structure whose fields add up to 3 bytes is a 3 byte element,
 * rather than using sizeof() and getting 4) may result in addressing
 * errors, especially if you use the "i = ptr[j]" syntax shown above.
 * N should of course not be negative...
 */

EXTERN(int *, xrealloc, (int *x, int n));
/*	int *x;		Existing xarray to reallocate.
 *	int n;		Size (ie. # elements) to make x
 *
 * Reallocate the array x.  This routine need not be called to grow the
 * array if xput,xget is used, since they grow the array as needed.
 * This routine may however be useful in reducing the size of the array
 * if it is larger than now needed.
 */

EXTERN(int *, xget, (int *x, int n, char *elt));
/*	int *x;		Xarray from which to get index'th element
 *	int index;	Index of the element to get.
 *	char *elt;	Place to put the element we are getting.
 *
 * Copies the element x[index] into the memory pointed to by elt.
 * Extends the array if index is beyond the current extent of the array.
 * (In this case, the gotten element is of course undefined.)
 */

EXTERN(int *, xput, (int *x, int n, char *elt));
/*	int *x;		Xarray into which to put index'th element
 *	int index;	Index of the element to put.
 *	char *elt;	Pointer to the element we are putting.
 *
 * Copies the element pointed to be elt to the array location x[index].
 * Extends the array if index is beyond the current extent of the array.
 */

EXTERN(void, xfree, (int *x));
/*	int *x;		An existing extending array gotten from xalloc().
 *
 * Frees the extending array x.
 */

#endif
