/* $Id: lazy_table_sm.C,v 1.1 1997/06/25 14:48:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
	/********************************************************/
	/* 							*/
	/* 		      lazy_table_sm.c			*/
	/*	      Table display screen module.		*/
	/* 							*/
	/********************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/monitor/items/lazy_table_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/optim_sm.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>
#include <libs/support/msgHandlers/log_msg.h>

# ifndef NULL
#   define NULL ((char *)0)
# endif

/*
 * declare the screen module
 */
STATIC(void,		lazy_table_start,(void));
STATIC(void,		lazy_table_create,(Pane *p));
STATIC(void,		lazy_table_resize,(Pane *p));
STATIC(void,		lazy_table_destroy,(Pane *p));
STATIC(void,		lazy_table_input,(Pane *p, Rectangle r));

STATIC(Point,	lazy_table_map, (Pane *p, Point info));

static	aScreenModule	scr_mod_table =
			{
			"lazy table",
			lazy_table_start,
			standardFinish,
			lazy_table_create,
			lazy_table_resize,
			standardNoSubWindowPropagate,
			lazy_table_destroy,
			lazy_table_input,
			standardTileNoWindow,
			standardDestroyWindow
			};

/*
 * local datatypes
 */

typedef FUNCTION_POINTER (char *,StringFunc,(Generic,int,int));

struct	table_pane_info	{		/* LOCAL TABLE PANE INFORMATION	*/
	Pane	       *op;		/* the optim slave pane		*/
	Point		disp_size;	/* the display size in entries	*/
	Point		nstrings;
	StringFunc		string;
	Generic		client;		/* handle to be passed in string*/
	char	     ***strings;
	Point		selected;
	short		font_id;

	Point		shift;			
	int		item_width;
};

#define	OP(p)	      	((struct table_pane_info *) p->pane_information)->op
#define	SHIFT(p)	((struct table_pane_info *) p->pane_information)->shift
#define	SELECTED(p)	((struct table_pane_info *) p->pane_information)->selected
#define	DISP_SIZE(p)	((struct table_pane_info *) p->pane_information)->disp_size
#define	ITEM_WIDTH(p)	((struct table_pane_info *) p->pane_information)->item_width
#define	STRINGS(p)	((struct table_pane_info *) p->pane_information)->strings
#define	STRING(p)	((struct table_pane_info *) p->pane_information)->string
#define	CLIENT(p)	((struct table_pane_info *) p->pane_information)->client
#define	NSTRINGS(p)	((struct table_pane_info *) p->pane_information)->nstrings
#define	FONT_ID(p)	((struct table_pane_info *) p->pane_information)->font_id

static short    optim_sm;

/*
 * SCREEN MODULE DEFINITION ROUTINES
 */


/*
 * Start the list screen module
 */
static void lazy_table_start ()
{
    optim_sm = sm_optim_get_index();
}


/*
 * Create a table pane
 */
static void lazy_table_create (Pane *p)
{
    p->pane_information	= (Generic)get_mem(sizeof (struct table_pane_info),
					   "lazy_table_sm.c: pane info str");
    p->border_width	= 1;

    OP(p) = newSlavePane(p, optim_sm, p->position, p->size, p->border_width);

    sm_optim_set_move_status(OP(p), false, false);

    STRINGS(p)		= (char ***)0;
    NSTRINGS(p)		= Origin;
    SHIFT(p)		= Origin;
}


/*
 * Resize/reposition a table pane
 */
static void lazy_table_resize (Pane *p)
{
    Point optim_size;

    resizePane (OP(p), p->position, p->size);

    SHIFT(p)       = Origin;

    optim_size     = sm_optim_size (OP (p));
    DISP_SIZE(p).x = optim_size.x / (ITEM_WIDTH(p) + 1);
    DISP_SIZE(p).y = optim_size.y;
    if ( DISP_SIZE(p).x > NSTRINGS(p).x ) DISP_SIZE(p).x = NSTRINGS(p).x;
    if ( DISP_SIZE(p).y > NSTRINGS(p).y ) DISP_SIZE(p).y = NSTRINGS(p).y;

    sm_optim_resizing(OP(p), true);
    sm_lazy_table_paint(p);
    sm_optim_resizing(OP(p), false);
}


