/* $Id: text.h,v 1.3 1997/03/11 14:33:12 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*				   text.h				*/
/*			   text item include file			*/
/*									*/
/* Text provides a way for the user to modify string data.  The data is	*/
/* always in the form of a pointer to allocated, null-terminated memory,*/
/* which may be modified by the item, but which must be initially	*/
/* allocated and later freed by the client.				*/
/* 									*/
/************************************************************************/

#ifndef items_text_h
#define items_text_h

EXTERN(DiaDesc *, item_text, (Generic item_id, char *title,
 short font, char **vptr, short width));
/* 1-d text string item desc.	*/
/* Takes five parameters:  (Generic item_id) the id of this item,	*/
/* (char *title) the item's title (or (char *) 0 for no title),		*/
/* (short font) the font to use, (char **vptr) a pointer to an		*/
/* allocated, null-terminated string to be edited, and (short width) the*/
/* width of the editing region in characters.  The text is limited to	*/
/* one line (no newlines).  The dialog descriptor of the item is	*/
/* returned.								*/

EXTERN(DiaDesc *, item_text2, (Generic item_id, char *title,
 short font, char **vptr, Point size));
/* 2-d text string item desc.	*/
/* Takes five parameters:  (Generic item_id) the id of this item,	*/
/* (char *title) the text name (or (char *) 0 for no title),		*/
/* (short font) the font to use, (char **vptr) a pointer to an		*/
/* allocated, null-terminated string to be edited, and (Point size) the	*/
/* size of the editing region in characters.  The dialog descriptor for	*/
/* the item is returned.						*/

EXTERN(void, item_text_handle_keyboard, (DiaDesc *dd, KbChar ch,
 Boolean ignoreFocus));
/* send KbChar to item	*/
/* Takes three parameters:  (DiaDesc *dd) the dialog descriptor of the	*/
/* text item (either 1-d or 2-d), (KbChar ch) the character to send, and*/
/* (Boolean ignoreFocus) true if it doesn't matter if the item has the	*/
/* input focus.  Currently dialogs must send KB_Enter characters to the	*/
/* item for the dialog manager as it intercepts KB_Enter's as "do the	*/
/* default action."  This call sets up an item modification for the	*/
/* item; it calls the dialog modification handler (which may be a	*/
/* recursive call).							*/

EXTERN(void, item_text_set_xy, (DiaDesc *dd, Point pt));
/* set new dot location		*/
/* Takes two parameters:  (DiaDesc *dd) the dialog descriptor of the	*/
/* text item (either 1-d or 2-d) and (Point pt) the new dot location.	*/

#endif
