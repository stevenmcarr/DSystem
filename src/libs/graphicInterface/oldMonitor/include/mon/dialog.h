/* $Id: dialog.h,v 1.6 1997/03/11 14:33:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*				 dialog.h				*/
/*			       dialog boxes				*/
/*									*/
/* Dialogs are the means by which command processors can allow the	*/
/* user to modify data values directly.  Dialogs come in two flavors:	*/
/* modeless and modal.  Modal dialogs must be completed before any	*/
/* other action can be done on the client and modeless dialogs can be	*/
/* ignored until they are relevant to the user's task.			*/
/*									*/
/* The constituents of a dialog ("items") currently include check	*/
/* boxes, radio buttons, buttons, text, and integers.  More entries will*/
/* be added as they are needed.  Items in a dialog must be given an	*/
/* index which is later used in the specified modification handler (and */
/* helper if any).  The indices DIALOG_DEFAULT_ID, DIALOG_UNUSED_ID,	*/
/* and DIALOG_CANCEL_ID have special meaning.  DIALOG_UNUSED_ID may be	*/
/* specified if there is no need to take action on the item in the	*/
/* modification handler.  The index DIALOG_DEFAULT_ID is reported to	*/
/* have been modified when the user hits the return key.  Therefore, the*/
/* dialog may want to make the default item (such as the OK button) also*/
/* have an index of DIALOG_DEFAULT_ID.  Finally, DIALOG_CANCEL_ID will  */
/* be indicated on a "quit" of the dialog.				*/
/*									*/
/* Items can either be enabled or disabled depending on whether they are*/
/* relevant at the particular time.  Enabled items can be modified by	*/
/* the user and when this is done, a client supplied notification	*/
/* routine is called which may in turn modify other values of the	*/
/* dialog or directly call procedures in the client.			*/
/*									*/
/* A set of items can be indicated as being in error and an appropriate	*/
/* error message can be given.  The effects of the error call are	*/
/* canceled before the client supplied handler is called again.  In	*/
/* addition, a message can be given instead of an error with the same	*/
/* properties as the error message.					*/
/* 									*/
/* Information regarding the behavior and appearance of a particular	*/
/* instance of an item are put into a dialog descriptor (DiaDesc)	*/
/* by the dialog description creator of that particular item.  Groups	*/
/* of dialog descriptions can be combined into a single dialog		*/
/* description or more space may be put around a dialog descriptor.	*/
/* Using these combinations is the means of specifying the dialog 	*/
/* layout.  The descriptor for the entire dialog is what is used, in	*/
/* part, to create the dialog.						*/
/*									*/
/* Clients may create their own, special-purpose items if the provided	*/
/* items are not adequate for their needs.  See item_def.h for more	*/
/* information.								*/
/*									*/
/************************************************************************/

#ifndef dialog_h
#define dialog_h

typedef
struct	dialog	Dialog;			/* DIALOG TYPE			*/

/*********************** DIALOG DESCRIPTOR INFO *************************/

	/* SPECIAL ITEM IDs	*/
#define		DIALOG_DEFAULT_ID 0	/* "default" item id		*/
#define		DIALOG_UNUSED_ID  -1	/* "no interaction" item id	*/
#define		DIALOG_CANCEL_ID  -2	/* cancel the dialog id		*/

typedef
struct	dia_des	DiaDesc;		/* dialog descriptor structure	*/

typedef enum	{			/* HOW TO ARRANGE DESCRIPTORS	*/
		DIALOG_HORIZ_TOP,	/* left to right flush top	*/
		DIALOG_HORIZ_CENTER,	/* left to right centered	*/
		DIALOG_HORIZ_BOTTOM,	/* left to right flush bottom	*/
		DIALOG_VERT_LEFT,	/* top to bottom flush left	*/
		DIALOG_VERT_CENTER,	/* top to bottom centered	*/
		DIALOG_VERT_RIGHT	/* top to bottom flush right	*/
		} aDialogDir;

