/* $Id: check_box.C,v 1.1 1997/06/25 14:48:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                              check_box.c                             */
/*                            check box item                            */
/*                                                                      */
/************************************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/item_def.h>
#include <libs/graphicInterface/oldMonitor/include/items/check_box.h>
#include <libs/graphicInterface/oldMonitor/include/sms/check_sm.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>


    /*** LOCAL INFORMATION ***/

struct  cb_item {                       /* ITEM STARTUP INFORMATION     */
    char                *title;         /* the title of the check box   */
    short               font;           /* the font to use              */
    Pane                *p;             /* the check pane               */
    Boolean             *vptr;          /* the variable pointer         */
    Boolean             value;          /* the current value            */
                };


/************************** ITEM DEFINITION *****************************/


/* Get the pixel size of the check box item.                            */
static
Point
check_box_item_get_size(DiaDesc *dd)
{
struct  cb_item *info;                  /* the information parameter    */

    info = (struct cb_item *) dd->item_info;
    return (sm_check_pane_size(info->title, info->font, csmWithBox));
}


/* Place the item and initialize the pane.                              */
static
void
check_box_item_initialize(DiaDesc *dd, Point ulc)
{
struct  cb_item *info;                  /* the information parameter    */

    info = (struct cb_item *) dd->item_info;

    info->p = dialog_item_make_pane(
            dd,
            sm_check_get_index(),
            ulc,
            check_box_item_get_size(dd),
            0
    );
    sm_check_set_text(
            info->p,
            info->title,
            info->font,
            STYLE_BOLD,
            csmLeft,
            csmWithBox,
            *info->vptr
    );
    free_mem((void*) info->title);
    info->title = (char *) 0;
    info->value = *info->vptr;
}


/* Handle the current event.                                            */
static
FocusStatus
check_box_item_handle_event(DiaDesc *dd)
{
struct  cb_item *info;                  /* the information parameter    */

    info = (struct cb_item *) dd->item_info;

    if (mon_event.type == EVENT_SELECT)
    {/* a selection to our pane */
        *info->vptr = NOT(*info->vptr);
        dialog_item_user_change(dd);
    }
    return FocusOK;
}


/* Handle a modification of the item.                                   */
/*ARGSUSED*/
static
void
check_box_item_modified(DiaDesc *dd, Boolean user)
{
struct  cb_item *info;                  /* the information parameter    */

    info = (struct cb_item *) dd->item_info;

    if (info->value != *info->vptr)
    {/* the value was changed */
        info->value = NOT(info->value);
        sm_check_change_check(info->p,*info->vptr);
    }
}


/* Destroy item specific information.                                   */
/*ARGSUSED*/
static
void
check_box_item_destroy(DiaDesc *dd)
{
    free_mem((void*)dd->item_info);
}


static  Item    check_box_item = {      /* the check box item           */
                        "check box",
                        check_box_item_get_size,
                        (item_initialize_func)check_box_item_initialize,
                        check_box_item_handle_event,
                        standardNoFocusSetter,
                        check_box_item_modified,
                        check_box_item_destroy
                };

/************************* EXTERNAL ROUTINES ****************************/


/* Create a dialog descriptor for a check box.                          */
DiaDesc *
item_check_box(Generic item_id, char *title, short font, Boolean *vptr)
{
struct  cb_item *info;                  /* the cb_item information      */

    info = (struct cb_item *) get_mem(
            sizeof(struct cb_item),
            "check box item cb_item information"
    );
    info->title = ssave(title);
    info->font  = font;
    info->vptr  = vptr;
    info->p     = NULL_PANE;
    info->value = *vptr;
    return (
        makeDialogDescriptor(
                &check_box_item,
                (Generic) info,
                item_id
        )
    );
}
