/* $Id: function_sm.C,v 1.1 1997/06/25 14:57:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
	/********************************************************/
	/* 							*/
	/* 		      function_sm.c			*/
	/*	function (i.e. vector) display screen module	*/
	/* 							*/
	/********************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/sms/function_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/support/msgHandlers/log_msg.h>


# define BORDER_WIDTH  4
# define BORDER_WIDTH2 2


/*
 * declare the screen module
 */
STATIC(void,	function_start,(void));
STATIC(void,	function_finish,(void));
STATIC(void,	function_create,(Pane *p));
STATIC(void,	function_resize,(Pane *p));
STATIC(void,	function_destroy,(Pane *p));
STATIC(void,	function_input,(Pane *p, Rectangle r));
STATIC(void,	function_window_tile,(Window *w, Point *p_size, Boolean New));

STATIC(void,	mouse_drag_up,(Pane *p, int y));
STATIC(void,	mouse_move_up,(Pane *p, int t, int b, int old_t, int old_b));
STATIC(void,	function_paint,(Pane *p));
STATIC(void,	outline,(Pane *p, int top, int btm));
STATIC(void,	trace,(Pane *p, int top, int btm));
STATIC(void,	clear_sel_box,(Pane *p));
STATIC(int,	pix2ind,(Pane *p, int y));
STATIC(int,	ind2pix,(Pane *p, int y));

EXTERN(void,	ex_draw_point,(Pane*,Point,Rectangle));

static	aScreenModule	scr_mod_function = {
	"function display",
	function_start,
	function_finish,
	function_create,
	function_resize,
	standardPropagate,
	function_destroy,
	function_input,
	(sm_window_tile_func)function_window_tile,
	standardDestroyWindow
};

typedef FUNCTION_POINTER(void,NotifyFunc,(Generic,Point,Boolean));

/*
 * instance data
 */
struct function_pane_info {
	Pane	        *pane;
	Window		*window;
	Point		 size;		/* pixel size of display area	*/

	double		*val;		/* val[0...num_v-1] are the ...	*/
	int		 num_v;		/*  ... numbers to be graphed	*/
	Boolean		*redraw;	/* redraw[i] iff val[i] or ...	*/
					/*  ... a neighbor changed	*/

	Boolean		 rotate;	/* if should be rotated by pi/2	*/

	Boolean		 move_z;	/* if can move a zoomed region	*/

	int		 top,		/* the selection range ...	*/
			 btm;		/*  ... (2 y vals in pixels ...	*/
					/*  ... in *pane* coordinates)	*/

	NotifyFunc		 notify;	/* fn to call when sel changes	*/
	Generic		 handle;	/* handle to pass to notify	*/

	int		 d_last;	/* index of last value to displ	*/
	int		 z_top,	z_btm;	/* the zoom    range in indices	*/

	double		 fmin, fmax;	/* min & max of the fn's range	*/

	int		 out_pt;	/* threshold for outlining pts	*/

	Rectangle	 rect;		/* last-inverted box		*/
	Rectangle	 disp;		/* display area			*/
};

#define	PANE(p)	  ((struct function_pane_info *)p->pane_information)->pane
#define	WINDOW(p) ((struct function_pane_info *)p->pane_information)->window
#define	SIZE(p)	  ((struct function_pane_info *)p->pane_information)->size
#define	VAL(p)	  ((struct function_pane_info *)p->pane_information)->val
#define	NUM_V(p)  ((struct function_pane_info *)p->pane_information)->num_v
#define	P_VAL(p)  ((struct function_pane_info *)p->pane_information)->p_val
#define	REDRAW(p) ((struct function_pane_info *)p->pane_information)->redraw
#define	ROTATE(p) ((struct function_pane_info *)p->pane_information)->rotate
#define	MOVE_Z(p) ((struct function_pane_info *)p->pane_information)->move_z
#define TOP(p)	  ((struct function_pane_info *)p->pane_information)->top
#define BTM(p)	  ((struct function_pane_info *)p->pane_information)->btm
#define NOTIFY(p) ((struct function_pane_info *)p->pane_information)->notify
#define HANDLE(p) ((struct function_pane_info *)p->pane_information)->handle
#define D_LAST(p) ((struct function_pane_info *)p->pane_information)->d_last
#define Z_TOP(p)  ((struct function_pane_info *)p->pane_information)->z_top
#define Z_BTM(p)  ((struct function_pane_info *)p->pane_information)->z_btm
#define	FMIN(p)	  ((struct function_pane_info *)p->pane_information)->fmin
#define	FMAX(p)	  ((struct function_pane_info *)p->pane_information)->fmax
#define OUT_PT(p) ((struct function_pane_info *)p->pane_information)->out_pt
#define	RECT(p)	  ((struct function_pane_info *)p->pane_information)->rect
#define	DISP(p)	  ((struct function_pane_info *)p->pane_information)->disp



