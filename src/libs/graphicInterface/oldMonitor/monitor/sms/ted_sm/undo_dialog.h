/* $Id: undo_dialog.h,v 1.4 1997/03/11 14:34:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*				undo_dialog.h				*/
/*		      undo dialog include file 				*/
/*									*/
/* This include file really represents two dialogs that work together	*/
/* to allow the user to find and/or replace strings in the client.	*/
/* 									*/
/************************************************************************/

typedef
struct	undia	Undia;			/* undo dialog structure*/

	/* CALLS */

EXTERN(Undia, *undo_dialog_create,(char *title));
/* create a undo dialog	*/
/* Takes three parameters:  (char *title) the title of the dialog; 	*/
/* (PFB UNDOER), the client-supplied callbacks described below;		*/
/* and (Generic owner) the client identifier used in those callbacks.	*/

EXTERN(Boolean,	undo_confirm,(Undia *und)); 
/* run the undo dialog */
/* Takes one parameter:  (Undia *und) the undo dialog structure.	*/
/* returns whether the user wishes to continue or not 			*/

EXTERN(void, undo_dialog_hide,(Undia *und)); 
/* hides the dialog from the user */
/* Takes one parameter: (Undia	*und) the undo dialog structure.	*/


EXTERN(void, undo_dialog_destroy,(Undia *und));	
/* destroy a undo dialog */
/* Takes one parameter:  (Undia *und) the undo dialog to		*/
/* destroy.								*/


