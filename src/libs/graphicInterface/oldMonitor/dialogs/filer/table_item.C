/* $Id: table_item.C,v 1.1 1997/06/25 14:44:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                               table_item.c                           */
/*                                table item                            */
/*                                                                      */
/************************************************************************/

#include <string.h>

#include <libs/graphicInterface/oldMonitor/include/mon/item_def.h>
#include <libs/graphicInterface/oldMonitor/dialogs/filer/table_item.h>
#include <libs/graphicInterface/oldMonitor/dialogs/filer/table_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/scroll_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>

 /*** LOCAL INFORMATION ***/

struct table
{				       /* ITEM STARTUP INFORMATION     */
    short           font;	       /* the font to use              */
    Pane           *table_pane;
    Pane           *horiz_pane;
    Point           size;	       /* the size of the table pane   */
    short          *numentries;
    char         ***entries;
    short          *selection;
    Boolean        *redraw_table;
};

STATIC(Point, table_item_get_size, (DiaDesc *dd));
STATIC(void, table_item_initialize, (DiaDesc *dd, Point ulc));
STATIC(FocusStatus, table_item_handle_event, (DiaDesc *dd));
STATIC(void, table_item_modified, (DiaDesc *dd, Boolean user));
STATIC(void, table_item_destroy, (DiaDesc *dd));
STATIC(void, table_scroller, (Pane *scrollee, Pane *sb, short direction, int curval));

/************************** ITEM DEFINITION *****************************/


/* Get the pixel size of the table item.                                   */
static Point table_item_get_size(DiaDesc *dd)
{
    struct table   *info;	       /* the information parameter    */
    Point           table_size;	       /* the size of the table pane   */

    info = (struct table *) dd->item_info;

    table_size = sm_table_pane_size(info->size, info->font);
    table_size.x += SB_WIDTH - 1;
    table_size.y += SB_WIDTH - 1;

    return (table_size);
}


/* Place the item and initialize the pane.                              */
static void table_item_initialize(DiaDesc *dd, Point ulc)
{
    Point           table_size;	       /* the size of the table pane      */
    struct table   *info;	       /* the information parameter    */
    Point           pane_loc;

    info = (struct table *) dd->item_info;

 /* get and adjust the pane sizes */
    table_size = sm_table_pane_size(info->size, info->font);

    pane_loc = ulc;

 /* create and initialize the table pane */
    info->table_pane = dialog_item_make_pane(
	dd,
	sm_table_get_index(),
	ulc,
	table_size,
	2);

    pane_loc = ulc;
    pane_loc.y += table_size.y - 1;
    info->horiz_pane = dialog_item_make_pane(dd,
	sm_scroll_get_index(),
	pane_loc,
	makePoint(table_size.x, SB_WIDTH),
	2);

    sm_table_initialize(info->table_pane, info->font);

    sm_scroll_set_step(info->horiz_pane, 1, 8);
    sm_scroll_scrollee(info->horiz_pane, (Generic) info->table_pane, 
                       (sm_scroll_scrollee_callback)table_scroller);
    sm_scroll_activate(info->horiz_pane,  true);
}


/* Handle the current event.                                            */
static FocusStatus table_item_handle_event(DiaDesc *dd)
{
    struct table   *info;	       /* the information parameter    */
    short           fullwidth,
                    viewwidth,
                    offset;

    info = (struct table *) dd->item_info;

    if ((mon_event.type == EVENT_SELECT) &&
	(mon_event.from == (Generic) info->table_pane))
    {	
	*info->selection = sm_table_get_selection(info->table_pane);
	dialog_item_user_change(dd);
    }

    else if ((mon_event.type == EVENT_MOVE) &&
	    (mon_event.from == (Generic) info->table_pane))
    {
	sm_table_shift_relative(info->table_pane, mon_event.info.x);
	sm_table_configuration(info->table_pane, &fullwidth,&viewwidth, &offset);

	if (viewwidth >= fullwidth)
	    sm_scroll_set(info->horiz_pane, 0, 0, offset, true);
	else
	    sm_scroll_set(info->horiz_pane, 0, fullwidth - viewwidth, offset, true);
    }
    return FocusOK;
}


/* Handle a modification of the item.                                   */
static void table_item_modified(DiaDesc *dd, Boolean user)
{
    struct table   *info;	       /* the information parameter    */
    short           fullwidth,
                    viewwidth,
                    offset;

    info = (struct table *) dd->item_info;

    /* I may have a new table to display, or I may need to just redraw
       the one I have */

    if (NOT(*info->redraw_table))	/* a new table */
	sm_table_new_table(info->table_pane, *info->numentries, *info->entries, *info->selection);
    else
	sm_table_update_table(info->table_pane, *info->numentries, *info->entries, *info->selection);

    sm_table_configuration(info->table_pane, &fullwidth, &viewwidth, &offset);

    if (viewwidth >= fullwidth)
	sm_scroll_set(info->horiz_pane, 0, 0, offset, true);
    else
	sm_scroll_set(info->horiz_pane, 0, fullwidth - viewwidth, offset, true);
}

/* Destroy item specific information. */
static void table_item_destroy(DiaDesc *dd)
{
    free_mem((void*)dd->item_info);
}


static Item     table_item = {	       /* the table item                  */
    "table",
    (item_get_size_func)table_item_get_size,
    (item_initialize_func)table_item_initialize,
    (item_handle_event_func)table_item_handle_event,
    (item_set_focus_func)standardNoFocusSetter,
    (item_modified_func)table_item_modified,
    (item_destroy_func)table_item_destroy
};


/************************* EXTERNAL ROUTINES ****************************/


/* Create a dialog descriptor for the table item.                       */
DiaDesc* item_table(Generic item_id, short font, short *numentries, char ***entries, 
                    short *selection, Boolean *redraw_table, Point size)
{
    struct table   *info;	       /* the table information           */

    info = (struct table *) get_mem(
	sizeof(struct table),
	"table item information");

    info->font = font;

    info->table_pane   = NULL_PANE;
    info->horiz_pane   = NULL_PANE;

    info->numentries   = numentries;
    info->entries      = entries;
    info->selection    = selection;
    info->redraw_table = redraw_table;

    info->size = size;

    return (
	makeDialogDescriptor(
	    &table_item,
	    (Generic) info,
	    item_id
	    )
	);
}

static void table_scroller(Pane *scrollee, Pane *sb, short direction, int curval)
{
    sm_table_shift_absolute(scrollee, curval);
}