/*
 * ****************** SCREEN MODULE DEFINITION ROUTINES ******************
 */

/*
 * start the function sm's
 */
static void function_start ( )
{
}



/*
 * Finish the function_sm's
 */
static void function_finish ( )
{
}



/*
 * Create a function pane
 */
static void function_create (Pane *p)
{
    p->pane_information	= (Generic)get_mem(sizeof (struct function_pane_info),
					   "function_sm.c: pane info str");
    p->border_width	= 1;

    PANE(p)		= p;
    WINDOW(p)		= (Window *)0;
    REDRAW(p)		= (Boolean *)0;
}


/*
 * Resize/reposition a function pane
 */
static void function_resize (Pane *p)
{
    int top, btm, i;

    if ( WINDOW(p) != (Window *)0 ) {
	top = pix2ind(p, TOP(p));
	btm = pix2ind(p, BTM(p));
    }

    SIZE(p).x = p->size.x - 2 * BORDER_WIDTH;
    SIZE(p).y = p->size.y - 2 * BORDER_WIDTH;
    DISP(p)   = makeRect(makePoint(BORDER_WIDTH, BORDER_WIDTH),
			 makePoint(p->size.x - BORDER_WIDTH,
				   p->size.y - BORDER_WIDTH)
			);
    if ( WINDOW(p) != (Window *)0 ) {
    	modifyWindow(WINDOW(p), (Generic)&p->size);
	TOP(p) = ind2pix(p, top);
	BTM(p) = ind2pix(p, btm);

	for ( i = Z_TOP(p); i <= Z_BTM(p); i++ )
	    REDRAW(p)[i] = true;

	sm_function_clear_pane(p);
	function_paint(p);
    } else {
	WINDOW(p) = createWindow(p, (Generic)&p->size);
	showWindow(WINDOW(p));
    }
    drawWindowsInPane(p);
}


/*
 * Destroy the pane and all structures below it
 */
static void function_destroy (Pane *p)
{
    if ( REDRAW(p) )
	free_mem((void*)REDRAW(p));

    destroyWindow(WINDOW(p));
    free_mem((void*) p->pane_information);
    p->pane_information = 0;
}


/*
 * Handle input to the function screen module
 */
