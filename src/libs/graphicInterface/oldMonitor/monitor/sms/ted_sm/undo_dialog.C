/* $Id: undo_dialog.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                              undo_dialog.c                           */
/*                         			                        */
/*                                                                      */
/************************************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/dialog_def.h>
#include <libs/graphicInterface/oldMonitor/include/items/title.h>
#include <libs/graphicInterface/oldMonitor/include/items/button.h>
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/undo_dialog.h>

struct undia {
	Generic result;
	Dialog  *undo_dialog;
	};


/* Handle a modification to a yes/no dialog.                            */
/*ARGSUSED*/
static
Boolean
undo_handler(Dialog *di, Undia *owner, Generic item_id)
/*Dialog          *di;                    the dialog instance          */
/*Undia           *owner;                 the selection variable       */
/*Generic         item_id;                the id of the item           */
{
    owner->result = item_id;       /* note the selected item */
    return DIALOG_QUIT;
}


/* Create a message window with text and returns false if the user      */
/*  cancells, and true if he makes a selection.                         */
Boolean
undo_confirm(Undia *und)
{
    dialog_modal_run(und->undo_dialog);
    dialog_modeless_show (und->undo_dialog);
    return BOOL(und->result == DIALOG_DEFAULT_ID);
}

/* Create a undo dialog.					*/
Undia *
undo_dialog_create(char *title)
{
Undia		*und;			/* the undo dialog	*/

    /* create and initialize the dialog */
	und = (Undia *) get_mem(sizeof(Undia), "undo dialog");

        und->undo_dialog = dialog_create(
            title,
            (dialog_handler_callback)undo_handler,
	    (dialog_helper_callback) 0,
            (Generic) und,
            dialog_desc_group(
                    DIALOG_HORIZ_CENTER,
                    2,
                    item_title(DIALOG_UNUSED_ID, "undo more?", DEF_FONT_ID),
                    item_button(
                            DIALOG_DEFAULT_ID,
                            "yes",
                            DEF_FONT_ID,
                            true
                    )
            )
        );

	
    return (und);
}

/* Run a undo dialog.							*/
void
undo_dialog_hide(Undia *und)
{
    dialog_modeless_hide(und->undo_dialog);
}


/* Destroy a undo dialog.					*/
void
undo_dialog_destroy(Undia *und)
{
    dialog_destroy(und->undo_dialog);
    free_mem((void*) und);
}
