/* $Id: find.h,v 1.4 1997/03/11 14:33:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*				   find.h				*/
/*		      find/replace dialog include file			*/
/*									*/
/* This include file really represents two dialogs that work together	*/
/* to allow the user to find and/or replace strings in the client.	*/
/* 									*/
/************************************************************************/

#ifndef dialogs_find_h
#define dialogs_find_h

typedef
struct	frdia	aFRDia;			/* find/replace dialog structure*/


	/* DIRECTIONS */

#define	FRD_FORWARD	true		/* the forward direction	*/
#define	FRD_BACKWARD	false		/* the backward direction	*/


	/* CALLBACKS */

typedef FUNCTION_POINTER(Boolean, find_dialog_finder_func,
 (Generic owner, aFRDia *frd, char *find, Boolean direction,
 Boolean case_fold));
/* find an occurance of a string*/
/* Takes five parameters: (Generic owner) the owner id, (aFRDia *frd)	*/
/* the find/replace dialog making the request, (char *find) the string	*/
/* to search for, (Boolean direction) the direction to travel, and	*/
/* (Boolean case_fold) the case fold flag.  The success of the search	*/
/* should be returned.  Also, if the string was found, a call should be	*/
/* later made to the find_dialog_dirty call when a replace of the found	*/
/* string is no longer valid.						*/

typedef FUNCTION_POINTER(void, find_dialog_replacer_func,
 (Generic owner, aFRDia *frd, char *find, Boolean case_fold, char *replace));
/* replace a string with another*/
/* Takes five parameters:  (Generic owner) the owner id, (aFRDia *frd)	*/
/* the find/replace dialog making the request, (char *find) the string	*/
/* which has been previously found, (Boolean case_fold) the case fold	*/
/* value, and (char *replace) the string to replace it with.  The	*/
/* find_dialog_dirty call should be made if the string at the		*/
/* replacement site is not the find string.  (This almost always is the	*/
/* case.)								*/

typedef FUNCTION_POINTER(int, find_dialog_global_replacer_func,
 (Generic owner, aFRDia *frd, char *find, Boolean globally, Boolean direction,
 Boolean case_fold, char *replace));
/* replace globally		*/
/* Takes seven parameters:  (Generic owner) the owner id, (aFRDia *frd)	*/
/* the find/replace dialog making the request, (char *find) the string	*/
/* to be replaced, (Boolean globally) true if the replacement is to	*/
/* be done globally or just for the remainder, (Boolean direction) the	*/
/* direction to do the replacement (if globally == false),		*/
/* (Boolean case_fold) the case fold flag, and (char *replace) the	*/
/* replacement string.  The number of replacements made is returned.	*/


	/* CALLS */

EXTERN(aFRDia *, find_dialog_create, (char *find_title,
 char *replace_title, char *find_string, char *replace_string,
 Boolean direction, Boolean case_fold, find_dialog_finder_func finder,
 find_dialog_replacer_func replacer,
 find_dialog_global_replacer_func global_replacer, Generic owner));
/* create a find/replace dialog	*/
/* Takes ten parameters:  (char *find_title) the title of the find 	*/
/* dialog; (char *replace_title) the title of the replace dialog;	*/
/* (char *find_string), (char *replace_string), (Boolean direction),	*/
/* (Boolean case_fold) the initial values for the find and replace	*/
/* strings, direction and case fold; (find_dialog_finder_func finder),	*/
/* (find_dialog_replacer_func replacer) and				*/
/* (find_dialog_global_replacer_func global_replacer) the		*/
/* client-supplied callbacks described above; and (Generic owner) the	*/
/* client identifier used in those callbacks.  A find/replace dialog is	*/
/* created and a handle is returned.  The dialog must later be		*/
/* destroyed with the find_dialog_destroy call.  NOTE:  the replacer	*/
/* and global_replacer may be set to 0 with only a loss of		*/
/* functionality.							*/

EXTERN(void, find_dialog_run_find, (aFRDia *frd));
/* run the "find" dialog	*/
/* Takes one parameter:  (aFRDia *frd) the find/replace dialog to	*/
/* run the find portion of.						*/

EXTERN(void, find_dialog_run_replace, (aFRDia *frd));
/* run the "replace" dialog	*/
/* Takes one parameter:  (aFRDia *frd) the find/replace dialog to	*/
/* run the replace portion of.						*/

EXTERN(void, find_dialog_set_values, (aFRDia *frd,
 char *find_string, char *replace_string, Boolean direction,
 Boolean case_fold));
/* set the current values	*/
/* Takes five parameters:  (aFRDia *frd) the find/replace dialog, 	*/
/* (char *find_string), (char *replace_string), (Boolean direction), and*/
/* (Boolean case_fold) variables from the client to be installed	*/
/* as the current values of the find and replace strings, direction,	*/
/* and case fold.							*/

EXTERN(void, find_dialog_get_values, (aFRDia *frd,
 char **find_string, char **replace_string, Boolean *direction,
 Boolean *case_fold));
/* get the current values	*/
/* Takes five parameters:  (aFRDia *frd) the find/replace dialog, 	*/
/* (char **find_string), (char **replace_string), (Boolean *direction), */
/* (Boolean *case_fold) pointers to the variables in the client to put	*/
/* the current values of the find and replace strings, direction, and	*/
/* case fold.								*/

EXTERN(void, find_dialog_dirty, (aFRDia *frd));
/* notify of invalidation	*/
/* Takes one parameter:  (aFRDia *frd) the find/replace dialog.  This	*/
/* call should be made to let the dialog know that the last find	*/
/* performed by the client is no longer valid.  The call may be made	*/
/* more often than once per find.					*/

EXTERN(void, find_dialog_destroy, (aFRDia *frd));
/* destroy a find/replace dialog*/
/* Takes one parameter:  (aFRDia *frd) the find/replace dialog to	*/
/* destroy.								*/

#endif
