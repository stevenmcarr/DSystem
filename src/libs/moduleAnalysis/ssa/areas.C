/* $Id: areas.C,v 1.1 1997/06/25 15:10:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <memory.h>
#include <libs/support/misc/general.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/moduleAnalysis/ssa/areas.h>

#define AREA_ZAP		(0x0F)
#define AREA_SEG_TOTAL		(AREA_CHUNK_SIZE * AREA_SEG_SIZE)
#define AREA_WHICH_SEG(id)	(id >> (AREA_CHUNK_MAG + AREA_SEG_MAG))
#define AREA_SEG_OFFSET(id)	(id & (AREA_SEG_TOTAL -1))


Area area_create(int size, AREA_INIT_FN init, char *remark)
    /* int size;	    size in bytes of an element */
    /* AREA_INIT_FN init;   Function to initialize an element */
    /* char *remark;        string to tag allocation with */
{
    Area a;
    int i;

    a = (Area) get_mem (sizeof(struct Area_struct_type), remark);

    a->initFn	= init;
    a->highMk	= 0;		/* nothing allocated yet */
    a->freeList = AREA_NIL_ID;
    a->elemSize = (size + sizeof(Generic) -1) / sizeof(Generic); /* round up */

    for (i = 0; i < AREA_SEG_SIZE; i++) a->chunks[i] = (Generic *) 0;

    return a;
}

void area_destroy(Area *ap)
{
    Area a = *ap;
    int lastChunk = AREA_WHICH_CHUNK(a->highMk -1);
    int i;

    for (i = 0; i <= lastChunk; i++) free_mem((void*) (a->chunks)[i]);

    free_mem((void*) a);
    *ap = (Area) 0;
}

AreaId area_new(Area *ap)
{
    Area a = *ap;
    AreaId id;
    Generic *tp;
    int i;

    if (a->freeList != AREA_NIL_ID) {
	id = a->freeList;
	tp = area_addr(a, id);
	a->freeList = (AreaId) *tp;
	if (a->initFn) (a->initFn)(tp);
	return id;
    }

    /*
     *  Will return the old count, one more than the previous last valid
     *  index (and postincrement so the new count is again one more than
     *  the new highest valid index).
     */
    id = a->highMk++;

    /*
     *  Need a new chunk when the index is a multiple of the chunk size
     */
    if (!AREA_CHUNK_OFFSET(id))
    {
	if (!AREA_SEG_OFFSET(id))
	{
	    /*
	     *  Need a new segment -- i.e., space for a bunch of chunk
	     *  addresses -- wehn the index is a multiple of the segment
	     *  size.  Actually, don't really need this the first time,
	     *  when (id == 0), but doesn't hurt and makes the code more
	     *  consistent.
	     *
	     *  One table segment is already included in Area_struct_type,
	     *  so we only need to count space for AREA_WHICH_SEG(id) 
	     *  segments instead of that number plus one.
	     */
	    a = *ap = (Area) reget_mem ((void*) a,
					sizeof(struct Area_struct_type)
					+ (AREA_WHICH_SEG(id) 
					   * AREA_SEG_SIZE
					   * sizeof(Generic *)),
					"Growing area table");
	    for (i = AREA_WHICH_CHUNK(id);
		 i < (AREA_WHICH_CHUNK(id) + AREA_SEG_SIZE);
		 i++)
	    {
		a->chunks[i] = (Generic *) 0;
	    }
	}
	a->chunks[AREA_WHICH_CHUNK(id)] =
	    (Generic *)
		get_mem(a->elemSize * (AREA_CHUNK_SIZE * sizeof(Generic)),
			"Getting area chunk");
    }

    if (a->initFn)
    {
	tp = area_addr(a, id);
	(a->initFn)(tp);
    }
    return id;
}

void area_free(Area a, AreaId *ip)
    /* AreaId *ip;		pointer to index of element to get rid of */
{
    Generic *temp = area_addr(a, *ip);

    memset(temp, AREA_ZAP, a->elemSize * sizeof(Generic));

    *temp = (Generic) a->freeList;
    a->freeList = *ip;
    *ip = AREA_NIL_ID;
}

int area_size(Area a)
{
    return a->highMk;
}
