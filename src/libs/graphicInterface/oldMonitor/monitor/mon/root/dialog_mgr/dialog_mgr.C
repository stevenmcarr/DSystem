/* $Id: dialog_mgr.C,v 1.1 1997/06/25 14:50:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*			      dialog_mgr.c				*/
/*			    dialogue manager				*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/mon/manager.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/desk_sm.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/dialog_mgr.h>
#include <libs/graphicInterface/oldMonitor/include/mon/dialog.h>
#include <libs/graphicInterface/oldMonitor/include/mon/item.h>
#include <libs/graphicInterface/oldMonitor/include/sms/message_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/support/strings/rn_string.h>


struct	p_list	{			/* LIST OF PANES		*/
	Pane		*p;		/* the pane involved with dd	*/
	Point		size;		/* the size of the pane		*/
	struct	p_list	*next;		/* next pane entry on the list	*/
		};


    /*** DIALOGUE INSTANCE ***/

struct dia_info;

struct	dialog	{			/* DIALOGUE INSTANCE STRUCTURE	    */
    struct dia_info	*manager;	/* the owning manager		    */
    Window		*w;		/* the dialogue window		    */
    dialog_handler_callback			handler;	/* the dialogue event handler	    */
    dialog_helper_callback			helper;		/* the dialogue help event handler  */
    Generic		owner_id;	/* the owner id			    */
    Pane		*msg_pane;	/* the message pane		    */
    Boolean		msg_empty;	/* true if msg pane is empty	    */
    DiaDesc		*root;		/* the root dialogue descriptor	    */
    Boolean		forced;		/* true if the dialog is forced	    */
    DiaDesc		*error_dd;	/* the list of error dd's	    */
    DiaDesc		*registered_dd; /* the list of registered dd's	    */
    DiaDesc		*focus;		/* the focus of the keyboard	    */
    Dialog		*next;		/* the next dialogue by manager	    */
		};
#define		NULL_INSTANCE (Dialog *) 0  /* null dialog instance	    */


    /*** MANAGER INFORMATION ***/

typedef
struct	dia_info {			/* MANAGER INFORMATION STRUCT.	*/
    Generic		manager_id;	/* the id of the manager	*/
    Dialog		*dia_list;	/* dialogue registration list	*/
		} aDiaMgr;

STATIC(void,	dia_start,(void));		
     /* manager start routine	*/
STATIC(Generic, dia_create,(Generic manager_id));		
     /* manager create routine	*/
STATIC(void,	dia_event,(aDiaMgr *m));		
     /* manager event handler	*/
STATIC(Point,	dia_window_tile,(aDiaMgr *m, Window *w, Dialog *di, 
                                 Point ulc, Boolean New));	
     /* manager window tiler		*/
STATIC(void,	dia_destroy,(aDiaMgr *m));       	
     /* manager destroy routine	*/
STATIC(void,	dia_finish,(void));		
     /* manager finish routine	*/

aManager	dialog_manager = {	/* MANAGER DECLARATION		*/
			"dialogue manager",
			0,
			dia_start,
			dia_create,
			(ManagerEventFunc)dia_event,
			(ManagerWindowTileFunc)dia_window_tile,
			(ManagerDestroyFunc)dia_destroy,
			dia_finish
		};


	/*** LOCAL SYMBOLS ***/

static	aDiaMgr *current_dm;		/* the current dialog manager	*/
static	short	message_sm;		/* index of the message sm	*/
static	short	vanilla_sm;		/* index of the vanilla sm	*/
static	Bitmap	disabled_bitmap;		/* the disabled pattern to use	*/
#define		ERROR_BOX_WIDTH	    1	/* width of the error box	*/


/************************* INTERNAL ROUTINES ****************************/


