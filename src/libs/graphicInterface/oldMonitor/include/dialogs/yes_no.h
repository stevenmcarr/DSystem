/* $Id: yes_no.h,v 1.3 1997/03/11 14:33:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*				 yes_no.h				*/
/*			yes/no dialog include file			*/
/*									*/
/* The yes/no dialog allows the user to select from a yes/no choice.	*/
/* The client may specify a default choice.				*/
/* 									*/
/************************************************************************/

#ifndef dialogs_yes_no_h
#define dialogs_yes_no_h

EXTERN(Boolean, yes_no, (char *prompt, Boolean *answer,
 Boolean def));
/* user selects from yes or no	*/
/* Takes three parameters:  (char *prompt) the prompt string,		*/
/* (Boolean *answer) the answer pointer, and (Boolean default) the	*/
/* value to emphisize.  The user make s choice of yes, no, or cancel.	*/
/* If cancel is selected, the return value is false and the default	*/
/* value is put into the answer field.  Otherwise, the return value is	*/
/* true and and the choice is put into the return value.		*/

#endif
