/* $Id: list_sm.h,v 1.7 1997/03/11 14:33:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		       list_sm.h			*/
		/*            List screen module include file.		*/
		/* 							*/
		/********************************************************/

/* List display screen module allows automatic display of a list.  Horizontal and	*/
/* vertical shifting can be done automatically or through client supervision.  Lines may*/
/* be specified as unselectable/selectable, or selected/deselected.  All events are	*/
/* relative to the list--not the display.  Works through a single user provided		*/
/* callback which lazily evaulates the list (no line is asked for more than once).	*/

/* Returned events are as follows:							*/
/* EVENT_SELECT:  info is the *list* coordinate of the selection, (char, line).  This	*/
/* 		  event is not returned for non-selectable lines or lines off the list.	*/
/* 		  msg  is the id of the line selelected.				*/
/* EVENT_HELP:    info is the *list* coordinate of the click (char, line) or (UNUSED,	*/
/*		  UNUSED) for off the list.  msg is the id of the line selected or	*/
/* 		  UNUSED.								*/

#ifndef list_sm_h
#define list_sm_h

#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>

EXTERN(short, sm_list_get_index, (void));
/* get the list_sm index number		*/
/* Takes no parameters.  Returns the index of the list display screen module.		*/

EXTERN(Point, sm_list_pane_size, (Point size, short font, Boolean hscroll,
 Boolean vscroll));
/* get the minimum pane size for a block*/
/* Takes four parameters:  (Point size) the size of the character block, (short font)	*/
/* the font in which it will be displayed, (Boolean hscroll, vscroll) whether the size	*/
/* is to include a horizontal and/or a vertical scrollbar.  Returns the minimum pane	*/
/*  size necessary to display the block.						*/
#define	LSM_H_SCROLLBAR		true		/* horizontal scrollbar should exist	*/
#define	LSM_NO_H_SCROLLBAR	false		/* horizontal scrollbar shouldn't exist	*/
#define	LSM_V_SCROLLBAR		true		/* vertical scrollbar should exist	*/
#define	LSM_NO_V_SCROLLBAR	false		/* vertical scrollbar shouldn't exist	*/

EXTERN(Point, sm_list_min_block, (short font));
/* the minimum suggested character size	*/
/* Takes one parameter:  (short font) the font to use.  Returns the minimum number of	*/
/* characters (x and y directions) that are required to have scrollbars show.  Note:	*/
/* if the pane is made too small, requested scrollbars will not be shown.		*/

typedef	struct		{			/* LINE INFORMATION RETURN STRUCTURE	*/
	char		*text;			/* text of the line (null terminated)	*/
	Boolean		should_free;		/* the text should be freed by lsm	*/
	short		len;			/* the length of the line		*/
						/* if UNUSED, the length will be set	*/
	Generic		id;			/* identifier associated with the line	*/
	Boolean		selected;		/* true if the line is selected		*/
	Boolean		selectable;		/* true if the line is selectable	*/
			} lsm_line_info;


typedef FUNCTION_POINTER(lsm_line_info *, lsm_generator_callback, (Pane *p, Generic client, Boolean val, Generic id));
/* user defined list generator callback	*/
/* Takes four parameters (Pane *p) the list display pane making the request,		*/
/* (Generic client) the client handle for this list, (Boolean val) LSM_REQ_FIRST if the	*/
/* first line is being requested and LSM_REQ_NEXT if the next line is being requested,	*/
/* and (Generic id) the id of the previous line (val == LSM_REQ_FIRST) and undefined	*/
/* otherwise.  Returns a pointer to an allocated line information structure for the line*/
/* of the request.  If the line requested does not exist, 0 should be returned.  Note: 	*/
/* the id is user defined and it is meant to assist in walking linked lists or arrays.	*/
#define	LSM_REQ_FIRST	true			/* return the first line of the list	*/
#define	LSM_REQ_NEXT	false			/* return the next line of the list	*/

typedef FUNCTION_POINTER(void, lsm_shift_callback, (Pane *p, Generic client, Point shift));
/* user defined shift routine		*/
/* Takes three parameters (Pane *p) the list display making the request, (Generic	*/
/* client) the client handle for the list, and (Point shift) the relative amount to	*/
/* shift the list.  This routine may call sm_list_shift().				*/


EXTERN(void, sm_list_initialize, (Pane *p, Generic client,
 lsm_generator_callback generator, short font_id,
 lsm_shift_callback shiftback, Boolean hscroll, Boolean vscroll));
/* set the status of the list pane	*/
/* Takes seven parameters (Pane *p) the list pane, (Generic client) the client handle	*/
/* for callbacks, (lsm_generator_callback generator) the line generator callback (see	*/
/* above), (short font_id) the font of the list, (lsm_shift_callback shiftback) the	*/
/* shifting callback (see above) or LSM_SHIFT_AUTO if shifting is to be done		*/
/* automatically, and (Boolean hscroll, vscroll) for specifying whether there should be	*/
/* horizontal or vertical scrollbars (use LSM_H_SCROLLBAR, etc. from above).  Since	*/
/* this routine does not redisplay the list the list modified call should be made soon	*/
/* after this one.									*/
#define	LSM_SHIFT_AUTO		((lsm_shift_callback) 0)/* shift automatically		*/

