/* $Id: title.h,v 1.4 1997/03/11 14:33:12 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*				  title.h				*/
/*			  title item include file			*/
/*									*/
/* Titles provide a way to label a group of items.			*/
/* 									*/
/************************************************************************/

#ifndef items_title_h
#define items_title_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef dialog_h
#include <libs/graphicInterface/oldMonitor/include/mon/dialog.h>
#endif

EXTERN(DiaDesc *, item_title, (Generic item_id, char *title, short font));
/* title item descriptor	*/
/* Takes three parameters:  (Generic item_id) the name that specifies	*/
/* this title item instance, (char *title) the title to use, and	*/
/* (short font) the font in which the title should appear.  Note that	*/
/* titles may include newlines, and the default text positioning is 	*/
/* center justicication.  A dialog descriptor of the item is returned.	*/

EXTERN(void, item_title_change, (DiaDesc *dd, char *title));
/* modify existing title item	*/
/* Takes two parameters: (DiaDesc *dd) the dialog descriptor of the	*/
/* title item to modify and (char *title) the new title.		*/
/* Note that no resizing will occur because of this call, so it is wise	*/
/* to call item_title() with the largest title that will be displayed	*/
/* by that item, and then switch to other (possibly smaller) titles as	*/
/* desired.								*/

EXTERN(void, item_title_justify_center, (DiaDesc *dd));
/* set justification to centered*/
/* Takes one parameters: (DiaDesc *dd) the dialog descriptor of the	*/
/* title item to modify.  Sets text justification to centered.		*/

EXTERN(void, item_title_justify_left, (DiaDesc *dd));
/* set justification to left	*/
/* Takes one parameters: (DiaDesc *dd) the dialog descriptor of the	*/
/* title item to modify.  Sets text justification to left.		*/

#endif
