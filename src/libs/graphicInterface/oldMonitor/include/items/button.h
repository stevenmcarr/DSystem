/* $Id: button.h,v 1.4 1997/03/11 14:33:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*				 button.h				*/
/*			 button item include file			*/
/*									*/
/* Buttons provide a way for the user to initiate an action.		*/
/* 									*/
/************************************************************************/

#ifndef items_button_h
#define items_button_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef dialog_h
#include <libs/graphicInterface/oldMonitor/include/mon/dialog.h>
#endif

EXTERN(DiaDesc*, item_button, (Generic item_id, char *title,
 short font, Boolean emph));
/* button item descriptor	*/
/* Takes four parameters:  (Generic item_id) the id of this item,	*/
/* (char *title) the button name, (short font) the button font, and	*/
/* (Boolean emph) whether or not the button should be emphasized.  A	*/
/* dialog descriptor of the item is returned.				*/
/* Note:  this item does not modify a value, so the modification handler*/
/* will need to initiate some action on a button modification (selection*/
/* of the button).							*/

#endif
