/* $Id: help_file.h,v 1.6 1997/03/11 14:30:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/****************************************************************/
		/*			help_file.h				*/
		/* This file is for clients of the help_cp who wish to process	*/
		/* help files.							*/
		/*								*/
		/****************************************************************/

/* The client provides several callbacks to helpcp_file_read.  These callbacks are made	*/
/* as the file is read, according to the structure of the file.  The callbacks are made	*/
/* to define a tree of headings with optional transfers, identifications, positionings,	*/
/* and text.  It is up to the client to "remember" what has been seen.  The first	*/
/* callback is guaranteed to be the handle_heading() callback.  Between it and the next	*/
/* handle_heading() call, any number of other callbacks may be made.  The callbacks are	*/
/* to refer to the node of the last handle_heading() callback.  Text returned in the	*/
/* handle_heading() and handle_text() callbacks may contain special characters.  The	*/
/* PARAGRAPH character may appear in text (from handle_text() only) to delimit		*/
/* paragraphs.  The QUOTE_CHAR signifies that the next character is a non-printing	*/
/* character and should be taken "as is".  Currently, this is only used for special	*/
/* font characters.  A keyboard sequence is surrounded by a BEGIN_KEYBOARD and		*/
/* END_KEYBOARD characters.  The intervening characters do not need translation.	*/
/* "Symbolic" keysequences in the help file are converted to "Actual" keysequences which*/
/* would appear on an actual machine (depending on the value of kb_keyboard_id.		*/

/* The following factors can influence how the helpfile is evaluated:			*/
/* 	argc, argv    --The helpcp_file_read() arguments for the helpfile arguments	*/
/* 	interactive   --The helpcp_file_read() argument for on/off line.		*/
/* 	kb_keyboard_id--Affects the translation of keyboard symbols.			*/
/* 	kb_swap_bs_del--This value states whether the BS and DEL keys have been swapped.*/

#ifndef help_file_h
#define help_file_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#define PARAGRAPH		'\\'		/* paragraph delimiter character	*/
#define	QUOTE_CHAR		''		/* quote character			*/
#define	BEGIN_KEYBOARD		''		/* begin keyboard sequence		*/
#define END_KEYBOARD		''		/* end keyboard sequence		*/

typedef FUNCTION_POINTER(void, helpcp_heading_callback, (Generic handle, char *buffer, short len, short level));
/* begin a new heading			*/
/* Takes four parameters:  (Generic handle) the client handle, (char *buffer) the	*/
/* null-terminated heading text (see above), (short len) the length of the buffer, and	*/
/* (short level) the level of the outline to which the node belongs (root == 0).	*/
/* Guaranteed to be the first callback made (with level = 0).				*/

typedef FUNCTION_POINTER(void, helpcp_transfer_callback, (Generic handle, short id));
/* handle a new transfer id		*/
/* Takes two parameters:  (Generic handle) the client handle and (short id) the new	*/
/* transfer id.  This id is to be associated with the last heading.			*/

typedef FUNCTION_POINTER(void, helpcp_identification_callback, (Generic handle, short id));
/* handle a new identification id	*/
/* Takes two parameters:  (Generic handle) the client handle and (short id) the new	*/
/* identification id.  This id is to be associated with the last heading.		*/

typedef FUNCTION_POINTER(void, helpcp_positioning_callback, (Generic handle, short id));
/* handle a new positioning id		*/
/* Takes two parameters:  (Generic handle) the client handle and (short id) the new	*/
/* positioning id.  This id is to be associated with the last heading.  This call is	*/
/* made at most once per heading.							*/

typedef FUNCTION_POINTER(void, helpcp_text_callback, (Generic handle, char *buffer, short len));
/* handle a block of text for a heading	*/
/* Takes three parameters:  (Generic handle) the client handle, (char *buffer) the	*/
/* null-terminated text associated with the last heading (see above), and (short len) 	*/
/* the length of the buffer.  This call is made at most once per heading.		*/

EXTERN(char *, helpcp_file_read, (char *file_name, short num_args,
 short *arg_list, Boolean interactive, Generic handle,
 PFV handle_heading, PFV handle_transfer, PFV handle_identification, 
 PFV handle_positioning, PFV handle_text));
/* read the contents of a help file	*/
/* Takes ten arguments:  (char *file_name) the name of the file to read--standard paths	*/
/* will be searched, (short num_args) the number of "help arguments", (short *arg_list) */
/* the pointer to the actual argument list, (Boolean interactive) true if the file is   */
/* being read "on-line", (Generic handle) the client supplied handle for use in         */
/* callbacks, and (handle_heading, handle_transfer, handle_identification,              */
/* handle_positioning, handle_text) the client supplied callbacks (or 0) described	*/
/* above.  If the file was successfully read, a (char *) 0 is returned.  If there is an */
/* error, the returned value is a pointer to a string describing the error.  Keep in    */
/* mind that any number of callbacks may have been made when the error is recognized. 	*/

#endif
