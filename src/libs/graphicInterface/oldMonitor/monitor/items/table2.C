/* $Id: table2.C,v 1.1 1997/06/25 14:48:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                                table2.c                              */
/*                              table  item                             */
/*                                                                      */
/************************************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/item_def.h>
#include <libs/graphicInterface/oldMonitor/include/items/table2.h>
#include <libs/graphicInterface/oldMonitor/monitor/items/lazy_table_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/scroll_sm.h>
#include <string.h>

 /*** LOCAL INFORMATION ***/

struct table
{				        /* ITEM STARTUP INFORMATION     */
    short           font;	  	/* the font to use              */
    Pane           *table_pane;
    Pane           *horiz_pane;		/* horiz and ...		*/
    Pane           *verti_pane;		/*   ... vertical scroll bars	*/

    Point          *numentries,		/* total no of strings in table	*/
    		    old_numents;	/* old value of numentries	*/
    int		   *item_width,
		    old_width;
    PFS             entry;		/* callback to get contents	*/
    Generic	    client;		/* handle to be passed in entry	*/
    Point          *selection,		/* pos'n of current selection	*/
		    old_sel;		/* pos'n of former  selection	*/
    Point	   *changed_item;	/* index of string that changed	*/
    Boolean        *redraw_table;	/* strings have changed		*/
    Boolean        *shift_table;	/* if selection should be shown	*/


    Point           size;	 	/* number of entries displayed	*/
};

STATIC(void,     horiz_scroller,(Pane *scrollee, Pane *sb, short direction,
                                 int curval));
STATIC(void,     verti_scroller,(Pane *scrollee, Pane *sb, short direction,
                                 int curval));
STATIC(void,	 adjust_scroll_bars,(struct table *info));


/************************** ITEM DEFINITION *****************************/

/*
 * Get the pixel size of the table item
 */
static Point table2_get_size (DiaDesc *dd)
{
    struct table   *info;	       /* the information parameter	*/
    Point           item_size;	       /* pixel size of the table item	*/

    info = (struct table *) dd->item_info;

    item_size = sm_lazy_table_pane_size(
				makePoint(info->size.x*(*info->item_width+1),
					  info->size.y),
				info->font);
    item_size.x += SB_WIDTH - 1;
    item_size.y += SB_WIDTH - 1;

    return (item_size);
}


/*
 * Place the item and initialize the pane
 */
static void table2_initialize (DiaDesc *dd, Point ulc)
{
    Point           pane_size;	       /* pixel size of the table pane	*/
    struct table   *info;
    Point           pane_loc;

    info = (struct table *) dd->item_info;

 /* get and adjust the pane sizes */
    pane_size = sm_lazy_table_pane_size(
				makePoint(info->size.x*(*info->item_width+1),
					  info->size.y),
				info->font);

 /* create and initialize the table pane */
    info->table_pane = dialog_item_make_pane(
	dd,
	sm_lazy_table_get_index(),
	ulc,
	pane_size,
	2);

    pane_loc = ulc;
    pane_loc.y += pane_size.y - 1;
    info->horiz_pane = dialog_item_make_pane(dd,
	sm_scroll_get_index(),
	pane_loc,
	makePoint(pane_size.x, SB_WIDTH),
	2);

    pane_loc = ulc;
    pane_loc.x += pane_size.x - 1;
    info->verti_pane = dialog_item_make_pane(dd,
	sm_scroll_get_index(),
	pane_loc,
	makePoint(SB_WIDTH, pane_size.y),
	2);

    (void) sm_lazy_table_initialize(info->table_pane, info->font);

    /* initialize the screen module */
	sm_lazy_table_new_table(info->table_pane,
				*info->numentries, 
				info->entry,
				info->client,
				*info->selection,
				*info->item_width);

    /* connect the scroll bars */
	sm_scroll_scrollee(info->horiz_pane, (Generic) info->table_pane, 
			(sm_scroll_scrollee_callback)horiz_scroller);
	sm_scroll_activate(info->horiz_pane,  true);

	sm_scroll_scrollee(info->verti_pane, (Generic) info->table_pane,
			(sm_scroll_scrollee_callback)verti_scroller);
	sm_scroll_activate(info->verti_pane,  true);

    /* adjust the scroll bar appearance and behavior */
	adjust_scroll_bars(info);
}


/*
 * Handle the current event
 */
static FocusStatus table2_handle_event (DiaDesc *dd)
{
    struct table   *info;	       /* the information parameter    */
    Point	    sel;	       /* the selection coordinates    */

    info = (struct table *) dd->item_info;

    if (  mon_event.from == (Generic) info->table_pane ) {
    	switch ( mon_event.type ) {
	    case EVENT_SELECT:
		sel = sm_lazy_table_get_selection(info->table_pane);
		*info->selection = sel;
		dialog_item_user_change(dd);
		break;
	    case EVENT_MOVE:
		break;
	    default:
		break;
	}
    }
    return FocusOK;
}


/*
 * Handle a modification of the item
 */