/* Handle a user modification to an item.				*/
static
void
handle_item_event(Dialog *di, Generic item_id)
{
DiaDesc		*dd;			/* the dialogue descriptor	*/

    dd = dialog_desc_lookup(di, item_id);
    if (dd && NOT(dd->able))
    {/* do not notify about a disabled item */
	return;
    }
    
    while (di->error_dd)
    {/* remove the errors */
	boxColor(di->w->image, di->error_dd->error_box,
		ERROR_BOX_WIDTH, NULL_BITMAP, Origin, UnusedRect, false,
		windowBackground(di->w), NULL_COLOR);  /* change to boxPane if can find pane */
	touchPaneRect(
		di->msg_pane,
		subRect(
			di->error_dd->error_box,
			di->msg_pane->position
		)
	);
	di->error_dd = di->error_dd->next_error;
    }
    if (NOT(di->msg_empty))
    {/* clear the message pane */
	sm_message_new(di->msg_pane, "");
	di->msg_empty = true;
    }

    if ((di->handler)(di, di->owner_id, item_id) == DIALOG_QUIT)
    {/* quit the dialog */
	if (di->forced)
	{/* release a modal dialog */
	    sm_desk_win_release(di->w);
	    di->forced = false;
	}
	sm_desk_win_hide(di->w);
    }
}


/* Handle a user help request to an item.				*/
static
void
handle_item_help(Dialog *di, Generic item_id)
{
    if (di->helper)
    {/* help available */
	(di->helper)(di, di->owner_id, item_id);
    }
    else
    {/* use the default help */
	message("Manipulate the dialogue to change values.");
    }
}


/* Look up the dialogue descriptor for an item with the id.		*/
static
void
handle_dd_event(Dialog *di, DiaDesc *dd)
{
    switch ((dd->item->handle_event)(dd))
    {/* handle the focus status */
	case FocusThis: /* change the focus to here */
	    while (di->focus != dd)
	    {/* go to the next possible focus */
		di->focus = (di->root->item->set_focus)(di->root, FOCUS_RESET);
	    }
	    break;

	case FocusNext: /* move the focus to the next */
	    di->focus = (di->root->item->set_focus)(di->root, FOCUS_RESET);
	    if (di->focus == NULL_DIA_DESC)
	    {/* try again--focus may have wrapped */
		di->focus = (di->root->item->set_focus)(di->root, FOCUS_RESET);
	    }
	    break;

	case FocusOK:	/* do not change the focus */
	    break;
    }
}


/* Display a disabled pane.						*/
static
void
display_disabled_pane(Pane *p, Boolean resize_before, Boolean disable_after)
{
	if (resize_before)
		resizePane(p, p->position, p->size);
	ColorPaneWithPatternColor(p, makeRectFromSize(Origin, p->size),
			     disabled_bitmap, Origin, false,
			     NULL_COLOR, paneBackground(p));
	touchPane(p);
	if (disable_after)
		resizePane(p, p->position, Origin);

	return;
}



/************************* DIALOGUE CALLBACKS ***************************/


/* Get a dialog descriptor for an item (or return NULL_DIA_DESC).	*/
DiaDesc *
dialog_desc_lookup(Dialog *di, Generic item_id)
{
DiaDesc		*dd;			/* the dialogue descriptor	*/

    if (item_id != DIALOG_UNUSED_ID)
    {/* look up the item id */
	for (dd = di->registered_dd; dd; dd = dd->next_registered)
	{/* walk down the registered list */
	    if (dd->item_id == item_id)
	    {/* we have found one */
		return dd;
	    }
	}
    }
    return NULL_DIA_DESC;
}