/*
 * Destroy the pane and all structures below it
 */
static void lazy_table_destroy (Pane *p)
{
    destroyPane (OP (p));
    free_mem ((void*) p->pane_information);
}


/*
 * Handle input to the table screen module
 */
static void lazy_table_input (Pane *p, Rectangle r)
{
    while ( mon_event.type < MOUSE_KEYBOARD
	 && pointInRect (mon_event.loc, r) ) {	/* we can handle this event */
	handlePane (OP (p));
	switch ( mon_event.type ) {	/* handle the returned event */
	    case EVENT_SELECT:
		/*
		 * return the event in info.x 
		 */
		mon_event.info = lazy_table_map(p, mon_event.info);
		SELECTED(p)    = mon_event.info;
		sm_lazy_table_paint (p);
		break;
	    case EVENT_HELP:
		break;
	}
    }
}


void sm_lazy_table_initialize (Pane *p, short font_id)
{
    FONT_ID(p) = font_id;
}


void sm_lazy_table_new_table (Pane *p, Point nstrings, StringFunc string, Generic client, 
                              Point selected, int item_width)
{
    Point	optim_size;
    int		i, j;

    /* free the old strings */
    if (STRINGS(p) != (char ***)0) {
	for ( i = 0; i < NSTRINGS(p).y; i++ ) {
	    for ( j = 0; j < NSTRINGS(p).x; j++ )
		if ( STRINGS(p)[i][j] != NULL )
		    sfree (STRINGS(p)[i][j]);
	    free_mem ((void*)STRINGS(p)[i]);
	}
	free_mem ((void*)STRINGS(p));
    }

    /* stash away the new ones (initially just NULL ptrs) */
    STRINGS(p) = (char ***) get_mem (nstrings.y * sizeof (char **),
				     "table entries");
    for ( i = 0; i < nstrings.y; i++ ) {
    	STRINGS(p)[i] = (char **) get_mem (nstrings.x * sizeof (char *),
					"table entries");
	for ( j = 0; j < nstrings.x; j++ )
	    STRINGS (p)[i][j] = NULL;
    }

    NSTRINGS(p)    = nstrings;
    STRING (p)     = string;
    CLIENT (p)     = client;
    
    SELECTED (p)   = selected;
    ITEM_WIDTH (p) = item_width;

    optim_size      = sm_optim_size (OP (p));
    DISP_SIZE(p).x  = optim_size.x / (ITEM_WIDTH(p) + 1);
    DISP_SIZE(p).y  = optim_size.y;
    if ( DISP_SIZE(p).x > NSTRINGS(p).x ) DISP_SIZE(p).x = NSTRINGS(p).x;
    if ( DISP_SIZE(p).y > NSTRINGS(p).y ) DISP_SIZE(p).y = NSTRINGS(p).y;

    /* decrease in ITEM_WIDTH may have added columns that don't exist */
	if ( SHIFT(p).x + DISP_SIZE(p).x > NSTRINGS(p).x )
	    SHIFT(p).x = NSTRINGS(p).x - DISP_SIZE(p).x;
	if ( SHIFT(p).y + DISP_SIZE(p).y > NSTRINGS(p).y )
	    SHIFT(p).y = NSTRINGS(p).y - DISP_SIZE(p).y;
}


/* USER CALLBACKS */


/*
 * Get the index of this screen module
 */
short sm_lazy_table_get_index ()
{
    return ( getScreenModuleIndex(&scr_mod_table) );
}


/*
 * Calculate the minimum sized pane (in pixels)
 *	to display a character block
 */
Point sm_lazy_table_pane_size (Point size, short font)
{
    return ( sm_optim_pane_size(size, font) );
}


/*
 * redisplay the strings in the table
 */