EXTERN(DiaDesc *, dialog_desc_group, (aDialogDir dir, int num_dds, ...));
/* Takes at least two parameters (variable):  (aDialogDir how) how to	*/
/* layout the mentioned descriptors, (int num) the number of descriptors*/
/* in the group and (DiaDesc *dd) the dialog descriptors of each of	*/
/* members of this group.  A dialog descriptor of the group is returned.*/

EXTERN(DiaDesc *, dialog_desc_expand, (DiaDesc *dd));
/* Takes one parameter:  (aDiaDesc *dd) the dialog descriptor of an	*/
/* item or a group.  Returns the dialog descriptor of the expansion.	*/

EXTERN(DiaDesc *, dialog_desc_lookup, (Dialog *di,
 Generic item_id));
/* Takes two parameters:  (Dialog *di) the dialog and (Generic item_id)	*/
/* the id of the item.  Returns the first associated dialog descriptor	*/
/* for the item.  The call will return NULL_DIA_DESC if the item is not	*/
/* part of the dialog.  Needed only for direct modification of items.	*/


/************************ CALLBACK INFORMATION **************************/

typedef FUNCTION_POINTER(Boolean, dialog_handler_callback,
 (Dialog *di, Generic owner_id, Generic item_id));
/* client provided handler */
/* Takes three parameters:  (Dialog *di) the dialog instance which has	*/
/* had a value modified, (Generic owner_id) the id value as specified by*/
/* the owner of this dialog, and (Generic item_id) the id of the	*/
/* component responsible for the changed value.  The value returned	*/
/* must be one of DIALOG_QUIT for terminating the dialog or		*/
/* DIALOG_NOMINAL for continuing the dialog.  This routine will be	*/
/* called in response to an user initiated change to data values of the	*/
/* dialog.  The effects of the last message and error indication are	*/
/* canceled immediately before this call.  If this routine, in turn,	*/
/* modifies other values, the dialog_item_modified() call (above)	*/
/* should be called.  This routine can also call client specific code,	*/
/* or send messages to the client to do updates to its window.		*/

#define		DIALOG_QUIT	true	/* quit the dialog		*/
#define		DIALOG_NOMINAL	false	/* continue running the dialog	*/

typedef FUNCTION_POINTER(void, dialog_helper_callback,
 (Dialog *di, Generic owner_id, Generic item_id));
/* client provided help handler	*/
/* Takes three parameters:  (Dialog *di) the dialog instance which has	*/
/* had a value modified, (Generic owner_id) the id value as specified	*/
/* by the owner of this dialog, and (Generic item_id) the id of the	*/
/* component responsible for the help-selected value (or		*/
/* DIALOG_UNUSED_ID for the entire dialog).				*/


/**************** DIALOG & DIALOG ITEM MANIPULATION *********************/

EXTERN(Dialog *, dialog_create, (char *title,
 dialog_handler_callback handler, dialog_helper_callback helper,
 Generic owner_id, DiaDesc *dd));
/* Takes five parameters:  (char *title) the title of the dialog or	*/
/* DIALOG_NO_TITLE for no title, (dialog_handler_callback handler) the	*/
/* handler to be called upon modification of values,			*/
/* (dialog_helper_callback helper) a help event handler or		*/
/* (dialog_helper_callback) 0 for the default handler,			*/
/* (Generic owner_id) a client defined variable passed to the handlers,	*/
/* and (DiaDesc *dd) the dialog descriptor.  A handle to the dialog is	*/
/* returned.								*/

#define		DIALOG_NO_TITLE (char *)0/* no title in a dialog	*/

EXTERN(void, dialog_set_handler, (Dialog *di,
 dialog_handler_callback handler));
/* Takes two parameters:  (Dialog *di) the dialog instance to modify,	*/
/* and (dialog_handler_callback handler) the new handler function to	*/
/* use with it.								*/

