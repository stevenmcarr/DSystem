/* $Id: radio_btns.h,v 1.5 1997/03/11 14:33:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*				radio_btns.h				*/
/*		      radio buttons item include file			*/
/*									*/
/* Radio buttons allow the user to pick one of a set of choices.  Each	*/
/* choice assigns a unique value to a Generic.				*/
/* 									*/
/************************************************************************/

#ifndef items_radio_btns_h
#define items_radio_btns_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef dialog_h
#include <libs/graphicInterface/oldMonitor/include/mon/dialog.h>
#endif

EXTERN(DiaDesc*, item_radio_buttons, (Generic item_id, char *title, 
                                      short font, Generic *vptr, int cols, int rows,
                                      /*char *btn1_text, Generic btn1_id,*/ ...));
/* Takes at least six parameters (variable):  (Generic item_num) the	*/
/* id of this item, (char *title) the title of the set of buttons or	*/
/* DIALOG_NO_TITLE for no title, (short font) the font of the radio	*/
/* buttons, (Generic *vptr) the value which is modified by this set of	*/
/* buttons, (int cols, rows) the overall number of radio buttons in each*/
/* direction.  Then, for each individual radio button (in row major	*/
/* order) the following two parameters:  (char *btn?_text) the title of */
/* this button or DIALOG_NO_TITLE for no radio button in this position  */
/* (if this is the case, the following parameter will be ignored),	*/
/* (Generic btn?_id) the value put into the *vptr upon selection of this*/
/* radio button.  A dialog descriptor of the item is returned.		*/

#endif