void sm_lazy_table_paint (Pane *p)
{
    TextChar        tc;
    Point           nstrings, dispsize, selected, shift;
    int		    item_width, i, j, k;
    char           *name, *name2;
    int             rows, cols, len;

    sm_optim_clear(OP(p));

    nstrings = NSTRINGS(p);
    dispsize = DISP_SIZE(p);
    if ( (nstrings.x == 0) || (dispsize.y == 0) ) {
	/* no work to do here */
	    sm_optim_touch (OP (p));
	    return;
    }

    selected   = SELECTED(p) ;
    item_width = ITEM_WIDTH(p) ;
    shift      = SHIFT(p) ;

    rows = dispsize.y;
    cols = dispsize.x;

    for ( i = 0; i < rows; i++ ) {
	for ( j = 0; j < cols; j++ ) {

	    if (i + shift.y != selected.y || j + shift.x != selected.x )
		tc.style = STYLE_NORMAL;
	    else
		tc.style = STYLE_BOLD;

	    name = STRINGS(p)[i+shift.y][j+shift.x];
	    if ( name == NULL ) {
	    	name = ssave(STRING(p)(CLIENT(p), i+shift.y, j+shift.x));
		len = strlen(name);
		if ( len > item_width ) { /* truncate if too long */
		    name2 = ssave(name);
		    sfree(name);
		    name2[item_width] = '\0';
		    name = ssave(name2);
		    name2[item_width] = ' ';
		    sfree(name2);
		    len = item_width;
		}
		STRINGS (p)[i+shift.y][j+shift.x] = name;
	    } else {
		len = strlen(name);
	    }

	    for ( k = 0; k < item_width; k++ ) {
		if ( k < len )
		    tc.ch = name[k];
		else
		    tc.ch = ' ';
		sm_optim_putchar(OP(p), makePoint(j*(item_width+1)+k, i), tc);
	    }
	}
    }
    sm_optim_touch(OP(p));
}


/*
 *  the item in location changed_loc has been changed, invalidate it
 */
void sm_lazy_table_invalidate_item (Pane *p, Point changed_loc)
{
    char **loc;

    if ( STRINGS(p) != (char ***)0 ) {
	loc = &STRINGS(p)[changed_loc.y][changed_loc.x];
	if ( *loc != NULL ) {
	    sfree(*loc);
	    *loc = NULL;
	}
    }
}


/*
 *  return the <row,col> (0-index) number of the string
 *	corresponding to character location `info'
 */
static Point lazy_table_map (Pane *p, Point info)
{
    int         col,
                row;
    int		width;

    if (NSTRINGS(p).x == 0)  /* no files to select */
	return UnusedPoint;

    width = ITEM_WIDTH(p) + 1;
    col   = ( info.x + SHIFT(p).x * width ) / width;
    row   = info.y + SHIFT(p).y;

    return(makePoint(col,row));
}


Point sm_lazy_table_get_selection (Pane *p)
{
    return SELECTED(p);
}


void sm_lazy_table_configuration (Pane *p, int *width, int *height, Point *offset)
{
    *width  = DISP_SIZE(p).x;
    *height = DISP_SIZE(p).y;
    *offset = SHIFT(p);
}


void sm_lazy_table_shift_absolute (Pane *p, Point curval)
{
    int	    curvalx, curvaly;

    curvalx = curval.x;
    curvaly = curval.y;

    if ( curvalx < 0 ) curvalx = 0;
    if ( curvaly < 0 ) curvaly = 0;

    if ( curvalx > NSTRINGS(p).x - DISP_SIZE(p).x )
	curvalx = NSTRINGS(p).x - DISP_SIZE(p).x;
    if ( curvaly > NSTRINGS(p).y - DISP_SIZE(p).y )
	curvaly = NSTRINGS(p).y - DISP_SIZE(p).y;

    SHIFT(p) = makePoint(curvalx, curvaly);
    sm_lazy_table_paint(p);
}


void sm_lazy_table_shift_x_absolute (Pane *p, int curval)
{
    sm_lazy_table_shift_absolute(p, makePoint(curval, SHIFT(p).y));
}


void sm_lazy_table_shift_y_absolute (Pane *p, int curval)
{
    sm_lazy_table_shift_absolute(p, makePoint(SHIFT(p).x, curval));
}

void sm_lazy_table_shift_relative (Pane *p, Point shift)
{
    Point abs_shift;

    abs_shift =  makePoint(shift.x + SHIFT(p).x, shift.y + SHIFT(p).y);
    sm_lazy_table_shift_absolute(p, abs_shift);
}
