/* $Id: radio_btns.C,v 1.1 1997/06/25 14:48:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                               radio_btns.c                           */
/*                            radio buttons item                        */
/*                                                                      */
/************************************************************************/

#include <stdarg.h>
#include <libs/graphicInterface/oldMonitor/include/mon/item_def.h>
#include <libs/graphicInterface/oldMonitor/include/items/radio_btns.h>
#include <libs/graphicInterface/oldMonitor/include/sms/button_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>


    /*** LOCAL INFORMATION ***/

struct  rb_item {                       /* ITEM STARTUP INFORMATION     */
    char                *title;         /* title of the radio buttons   */
    short               font;           /* the font to use              */
    Pane                *title_pane;    /* the title pane of the layout */
    Pane                *button_pane;   /* the button pane of the layout*/
    aLayoutDef          ld;             /* the button layout definition */
    Generic             layout;         /* the button layout            */
    Generic             *vptr;          /* the variable pointer         */
    Generic             value;          /* the current value            */
                };


/************************** ITEM DEFINITION *****************************/


/* Get the pixel size of the radio buttons item.                        */
static
Point
radio_buttons_item_get_size(DiaDesc *dd)
{
struct  rb_item *info;                  /* the information parameter    */
Point           button_size;            /* the size of the button layout*/
Point           title_size;             /* the size of the title        */
Point           size;                   /* the overall size             */

    info = (struct rb_item *) dd->item_info;

    button_size = sm_button_layout_size(&info->ld, info->font);
    if (strlen(info->title) == 0)
        title_size = Origin;
    else
        title_size = sm_vanilla_pane_size(info->title, info->font);
    size.x = MAX(button_size.x, title_size.x);
    size.y = button_size.y + title_size.y;
    return (size);
}


/* Place the item and initialize the pane.                              */
static
void
radio_buttons_item_initialize(DiaDesc *dd, Point ulc)
{
Point           button_size;            /* the size of the button layout*/
Point           title_size;             /* the size of the title        */
aButtonDef      *button;                /* the button definition        */
short           i, j;                   /* dummy indices                */
struct  rb_item *info;                  /* the information parameter    */

    info = (struct rb_item *) dd->item_info;

    /* get and adjust the pane sizes */
        button_size = sm_button_layout_size(&info->ld, info->font);
        if (strlen(info->title) == 0)
            title_size = Origin;
        else
            title_size = sm_vanilla_pane_size(info->title, info->font);
        button_size.x = title_size.x = MAX(button_size.x, title_size.x);

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
        sfree(info->title);
        info->title = (char *) 0;

    /* create and initialize the button pane */
        ulc.y += title_size.y;
        info->button_pane = dialog_item_make_pane(
                dd,
                sm_button_get_index(),
                ulc,
                button_size,
                2
        );
        info->layout = sm_button_layout_create(
                info->button_pane,
                &info->ld,
                info->font,
                true
        );
        sm_button_layout_show(info->button_pane, (Window*)info->layout);

    /* free the layout descriptor */
        button = info->ld.buttons;
        for (j = 0; j < info->ld.size.y; j++)
        {/* walk down the rows */
            for (i = 0; i < info->ld.size.x; i++)
            {/* walk down the columns */
                if (button->num_faces)
                {/* there is face information to free here */
                    free_mem((void*) button->face_list->displayed_text);
                    free_mem((void*) button->face_list);
                }
                button++;
            }
        }
        free_mem((void*) info->ld.buttons);
        info->ld.size.x = info->ld.size.y = 0;
        info->ld.buttons = (aButtonDef *) 0;

    /* set the correct value */
        info->value = *info->vptr;
        sm_button_modify_button(
                info->button_pane,
                (Window*)info->layout,
                info->value,
                true,
                true
        );
}


