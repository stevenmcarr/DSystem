/* $Id: item.h,v 1.7 1997/06/25 14:46:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*				  item.h				*/
/*		       item information include file			*/
/*									*/
/* Items are used by dialogs to let the user modify a specific datum.	*/
/*									*/
/* An item is defined by a set of static routines as members of the Item*/
/* structure and one globally known routine to create a dialog		*/
/* descriptor.  The routine is the only handle to the specific item type*/
/* and the returned dialog descriptor serves as the only handle to the	*/
/* item instance.  The call makeDialogDescriptor() is provided for	*/
/* convenience (it does most of the work of the dialog descriptor	*/
/* creator).								*/
/*									*/
/* Each item in a dialog has a specific item id for each item in the	*/
/* dialog.  This id is to be installed into the dialog descriptor	*/
/* so that requests from the dialog manager can be sent to the		*/
/* appropriate item.  If the item does not interact with the user, the	*/
/* id should be set to UNUSED.						*/
/*									*/
/* Each item is implemented by a single pane (or no pane).  The pane	*/
/* is specified by the makeDialogDescriptor() call if one is desired.	*/
/* Each implementation pane will be created, destroyed, positioned,	*/
/* redrawn, and allowed to handle input all by way of the dialog	*/
/* manager.  The input handler on item implementation panes must adhere	*/
/* to two conventions:  First, it must return on MOUSE_KEYBOARD events	*/
/* so that the dialog manager may arbitrate them and second, it must	*/
/* return an EVENT_MODIFIED if a change has been made to the data via	*/
/* the mouse.  All other events coming from the pane are ignored.	*/
/*									*/
/************************************************************************/

#ifndef item_h
#define item_h

typedef	struct	item	Item;
struct p_list;
/****************** DIALOG DESCRIPTOR INFORMATION ***********************/

struct	dia_des {			/* DIALOG DESCRIPTOR TYPE	*/
	Dialog		*owner;		/* the owner dialog		*/
	struct	item	*item;		/* the dialog item to use	*/
	Generic		item_info;	/* item specific information	*/
	Generic		item_id;	/* the item id (for handler)	*/
	struct p_list	*pane_list;	/* the implementation panes	*/
	struct pane	*shadow;	/* pane underneath item for help*/
	Rectangle	error_box;	/* where to draw the error box	*/
	Boolean		able;		/* true if ability is set	*/
	struct	dia_des *next_registered;/* next in the registered list	*/
	struct	dia_des	*next_error;	/* next in the error list	*/
		};
#define		NULL_DIA_DESC	((DiaDesc *) 0) /* null dia. descriptor	*/

EXTERN(DiaDesc *, makeDialogDescriptor, (Item *item,
 Generic item_info, Generic item_id));
/* create & fill in descriptor	*/
/* Takes three parameters:  (Item *item) the item structure for this	*/
/* item, (Generic item_info) the item specific information field, and	*/
/* (Generic item_id) the dialog specific index for this item (or	*/
/* UNUSED) if the user will not be interacting with this item.  Returns	*/
/* a filled in dialog descriptor.					*/

EXTERN(void, initDialogDescriptor, (Dialog *di, DiaDesc *dd,
 Point ulc));
/* initialize a dialog desc.	*/
/* Takes three parameters:  (Dialog *di) the parent dialog, (DiaDesc	*/
/* *dd) the dialog descriptor to initialize, and (Point ulc) the	*/
/* position of the dialog descriptor.					*/

EXTERN(void, freeDialogDescriptor, (DiaDesc *dd));
/* free a dialog descriptor	*/
/* Takes one parameter:  (DiaDesc *dd) the dialog descriptor to be	*/
/* freed.  All panes created by the item are freed at this time.	*/


/************************* ITEM INFORMATION *****************************/

typedef
enum		{			/* FOCUS RETURN STATUS		*/
			FocusThis,	/* Move the focus here		*/
			FocusNext,	/* advance the focus		*/
			FocusOK		/* do not modify the focus	*/
		} FocusStatus;
	
typedef FUNCTION_POINTER(Point, item_get_size_func, (DiaDesc *dd));
/* Takes one parameter:  (DiaDesc *dd) the dialog descriptor of the	*/
/* item.  The call should use the information field to return the size	*/
/* of the item in pixels.						*/

