/* $Id: text_item.C,v 1.1 1997/06/25 14:48:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                                text.c                                */
/*                     1- and 2-dimensional text items                  */
/*                                                                      */
/************************************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/item_def.h>
#include <libs/graphicInterface/oldMonitor/include/items/text.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/ted_sm.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>

EXTERN(void,sm_ted_end_of_buffer,(Pane*));

    /*** LOCAL INFORMATION ***/

struct  text    {                       /* ITEM STARTUP INFORMATION     */
    char                *title;         /* title of the text            */
    short               font;           /* the font to use              */
    Pane                *title_pane;    /* the title pane of the layout */
    Pane                *edit_pane;     /* the ted pane of the layout   */
    char                **vptr;         /* the variable pointer         */
    Point               size;           /* size of edit pane in chars   */
    Boolean             is_focus;       /* true if we are the focus     */
                };


/************************** ITEM DEFINITION *****************************/


/* Get the pixel size of the text item.                                 */
static
Point
text_item_get_size(DiaDesc *dd)
{
struct  text    *info;                  /* the information parameter    */
Point           title_size;             /* the size of the title        */
Point           edit_size;              /* the size of the edit pane    */
Point           size;                   /* the overall size             */

    info = (struct text *) dd->item_info;

    edit_size  = sm_ted_pane_size(info->size, info->font);
    if (strlen(info->title) == 0)
        title_size = Origin;
    else
        title_size = sm_vanilla_pane_size(info->title, info->font);
    if( info->size.y == 1 )
    {/* title to left of text */
	size.x = edit_size.x + title_size.x;
	size.y = MAX(edit_size.y, title_size.y);
    }
    else
    {/* title above text */
	size.x = MAX(edit_size.x, title_size.x);
	size.y = edit_size.y + title_size.y;
    }

    return (size);
}


/* Place the item and initialize the pane.                              */
static
void
text_item_initialize(DiaDesc *dd, Point ulc)
{
Point           title_size;             /* the size of the title        */
Point           edit_size;              /* the size of the edit pane    */
struct  text    *info;                  /* the information parameter    */

    info = (struct text *) dd->item_info;

    /* get and adjust the pane sizes */
        edit_size   = sm_ted_pane_size(info->size, info->font);
        if (strlen(info->title) == 0)
	    title_size = Origin;
        else
            title_size  = sm_vanilla_pane_size(info->title, info->font);
        if (info->size.y == 1)
            edit_size.y = title_size.y = MAX(edit_size.y, title_size.y);
        else
            edit_size.x = title_size.x = MAX(edit_size.x, title_size.x);

    /* create and initialize the title pane */
        info->title_pane = dialog_item_make_pane(
                dd,
                sm_vanilla_get_index(),
                ulc,
                title_size,
                0
        );
        sm_vanilla_set_text(
                info->title_pane,
                info->title,
                info->font,
                STYLE_BOLD,
                VSM_JUSTIFY_CENTER
        );

    /* free the stored title */
        free_mem((void*) info->title);
        info->title = (char *) 0;

    /* create and initialize the edit pane */
        if (info->size.y == 1)
            ulc.x += title_size.x;
        else
            ulc.y += title_size.y;

        info->edit_pane = dialog_item_make_pane(
                dd,
                sm_ted_get_index(),
                ulc,
                edit_size,
                1
        );
        sm_ted_win_change_font(info->edit_pane, info->font);
        sm_ted_buf_set_text(info->edit_pane, *info->vptr, strlen(*info->vptr));
        sm_ted_end_of_buffer(info->edit_pane);
        sm_ted_win_active(info->edit_pane, false);
}


/* Handle the current event.                                            */
static
FocusStatus
text_item_handle_event(DiaDesc *dd)
{
struct  text    *info;                  /* the information parameter    */

    info = (struct text *) dd->item_info;

    if (mon_event.type == EVENT_SELECT)
    {/* a selection to our pane--move the focus here */
        if (mon_event.from == (Generic) info->edit_pane)
        {/* move the cursor at the same time */
            sm_ted_win_set_xy(info->edit_pane, mon_event.info);
            sm_ted_repair(info->edit_pane);
        }
        if (NOT(info->is_focus))
            dialog_item_user_change(dd);
        return FocusThis;
    }
    else if ((mon_event.type == EVENT_KEYBOARD) && info->is_focus)
    {/* a character to us */
        sm_ted_handle_keyboard(info->edit_pane, (KbChar) mon_event.info.x);
        dialog_item_user_change(dd);
    }
    return FocusOK;
}