/* Handle the current event.                                            */
static
FocusStatus
radio_buttons_item_handle_event(DiaDesc *dd)
{
struct  rb_item *info;                  /* the information parameter    */

    info = (struct rb_item *) dd->item_info;

    if (
        (mon_event.from == (Generic) info->button_pane) &&
        (mon_event.type == EVENT_SELECT)
    )
    {/* a selection to our pane */
        *info->vptr = mon_event.msg;
        dialog_item_user_change(dd);
    }
    return FocusOK;
}


/* Handle a modification of the item.                                   */
/*ARGSUSED*/
static
void
radio_buttons_item_modified(DiaDesc *dd, Boolean user)
{
struct  rb_item *info;                  /* the information parameter    */

    info = (struct rb_item *) dd->item_info;

    if (info->value != *info->vptr)
    {/* the value was changed */
        sm_button_modify_button(
                info->button_pane,
                (Window*)info->layout,
                info->value,
                false,
                true
        );
        info->value = *info->vptr;
        sm_button_modify_button(
                info->button_pane,
                (Window*)info->layout,
                info->value,
                true,
                true
        );
    }
}


/* Destroy item specific information.                                   */
/*ARGSUSED*/
static
void
radio_buttons_item_destroy(DiaDesc *dd)
{
struct  rb_item *info;                  /* the information parameter    */

    info = (struct rb_item *) dd->item_info;
    sm_button_layout_destroy(info->button_pane, (Window*)info->layout);
    free_mem((void*)dd->item_info);
}


static  Item    radio_buttons_item = {  /* the radio buttons item       */
                        "radio buttons",
                        radio_buttons_item_get_size,
                        (item_initialize_func)radio_buttons_item_initialize,
                        radio_buttons_item_handle_event,
                        standardNoFocusSetter,
                        radio_buttons_item_modified,
                        radio_buttons_item_destroy
                };

/************************* EXTERNAL ROUTINES ****************************/

/* Create a dialog descriptor for radio buttons.                        */
DiaDesc *
item_radio_buttons(Generic item_id, char *title, short font, Generic *vptr,
                   int cols, int rows, ...)
{
va_list         arg_list;               /* argument list as per varargs */
char            *text;
struct  rb_item *info;                  /* the rb_item information      */
aButtonDef      *button;                /* the button definition        */
short           i, j;                   /* dummy indices                */

    va_start(arg_list, rows);

    /* create and initialize the information structure */
        info = (struct rb_item *) get_mem(
                sizeof(struct rb_item),
                "radio buttons item rb_item information"
        );
        if (title == (char *) 0)
            title = "";
        info->title       = ssave(title);
        info->font        = font;
        info->title_pane  = NULL_PANE;
        info->button_pane = NULL_PANE;
        info->vptr        = vptr;
        info->ld.size.x   = cols;
        info->ld.size.y   = rows;
        info->value       = *info->vptr;
        info->ld.buttons  = (aButtonDef *) get_mem(
                info->ld.size.x * info->ld.size.y * sizeof(aButtonDef),
                "radio button button definition"
        );

    /* initialize the layout description */
        button = info->ld.buttons;
        for (j = 0; j < info->ld.size.y; j++)
        {/* do the button row */
            for (i = 0; i < info->ld.size.x; i++)
            {/* do the button column */
                text       = va_arg(arg_list, char *);
                button->id = va_arg(arg_list, Generic);
                if (text == DIALOG_NO_TITLE)
                {/* there is no text for this positon */
                    button->num_faces = 0;
                    button->face_list = (aFaceDef *) 0;
                    button->id        = UNUSED;
                }
                else
                {/* there is text here */
                    button->num_faces = 1;
                    button->face_list =  (aFaceDef *) get_mem(
                            sizeof(aFaceDef),
                            "radio button face definition"
                    );
                    button->face_list->displayed_text = strcpy(
                            (char *) get_mem(
                                    strlen(text) + 1,
                                    "copy button name"
                            ),
                            text
                    );
                    button->face_list->help_text = (char *) 0;
                }
                button++;
            }
        }

    va_end(arg_list);
    return (
        makeDialogDescriptor(
                &radio_buttons_item,
                (Generic) info,
                item_id
        )
    );
}
