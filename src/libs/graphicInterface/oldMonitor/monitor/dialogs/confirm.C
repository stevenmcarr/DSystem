/* $Id: confirm.C,v 1.1 1997/06/25 14:47:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Id: con
   confirm.c,v 1.2 92/10/03 16:15:57 rn Exp Locker: patton $ */
/************************************************************************/
/*                                                                      */
/*                              confirm.c                               */
/*                         Confirmation dialog                          */
/*                                                                      */
/************************************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/dialog_def.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/confirm.h>
#include <libs/graphicInterface/oldMonitor/include/items/title.h>
#include <libs/graphicInterface/oldMonitor/include/items/button.h>

STATIC(Boolean, confirm_handler, (Dialog* di, Generic* owner, Generic item_id));

/* Handle a modification to a confirm dialog.                           */
static Boolean confirm_handler(Dialog* di, Generic* owner, Generic item_id)
{
    *owner = item_id;       /* note the selected item */
    return DIALOG_QUIT;
}


/* Create a message window with text and returns false if the user      */
/*  cancells, and true if he makes a selection.                         */
Boolean confirm(char* prompt, char* button)
{
Dialog          *di;                    /* the dialog instance          */
Generic         selection;              /* the selected item            */

    di = dialog_create(
            "confirmation",
            (dialog_handler_callback)confirm_handler,
	    (dialog_helper_callback) 0,
            (Generic) &selection,
            dialog_desc_group(
                    DIALOG_HORIZ_CENTER,
                    2,
                    item_title(DIALOG_UNUSED_ID, prompt, DEF_FONT_ID),
                    item_button(
                            DIALOG_DEFAULT_ID,
                            button,
                            DEF_FONT_ID,
                            true
                    )
            )
    );
    dialog_modal_run(di);
    dialog_destroy(di);

    return BOOL(selection == DIALOG_DEFAULT_ID);
}