static void function_input (Pane *p, Rectangle r)
{
    int		*p_new, old;
    Boolean	 dragging = false;
    Boolean	 moving = false;
    int		 size, diff;
    static int	 old_ztop, old_zbtm, delta, orig_m, orig_t, orig_b,
		 new_max, new_min;   

    if ( ROTATE(p) ) {
	p_new = &mon_event.loc.y;
	size = SIZE(p).y;
    } else {
	p_new = &mon_event.loc.x;
	size = SIZE(p).x;
    }

    while ( mon_event.type<MOUSE_KEYBOARD && pointInRect(mon_event.loc, r) ) {
	/* make sure event is inside of DISP(p) */
	    if ( *p_new < BORDER_WIDTH )
		*p_new = BORDER_WIDTH;
	    if ( *p_new > BORDER_WIDTH + size )
		*p_new = BORDER_WIDTH + size;

	switch ( mon_event.type ) {
	    case MOUSE_DRAG:
		if ( dragging ) {
		    old = *p_new;
		    outline(p, TOP(p), *p_new);
		} else if ( moving ) {
		    /* don't let outlined box go over either end of pane */
			delta = *p_new - orig_m;
			if ( *p_new < new_min ) {
			    diff    = *p_new - new_min;
			    orig_m  += diff;
			    delta   -= diff;
			    new_max += diff;
			    new_min  = *p_new;
			} else if ( *p_new > new_max ) {
			    diff     = *p_new - new_max;
			    orig_m  += diff;
			    delta   -= diff;
			    new_min += diff;
			    new_max  = *p_new;
			}
		    trace(p, orig_t + delta, orig_b + delta);
		    outline(p, orig_t + delta, orig_b + delta);
		}
		break;
	    case MOUSE_DOWN:
		if ( mon_event.info.x == BUTTON_SELECT ) {
		    dragging = true;
		    old	     = *p_new;
		    TOP(p)   = *p_new;
		    outline(p, TOP(p), TOP(p));
		} else if ( mon_event.info.x == BUTTON_MOVE ) {
		    moving   = true;
		    orig_m   = *p_new;
		    old_ztop = Z_TOP(p);
		    old_zbtm = Z_BTM(p);
		    Z_TOP(p) = 0;
		    Z_BTM(p) = NUM_V(p) - 1;
		    orig_t   = ind2pix(p, old_ztop);
		    orig_b   = ind2pix(p, old_zbtm);
		    new_min  = orig_m - orig_t + BORDER_WIDTH;
		    new_max  = size + BORDER_WIDTH - (orig_b - orig_m);
		    delta    = 0;
		    trace(p, orig_t, orig_b);
		    outline(p, orig_t, orig_b);
		}
		break;
	    case MOUSE_UP:
		if ( dragging ) {  /* ignore unless click initiated in pane */
		    mouse_drag_up(p, *p_new);
		    dragging = false;
		    return;
		} else if ( moving ) {
		    mouse_move_up(p, delta + orig_t, delta + orig_b, old_ztop,
					old_zbtm);
		    moving  = false;
		    return;
		}
		break;
	    default:
		break;
	}
	getEvent();
    }

    /* if we left the pane while dragging the mouse--treat as MOUSE_UP */
	if ( dragging && NOT(pointInRect(mon_event.loc, r)) )
	    mouse_drag_up(p, old);
	else if ( moving && NOT(pointInRect(mon_event.loc, r)) )
	    mouse_move_up(p, delta + orig_t, delta + orig_b, old_ztop,
					old_zbtm);
}


static void mouse_drag_up (Pane *p, int y)
{
    Point t;

    if ( TOP(p) > y ) {
	BTM(p) = TOP(p);
	TOP(p) = y;
    } else {
	BTM(p) = y;
    }
    outline(p, TOP(p), BTM(p));
    /* mon_event.type = EVENT_SELECT; */

    t = sm_function_get_selection(p);
    if ( NOTIFY(p) )
	(NOTIFY(p))(HANDLE(p), t, false);
}


static void mouse_move_up (Pane *p, int t, int b, int old_t, int old_b)
{
    int		 top, btm;
    Point	 sel;

    TOP(p) = -1;
    BTM(p) = -1;

    Z_TOP(p) = 0;
    Z_BTM(p) = NUM_V(p) - 1;

    top = pix2ind(p, t);
    btm = pix2ind(p, b);

    if ( top != old_t || btm != old_b ) {
	sm_function_set_zoom_range(p, makePoint(top, btm));
	sm_function_redraw(p);
    } else {
	Z_TOP(p) = old_t;
	Z_BTM(p) = old_b;
	clear_sel_box(p);
    }

    sel = makePoint(top, btm);
    if ( NOTIFY(p) )
	(NOTIFY(p))(HANDLE(p), sel, true);
}


/*
 * tile the function window
 */
static void function_window_tile (Window *w, Point *p_size, Boolean New)
{
    Point	 size; 
    Pane	*p;

    size	= *p_size;
    w->border	= makeRect(Origin, size);

    if ( New ) {
	w->border_width	= 0;
	p = newPane(w, sm_vanilla_get_index(), Origin, size, 0);
	w->window_information = (Generic) p;
    } else {
	resizePane((Pane *)w->window_information, Origin, size);
    }
}


/*
 * ********************* USER CALLBACKS **********************
 */

/*
 * Get the index of this screen module
 */
short sm_function_get_index ()
{
    return ( getScreenModuleIndex(&scr_mod_function) );
}


/*
 * Calculate the min pixel size to have a display area of pixel size "size"
 *	(really just need to add in the borders)
 */
Point sm_function_pane_size (Point size)
{
   return ( makePoint(size.x + 2 * BORDER_WIDTH, size.y + 2 * BORDER_WIDTH) );
}


/*
 * start up a new graph
 *
 *	the items to graph are in value[0..num_vals-1]
 */