/* Create a dialogue instance.						*/
Dialog *
dialog_create(char *title, dialog_handler_callback handler, 
	      dialog_helper_callback helper, Generic owner_id, 
              DiaDesc *dd)
{
Dialog		*di;			/* the dialogue being created	*/

    /* create and initialize the dialogue */
	di = (Dialog *) get_mem(sizeof(Dialog), "new dialog instance");
	di->manager	  = current_dm;
	di->handler	  = handler;
	di->helper	  = helper;
	di->owner_id	  = owner_id;
	di->root	  = dd;
	di->forced	  = false;
	di->msg_pane	  = NULL_PANE;
	di->msg_empty	  = true;
	di->error_dd	  = NULL_DIA_DESC;
	di->registered_dd = NULL_DIA_DESC;

    /* install the dialog */
	di->next = current_dm->dia_list;
	current_dm->dia_list = di;

    /* set up the window */
	(void) sm_desk_win_create(
		(aMgrInst*)di->manager->manager_id,
		(Generic) di,
		DEF_FONT_ID,
		DSM_WIN_NO_RESIZE);
	sm_desk_win_title(di->w, title);

    /* set the focus */
	di->focus = (di->root->item->set_focus)(di->root, FOCUS_RESET);

    return di;
}


/* Set the event handler for the dialog to a new value			*/
void
dialog_set_handler(Dialog *di, dialog_handler_callback handler)
{
    di->handler = handler;
}


/* Set the event helper for the dialog to a new value			*/
void
dialog_set_helper(Dialog *di, dialog_helper_callback helper)
{
    di->helper = helper;
}


/* Destroy a dialogue instance.						*/
void
dialog_destroy(Dialog *di)
{
Dialog		**dp;			/* the dialogue pointer		*/

    for (dp = &di->manager->dia_list; *dp; dp = &(*dp)->next)
    {/* walk down the parent's dialogue list */
	if (*dp == di)
	{/* this is the current dialogue--delete it */
	    *dp = (*dp)->next;
	    freeDialogDescriptor(di->root);
	    sm_desk_win_destroy(di->w);
	    free_mem((void*) di);
	    return;
	}
    }
    die_with_message("dialog_destroy():	 non-existant dialogue.");
}


/* Move the focus to an item.						*/
Boolean
dialog_item_set_focus(Dialog *di, Generic item_id)
{
DiaDesc		*old = di->focus;	/* the original focus		*/
DiaDesc		*New;			/* the new focus		*/

    New = dialog_desc_lookup(di, item_id);
    if (New == NULL_DIA_DESC)
    {/* cannot find item */
	die_with_message("dialog_item_set_focus():  Unknown item id (%d).", item_id);
    }
    if (New == old)
    {/* we are already there */
	return true;
    }
    do
    {/* move the focus one step */
	di->focus = (di->root->item->set_focus)(di->root, FOCUS_RESET);
    } while ((di->focus != old) && (di->focus != New));
    return BOOL(di->focus == New);
}


/* Notify dialogue of a value modification.				*/
void
dialog_item_modified(Dialog *di, Generic item_id)
{
DiaDesc		*dd;			/* the dialogue descriptor	*/
struct	p_list	*pl;			/* the pane list entry		*/

    for (dd = di->registered_dd; dd; dd = dd->next_registered)
    {/* walk down the dialogue list */
	if (dd->item_id == item_id)
	{/* this is one of the dd's */
	    (dd->item->modified)(dd, false);
	    if (NOT(dd->able))
	    {/* the item is disabled */
		for (pl = dd->pane_list; pl; pl = pl->next)
		{/* repaint each pane & disable */
		    display_disabled_pane(pl->p, true, true);
		}
	    }
	}
    }
}


