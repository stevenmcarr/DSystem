/* $Id: expand_item.C,v 1.1 1997/06/25 14:50:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                            expand_item.c                             */
/*                             expand item                              */
/*                                                                      */
/************************************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/item_def.h>


    /*** LOCAL INFORMATION ***/

struct  expand {                        /* EXPAND INFORMATION STRUCT.   */
    DiaDesc             *dd;            /* the dd that we are expanding */
    Point               slop;           /* the top-left slop amount     */
                };


/************************** ITEM DEFINITION *****************************/


/* Get the pixel size of the expand item.                               */
static
Point
expand_item_get_size(DiaDesc *dd)
{
struct  expand  *info;                  /* the information parameter    */

    info = (struct expand *) dd->item_info;

    return (
        transPoint(
                (info->dd->item->get_size)(info->dd),
                transPoint(
                        info->slop,
                        info->slop
                )
        )
    );
}


/* Place the item and initialize the pane.                              */
static
void
expand_item_initialize(DiaDesc *dd, Point ulc)
{
struct  expand  *info;                  /* the information parameter    */

    info = (struct expand *) dd->item_info;

    initDialogDescriptor(dd->owner, info->dd, transPoint(ulc, info->slop));
}


/* Handle the current event.                                            */
/*ARGSUSED*/
static
FocusStatus
expand_item_handle_event(DiaDesc *dd)
{
    die_with_message("expand_item_handle_event():  why is this called?");
    return FocusOK;
}


/* Set the focus.                                                       */
static
DiaDesc *
expand_item_set_focus(DiaDesc *dd, Boolean fs)
{
struct  expand  *info;                  /* the information parameter    */

    info = (struct expand *) dd->item_info;

    return (info->dd->item->set_focus)(info->dd, fs);
}


/* Handle a modification of the item.                                   */
/*ARGSUSED*/
static
void
expand_item_modified(DiaDesc *dd, Boolean user)
{
    die_with_message("expand_item_modified():  why is this called?");   
}


/* Destroy item specific information.                                   */
/*ARGSUSED*/
static
void
expand_item_destroy(DiaDesc *dd)
{
struct  expand  *info;                  /* the information parameter    */

    info = (struct expand *) dd->item_info;

    freeDialogDescriptor(info->dd);
    free_mem((void*)dd->item_info);
}


static  Item    expand_item = {         /* the expand item              */
                        "expand",
                        expand_item_get_size,
                        (item_initialize_func)expand_item_initialize,
                        expand_item_handle_event,
                        expand_item_set_focus,
                        expand_item_modified,
                        expand_item_destroy
                };


/************************* EXTERNAL ROUTINES ****************************/


/* Create a dialog descriptor for a expand.                             */
DiaDesc *
dialog_desc_expand(DiaDesc *dd)
{
struct  expand  *info;                  /* the expand information       */

    info = (struct expand *) get_mem(
            sizeof(struct expand),
            "expand item expand information"
    );
    info->dd   = dd;
    info->slop = fontSize(DEF_FONT_ID);
    return (
        makeDialogDescriptor(
                &expand_item,
                (Generic) info,
                DIALOG_UNUSED_ID
        )
    );
}