extern void  sm_function_setup (Pane *p, int num_vals, double *value, Boolean rotate,
				Boolean move_zoomed, NotifyFunc select_notify, 
                                Generic notify_handle)
{
    int i;

    NUM_V(p)	= num_vals;
    VAL(p)	= value;
    ROTATE(p)	= rotate;
    MOVE_Z(p)	= move_zoomed;
    NOTIFY(p)	= select_notify;
    HANDLE(p)	= notify_handle;

    OUT_PT(p)	= 5;

    TOP(p)	= -1;
    BTM(p)	= -1;

    D_LAST(p)	= num_vals - 1;

    Z_TOP(p)	= 0;
    Z_BTM(p)	= num_vals - 1;

    REDRAW(p)	= (Boolean *)get_mem(sizeof(Boolean)*num_vals,
					"sm_function redraw");

    FMIN(p)  = value[0];
    FMAX(p)  = value[0];
    for ( i = 0; i < NUM_V(p); i++ ) {
	REDRAW(p)[i] = true;
	if ( value[i] < FMIN(p) ) FMIN(p) = value[i];
	if ( value[i] > FMAX(p) ) FMAX(p) = value[i];
    }
}


/*
 * there are all new values for the graph
 */
void sm_function_new_values (Pane *p, double *values, Boolean rescale)
{
    int		 i;
    double	*value;

    VAL(p) = values;

    /* if asked to, then establish FMIN and FMAX */
	if ( rescale ) {
	    value = VAL(p);
	    FMIN(p)  = value[0];
	    FMAX(p)  = value[0];
	    for ( i = 1; i < NUM_V(p); i++ ) {
		if ( value[i] < FMIN(p) ) FMIN(p) = value[i];
		if ( value[i] > FMAX(p) ) FMAX(p) = value[i];
	    }
	}

    for ( i = 0; i < NUM_V(p); i++ )
	REDRAW(p)[i] = true;
}


/*
 * there is a new value for VAL(p)[index]
 */
void sm_function_change_point (Pane *p, int index)
{
    REDRAW(p)[index] = true;
}


/*
 * redraw the function
 */
void sm_function_redraw (Pane *p)
{
    function_paint(p);
}


/*
 * clear the pane p
 */
void sm_function_clear_pane (Pane *p)
{
    Rectangle	 R;
    Window	*w;
    Pane	*subp;

    /* set up */
	w	= p->child[FRST_NEXT];
	subp	= (Pane *)w->window_information;

    /* clear the display area */
	R = makeRect(makePoint(0,0), makePoint(SIZE(p).x + 2 * BORDER_WIDTH,
					       SIZE(p).y + 2 * BORDER_WIDTH));
	ColorPaneWithPatternColor(subp, R, NULL_BITMAP, Origin, true, paneBackground(subp), NULL_COLOR);
	RECT(p) = makeRect(makePoint(-1,-1), makePoint(-1,-1));
	touchPane(subp);
}


/*
 *  return the indices of the top and bottom selected elements
 */
Point sm_function_get_selection (Pane *p)
{
    return makePoint( Z_TOP(p) + pix2ind(p, TOP(p)),
		     Z_TOP(p) + pix2ind(p, BTM(p)) );
}


/*
 *  set the indices of the top and bottom selected elements
 */
void sm_function_set_selection (Pane *p, Point sel)
{
    TOP(p) = ind2pix(p, sel.x);
    BTM(p) = ind2pix(p, sel.y);
}


/*
 *  return the indices of the last displayed element
 */
int sm_function_get_last_display (Pane *p)
{
    return D_LAST(p);
}


/*
 *  set the indices of the last displayed element
 */
void sm_function_set_last_display (Pane *p, int last)
{
    D_LAST(p) = last;
}


/*
 *  return the indices of the top and bottom of the zoom range
 */
Point sm_function_get_zoom_range (Pane *p)
{
    return makePoint(Z_TOP(p), Z_BTM(p));
}


/*
 *  set the indices of the top and bottom of the zoom range
 */
void sm_function_set_zoom_range (Pane *p, Point sel)
{
    int i;

    Z_TOP(p) = sel.x;
    Z_BTM(p) = sel.y;

    for ( i = sel.x; i <= sel.y; i++ )
	REDRAW(p)[i] = true;
}