/* Enable/disable a dialogue item.					*/
void
dialog_item_ability(Dialog *di, Generic item_id, Boolean enable)
{
DiaDesc		*dd;			/* the dialogue descriptor	*/
struct	p_list	*pl;			/* the pane list entry		*/

    for (dd = di->registered_dd; dd; dd = dd->next_registered)
    {/* walk down the dialogue list */
	if (dd->item_id == item_id)
	{/* this is one of the dd's */
	    if (dd->able != enable)
	    {/* something needs to be done */
		dd->able = enable;
		if (enable)
		{/* reinable the item */
		    for (pl = dd->pane_list; pl; pl = pl->next)
		    {/* reenable each real pane */
			resizePane(pl->p, pl->p->position, pl->size);
			touchPane(pl->p);
		    }
		    if (di->focus == NULL_DIA_DESC)
		    {/* the focus may now exist */
			di->focus = (di->root->item->set_focus)(
				di->root,
				FOCUS_RESET
			);
		    }
		}
		else
		{/* disable the item */
		    if (di->focus == dd)
		    {/* move the focus */
			di->focus = (di->root->item->set_focus)(
				di->root,
				FOCUS_RESET
			);
			if (di->focus == NULL_DIA_DESC)
			{/* try again--focus may have wrapped */
			    di->focus = (di->root->item->set_focus)(
				    di->root,
				    FOCUS_RESET
			    );
			}
			if (di->focus == dd)
			{/* there is only one focus */
			    (void) (di->root->item->set_focus)(
				    di->root,
				    FOCUS_CLEAR
			    );
			    di->focus = NULL_DIA_DESC;
			}
		    }
		    for (pl = dd->pane_list; pl; pl = pl->next)
		    {/* disable each real pane */
			display_disabled_pane(pl->p, false, true);
		    }
		}
	    }
	}
    }
}


/* Indicate a dialogue message.						*/
void dialog_message(Dialog *di, char *format, ...)
{
  va_list  arg_list;		/* the varargs argument list	*/
  char     buf[1024];		/* temporary message buffer	*/

  va_start(arg_list, format);
    {
      vsprintf(buf, format, arg_list);
    }
  va_end(arg_list);

  sm_message_new(di->msg_pane, "%s", buf);
  di->msg_empty = false;
}


/* Indicate a dialogue error.						*/
void dialog_error(Dialog *di, char *message, int num, ...)
{
  va_list		arg_list;		/* argument list as per varargs */
  DiaDesc		*dd;			/* the dialogue descriptor	*/
  Generic		item_id;		/* the error item		*/

  va_start(arg_list, num);
    {
    sm_message_new(di->msg_pane, "Error:  %s", message);
    di->msg_empty = false;

    while (di->error_dd)
    {/* remove the current errors */
	boxColor(di->w->image, di->error_dd->error_box,
		ERROR_BOX_WIDTH, NULL_BITMAP, Origin, UnusedRect, false,
		windowBackground(di->w), NULL_COLOR); /* change to boxPane if can find pane */
	touchPaneRect(
		di->msg_pane,
		subRect(
			di->error_dd->error_box,
			di->msg_pane->position
		)
	);
	di->error_dd = di->error_dd->next_error;
    }
    while (num--)
    {/* mark all of the items on the list as having errors */
	item_id = va_arg(arg_list, Generic);
	for (dd = di->registered_dd; dd; dd = dd->next_registered)
	{/* look for dd's with this item number */
	    if (dd->item_id == item_id)
	    {/* this is one */
		dd->next_error = di->error_dd;
		di->error_dd   = dd;
		boxColor(dd->owner->w->image,
			dd->error_box,
			ERROR_BOX_WIDTH, NULL_BITMAP, Origin, UnusedRect, false,
			windowForeground(dd->owner->w), NULL_COLOR);
			/* change to boxPane if can find correct pane */
		touchPaneRect(
			dd->owner->msg_pane,
			subRect(dd->error_box, dd->owner->msg_pane->position)
		);
	    }
	}
    }

    }
  va_end(arg_list);
}


/* Run a modal dialogue until quit.					*/
void dialog_modal_run(Dialog *di)
{
    di->forced = true;
    sm_desk_win_force(di->w, standard_cursor);
}


/* Run a modeless dialogue.						*/
void
dialog_modeless_show(Dialog *di)
{
    sm_desk_win_top(di->w);
}


/* Hide a modeless dialogue.						*/
void
dialog_modeless_hide(Dialog *di)
{
    sm_desk_win_hide(di->w);
}


