/* $Id: filer.h,v 1.7 1997/03/11 14:33:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*				  filer.h				*/
/*			 filer dialog include file			*/
/*									*/
/* The filer dialog provides a user interface for viewing and		*/
/* manipulating the unix file system.					*/
/* 									*/
/************************************************************************/

#ifndef dialogs_filer_h
#define dialogs_filer_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

typedef struct	filedia	aFileDia;		/* file dialog structure	*/

typedef FUNCTION_POINTER(Boolean, file_dialog_okay_func, (Generic owner, char *file));
/* ((Generic owner) the owner id and (char *file) the file name to	*/
/* verify) which will return true if the "doit" operation can be	*/
/* performed on the specified file.					*/

EXTERN(void, file_update_all, (aFileDia *pd, char *filename));

EXTERN(Boolean, file_select_ok_to_write, (Generic handle, char *filename));
/* An instance of file_dialog_okay_func which invokes file_ok_to_write, */
/* provided by unix_file.c                                              */

EXTERN(Boolean, file_select_dialog_run, (char *title,
 char *bname, char **toptr, file_dialog_okay_func okayer, Generic handle));
/* select a file dialog	*/
/* Takes five parameters:  (char *title) the title of the dialog,	*/
/* (char bname) the name of the "doit" button, (char **toptr) a 	*/
/* pointer to a get_mem'd string containing the file name to start the	*/
/* dialog, (file_dialog_okay_func okayer) a pointer to the okay		*/
/* function and (Generic handle) the owner handle given to the okayer	*/
/* callback.  The return value is true if the "doit" button was		*/
/* selected.  Regardless, toptr will point to the current string that	*/
/* the user was working with.  (This should be saved for the next call	*/
/* to this function.)							*/

#endif
