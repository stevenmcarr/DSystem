/* $Id: prompt.h,v 1.3 1997/03/11 14:33:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*				 prompt.h				*/
/*			 prompt dialog include file			*/
/*									*/
/* Prompted string edits allow the client to give the user a prompt	*/
/* and then the user may then edit a string given by the client.	*/
/* 									*/
/************************************************************************/

#ifndef dialogs_prompt_h
#define dialogs_prompt_h

EXTERN(char *, prompted_string_edit, (char *prompt, Point size,
 char *buffer));
/* user edits a string w/ prompt*/
/* Takes three parameters:  (char *prompt) the prompt to show,		*/
/* (Point size) the size of edit pane (in chars), and (char *buffer) the*/
/* buffer to be modified.  The user is allowed to modify the buffer	*/
/* until a <RETURN> or a select. The buffer it returns is of variable	*/
/* length and should be freed by the client.				*/

#endif