/*************************** ITEM CALLBACKS *****************************/


/* Handle a change in an item.						*/
void
dialog_item_user_change(DiaDesc *dm)
{
DiaDesc		*dd;			/* the dialogue descriptor	*/
struct	p_list	*pl;			/* the pane list entry		*/
Dialog		*di = dm->owner;	/* the owning dialog		*/

    (dm->item->modified)(dm, true);
    if (NOT(dm->able))
    {/* the item is disabled */
	for (pl = dm->pane_list; pl; pl = pl->next)
	{/* repaint each pane and redisable */
	    display_disabled_pane(pl->p, true, true);
	}
    }
    for (dd = di->registered_dd; dd; dd = dd->next_registered)
    {/* walk down the dialogue list */
	if ((dd->item_id == dm->item_id) && (dd != dm))
	{/* this is an equivalent descriptor */
	    (dd->item->modified)(dd, false);
	    if (NOT(dd->able))
	    {/* the item is disabled */
		for (pl = dd->pane_list; pl; pl = pl->next)
		{/* repaint each pane and disable */
		    display_disabled_pane(pl->p, true, true);
		}
	    }
	}
    }
    handle_item_event(di, dm->item_id);
}


/* Register a dialogue item with the parent.				*/
Pane *
dialog_item_make_pane(DiaDesc *dd, short id, Point position, Point size, 
                      short border)
{
DiaDesc		*c;			/* the checking dd		*/
struct	p_list	*pl;			/* the new pane list entry	*/

    for (c = dd->owner->registered_dd; c && c != dd; c = c->next_registered)
    {/* look for this dialog descriptor in the registration table */
    }
    if (c == NULL_DIA_DESC)
    {/* this dialog descriptor has not been registered */
	dd->next_registered = dd->owner->registered_dd;
	dd->owner->registered_dd = dd;
    }
    pl = (struct p_list *) get_mem(
	    sizeof(struct p_list),
	    "new entry on the dialog descriptor's pane list"
    );
    pl->p	  = newPane(dd->owner->w, id, position, size, border);
    pl->size	  = size;
    pl->next	  = dd->pane_list;
    dd->pane_list = pl;
    return pl->p;
}


/* Redraw a pane.							*/
void
dialog_item_redraw_pane(DiaDesc *dd, Pane *p)
{
struct	p_list	*pl;			/* the current pane list entry	*/

    for (pl = dd->pane_list; pl; pl = pl->next)
    {/* search for the pane */
	if (pl->p == p)
	{/* redraw this pane */
	    resizePane(p, p->position, pl->size);
	    if (NOT(dd->able))
	    {/* grey out the pane */
		display_disabled_pane(p, false, NOT(dd->able));
	    }
	}
    }
}


/* Set the focus for an item which does not accept keyboard input.	*/
/*ARGSUSED*/
DiaDesc *
standardNoFocusSetter(DiaDesc *dd, Boolean set)
{
    return NULL_DIA_DESC;
}


/* Make a new dialogue descriptor.					*/
DiaDesc *
makeDialogDescriptor(Item *item, Generic item_info, Generic item_id)
{
DiaDesc		*dd;			/* the dialogue descriptor	*/

    dd = (DiaDesc *) get_mem(
	    sizeof(DiaDesc),
	    "new dialog descriptor (%s)",
	    item->name
    );
    dd->owner		= (Dialog *) 0;
    dd->item		= item;
    dd->item_info	= item_info;
    dd->item_id		= item_id;
    dd->pane_list	= (struct p_list *) 0;
    dd->error_box	= makeRectFromSize(Origin, Origin);
    dd->able		= true;
    dd->next_registered = NULL_DIA_DESC;
    dd->next_error	= NULL_DIA_DESC;
    return dd;
}