/*ARGSUSED*/
static void table2_modified (DiaDesc *dd, Boolean user)
{
    struct table	*info;
    int			 x, y;

    info = (struct table *) dd->item_info;

    /* see if something was modified */
	if ( *info->item_width   != info->old_width
	  || info->numentries->x != info->old_numents.x
	  || info->numentries->y != info->old_numents.y
	  || *info->redraw_table		 ) {
	    /* repaint with new item_width and strings */
		sm_lazy_table_new_table(info->table_pane,
					*info->numentries, 
					info->entry,
					info->client,
					*info->selection,
					*info->item_width);
		adjust_scroll_bars(info);
		sm_lazy_table_paint(info->table_pane);
		*info->redraw_table = false;
		info->old_width     = *info->item_width;
		info->old_numents   = *info->numentries;
	} else if ( info->changed_item->x != -1
		 || info->changed_item->y != -1 ) {
	    /* invalidate changed_item and repaint */
		sm_lazy_table_invalidate_item(info->table_pane,
					      *info->changed_item);
		sm_lazy_table_paint(info->table_pane);
		info->changed_item->x = -1;
		info->changed_item->y = -1;
	} else if ( info->selection->x != info->old_sel.x
		 || info->selection->y != info->old_sel.y ) {
	    /* repaint, no other changes */
		sm_lazy_table_paint(info->table_pane);
		info->old_sel = *info->selection;
	}
	if ( *info->shift_table ) {
	    /* shift and repaint */
		x = info->selection->x - info->size.x/2;
		y = info->selection->y - info->size.y/2;
		sm_lazy_table_shift_absolute(info->table_pane,makePoint(x, y));
		*info->shift_table = false;
		adjust_scroll_bars(info);
	}
}


/*
 * Destroy item specific information
 */
static void table2_destroy (DiaDesc *dd)
{
    free_mem((void*)dd->item_info);
}


static Item     table2 = {	       /* the table item                  */
    "table item",
    table2_get_size,
    (item_initialize_func)table2_initialize,
    table2_handle_event,
    standardNoFocusSetter,
    table2_modified,
    table2_destroy
};


/************************* EXTERNAL ROUTINES ****************************/


/*
 * Create a dialog descriptor for the table item
 */
DiaDesc *item_table2 (Generic item_id, short font, Point *numentries, int *item_width,
		      PFS entry, Generic client, Point *selection, Point *changed_item,
		      Boolean *redraw_table, Boolean *shift_table, Point size)
{
    struct table   *info;

    info = (struct table *) get_mem(sizeof(struct table),
				    "table item information");

    info->font = font;

    info->table_pane   =  NULL_PANE;
    info->horiz_pane   =  NULL_PANE;
    info->verti_pane   =  NULL_PANE;

    info->numentries   =  numentries;
    info->old_numents  = *numentries;
    info->item_width   =  item_width;
    info->old_width    = *item_width;
    info->entry        =  entry;
    info->client       =  client;
    info->selection    =  selection;
    info->old_sel      = *selection;
    info->changed_item =  changed_item;
    info->redraw_table =  redraw_table;
    info->shift_table  =  shift_table;

    /* make sure we don't have empty spots in table */
	if ( size.x > numentries->x )
	    size.x = numentries->x;
	if ( size.y > numentries->y )
	    size.y = numentries->y;
	info->size = size;

    return (makeDialogDescriptor(&table2, (Generic)info, item_id));
}


/*ARGSUSED*/
static void horiz_scroller (Pane *scrollee, Pane *sb, short direction, int curval)
{
    sm_lazy_table_shift_x_absolute(scrollee, curval);
}


/*ARGSUSED*/
static void verti_scroller (Pane *scrollee, Pane *sb, short direction, int curval)
{
    sm_lazy_table_shift_y_absolute(scrollee, curval);
}


static void adjust_scroll_bars (struct table *info)
{
    int             width,
                    fullwidth,
                    height,
                    fullheight;
    int		    x, y;
    Point           offset;

    /* determine where in object the dispay is */
	sm_lazy_table_configuration(info->table_pane, &width, &height, &offset);
	x = offset.x;
	y = offset.y;

	fullwidth  = info->numentries->x;
	fullheight = info->numentries->y;
	if ( fullwidth < width )
	    sm_scroll_set(info->horiz_pane, 0, 0, x, true);
	else
	    sm_scroll_set(info->horiz_pane, 0, fullwidth - width, x, true);

	if ( fullheight < height )
	    sm_scroll_set(info->verti_pane, 0, 0, x, true);
	else
	    sm_scroll_set(info->verti_pane, 0, fullheight - height, y, true);

    /*
     * set step sizes to be
     *	1			(small) and 
     *	num elements - 1	(large)
     */
	if ( width > info->numentries->x )
	    width = info->numentries->x;
	info->size.x = width;
	sm_scroll_set_step(info->horiz_pane, 1, info->size.x - 1);
	sm_scroll_set_step(info->verti_pane, 1, info->size.y - 1);
}