EXTERN(void, dialog_set_helper, (Dialog *di,
 dialog_helper_callback helper));
/* Takes two parameters:  (Dialog *di) the dialog instance to modify,	*/
/* and (dialog_helper_callback helper) the new helper function to use	*/
/* with it.  The default behavior can be realized by using the value	*/
/* (dialog_helper_callback) 0.						*/

EXTERN(void, dialog_destroy, (Dialog *di));
/* Takes one parameter:  (Dialog *di) the dialog instance to destroy.	*/

EXTERN(Boolean, dialog_item_set_focus, (Dialog *di, Generic item_id));
/* Takes two parameters:  (Dialog *di) the dialog instance and (Generic */
/* item_id) the item id to which to try to move the focus.  Returns its */
/* success.  Note:  only an enabled, character manipulating item can    */
/* have the focus.                                                      */

EXTERN(void, dialog_item_modified, (Dialog *di,
 Generic item_id));
/* Takes two parameters:  (Dialog *di) the dialog instance which has	*/
/* had a value modified and (Generic item_id) the id of the dialog item	*/
/* which is associated with the changed value.  The dialog is modified	*/
/* accordingly.								*/

EXTERN(void, dialog_item_ability, (Dialog *di, Generic item_id,
 Boolean enable));
/* Takes three parameters:  (Dialog *di) the dialog instance which	*/
/* contains the item, (Generic item_id) the id of the item to enable or	*/
/* disable, and (Boolean enable) one of DIALOG_ENABLE or DIALOG_DISABLE.*/
/* The item is modified accordingly.					*/
#define		DIALOG_ENABLE	true	/* enable the dialog		*/
#define		DIALOG_DISABLE	false	/* disable the dialog		*/

EXTERN(void, dialog_error, (Dialog *di, char *message, int num, ...));
/* Takes at least three parameters:  (Dialog *di) the dialog to		*/
/* indicate has an error, (char *message) the error message to give,	*/
/* (short num) the number of items that should be indicated as erroneous*/
/* and the next num arguments are the (Generic item_id) item numbers of	*/
/* the items which should be indicated as erroneous.  The effects of	*/
/* this call are canceled by the next user action.			*/

EXTERN(void, dialog_message, (Dialog *di, char *format, ...));
/* Takes at least two parameters:  (Dialog *di) the dialog to indicate	*/
/* the message and (char *format) the format of the message to give as  */
/* per varargs.  Other parameters may need to be included depending on  */
/* format.  The effects of this call are cancelled by the next user     */
/* action.                                                       	*/

/********************* MODAL DIALOG INFORMATION *************************/

EXTERN(void, dialog_modal_run, (Dialog *di));
/* Takes one parameter:  (Dialog *di) the dialog instance to run.	*/
/* All mouse and character interaction will go to the dialog until the	*/
/* handler routine gives the quit signal at which point control will	*/
/* return the command processor.  All dialog items are assumed to have	*/
/* been modified prior to running the dialog.  If the dialog is		*/
/* currently showing because of a dialog_modeless_show() call, it will	*/
/* be turned into a modal dialog and run to completion.			*/


/********************* MODELESS DIALOG INFORMATION **********************/

EXTERN(void, dialog_modeless_show, (Dialog *di));
/* Takes one parameter:  (Dialog *di) the dialog instance to show.	*/
/* This routine brings the dialog window into view so as to focus the	*/
/* user's attention on the dialog.  The dialog need not have been	*/
/* hidden to make this call.						*/

EXTERN(void, dialog_modeless_hide, (Dialog *di));
/* Takes one parameter:  (Dialog *di) the dialog instance to hide.	*/
/* This call is useful if a dialog is no longer relevant and its window	*/
/* should be removed from the screen.  Note:  the dialog window will be	*/
/* hidden as it is destroyed.						*/

#endif