#define		ERROR_SLOP  2		/* # pixels outside pane er. box*/
/* Initialize dialogue descriptor.					*/
void
initDialogDescriptor(Dialog *di, DiaDesc *dd, Point ulc)
{
struct	p_list	*pl;			/* the pane list holder		*/

    dd->owner = di;
    (dd->item->initialize)(dd, ulc);

    if (dd->pane_list)	
    {/* there is a first pane--figure the error box */
	dd->error_box = makeRectFromSize(
		dd->pane_list->p->position,
		dd->pane_list->p->size
	);
	for (pl = dd->pane_list->next; pl; pl = pl->next)
	{/* add this pane's area to the error box */
	    dd->error_box = unionRect(
		    dd->error_box,
		    makeRectFromSize(pl->p->position, pl->p->size)
	    );
	}
	/* add a little room */
	    dd->error_box.ul.x -= ERROR_SLOP;
	    dd->error_box.ul.y -= ERROR_SLOP;
	    dd->error_box.lr.x += ERROR_SLOP;
	    dd->error_box.lr.y += ERROR_SLOP;
    }
}


/* Free a dialogue descriptor and all associated information.		*/
void
freeDialogDescriptor(DiaDesc *dd)
{
struct	p_list	*pl;			/* the pane list holder		*/

    (dd->item->destroy)(dd);
    while (dd->pane_list)
    {/* destroy all panes */
	pl = dd->pane_list;
	dd->pane_list = pl->next;
	destroyPane(pl->p);
	free_mem((void*) pl);
    }
    free_mem((void*) dd);
}


/************************** MANAGER CALLBACKS ***************************/


/* Set the manager to use for subsequent dialogue creation calls.	*/
Generic
dialog_mgr_use(aDiaMgr *dm)
{
aDiaMgr		*saved;			/* the saved handle		*/

    saved = current_dm;
    current_dm = dm;
    return ((Generic) saved);
}


/************************ MANAGER ENTRY POINTS **************************/


/* Start the dialogue manager.						*/
static
void
dia_start()
{
static BITMAPM_UNIT disabled_data[]  = {/* 'half' intensity bit pattern */
			0xA5A5, 0x5A5A, 0xA5A5, 0x5A5A,
			0xA5A5, 0x5A5A, 0xA5A5, 0x5A5A,
			0xA5A5, 0x5A5A, 0xA5A5, 0x5A5A,
			0xA5A5, 0x5A5A, 0xA5A5, 0x5A5A
		};

    disabled_bitmap = makeBitmapFromData(makePoint(16, 16), disabled_data, "dialog_mgr.c: dia_start()");
    message_sm = sm_message_get_index();
    vanilla_sm = sm_vanilla_get_index();
}


/* Finish the dialogue manager.						*/
static
void
dia_finish()
{
    freeBitmap(disabled_bitmap);
}


/* Create an instance of the dialogue manager.				*/
static
Generic
dia_create(Generic manager_id)
{
aDiaMgr		*m;			/* manager instance structure	*/

    m = (aDiaMgr *) get_mem(
	    sizeof(aDiaMgr),
	    "dia_create information structure"
    );
    m->manager_id = manager_id;
    m->dia_list	  = (Dialog *) 0;
    return ((Generic) m);
}


/* Destroy an instance of the dialogue manager.				*/
static
void
dia_destroy(aDiaMgr *m)
{
    while (m->dia_list)
    {/* there dialogue instances to kill */
	dialog_destroy(m->dia_list);
    }
    free_mem((void*) m);
}


