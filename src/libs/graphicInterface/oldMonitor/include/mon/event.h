/* $Id: event.h,v 1.7 1997/06/25 14:46:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 			event.h				*/
		/* 	       event calls and information		*/
		/* 		     (root/event.c)			*/
		/********************************************************/

#ifndef event_h
#define event_h

typedef struct	{				/* STANDARD EVENT STRUCTURE		*/
	short		type;			/* the event code			*/
	Generic		to;			/* who the event is to			*/
	Generic		from;			/* who created the returned event	*/
	Point		info;			/* the current info (sm defined)	*/
	Generic		msg;			/* the current message			*/
	Point		loc;			/* the pane position of the mouse cursor*/
	Point		offset;			/* the current pane position offset	*/
		} anEvent;

	/* Button identification */
#define BUTTON_SELECT	1			/* left button				*/
#define	BUTTON_MOVE	2			/* middle button			*/
#define BUTTON_HELP	3			/* right button				*/

EXTERN(Boolean, readyEvent, (void));
/* Takes no parameters.  Returns true if there are mouse, file, or child events.	*/

EXTERN(void, getEvent, (void));
/* Takes no parameters.  Loads global variable mon_event.				*/

EXTERN(void, ungetEvent, (void));
/* Takes no parameters.  Makes next getEvent() have same value.				*/

EXTERN(void, flushEvents, (void));
/* Takes no parameters.  Flushes all pending keyboard and mouse events.			*/

EXTERN(void, holdNonUrgentEvents, (Boolean flag));
/* Takes one parameter (Boolean flag) which is true if the restriction counter is to be	*/
/* is to be incremented and false if the restriction counter is to be decremented.	*/
/* Inhibits non-urgent events from being returned from getEvent().			*/

EXTERN(void, giveNullEvent, (void));
/* Takes no parameters.  Sets flag to give an EVENT_NULL before blocking.		*/

EXTERN(void, registerAsyncIO, (short fd, Generic owner, Boolean buffer, Boolean urgent));
/* Takes three parameters (short fd) the file descriptor to begin polling for		*/
/* asynchronous input, (Generic id) the identification associated with the file		*/
/* descriptor, and (Boolean urgent) true if IO should not be held.  All registered	*/
/* file descriptors must be unregistered.						*/

EXTERN(void, unregisterAsyncIO, (short fd, Generic owner));
/* Takes one parameter (short fd) the file descriptor which is no longer of interest.	*/

EXTERN(void, registerChildProcess, (int pid, Generic owner, Boolean urgent));
/* Takes three parameters (int pid) the process id of interest, (Generic id) the	*/
/* identification associated with the file descriptor, and (Boolean urgent) true if	*/
/* events should not be held.  All registered processes must be unregistered.		*/

EXTERN(void, unregisterChildProcess, (int pid, Generic owner));
/* Takes two parameters (int pid) the process id which is no longer of interest and     */
/* (Generic owner) the identification associated with the file descriptor.		*/

extern	anEvent		mon_event;		/* the current input event		*/

EXTERN(void,add_select_mask,(int fd));
EXTERN(void,remove_select_mask,(int fd));
EXTERN(void,startEvents,(void));
EXTERN(void,stopEvents,(void));

#endif