EXTERN(Point, sm_list_map_size, (Pane *p));
/* return the number of displayable char*/
/* Takes one parameter (Pane *p) the list pane.  Returns the number of characters	*/
/* that can be displayed in either direction.  Note:  this proceedure may be called	*/
/* after the list pane has been initialized.						*/

EXTERN(Point, sm_list_modified, (Pane *p, Point data_shift, short count));
/* notify of change in the list		*/
/* Takes three parameters (Pane *p) the list pane, (Point data_shift) the zero based	*/
/* (column number, line number) of the (0, 0) position of the screen (use the old shift	*/
/* if (UNUSED, UNUSED)), and (short num) the number of lines in the new list or UNUSED	*/
/* if the number of lines is not known (this defeats lazy evaluation).  Returns the	*/
/* shift value actually used.  Note:  the list screen module doesn't allow a list to be	*/
/* shifted off the screen.								*/

EXTERN(void, sm_list_line_modified, (Pane *p, Generic id));
/* a line's information has changed	*/
/* Takes two parameters (Pane *p) the list pane and (Generic id) the id of the line	*/
/* whose information has changed.  It is assumed that all other lines are the same.  The*/
/* line is redisplayed if necessary.							*/

EXTERN(void, sm_list_line_change, (Pane *p, Generic id, Boolean selected,
 Boolean selectable));
/* set the select__ attributes of a line*/
/* Takes four parameters (Pane *p) the list pane, (Generic id) the id of the line to	*/
/* change, (Boolean selected) true if the line is to be selected, and (Boolean		*/
/* selectable) true if the line is selectable.  It is assumed that all other lines are	*/
/* the same.  The line is redisplayed if necessary.					*/

EXTERN(Point, sm_list_line_insert, (Pane *p, lsm_line_info *line, short dir,
 Generic id));
/* insert a new line into a list	*/
/* Takes four parameters (Pane *p) the list pane, (lsm_line_info *line) the (allocated)	*/
/* line info, (short dir) the insertion code (see below), and (Generic id) the mentioned*/
/* line id (only used for insert after and insert before).  The new shift of the list is*/
/* returned.  The user provided handler is not called during this call, but it should 	*/
/* be able to handle later requests with this line added.  Note that 			*/
#define	LSM_INS_FIRST	0			/* make the line be the new first line	*/
#define	LSM_INS_LAST	1			/* make the line be the new last line	*/
#define LSM_INS_BEFORE	2			/* make the line be before id		*/
#define	LSM_INS_AFTER	3			/* make the line be after id		*/

EXTERN(Point, sm_list_line_delete, (Pane *p, Generic id));
/* delete a line in the list		*/
/* Takes two parameters (Pane *p) the list pane and (Generic id) the id of the line to	*/
/* delete.  The list is redisplayed accordingly and the new list shift is returned.	*/
/* The list is only shifted if the line is above the screen (the screen is not redrawn),*/
/* deleting the line would make the screen blank.  The shift is left alone otherwise.	*/
/* Note that the user provided handler may have to be called during this routine and it	*/
/* *should know* that the mentioned line has been deleted.				*/

EXTERN(Point, sm_list_shift, (Pane *p, Point amount, Boolean rel));
/* shift the list display		*/
/* Takes three parameters (Pane *p) the list pane, (Point amount) the amount of the	*/
/* shift, and (Boolean rel) true if the shift is to be added to the current shift.	*/
/* Returns the absolute shift amount used.						*/
#define	LSM_SHIFT_REL	true			/* the shift value is relative		*/
#define	LSM_SHIFT_ABS	false			/* the shift value is absolute		*/

EXTERN(short, sm_list_line_distance, (Pane *p, Generic id));
/* figure minimum shift distance to show*/
/* Takes two parameters (Pane *p) the list pane, (Generic id) the id of the line of	*/
/* interest.  Returns the y value of the relative shift necessary to display the line.	*/
/* The mentioned line must be in the list.						*/

EXTERN(Point, sm_list_line_show, (Pane *p, Generic id, Boolean is_new,
 short count));
/* show a line of the display		*/
/* Takes four parameters (Pane *p) the list pane, (Generic id) the id of the line to	*/
/* show, (Boolean is_new) true if the list has been modified, and (short count) if the	*/
/* list had been modified then count will indicate the number of entries in the new	*/
/* list.  If the number is unknown, UNUSED may be specified with a loss of lazy		*/
/* evaluation.  The new shift of the display is returned.  The mentioned line will be	*/
/* centered and shifted to x=0 if the line was off the display.  If the line was on the	*/
/* display, the display will just be shifted to x=0.  The mentioned line must be in the	*/
/* list.										*/

#endif