/* Handle an event to the dialogue manager.				*/
static
void
dia_event(aDiaMgr *m)
{
Dialog		*di;			/* current dialog		*/
DiaDesc		*dd;			/* the dialogue descriptor	*/
struct	p_list	*pl;			/* the pane list entry		*/
Boolean		quit;			/* stop searching for this one	*/

    for (di = m->dia_list; di; di = di->next)
    {/* search for the approporiate dialog instance */
	if (di->w == (Window *) mon_event.from)
	{/* an event from the window itself */
	    switch (mon_event.type)
	    {/* figure based on the window event */
		case EVENT_KILL:
		    handle_item_event(di, DIALOG_CANCEL_ID);
		    break;

		case EVENT_SELECT:
		    if (di->forced)
		    {/* modal dialogues do default on window selection */
			handle_item_event(di, DIALOG_CANCEL_ID);
		    }
		    break;

		case EVENT_KEYBOARD:
		    if (toKbChar(mon_event.info.x) == KB_Enter)
		    {/* try to quit the dialogue */
			handle_item_event(di, DIALOG_DEFAULT_ID);
		    }
		    else if (toKbChar(mon_event.info.x) == KB_Linefeed)
		    {/* move the focus to the next position */
			di->focus = (di->root->item->set_focus)(
				di->root,
				FOCUS_RESET
			);
			if (di->focus == NULL_DIA_DESC)
			{/* try again--focus may have wrapped */
			    di->focus = (di->root->item->set_focus)(
				    di->root,
				    FOCUS_RESET
			    );
			}
		    }
		    else if (di->focus)
		    {/* a normal keyboard event */
			handle_dd_event(di, di->focus);
		    }
		    break;

		case EVENT_HELP:
		    quit = false;
		    for (dd = di->registered_dd; NOT(quit) && dd; dd = dd->next_registered)
		    {/* walk down the registered list */
			if (pointInRect(subPoint(mon_event.loc, di->w->border.ul), dd->error_box))
			{/* a help event in the region of this item */
			    handle_item_help(di, dd->item_id);
			    quit = true;
			}
		    }
		    if (NOT(quit))
		    {/* apparently not for one of the items */
			handle_item_help(di, DIALOG_UNUSED_ID);
		    }
		    break;
	    }
	    break;
	}
	else if (di->w == ((Pane *) mon_event.from)->parent)
	{/* an event from one of the panes */
	    if (di->msg_pane == (Pane *) mon_event.from)
	    {/* event in the message pane */
		if (mon_event.type == EVENT_HELP)
		{/* help event */
		    handle_item_help(di, DIALOG_UNUSED_ID);
		}
	    }
	    else
	    {/* try each item */
		quit = false;
		for (dd = di->registered_dd; NOT(quit) && dd; dd = dd->next_registered)
		{/* walk down the registered list */
		    for (pl = dd->pane_list; NOT(quit) && pl; pl = pl->next)
		    {/* walk down the pane list */
			if (pl->p == (Pane *) mon_event.from)
			{/* an event from the real pane */
			    if (mon_event.type == EVENT_HELP)
			    {/* help event */
				handle_item_help(di, dd->item_id);
			    }
			    else
			    {/* other event */
				handle_dd_event(di, dd);
			    }
			    quit = true;
			}
		    }
		}
	    }
	    break;
	}
    }
}


#define		SLOP	6		/* internal border		*/
/* Handle the creation of a new window.	 Return desired size.		*/
/*ARGSUSED*/
static
Point
dia_window_tile(aDiaMgr *m, Window *w, Dialog *di, Point ulc, Boolean New)
{
Point		dd_size;		/* size of the dialogue stuff	*/
Point		msg_size;		/* size of the message pane	*/

    /* get the desired sizes & adjust */
	dd_size	 = transPoint(
		(di->root->item->get_size)(di->root),
		makePoint(SLOP * 2, SLOP * 2)
	);
	msg_size = sm_message_pane_size(1, DEF_FONT_ID);
	msg_size.x = dd_size.x;

    /* create the dialog descriptor & message pane */
	di->w = w;
	initDialogDescriptor(
		di,
		di->root,
		transPoint(ulc, makePoint(SLOP, SLOP))
	);
	ulc.y += dd_size.y;
	di->msg_pane = newPane(w, message_sm, ulc, msg_size, 0);

    dd_size.y += msg_size.y;
    return (dd_size);
}




