/* $Id: manager.h,v 1.2 1997/03/11 14:33:18 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef manager_h
#define manager_h

#include <libs/support/misc/general.h>
#include <libs/graphicInterface/support/graphics/point.h>

/************************** MANAGER INFORMATION *************************/


typedef struct	manager {		/* MANAGER DEFINITION		*/

char		*name;			/* the name of the manager	*/

short		ref_count;		/* the number of users		*/

void		(*start)();		/* start the manager		*/
/* Takes no parameters.  Allows the creation of information global	*/
/* to all instances of a manager.  Called before other manager calls.	*/

Generic		(*create)();		/* create a manager instance	*/
/* Takes one parameter:  (Generic manager_id) the manager index to use	*/
/* in callbacks.  The manager instance should be created and a handle to*/
/* the manager should be returned.  Note:  only managers earlier in the	*/
/* declaration table will have been created at this time.		*/

void		(*event)();		/* handle an event		*/
/* Takes one parameter:  (Generic manager_handle) the handle to the	*/
/* manager which should handle the current mon_event.			*/

Point		(*window_tile)();	/* (re)tile a window		*/
/* Takes five parameters:  (Generic manager_handle) the handle to the	*/
/* manager requesting the (re)tile, (Window *w) the window to be (re)	*/
/* tiled, (Genric info) the manager specific window information field,	*/
/* (Point ulc) the position in the window to begin tiling, and		*/
/* (Boolean new) whether or not this is a new window.  The tiler should */
/* return the size of the tiling.					*/

void		(*destroy)();		/* destroy the item information */
/* Takes one parameter:  (Generic manager_handle) handle to the manager.*/
/* Information specific to the instance should be destroyed.  Note:	*/
/* managers later in the declaration table will have been destroyed by	*/
/* the time this is called.						*/

void		(*finish)();		/* finish the manager		*/
/* Takes no parameters.  Allows the destruction of information global	*/
/* to all instances of the manager.  Called after other manager calls.	*/

			} aManager;

#endif