/* Handle a focus modification.                                         */
static
DiaDesc *
text_item_set_focus(DiaDesc *dd, Boolean fs)
{
struct  text    *info;                  /* the information parameter    */

    info = (struct text *) dd->item_info;

    if (info->is_focus)
    {/* we currently have the focus--turn it off */
        info->is_focus = false;
        sm_ted_win_active(info->edit_pane, false);
        sm_ted_repair(info->edit_pane);
        return NULL_DIA_DESC;
    }
    else if ((fs == FOCUS_RESET) && dd->able)
    {/* we don't have the focus now--turn it on */
        info->is_focus = true;
        sm_ted_win_active(info->edit_pane, true);
        sm_ted_repair(info->edit_pane);
        return dd;
    }
    else
    {/* do not change status */
        return NULL_DIA_DESC;
    }
}


/* Handle a modification of the item.                                   */
static
void
text_item_modified(DiaDesc *dd, Boolean user)
{
struct  text    *info;                  /* the information parameter    */

    info = (struct text *) dd->item_info;

    if (user)
    {/* the user modified the value */
        free_mem((void*) *info->vptr);
        *info->vptr = sm_ted_buf_get_text(info->edit_pane);
    }
    else
    {/* the value was changed internally */
        sm_ted_buf_set_text(info->edit_pane, *info->vptr, strlen(*info->vptr));
        sm_ted_repair(info->edit_pane);
    }
}


/* Destroy item specific information.                                   */
/*ARGSUSED*/
static
void
text_item_destroy(DiaDesc *dd)
{
    free_mem((void*)dd->item_info);
}


static  Item    text_item = {           /* the text item                */
                        "text",
                        text_item_get_size,
                        (item_initialize_func)text_item_initialize,
                        text_item_handle_event,
                        text_item_set_focus,
                        text_item_modified,
                        text_item_destroy
                };

/************************* EXTERNAL ROUTINES ****************************/


/* Create a dialog descriptor for a 1-dimensional text item.            */
DiaDesc *
item_text(Generic item_id, char *title, short font, char **vptr, short width)
{
    return item_text2(item_id, title, font, vptr, makePoint(width, 1));
}




/* Create a dialog descriptor for a 2-dimensional text item.            */
DiaDesc *
item_text2(Generic item_id, char *title, short font, char **vptr, Point size)
{
struct  text    *info;                  /* the text information         */

    if (title == (char *) 0)
	title = "";
    info = (struct text *) get_mem(
            sizeof(struct text),
            "text item text information"
    );
    info->title      = ssave(title);
    info->font       = font;
    info->title_pane = NULL_PANE;
    info->edit_pane  = NULL_PANE;
    info->vptr       = vptr;
    info->size       = size;
    info->is_focus   = false;

    return (
        makeDialogDescriptor(
                &text_item,
                (Generic) info,
                item_id
        )
    );
}


/* Handle an "out of band" keyboard event to the item.			*/
void
item_text_handle_keyboard(DiaDesc *dd, KbChar ch, Boolean ignoreFocus)
{
struct  text    *info;                  /* the text information         */

    info = (struct text *) dd->item_info;

    if (ignoreFocus || info->is_focus)
    {/* we should handle this */
	sm_ted_handle_keyboard(info->edit_pane, ch);
        dialog_item_user_change(dd);
    }
}


/* Set the x and y dot location in a text item.				*/
void
item_text_set_xy(DiaDesc *dd, Point pt)
{
struct  text    *info;                  /* the text information         */

    info = (struct text *) dd->item_info;

    sm_ted_win_set_xy(info->edit_pane, pt);
    dialog_item_redraw_pane(dd, info->edit_pane);
}