/*
 *  return the min and max of the function values
 */
void sm_function_get_min_max (Pane *p, double *p_fmin, double *p_fmax)
{
    *p_fmin = FMIN(p);
    *p_fmax = FMAX(p);
}


/*
 *  set the min and max of the function values (for scaling purposes)
 */
void sm_function_set_min_max (Pane *p, double fmin, double fmax)
{
    int		 i;

    FMIN(p) = fmin;
    FMAX(p) = fmax;

    for ( i = Z_TOP(p); i <= Z_BTM(p); i++ )
	REDRAW(p)[i] = true;
}


/*
 *  return the number of pixels (between points) required before
 *	points are outlined
 */
int sm_function_get_draw_point_threshold (Pane *p)
{
    return OUT_PT(p);
}


/*
 *  set the number of pixels (between points) required before
 *	points are outlined to t
 */
void sm_function_set_draw_point_threshold (Pane *p, int t)
{
    OUT_PT(p) = t;
}


/*
 *  return the rotate status of p
 */
Boolean sm_function_get_rotate (Pane *p)
{
    return ROTATE(p);
}


/*
 *  set the number of pixels (between points) required before
 *	points are outlined to t
 */
void sm_function_set_rotate (Pane *p, Boolean r)
{
    int		 i;

    ROTATE(p) = r;

    for ( i = Z_TOP(p); i <= Z_BTM(p); i++ )
	REDRAW(p)[i] = true;
}


/*
 *  ********************* LOCAL FUNCTIONS **********************
 */

/*
 *  outline (using XOR) the rectangle from t to b
 *
 *
 *  the whole pane:
 *   
 *   
 *   BORDER_WIDTH                     BORDER_WIDTH
 *    ->|   |<-                        ->|   |<-
 *
 *      +------------------------------------+  --- --- ---
 *      |                                    |   |   |  BORDER_WIDTH
 *      |   +----------------------------+   |   |   |  ---
 *      |   |\                           |   |  top  |
 *      |   | \                          |   |   |   |
 *      |   |  ul of display area DISP(p)|   |   |   |
 *      |   |     (of size SIZE(p))      |   |   |   |
 *      | +-|----------------------------|-+ |  --- btm
 *      | | |                            | | |       |
 *      | | |                            | | |       |
 *      | | |                            | | |       |
 *      | | |                            | | |       |
 *      | +-|----------------------------|-+ |      ---
 *      |   |                            |  \|
 *      |   |                            |   \
 *      |   |                            |   |\
 *      |   |                            |   | rectangle to draw
 *      |   |                            |   |
 *      |   +----------------------------+   |  ---
 *      |                                    |  BORDER_WIDTH
 *      +------------------------------------+  ---
 *
 *      ->| |<-                        ->| |<-
 *   BORDER_WIDTH2                  BORDER_WIDTH2
 *
 */
static void outline (Pane *p, int top, int btm)
{
    Rectangle	 R;
    Point	 ul, lr;
    Window	*w;
    Pane	*subp;
    int		 t;

    /* set up */
	w	= p->child[FRST_NEXT];
	subp	= (Pane *)w->window_information;

    /* make sure top < btm */
	if ( top > btm ) {
	    t   = top;
	    top = btm;
	    btm = t;
	}

    if ( ROTATE(p) ) {
	ul = makePoint(BORDER_WIDTH - BORDER_WIDTH2,                 top);
	lr = makePoint(BORDER_WIDTH + BORDER_WIDTH2 + SIZE(p).x - 1, btm);
    } else {
	ul = makePoint(top, BORDER_WIDTH - BORDER_WIDTH2                );
	lr = makePoint(btm, BORDER_WIDTH + BORDER_WIDTH2 + SIZE(p).y - 1);
    }
    R  = makeRect(ul, lr);

    if ( RECT(p).ul.x != -1 )
        boxPaneColor(subp, RECT(p), 1, NULL_BITMAP, Origin, false, paneBackground(subp), NULL_COLOR);

    boxPane(subp, R, 1, NULL_BITMAP, Origin, false);

    RECT(p) = R;

    touchPane(subp);
}


