/* $Id: yes_no.C,v 1.1 1997/06/25 14:47:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                              yes_no.c                                */
/*                            yes/no dialog                             */
/*                                                                      */
/************************************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/dialog_def.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/yes_no.h>
#include <libs/graphicInterface/oldMonitor/include/items/title.h>
#include <libs/graphicInterface/oldMonitor/include/items/button.h>

#define         YES_CHOICE      1       /* the yes choice               */
#define         NO_CHOICE       2       /* the no choice                */

/* Handle a modification to a yes/no dialog.                            */
/*ARGSUSED*/
static
Boolean
yes_no_handler(Dialog *di, Generic *owner, Generic item_id)
{
    *owner = item_id;       /* note the selected item */
    return DIALOG_QUIT;
}


/* Create a message window with text and returns false if the user      */
/*  cancells, and true if he makes a selection.                         */
Boolean
yes_no(char *prompt, Boolean *answer, Boolean def)
{
Dialog          *di;                    /* the dialog instance          */
Generic         selection;		/* the selected item		*/

    di = dialog_create(
            "yes / no",
            (dialog_handler_callback)yes_no_handler,
	    (dialog_helper_callback) 0,
            (Generic) &selection,
            dialog_desc_group(
                    DIALOG_HORIZ_CENTER,
                    2,
                    item_title(DIALOG_UNUSED_ID, prompt, DEF_FONT_ID),
                    dialog_desc_group(
                            DIALOG_VERT_CENTER,
                            2,
                            item_button(
                                    YES_CHOICE,
                                    "yes",
                                    DEF_FONT_ID,
                                    def
                            ),
                            item_button(
                                    NO_CHOICE,
                                    "no",
                                    DEF_FONT_ID,
                                    NOT(def)
                            )
                    )
            )
    );
    dialog_modal_run(di);
    dialog_destroy(di);

    switch (selection)
    {/* figure what to return */
        case YES_CHOICE:
            *answer = true;
            return true;
        case NO_CHOICE:
            *answer = false;
            return true;
        default:
            *answer = def;
            return BOOL(selection == DIALOG_DEFAULT_ID);
    }
}