typedef FUNCTION_POINTER(void, item_initialize_func, (DiaDesc *dd,
                                                      Point ulc));
/* Takes two parameters:  (DiaDesc *dd) the dialog descriptor of the	*/
/* item and (Point ulc) the upper left corner of the item.  The pane	*/
/* which has just been created should be initialized based on the item	*/
/* specific information.						*/

typedef FUNCTION_POINTER(FocusStatus, item_handle_event_func, (DiaDesc *dd));
/* Takes one parameter:  (DiaDesc *dd) the dialog descriptor of the	*/
/* event item.  Returns the focus status of the descriptor.		*/

typedef FUNCTION_POINTER(DiaDesc *, item_set_focus_func,
 (DiaDesc *dd, Boolean fs));
/* Takes two parameters:  (DiaDesc *dd) the dialog descriptor in which	*/
/* to set the focus, and (Boolean fs) the focus status.  If fs is	*/
/* FOCUS_RESET, the focus should be moved to the next available dd	*/
/* and that dd should be returned.  If no focus could be set,		*/
/* NULL_DIA_DESC should be returned.  If fs is FOCUS_CLEAR, the focus	*/
/* should be removed and the returned value is ignored.  Initially, the	*/
/* focus should not be at any dialog descriptor.			*/
#define		FOCUS_RESET	true	/* set the new keyboard focus	*/
#define		FOCUS_CLEAR	false	/* shut off the keyboard focus	*/

typedef FUNCTION_POINTER(void, item_modified_func,
 (DiaDesc *dd, Boolean user));
/* Takes two parameters:  (DiaDesc *dd) the dialog descriptor that has	*/
/* had a value modified and (Boolean user) which is true if the user	*/
/* has modified this value and false otherwise.  The item should be	*/
/* redisplayed accordingly.  The boolean is intended to allow the item	*/
/* to translate from the data actually manipulated by its implementation*/
/* screen module to the actual item datatype and back as necessary.	*/

typedef FUNCTION_POINTER(void, item_destroy_func, (DiaDesc *dd));
/* Takes one parameter:  (DiaDesc *dd) the dialog descriptor that is	*/
/* being destroyed.  The item should destroy only item specific		*/
/* information; the pane will be destroyed by the dialog manager.	*/


struct	item	{			/* DIALOG ITEM DEFINITION	*/

char		*name;			/* the name of the item		*/

item_get_size_func
		get_size;		/* get desired size		*/

item_initialize_func
		initialize;		/* initialize the item		*/

item_handle_event_func
		handle_event;		/* item handles input		*/

item_set_focus_func
		set_focus;		/* set the keyboard input focus*/

item_modified_func
		modified;		/* the value has been modified	*/

item_destroy_func
		destroy;		/* destroy the item information	*/

		};

EXTERN(DiaDesc *, standardNoFocusSetter, (DiaDesc *dd,
 Boolean set));
/* null item focus setter	*/
/* This routine may be attached to the Item structure in the case that	*/
/* the item does not handle keyboard input.				*/


/************************** ITEM CALLBACKS ******************************/


EXTERN(void, dialog_item_user_change, (DiaDesc *dm));
/* notify of a user change	*/
/* Takes one parameter:  (DiaDesc *dm) the dialog descriptor that	*/
/* changed.  This call is to be used when the user makes modifications	*/
/* to an item.  Other items with the same id will be modified		*/
/* accordingly.								*/

EXTERN(Pane *, dialog_item_make_pane, (DiaDesc *dd, short id,
 Point position, Point size, short border));
/* make a pane & register it	*/
/* Takes five parameters:  (DiaDesc *dd) the dialog descriptor which	*/
/* owns the pane, (short id) the screen module index of the pane,	*/
/* (Point postion, size) the position and size of the pane, and (short	*/
/* border) the border width of the pane.  The pane is registered and 	*/
/* returned.								*/

EXTERN(void, dialog_item_redraw_pane, (DiaDesc *dd, Pane *p));
/* redraw a pane		*/
/* Takes two parameters:  (DiaDesc *dd) the dialog descriptor which owns*/
/* the pane and (struct pane *pane) the pane to repaint.  This call	*/
/* should be made whenever the pane's appearance changes not as a result*/
/* of a change in the value of an item.  (This call mostly handles the	*/
/* painting of a disabled item.)					*/

#endif
