/* $Id: ExtensibleArray.C,v 1.1 1997/06/25 15:13:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/support/memMgmt/mem.h>
#include <libs/support/arrays/ExtensibleArray.h>

/*
Next three subroutines do extendable arrays whose elements
are of size >= sizeof(int)
	ptr = (type *)xalloc ( n, sizeof(type) );
	ptr = (type *)xrealloc( x, n );
	i = ptr[j];
	ptr = xput(ptr,&i,j);
	xfree(ptr);

We hide the element size in bytes in
	((int *)ptr)[-2]

We hide the number of elements current allocated in 
	((int *)ptr)[-1]
*/

#define ELT_SIZE(ptr)	(((int *)ptr)[-2])
#define ARRAY_SIZE(ptr)	(((int *)ptr)[-1])
#define TRUE_BASE(ptr)	(&(((int *)ptr)[-2]))

int* xalloc(int n, int size)
{
	int *t;			/* a temporary */

	t = (int *) get_mem ( n*size + 2*sizeof(int) ,"xalloc");

	*t++ = size;
	*t++ = n;

	/* Right now,
		t[-2] is the element size in bytes
		t[-1] is the array size in elements
	*/
	return t;
}

int* xrealloc(int *x, int n)
{
	int *t;			/* a temporary */

	t = (int *) reget_mem ((void*) TRUE_BASE(x),
		n*ELT_SIZE(x) + 2*sizeof(int),
		"xrealloc");

	 t++;		/* Skip the element size field */
	*t++ = n;	/* Record the new array size */

	/* Right now,
		t[-2] is the element size in bytes
		t[-1] is the array size in elements
	*/
	return t;
}

void xfree(int *x)
{
	free_mem ((void*) TRUE_BASE(x));
}

int *xput(int *x, int n, char *elt)
{
	register int i;			/* loop variable */
	register int size = ELT_SIZE(x);/* size in bytes of an element of this array */
	register int o = ARRAY_SIZE(x);	/* current number of elements in this array */
	register char *telt;		/* pointer for copying new element into array position n */

	/* is the reference within current array bounds? */
	if ( n >= o)
		/* Get a bigger array */
		x = xrealloc(x,2*n+1);

	/* Put element into array */
	telt = (char *)x;
	telt += size * n;

	for (i = 0;i < size;i++)
	{
		telt[i] = elt[i];
	}

	return x;
}

int *xget(int *x, int n, char *elt)
{
	register int i;			/* loop variable */
	register int size = ELT_SIZE(x);/* size in bytes of an element of this array */
	register int o = ARRAY_SIZE(x);	/* current number of elements in this array */
	register char *telt;		/* pointer for copying new element into array position n */

	/* is the reference within current array bounds? */
	if ( n >= o)
		/* Get a bigger array */
		x = xrealloc(x,2*n+1);

	/* Get element from array */
	telt = (char *) x;
	telt += size * n;

	for (i = 0;i < size;i++)
	{
		elt[i] = telt[i];
	}

	return x;
}
