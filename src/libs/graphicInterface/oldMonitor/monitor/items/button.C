/* $Id: button.C,v 1.1 1997/06/25 14:48:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                               button.c                               */
/*                             button item                              */
/*                                                                      */
/************************************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/item_def.h>
#include <libs/graphicInterface/oldMonitor/include/items/button.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>


    /*** LOCAL INFORMATION ***/

#define         EMPH_BORDER     4       /* emphisized border            */
#define         NORMAL_BORDER   2       /* normal border                */
struct  startup {                       /* MANAGER INFORMATION STRUCT.  */
    char                *title;         /* the title of the button      */
    short               font;           /* the font to use              */
    Boolean             emph;           /* emphisize the button         */
                };


/************************** ITEM DEFINITION *****************************/


/* Get the pixel size of the button item.                               */
static
Point
button_item_get_size(DiaDesc *dd)
{
Point           size;                   /* the size of the pane         */
struct  startup *info;                  /* the information parameter    */

    info = (struct startup *) dd->item_info;
    size = sm_vanilla_pane_size(info->title, info->font);
    if (info->emph)
    {/* add on emphisized border */
        size.x += 2 * EMPH_BORDER;
        size.y += 2 * EMPH_BORDER;
    }
    else
    {/* add on the normal border */
        size.x += 2 * NORMAL_BORDER;
        size.y += 2 * NORMAL_BORDER;
    }
    return size;
}


/* Place the item and initialize the pane.                              */
static
void
button_item_initialize(DiaDesc *dd, Point ulc)
{
struct  startup *info;                  /* the information parameter    */
Pane            *p;                     /* the vanilla pane             */

    info = (struct startup *) dd->item_info;

    p = dialog_item_make_pane(
            dd,
            sm_vanilla_get_index(),
            ulc,
            button_item_get_size(dd),
            (info->emph) ?  EMPH_BORDER : NORMAL_BORDER
    );
    sm_vanilla_set_inversion(p, VSM_NORMAL_BKGND, VSM_INVERT_TRACK);
    sm_vanilla_set_text(
            p,
            info->title,
            info->font,
            STYLE_BOLD,
            VSM_JUSTIFY_CENTER
    );
    sfree(info->title);
    free_mem((void*) info);
    dd->item_info = 0;
}


/* Handle the current event.                                            */
static
FocusStatus
button_item_handle_event(DiaDesc *dd)
{
    if (mon_event.type == EVENT_SELECT)
    {/* a selection to our pane */
        dialog_item_user_change(dd);
    }
    return FocusOK;
}


/* Handle a modification of the item.                                   */
/*ARGSUSED*/
static
void
button_item_modified(DiaDesc *dd, Boolean user)
{
}


/* Destroy item specific information.                                   */
/*ARGSUSED*/
static
void
button_item_destroy(DiaDesc *dd)
{
}


static  Item    button_item = {         /* the button item              */
                        "button",
                        button_item_get_size,
                        (item_initialize_func)button_item_initialize,
                        button_item_handle_event,
                        standardNoFocusSetter,
                        button_item_modified,
                        button_item_destroy
                };

/************************* EXTERNAL ROUTINES ****************************/


/* Create a dialog descriptor for a button.                             */
DiaDesc *
item_button(Generic item_id, char *title, short font, Boolean emph)
{
struct  startup *info;                  /* the startup information      */

    info = (struct startup *) get_mem(
            sizeof(struct startup),
            "button item startup information"
    );
    info->title = ssave(title);
    info->font  = font;
    info->emph  = emph;
    return (
        makeDialogDescriptor(
                &button_item,
                (Generic) info,
                item_id
        )
    );
}
