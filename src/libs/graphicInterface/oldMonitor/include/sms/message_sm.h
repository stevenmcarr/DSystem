/* $Id: message_sm.h,v 1.5 1997/03/11 14:33:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		      message_sm.h			*/
		/* 	   Message screen module include file.		*/
		/* 							*/
		/********************************************************/

/* This screen module is used for the output of single line messages.  Handles		*/
/* scrolling.  Returns help and selection requests with all fields undefined.		*/

#ifndef message_sm_h
#define message_sm_h

EXTERN(short, sm_message_get_index, (void));
/* gets the message s. m. index number	*/
/* Takes no parameters.  Returns the installed screen module index number for the	*/
/* message screen module.								*/

EXTERN(Point, sm_message_pane_size, (short width, short font_id));
/* pane size needed to display a msg	*/
/* Takes two parameters (short width) the width of a typical message and (short font_id)*/
/* the font that the message is to appear in.  Returns the dimensions (in pixels) of the*/
/* appropriate pane.									*/

EXTERN(void, sm_message_change_font, (Pane *p, short font_id));
/* change the font of the messages	*/
/* Takes two parameters (Pane *p) the message pane and (short font_id) the new font to	*/
/* use.  The default font is DEF_FONT_ID.						*/

EXTERN(void, sm_message_new, (Pane *p, char *format, ...));
/* display a new message		*/
/* Takes a variable number of parameters:  (Pane *p) the message pane, (char *format)	*/
/* the "sprintf" style format string and (?) the "sprintf" parameters necessary to	*/
/* satisfy the format string.  The conversion is done and the message is displayed.	*/

#endif