static void trace (Pane *p, int top, int btm)
{
    int		 zt, zb, t, threshold;

    /* make sure top < btm */
	if ( top > btm ) {
	    t   = top;
	    top = btm;
	    btm = t;
	}

    Z_TOP(p) = 0;
    Z_BTM(p) = NUM_V(p) - 1;

    threshold = OUT_PT(p);
    OUT_PT(p) = 0x7fffffff;	/* (integral) positive infinity */

    zt = pix2ind(p, top);
    zb = pix2ind(p, btm);

    sm_function_set_zoom_range(p, makePoint(zt, zb));
    sm_function_redraw(p);

    OUT_PT(p) = threshold;
}


static void clear_sel_box (Pane *p)
{
    Window	*w;
    Pane	*subp;

    w		= p->child[FRST_NEXT];
    subp	= (Pane *)w->window_information;

    if ( RECT(p).ul.x != -1 ) {
        boxPaneColor(subp, RECT(p), 1, NULL_BITMAP, Origin, false, paneBackground(subp), NULL_COLOR);
	touchPaneRect(subp, RECT(p));
    	RECT(p) = makeRect(makePoint(-1,-1), makePoint(-1,-1));
    }
}


/*
 * redisplay the graph
 */
static void function_paint (Pane *p)
{
    int		 i, j, last_elt;
    double	 x, y, fmin, fmax, ratx, raty;
    int		 clr_top, clr_btm;
    Point	 p0, p1, p2;
    Rectangle	 R;

    Point	*poly;		/* poly[0..np] is the polyline to plot	*/
    int		 np;

    int		 t;

    Boolean	 outline_pts;

    Window	*w;
    Pane	*subp;

    Boolean	 in_run   = false;
    Boolean	 stop_run = false;

    if ( REDRAW(p) == (Boolean *)0
      || Z_TOP(p)  >= Z_BTM(p)
      || D_LAST(p) == Z_TOP(p) )
	return;		/* no line segments to draw */

    /* set up */
	w	= p->child[FRST_NEXT];
	subp	= (Pane *)w->window_information;

    /* allocate an array to hold the polyline coordinates */
	poly = (Point *)get_mem((NUM_V(p) + 2) * sizeof(Point),
				"function_paint poly");

    /* avoid division by zero when scaling */
	fmin	= FMIN(p);
	fmax	= FMAX(p);
	if ( fmin == fmax ) {
	    if ( fmin != 0.0 ) {
	    	fmin *= 0.9;
	    	fmax *= 1.1;
	    } else {
	    	fmin = -1.0;
	    	fmax =  1.0;
	    }
	}

    /* clear the old selection box */
	clear_sel_box(p);

    /*
     *	pretend that were plotting the function 90 degrees out of whack
     *		(this is the way it will look if ROTATE(p) is true)
     *
     *
     *	   (0,0)			x-pixel direction
     *		+ - - - - - - - - - - - - - - - +
     *		|				|
     *		|	o			|
     *		|	o			|
     *		|	o			|
     *		|	 o			|
     *		|	  o			|
     *		|	  o			|
     *		|	   o			|
     *		|	   o			|
     *		|	   o			|
     *		|	   o			|
     *		|	    o	y = f(x)	|
     *		|	     o			|
     *		|	      o			|
     *		|	       o		|
     *		|				|
     *		+ - - - - - - - - - - - - - - - +
     *	y-pixel
     *	direction
     *
     *	we will rotate it back later if NOT(ROTATE(p))
     *
     */

    /* compute scaling factors and whether point outline threshold exceeded */
	if ( ROTATE(p) ) {
	    ratx = ((double)SIZE(p).x - 1.0 ) / (        fmax     - fmin    );
	    raty = ((double)SIZE(p).y - 1.0 ) / ((double)Z_BTM(p) - Z_TOP(p));
	} else {
	    ratx = ((double)SIZE(p).y - 1.0 ) / (        fmax     - fmin    );
	    raty = ((double)SIZE(p).x - 1.0 ) / ((double)Z_BTM(p) - Z_TOP(p));
	}
	outline_pts = BOOL( raty > OUT_PT(p) );

    /* paint in the function */
	x  = (VAL(p)[Z_TOP(p)] - fmin) * ratx + 0.5;
	p1 = makePoint((int)x + BORDER_WIDTH, BORDER_WIDTH);
	p0 = p1;

	last_elt = min(Z_BTM(p), D_LAST(p));

	for ( i = Z_TOP(p)+1; i <= last_elt; i++ ) {
	    x	= (VAL(p)[i] - fmin)     * ratx + 0.5;
	    y	= ((double)i - Z_TOP(p)) * raty + 0.5;
	    p2	= makePoint(	(int)x + BORDER_WIDTH,
			        (int)y + BORDER_WIDTH);

	    if ( REDRAW(p)[i] ) {
		if ( in_run ) {
		    /* another new point--continue the run */
			stop_run   = false;
			poly[np++] = p2;
		} else {
		    /* first new point--begin the run */
			np	   = 0;
			in_run	   = true;
			clr_top	   = p1.y;
			poly[np++] = p0;
			poly[np++] = p1;
			poly[np++] = p2;
		}
	    } 
	    if ( stop_run || i == last_elt ) {
		/* last pt or 2nd consecutive old pt--terminate the run */
		    stop_run   = false;
		    in_run     = false;
		    clr_btm    = (stop_run ? p1.y : p2.y);
		    poly[np++] = p2;

		    /* clear out the old points and line segments */
			if ( ROTATE(p) ) {
			    R = makeRect(makePoint(0,
						   clr_top),
				         makePoint(SIZE(p).x + 2*BORDER_WIDTH,
						   i==last_elt
							? SIZE(p).y +
							      2*BORDER_WIDTH
							: clr_btm));
			} else {
			    R = makeRect(makePoint(clr_top,
						   0),
				         makePoint(i==last_elt
							? SIZE(p).x +
							      2*BORDER_WIDTH
							: clr_btm,
						   SIZE(p).y +2*BORDER_WIDTH));
			}
		        ColorPaneWithPatternColor(subp, R, NULL_BITMAP, Origin, false, paneBackground(subp), NULL_COLOR);

		    /* put in poly line */
			if ( ROTATE(p) ) {
			    R.ul.y = poly[0].y;
			    R.lr.y = poly[np-1].y;
			} else {
			    /* rotate and flip */
				for ( j = 0; j < np; j++ ) {
				    t         = poly[j].y;
				    poly[j].y = poly[j].x;
				    poly[j].x = t;
				    poly[j].y = SIZE(p).y - poly[j].y
						+ 2 * BORDER_WIDTH;
				}

			    R.ul.x = poly[0].x;
			    R.lr.x = poly[np-1].x;
			}
		        polyLinePane(subp, Origin, np, poly, 1, line_style_solid);

			if ( outline_pts )
			    for ( j = 0; j < np; j++ )
				ex_draw_point (subp, poly[j], DISP(p));
			touchPaneRect(subp, R);
	    }
	    if ( NOT(REDRAW(p)[i])  &&  in_run ) {
		/* 1st old point--begin to terminate the run */
		    stop_run   = true;
		    poly[np++] = p2;
	    }

	    /* slide "window" to next function value */
		p0 = p1;
		p1 = p2;

	    REDRAW(p)[i] = false;

	} /* for i */

    /* draw selection box back in, if applicable */
	if ( TOP(p) != -1 )
	    outline(p, TOP(p), BTM(p));
	else
	    touchPane(subp);

    free_mem((void*)poly);
}


/*
 *  map a pixel value (in the *pane*, not the DISP) into an element index
 */
static int pix2ind (Pane *p, int y)
{
    double d;
    int    ind;
    int	   size;

    if ( y == -1 )
	return -1;

    size = ROTATE(p) ? SIZE(p).y : SIZE(p).x;

    d  = (double)y - BORDER_WIDTH;
    d /= (double)size;
    d *= ((double)Z_BTM(p)  - (double)Z_TOP(p));
    d += 0.5;				/* round off */

    ind = (int) d;
    ind = max(ind, 0);
    ind = min(ind, Z_BTM(p) - Z_TOP(p));
    return ind;
}


/*
 *  map an element index into a pixel value (in the *pane*, not the DISP)
 */
static int ind2pix (Pane *p, int y)
{
    double d;
    int	   size;

    if ( y == -1 )
	return -1;

    size = ROTATE(p) ? SIZE(p).y : SIZE(p).x;

    y = max(y, 0);
    y = min(y, Z_BTM(p) - Z_TOP(p));

    d  = (double)y;
    d *= (double)size;
    d /= ((double)Z_BTM(p)  - (double)Z_TOP(p));
    d += BORDER_WIDTH;
    d += 0.5;

    return (int) d;
}


