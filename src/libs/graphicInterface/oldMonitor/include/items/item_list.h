/* $Id: item_list.h,v 1.6 1997/03/11 14:33:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	items/item_list.h							*/
/*									*/
/*	item_list -- dialog item made out of the list screen module		*/
/*									*/
/************************************************************************/

#ifndef items_list_h
#define items_list_h

#ifndef	general_h
#include <libs/support/misc/general.h>
#endif
#ifndef	point_h
#include <libs/graphicInterface/support/graphics/point.h>
#endif
#ifndef	dialog_h
#include <libs/graphicInterface/oldMonitor/include/mon/dialog.h>
#endif

typedef Generic	 ListItemEntry;		/* represents a list entry	*/
#define          NULL_LIST_ITEM_ENTRY	(ListItemEntry) 0

typedef FUNCTION_POINTER(ListItemEntry, item_list_elem_proc, (Generic owner,
 DiaDesc *dd, Boolean first, Generic id));
/* listElemProc takes four parameters:  (Generic owner) the owner handle*/
/* passed into item_list(), (DiaDesc *dd) the dialog descriptor of the	*/
/* item requesting the line, (Boolean first) true if the first entry	*/
/* of the list is being requested, (Generic id) the id of the previous	*/
/* line in the list if first = false.  The ListItemEntry of the		*/
/* requested line should be returned or NULL_LIST_ITEM_ENTRY if there	*/
/* are no more entries in the list.  Use item_list_create_entry() to	*/
/* create a ListItemEntry to be returned.				*/

EXTERN(DiaDesc *, item_list, (Generic item_id, char *title,
 Generic owner, item_list_elem_proc listElemProc, Generic *vptr,
 Generic null_id, Boolean toggle, short font, Point size));
/* list item descriptor		*/
/* Takes nine parameters:  (Generic item_id) the id of this item,	*/
/* (char *title) the title of the item (or (char *) 0 for no title,	*/
/* (Generic owner) the owner handle for the listElemProc callback,	*/
/* (item_list_elem_proc listElemProc) the list element generator	*/
/* callback, (Generic *vptr) the pointer to the value which 		*/
/* represents the selected item of the list, (Generic null_id) the value*/
/* of *vptr when nothing is selected, (Boolean toggle) true if selecting*/
/* a selected entry in the list should result in no selection,  	*/
/* (short font) the font to use, and (Point size) the size of the item	*/
/* in characters.  The dialog descriptor for the item is returned.	*/

EXTERN(ListItemEntry, item_list_create_entry, (DiaDesc *dd,
 Generic id, char *text, Boolean selectable));
/* create a list entry	*/
/* Takes four parameters:  (DiaDesc *dd) the dialog descriptor for the	*/
/* item, (Generic id) the id of the list entry, (char *text) the text	*/
/* for the line (which is copied), and (Boolean selectable) true if the	*/
/* line can be selected.  A ListItemEntry for this line is returned.	*/
/* Note:  this call should only be made within the listElemProc and only*/
/* to satisfy the current request.					*/

EXTERN(void, item_list_modified, (DiaDesc *dd));
/* forget the old list		*/
/* Takes one parameter (DiaDesc *dd) the dialog descriptor for the item.*/
/* The list is "repainted" while trying to move the list as little as	*/
/* possible.  The call will also handle a change in the selected line.	*/

EXTERN(void, item_list_line_modified, (DiaDesc *dd, Generic id));
/* change a line entry	*/
/* Takes two parameters:  (DiaDesc *dd) the dialog descriptor for the	*/
/* item and (Generic id) the id of the line entry which has changed.	*/
/* The line is updated on the screen.					*/

EXTERN(void, item_list_line_change, (DiaDesc *dd, Generic id,
 Boolean selectable));
/* change selectability of a ln	*/
/* Takes three parameters:  (DiaDesc *dd) the dialog descriptor for the	*/
/* item, (Generic id) the id of the line to change, and (Boolean        */
/* selectable) the new value of the selectability parameter.  The line  */
/* is updated on the screen.					       	*/

EXTERN(void, item_list_line_show, (DiaDesc *dd, Generic id,
 Boolean is_new));
/* show a particular line	*/
/* Takes three parameters:  (DiaDesc *dd) the dialog descriptor for the	*/
/* item, (Generic id) the id of the line, and (Boolean is_new) true if	*/
/* the entry is not in the list.  The entry is shown to the user.	*/

#endif
