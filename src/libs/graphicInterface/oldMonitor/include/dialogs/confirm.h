/* $Id: confirm.h,v 1.3 1997/03/11 14:33:06 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*				 confirm.h				*/
/*		      confirmation dialog include file			*/
/*									*/
/* The confirmation dialog allows the user to select a single item or	*/
/* cancel.								*/
/* 									*/
/************************************************************************/

#ifndef dialogs_confirm_h
#define dialogs_confirm_h

EXTERN(Boolean, confirm, (char *prompt, char *button));
/* user selects an option	*/
/* Takes two parameters:  (char *prompt) the prompt string and		*/
/* (char *button) the option's name.  The user may choose the option or	*/
/* cancel.  The dialog returns true if the option was selected.		*/

#endif
