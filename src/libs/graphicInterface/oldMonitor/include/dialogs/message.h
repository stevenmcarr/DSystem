/* $Id: message.h,v 1.5 1997/03/11 14:33:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*				  message.h				*/
/*			 message dialog include file			*/
/*									*/
/* Messages provide a means of giving the user a piece information which*/
/* he must acknowledge.							*/
/* 									*/
/************************************************************************/

#ifndef dialogs_message_h
#define dialogs_message_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

EXTERN(void, message, (char *format, ...));
/* Takes a variable number of arguments exactly like printf.  The	*/
/* arguments are formatted accordingly and put into a message box.	*/
/* Imbedded newlines are handled correctly.  The message pops up on the	*/
/* screen and waits for the user to select it.				*/

EXTERN(void, show_message2, (char *format, ...));
/* Takes a variable number of arguments exactly like printf.  The	*/
/* arguments are formatted accordingly and put into a message box.	*/
/* Imbedded newlines are handled correctly.  The message pops up, 	*/
/* and remains visible until hide_message2 is invoked.                  */

EXTERN(void, hide_message2, (void));
/* Hides the message dialog displayed by show_message2.                 */
#endif
