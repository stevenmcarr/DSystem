/* $Id: event_codes.h,v 1.4 1997/03/11 14:33:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*			       event_codes.h				*/
/*			   Standard event codes				*/
/*									*/
/************************************************************************/

#ifndef event_codes_h
#define event_codes_h

	/*** LOW LEVEL (MOUSE) EVENT CODES ***/

#define		MOUSE_MOVE	0	/* movement with button up	*/
/* info.x is unused.  info.y is 1 iff the SHIFT key is down and 0 	*/
/* otherwise.  from and msg are unused.					*/

#define		MOUSE_DRAG	1	/* movement with button down	*/
/* info.x is the mouse button held down.  info.y is 1 iff the SHIFT key	*/
/* is down and 0 otherwise.  from and msg are unused.			*/

#define		MOUSE_UP	2	/* mouse button release		*/
/* info.x is the button number.  info.y is 1 iff the SHIFT key is down	*/
/* and 0 otherwise.  from and msg are unused.				*/

#define		MOUSE_DOWN	3	/* down click on mouse		*/
/* info.x is the button number.  info.y is 1 iff the SHIFT key is down	*/
/* and 0 otherwise.  from and msg are unused.				*/

#define		EVENT_KEYBOARD	4	/* keyboard input		*/
#define		MOUSE_KEYBOARD	4	/* keyboard input		*/
/* from is the pane of the keyboard event.  info.x is the KbChar.	*/
/* msg points to the character string initiating the KbChar.		*/

#define		MOUSE_EXIT	5	/* mouse exits the window	*/
/* info, and msg are unused.  from is the pane of the mouse event.	*/


	/* HIGH LEVEL EVENT CODES */

#define		EVENT_SELECT	6	/* selection			*/
/* from is the pane of the select event.  info and msg depend on the	*/
/* generating pane.							*/

#define		EVENT_MOVE	7	/* move request			*/
/* from is the pane of the move event.  info and msg depend on the	*/
/* generating pane.							*/

#define		EVENT_HELP	8	/* help requested		*/
/* from is the pane of the help event.  info and msg depend on the	*/
/* generating pane.							*/

#define		EVENT_NULL	9	/* null request			*/
/* from is the pane of the null event.  info and msg depend on the	*/
/* generating pane.							*/

#define		EVENT_RESIZE	10	/* resize request		*/
/* from is the originator of the resize.  info is the requested size.	*/
/* msg is unused.							*/

#define		EVENT_KILL	11	/* death request		*/
/* from is the pane of the kill event.  info and msg depend on the	*/
/* generating pane.							*/

#define		EVENT_IO	12	/* asynchronous file input event*/
/* from is the file descriptor.  info.x is the number of bytes (buffered*/
/* only).  info.y is unused.  msg is the pointer to the buffer of	*/
/* characters (buffered	only).						*/

#define		EVENT_SIGCHLD	13	/* child process changes state	*/
/* from is the process id.  info is unused.  msg points to a		*/
/* 'union wait' containing the status of the child.			*/

#define		EVENT_MESSAGE	14	/* message event		*/
/* from is the originator of the message.  info and msg depend on the	*/
/* originator.								*/

#endif
