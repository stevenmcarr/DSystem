/* $Id: manager.h,v 1.3 1997/06/25 14:46:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef manager_h
#define manager_h

#include <libs/support/misc/general.h>
#include <libs/graphicInterface/support/graphics/point.h>

/************************** MANAGER INFORMATION *************************/

typedef FUNCTION_POINTER(void,ManagerStartFunc,(void));
typedef FUNCTION_POINTER(Generic,ManagerCreateFunc,(Generic));
typedef FUNCTION_POINTER(void,ManagerEventFunc,(Generic));
typedef FUNCTION_POINTER(Point,ManagerWindowTileFunc,(Generic,Window*,Generic,Point,Boolean));
typedef FUNCTION_POINTER(void,ManagerDestroyFunc,(Generic));
typedef FUNCTION_POINTER(void,ManagerFinishFunc,(void));



typedef struct	manager {		/* MANAGER DEFINITION		*/

char		*name;			/* the name of the manager	*/

short		ref_count;		/* the number of users		*/

ManagerStartFunc start;		/* start the manager		*/
/* Takes no parameters.  Allows the creation of information global	*/
/* to all instances of a manager.  Called before other manager calls.	*/

ManagerCreateFunc	create;		/* create a manager instance	*/
/* Takes one parameter:  (Generic manager_id) the manager index to use	*/
/* in callbacks.  The manager instance should be created and a handle to*/
/* the manager should be returned.  Note:  only managers earlier in the	*/
/* declaration table will have been created at this time.		*/

ManagerEventFunc	event;		/* handle an event		*/
/* Takes one parameter:  (Generic manager_handle) the handle to the	*/
/* manager which should handle the current mon_event.			*/

ManagerWindowTileFunc	window_tile;
/* Takes five parameters:  (Generic manager_handle) the handle to the	*/
/* manager requesting the (re)tile, (Window *w) the window to be (re)	*/
/* tiled, (Genric info) the manager specific window information field,	*/
/* (Point ulc) the position in the window to begin tiling, and		*/
/* (Boolean new) whether or not this is a new window.  The tiler should */
/* return the size of the tiling.					*/

ManagerDestroyFunc	destroy;		/* destroy the item information */
/* Takes one parameter:  (Generic manager_handle) handle to the manager.*/
/* Information specific to the instance should be destroyed.  Note:	*/
/* managers later in the declaration table will have been destroyed by	*/
/* the time this is called.						*/

ManagerFinishFunc  finish;		/* finish the manager		*/
/* Takes no parameters.  Allows the destruction of information global	*/
/* to all instances of the manager.  Called after other manager calls.	*/

			} aManager;

#endif
