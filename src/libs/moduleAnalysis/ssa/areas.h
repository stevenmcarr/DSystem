/* $Id: areas.h,v 3.2 1997/03/11 14:36:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef areas_h
#define areas_h

#define AREA_CHUNK_MAG		(6)
#define AREA_SEG_MAG		AREA_CHUNK_MAG
#define AREA_CHUNK_SIZE		(1 << AREA_CHUNK_MAG)
#define AREA_WHICH_CHUNK(id)	(id >> AREA_CHUNK_MAG)
#define AREA_CHUNK_OFFSET(id)	(id & (AREA_CHUNK_SIZE -1))
#define AREA_SEG_SIZE		(1 << AREA_SEG_MAG)
#define area_addr(area, id) \
    ((Generic *)\
     &((area->chunks[AREA_WHICH_CHUNK(id)])[AREA_CHUNK_OFFSET(id) \
					    * area->elemSize] ))
/*
 *  Typedef for init functions functions
 */
typedef FUNCTION_POINTER(void, AREA_INIT_FN, (Generic *handle));

typedef Generic AreaId;

typedef struct Area_struct_type {
    AREA_INIT_FN	initFn;		/* fn to initialize an element */
    AreaId		freeList;	/* list of ids new'd and freed */
    int			highMk;		/* number of ids returned so far */
    int			elemSize;	/* size of an element, in Generics */
    Generic *		chunks[AREA_SEG_SIZE];
} *Area;

#define AREA_NIL_ID	(-1)

EXTERN(Area, area_create,	(int elemSize,
					 AREA_INIT_FN initFn,
					 char * remark) );
    /*
     *  Creates a new area for items of size elemSize (size in bytes).
     */

EXTERN(void, area_destroy,	(Area *areaPtr) );
    /*
     *	Pops the area and any areas pushed under it.
     */

EXTERN(AreaId, area_new,	(Area *areaPtr) );
    /*
     *	Allocates an element in the area and returns the index.
     */

EXTERN(void, area_free,		(Area area, AreaId *areaIdPtr) );
    /*
     *	Frees and clobbers (with an ugly pattern) an element in the area.
     */

EXTERN(int, area_size,		(Area area) );
    /*
     *	Gives the number of elements allocated.
     *	Any valid index i must satisfy
     *		AREA_NIL_ID < i < area_size(area)
     */

#endif
