/* $Id: title.C,v 1.1 1997/06/25 14:48:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                               title.c                                */
/*                             title item                               */
/*                                                                      */
/************************************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/item_def.h>
#include <libs/graphicInterface/oldMonitor/include/items/title.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>


    /*** LOCAL INFORMATION ***/

struct  title {                         /* MANAGER INFORMATION STRUCT.  */
    char                *Title;         /* the title of the title       */
    short               font;           /* the font to use              */
    Pane                *p;             /* the vanilla title pane       */
    Boolean             justify;        /* text justification           */
                };


/************************** ITEM DEFINITION *****************************/


/* Get the pixel size of the title item.                                */
static
Point
title_item_get_size(DiaDesc *dd)
{
struct  title   *info;                  /* the information parameter    */

    info = (struct title *) dd->item_info;
    return (sm_vanilla_pane_size(info->Title, info->font));
}


/* Place the item and initialize the pane.                              */
static
void
title_item_initialize(DiaDesc *dd, Point ulc)
{
struct  title   *info;                  /* the information parameter    */

    info = (struct title *) dd->item_info;

    info->p = dialog_item_make_pane(
            dd,
            sm_vanilla_get_index(),
            ulc,
            title_item_get_size(dd),
            0
    );
    sm_vanilla_set_inversion(info->p, VSM_NORMAL_BKGND, VSM_NORMAL_TRACK);
    sm_vanilla_set_text(
            info->p,
            info->Title,
            info->font,
            STYLE_BOLD,
            info->justify
    );
}


/* Handle the current event.                                            */
/*ARGSUSED*/
static
FocusStatus
title_item_handle_event(DiaDesc *dd)
{
    return FocusOK; 
}


/* Handle a modification of the item.                                   */
/*ARGSUSED*/
static
void
title_item_modified(DiaDesc *dd, Boolean user)
{
}


/* Destroy item specific information.                                   */
/*ARGSUSED*/
static
void
title_item_destroy(DiaDesc *dd)
{
struct  title   *info;                  /* the information parameter    */

    info = (struct title *) dd->item_info;
    
    sfree(info->Title);
    free_mem((void*) info);
}


static  Item    title_item = {          /* the title item               */
                        "title",
                        title_item_get_size,
                        (item_initialize_func)title_item_initialize,
                        title_item_handle_event,
                        standardNoFocusSetter,
                        title_item_modified,
                        title_item_destroy
                };

/************************* EXTERNAL ROUTINES ****************************/


/* Create a dialog descriptor for a title.                              */
DiaDesc *
item_title(Generic item_id, char *Title, short font)
{
struct  title   *info;                  /* the title information        */

    info = (struct title *) get_mem(
            sizeof(struct title),
            "title item information"
    );
    info->Title   = ssave(Title);
    info->font    = font;
    info->justify = VSM_JUSTIFY_CENTER;
    info->p       = NULL_PANE;
    return (
        makeDialogDescriptor(
                &title_item,
                (Generic) info,
                item_id
        )
    );
}


/* Change the title displayed by the title item.			*/
void
item_title_change(DiaDesc *dd, char *Title)
{
struct	title	*info;			/* the title information	*/

    info = (struct title *) dd->item_info;

    sm_vanilla_set_text(
            info->p,
            Title,
            info->font,
            STYLE_BOLD,
            info->justify
    );
    dialog_item_redraw_pane(dd, info->p);
}


/* Left justify the title.						*/
void
item_title_justify_left(DiaDesc *dd)
{
struct	title	*info;			/* the title information	*/

    info = (struct title *) dd->item_info;

    info->justify = VSM_JUSTIFY_LEFT;
    if (info->p)
    {/* there is a pane to modify */
	sm_vanilla_set_text(
		info->p,
		(char *) 0,
		info->font,
		STYLE_BOLD,
		info->justify
	);
	dialog_item_redraw_pane(dd, info->p);
    }
}


/* Center justify the title.						*/
void
item_title_justify_center(DiaDesc *dd)
{
struct	title	*info;			/* the title information	*/

    info = (struct title *) dd->item_info;

    info->justify = VSM_JUSTIFY_CENTER;
    if (info->p)
    {/* there is a pane to modify */
	sm_vanilla_set_text(
		info->p,
		(char *) 0,
		info->font,
		STYLE_BOLD,
		info->justify
	);
	dialog_item_redraw_pane(dd, info->p);
    }
}
