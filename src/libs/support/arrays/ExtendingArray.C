/* $Id: ExtendingArray.C,v 1.2 1997/06/26 17:23:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <memory.h>
#include <stdio.h>

#include <include/bstring.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/arrays/ExtendingArray.h>

#define ZAP_CHAR (0x0F)

/*
 *  Next three subroutines do extendable arrays whose elements
 *  have size a multiple of sizeof(Generic)
 *	ptr = (type *)f_alloc ( n, sizeof(type), "tag string" );
 *	f_free(ptr);
 *	f_reset(ptr);
 *	f_clear(ptr);
 *	index = f_new(&ptr);
 *	f_dispose(ptr, index);
 *	f_address(ptr, index) -- only used here
 *	f_curr_size(ptr)
 *	f_unused(ptr, index)
 *
 * We hide the initialization function pointer in
 * 	((Generic *) ptr)[-5]
 *
 * We hide the element size in bytes in
 * 	((Generic *) ptr)[-4]
 *
 * We hide the number of elements in the array in
 *	((Generic *) ptr)[-3]
 *
 * We hide the high-water mark (first never-allocated elt) in
 *	((Generic *) ptr)[-2]
 * 
 * We hide the first element of the free list in
 *	((Generic *) ptr)[-1]
 */

typedef FUNCTION_POINTER(void, InitFunct_functPtr, (char*));

#define F_FIELDS	5
#define INIT_FUNC(ptr)	(InitFunct_functPtr)(((Generic*)ptr)[-5])
#define HIGH_WATER(ptr)	(((Generic*)ptr)[-4])
#define FREE_LIST(ptr)	(((Generic*)ptr)[-3])
#define ELT_SIZE(ptr)	(((Generic*)ptr)[-2])
#define ARRAY_SIZE(ptr)	(((Generic*)ptr)[-1])
#define TRUE_BASE(ptr)	(&(((Generic*)ptr)[-F_FIELDS]))
#define F_NIL		-1
#define f_address(ptr,id)	((Generic)&(((char *)ptr)[(id)*ELT_SIZE(ptr)]))

    /* n    number of elements to allocate initially */
    /* size size in bytes of an element of the array */
    /* tag  string to tag allocation with */
    /* init Function to initialize an element */

Generic f_alloc(int n, int size, char* tag, f_init_callback init)
{
    Generic *t;	/* a temporary */

    t = (Generic *) get_mem ( n*size + F_FIELDS*sizeof(Generic) , tag);

    *t++ = (Generic) init;/* INIT_FUNC */
    *t++ = 0;		/* HIGH_WATER */
    *t++ = F_NIL;	/* FREE_LIST */
    *t++ = size;	/* ELT_SIZE */
    *t++ = n;		/* ARRAY_SIZE */
    memset((char *)t, ZAP_CHAR, n*size);

    /* 
     *  Right now,
     *		t[-5] is a pointer to the element initialization function
     *		t[-4] is the high-water mark (0 -- first never-allocated elt)
     *		t[-3] is the free_list (F_NIL)
     *		t[-2] is the element size in ints
     *		t[-1] is the array size in elements
     */
    return (Generic) t;
}

void f_free(Generic f)
{
    free_mem ((void*)TRUE_BASE(f));
}

void f_reset(Generic f)
{
    HIGH_WATER(f) = 0;
    FREE_LIST(f)  = F_NIL;
}

void f_clear(Generic f)
{
    memset((char *)f, ZAP_CHAR, ARRAY_SIZE(f) * ELT_SIZE(f));
    HIGH_WATER(f) = 0;
    FREE_LIST(f)  = F_NIL;
}

    /* f - array pointer void **f */
int f_new(Generic* f)
{
    Generic id;
    char *ctemp;
    Generic  *itemp;
    if (FREE_LIST(*f) != F_NIL) {
	id = FREE_LIST(*f);
	ctemp = (char *) f_address(*f,id);
	bcopy(ctemp, (char *)&(FREE_LIST(*f)), sizeof(Generic));
	if (INIT_FUNC(*f)) (INIT_FUNC(*f))(ctemp);
	return id;
    }

    if (HIGH_WATER(*f) == ARRAY_SIZE(*f)) {
	int osize = ARRAY_SIZE(*f)*ELT_SIZE(*f);

	/* 
	 *  No empty elements, so re-allocate a bigger array
	 */
	ARRAY_SIZE(*f) *= 2;

	itemp = (Generic *) reget_mem((void*)TRUE_BASE(*f),
				2*osize + F_FIELDS*sizeof(Generic), "f_new");

	*f = (Generic) (itemp + F_FIELDS);	/* get past size etc. info */

	/*
	 *  osize indexes first char in new space, 
	 *  and is also equal to the number of chars in new space
	 */
	memset(((char *)(*f)) + osize, ZAP_CHAR, osize);
    }

    id = HIGH_WATER(*f)++;
    if (INIT_FUNC(*f)) {
	ctemp = (char *) f_address(*f,id);
	(INIT_FUNC(*f))(ctemp);
    }
    return id;
}

    /* f  - array pointer */
    /* id - index of element to get rid of */
void f_dispose(Generic f, int id)
{
    char *temp = (char *) f_address(f,id);

    memset(temp, ZAP_CHAR, ELT_SIZE(f));

    bcopy((const char *)&(FREE_LIST(f)), temp, sizeof(Generic));
    FREE_LIST(f) = id;
}

int f_curr_size(Generic f)
{
    return HIGH_WATER(f);
}

    /* f  - array pointer */
    /* id - index of element check */
Boolean f_unused(Generic f, int id)
{
    Generic freeList = FREE_LIST(f);

    if ((id < 0) || (id > HIGH_WATER(f)))
	return true;

    while (freeList != F_NIL )
    {
        if (freeList == id)
            return(true);

	bcopy((const char*)f_address(f, freeList), (char*)&freeList, sizeof(Generic));
    }
    return(false);
}
