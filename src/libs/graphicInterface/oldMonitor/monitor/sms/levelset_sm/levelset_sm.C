/* $Id: levelset_sm.C,v 1.1 1997/06/25 14:57:28 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
	/********************************************************/
	/* 							*/
	/* 		      levelset_sm.c			*/
	/*	levelset (i.e. contour) display screen module	*/
	/* 							*/
	/********************************************************/

#include <math.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/sms/levelset_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/support/msgHandlers/log_msg.h>


# define BORDER_WIDTH	  4
# define IDEAL_WIDTH	500
# define IDEAL_HEIGHT	500


typedef enum { Top = 0, Bottom = 1, Left = 2, Right = 3 } Edge;

/*
 * declare the screen module
 */
STATIC(void,	levelset_start,(void));
STATIC(void,	levelset_finish,(void));
STATIC(void,	levelset_create,(Pane *p));
STATIC(void,	levelset_resize,(Pane *p));
STATIC(void,	levelset_destroy,(Pane *p));
STATIC(void,	levelset_input,(Pane *p, Rectangle r));
STATIC(void,	levelset_window_tile,(Window *w, Point *p_size, Boolean New));

STATIC(double,	Fmax,(double d1, double d2));
STATIC(double,	Fmin,(double d1, double d2));
STATIC(void,	mouse_up,(Pane *p, Point sel));
STATIC(void,	outline,(Pane *p, Point top, Point btm));
STATIC(void,	levelset_paint,(Pane *p));
STATIC(Point,	pix2ind,(Pane *p, Point P));
STATIC(Point,	ind2pix,(Pane *p, Point P));
STATIC(Point,	d_ind2pix,(Pane *p, double x, double y));
STATIC(Point,	Endpoint,(Pane *p, Pane *s, int ix, int iy, Edge e, int l));
STATIC(void,	drawline,(Pane *p, Pane *s, Point p1, Point p2, int l));
STATIC(void,	ex_draw_plus,(Pane *s, Point p, Rectangle r));
	void	ex_draw_point(Pane *s, Point p, Rectangle r);
STATIC(void,	colorPoly,(Pane *p, Pane *subp, int n, Point **pt, Rectangle R,
                           int lev, int min_l, int max_l, double lev_per_gray));

extern	Boolean	monochrome_screen;

static	aScreenModule	scr_mod_levelset = {
	"levelset display",
	levelset_start,
	levelset_finish,
	levelset_create,
	levelset_resize,
	standardPropagate,
	levelset_destroy,
	levelset_input,
	(sm_window_tile_func)levelset_window_tile,
	standardDestroyWindow
};

static	Bitmap	inverse_gray, cover[9];


/*
 * instance data
 */
struct levelset_pane_info {
	Pane	        *pane;
	Window		*window;

	double	       **val;		/* array of columns of ...	*/
					/*  ... numbers to graph ...	*/
	Point		 num_v;		/*  ... val[num_v.x][num_v.y]	*/

					/* if "zoomed in" on a subpart:	*/
	Point		 d_org;		/* origin of subarray to disply	*/
	Point		 num_d;		/* number of elts to display	*/

	double		 fmin, fmax;	/* min & max of the fn's range	*/
					/*  (for all values)		*/
	double		 dmin, dmax;	/* min & max of the fn's range	*/
					/*  (for displayed values)	*/
	double		 maxdif;	/* max dist between adj elts	*/
	Point		 size;		/* pixel size of display area	*/
	Rectangle	 d_area;	/* pixel display area		*/
	Point		 sbox;		/* last highlighted selection	*/
	Point		 top,		/* ul and lr of selected region	*/
			 btm;		/*				*/
	Point		 cross;		/* center point of crosshair	*/
	Boolean		 line;		/* draw in lines		*/
	Boolean		 paint;		/* paint in colors		*/
	Boolean		 field;		/* show vector field		*/
	Boolean		 Auto;		/* how to do contours		*/
	double		 base,
			 step;
	int		 high_r,	/* color of max fn value	*/
			 high_g,
			 high_b,
			 low_r,		/* color of min fn value	*/
			 low_g,
			 low_b;
	Boolean		 auto_contrast;
};

#define	PANE(p)	  ((struct levelset_pane_info *)p->pane_information)->pane
#define	WINDOW(p) ((struct levelset_pane_info *)p->pane_information)->window
#define	VAL(p)	  ((struct levelset_pane_info *)p->pane_information)->val
#define	NUM_V(p)  ((struct levelset_pane_info *)p->pane_information)->num_v
#define	D_ORG(p)  ((struct levelset_pane_info *)p->pane_information)->d_org
#define NUM_D(p)  ((struct levelset_pane_info *)p->pane_information)->num_d
#define	FMIN(p)	  ((struct levelset_pane_info *)p->pane_information)->fmin
#define	FMAX(p)	  ((struct levelset_pane_info *)p->pane_information)->fmax
#define	DMIN(p)	  ((struct levelset_pane_info *)p->pane_information)->dmin
#define	DMAX(p)	  ((struct levelset_pane_info *)p->pane_information)->dmax
#define	MAXDIF(p) ((struct levelset_pane_info *)p->pane_information)->maxdif
#define	SIZE(p)	  ((struct levelset_pane_info *)p->pane_information)->size
#define	D_AREA(p) ((struct levelset_pane_info *)p->pane_information)->d_area
#define	SBOX(p)	  ((struct levelset_pane_info *)p->pane_information)->sbox
#define	TOP(p)	  ((struct levelset_pane_info *)p->pane_information)->top
#define	BTM(p)	  ((struct levelset_pane_info *)p->pane_information)->btm
#define	CROSS(p)  ((struct levelset_pane_info *)p->pane_information)->cross
#define	LINE(p)   ((struct levelset_pane_info *)p->pane_information)->line
#define	PAINT(p)  ((struct levelset_pane_info *)p->pane_information)->paint
#define	FIELD(p)  ((struct levelset_pane_info *)p->pane_information)->field
#define	AUTO(p)	  ((struct levelset_pane_info *)p->pane_information)->Auto
#define	BASE(p)	  ((struct levelset_pane_info *)p->pane_information)->base
#define	STEP(p)	  ((struct levelset_pane_info *)p->pane_information)->step
#define	HIGH_R(p) ((struct levelset_pane_info *)p->pane_information)->high_r
#define	HIGH_G(p) ((struct levelset_pane_info *)p->pane_information)->high_g
#define	HIGH_B(p) ((struct levelset_pane_info *)p->pane_information)->high_b
#define	LOW_R(p)  ((struct levelset_pane_info *)p->pane_information)->low_r
#define	LOW_G(p)  ((struct levelset_pane_info *)p->pane_information)->low_g
#define	LOW_B(p)  ((struct levelset_pane_info *)p->pane_information)->low_b
#define AUTO_CONTRAST(p) ((struct levelset_pane_info *)p->pane_information)->auto_contrast
/*typedef enum { Top = 0, Bottom = 1, Left = 2, Right = 3 } Edge;*/

/*
 * ****************** SCREEN MODULE DEFINITION ROUTINES ******************
 */


/*
 * start the levelset sm's
 */
static void levelset_start ( )
{
    static BITMAPM_UNIT gray_data[] = {		/* a 50% pattern */
	0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555,
	0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555
    };

    static BITMAPM_UNIT b100[] = {
						/*  87%	*/
	    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
	    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
    };
    static BITMAPM_UNIT b87[] = {
						/*  87%	*/
	    0xFBFB, 0xDFDF, 0xFEFE, 0xF7F7, 0xBFBF, 0xFDFD, 0xEFEF, 0x7F7F,
	    0xFBFB, 0xDFDF, 0xFEFE, 0xF7F7, 0xBFBF, 0xFDFD, 0xEFEF, 0x7F7F
    };
    static BITMAPM_UNIT b75[] = {
						/*  75%	*/
	    0xEEEE, 0xBBBB, 0xDDDD, 0x7777, 0xEEEE, 0xBBBB, 0xDDDD, 0x7777, 
	    0xEEEE, 0xBBBB, 0xDDDD, 0x7777, 0xEEEE, 0xBBBB, 0xDDDD, 0x7777
    };
    static BITMAPM_UNIT b62[] = {
						/*  62%	*/
	    0xECEC, 0xABAB, 0x5D5D, 0x7373, 0xCECE, 0xBABA, 0xD5D5, 0x3B3B,
	    0xECEC, 0xABAB, 0x5D5D, 0x7373, 0xCECE, 0xBABA, 0xD5D5, 0x3B3B
    };
    static BITMAPM_UNIT b50[] = {
						/*  50%	*/
	    0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555,
	    0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555
    };
    static BITMAPM_UNIT b37[] = {
						/*  37%	*/
	    0x1313, 0x5454, 0xA2A2, 0x8C8C, 0x3131, 0x4545, 0x2A2A, 0xC4C4,
	    0x1313, 0x5454, 0xA2A2, 0x8C8C, 0x3131, 0x4545, 0x2A2A, 0xC4C4
    };
    static BITMAPM_UNIT b25[] = {
						/*  25%	*/
	    0x1111, 0x4444, 0x2222, 0x8888, 0x1111, 0x4444, 0x2222, 0x8888, 
	    0x1111, 0x4444, 0x2222, 0x8888, 0x1111, 0x4444, 0x2222, 0x8888
    };
    static BITMAPM_UNIT b12[] = {
						/*  12%	*/
	    0x0404, 0x2020, 0x0101, 0x0808, 0x4040, 0x0202, 0x1010, 0x8080,
	    0x0404, 0x2020, 0x0101, 0x0808, 0x4040, 0x0202, 0x1010, 0x8080
    };
    static BITMAPM_UNIT b00[] = {
						/*  00%	*/
	    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
    };

    inverse_gray =  makeBitmapFromData(makePoint(16, 16), gray_data,
				      "inverse_gray cover");

    cover[0] =  makeBitmapFromData(makePoint(16, 16), b100, "cover array");
    cover[1] =  makeBitmapFromData(makePoint(16, 16), b87,  "cover array");
    cover[2] =  makeBitmapFromData(makePoint(16, 16), b75,  "cover array");
    cover[3] =  makeBitmapFromData(makePoint(16, 16), b62,  "cover array");
    cover[4] =  makeBitmapFromData(makePoint(16, 16), b50,  "cover array");
    cover[5] =  makeBitmapFromData(makePoint(16, 16), b37,  "cover array");
    cover[6] =  makeBitmapFromData(makePoint(16, 16), b25,  "cover array");
    cover[7] =  makeBitmapFromData(makePoint(16, 16), b12,  "cover array");
    cover[8] =  makeBitmapFromData(makePoint(16, 16), b00,  "cover array");
}


/*
 * Finish the levelset_sm's
 */
static void levelset_finish ( )
{
    int i;

    freeBitmap(inverse_gray);

    for ( i = 0; i < 9; i++ )
	freeBitmap(cover[i]);
}


/*
 * Create a levelset pane
 */
static void levelset_create (Pane *p)
{
    p->pane_information	= (Generic)get_mem(sizeof (struct levelset_pane_info),
					   "levelset_sm.c: pane info str");
    p->border_width	= 1;

    PANE(p)	= p;
    WINDOW(p)	= (Window *)0;
    NUM_V(p)	= Origin;

    AUTO_CONTRAST(p) = false;

    getRGBFromName("levelset.high", &HIGH_R(p), &HIGH_G(p), &HIGH_B(p));
    getRGBFromName("levelset.low",  &LOW_R(p),  &LOW_G(p),  &LOW_B(p));

/*    getRGBFromName("red",  &HIGH_R(p), &HIGH_G(p), &HIGH_B(p));
    getRGBFromName("blue", &LOW_R(p),  &LOW_G(p),  &LOW_B(p));*/
}


/*
 * Resize/reposition a levelset pane
 */
static void levelset_resize (Pane *p)
{
    Point top, btm;

    if ( WINDOW(p) != (Window *)0 ) {
	top = pix2ind(p, TOP(p));
	btm = pix2ind(p, BTM(p));
    }

    SIZE(p).x = p->size.x - 2 * BORDER_WIDTH;
    SIZE(p).y = p->size.y - 2 * BORDER_WIDTH;
    D_AREA(p) = makeRect(makePoint(BORDER_WIDTH, BORDER_WIDTH),
			 makePoint(p->size.x - BORDER_WIDTH,
				   p->size.y - BORDER_WIDTH)
			);
    if ( WINDOW(p) != (Window *)0 ) {
    	modifyWindow(WINDOW(p), (Generic)&p->size);
	TOP(p) = ind2pix(p, top);
	BTM(p) = ind2pix(p, btm);
	levelset_paint(p);
    } else {
	WINDOW(p) = createWindow(p, (Generic)&p->size);
	showWindow(WINDOW(p));
    }
    drawWindowsInPane(p);
}


/*
 * Destroy the pane and all structures below it
 */
static void levelset_destroy (Pane *p)
{
    destroyWindow(WINDOW(p));
    free_mem((void*) p->pane_information);
    p->pane_information = 0;
}


/*
 * Handle input to the levelset screen module
 */
static void levelset_input (Pane *p, Rectangle r)
{
    Point	oldsel;
    Boolean	dragging = false;

    while ( mon_event.type<MOUSE_KEYBOARD && pointInRect(mon_event.loc, r) ) {
	/* make sure event is inside of D_AREA(p) */
	    if ( mon_event.loc.x < BORDER_WIDTH )
		mon_event.loc.x = BORDER_WIDTH;
	    if ( mon_event.loc.y < BORDER_WIDTH )
		mon_event.loc.y = BORDER_WIDTH;
	    if ( mon_event.loc.x > BORDER_WIDTH + SIZE(p).x )
		mon_event.loc.x = BORDER_WIDTH + SIZE(p).x;
	    if ( mon_event.loc.y > BORDER_WIDTH + SIZE(p).y )
		mon_event.loc.y = BORDER_WIDTH + SIZE(p).y;

	switch ( mon_event.type ) {
	    case MOUSE_DRAG:
		if ( dragging ) {
		    outline(p, TOP(p), mon_event.loc);
		    oldsel = mon_event.loc;
		}
		break;
	    case MOUSE_DOWN:
		dragging = true;
		oldsel   = mon_event.loc;
		TOP(p)   = mon_event.loc;
		outline(p, TOP(p), TOP(p));
		break;
	    case MOUSE_UP:
		if ( dragging ) {  /* ignore unless click initiated in pane */
		    mouse_up(p, mon_event.loc);
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
	    mouse_up(p, oldsel);
}


static void mouse_up (Pane *p, Point sel)
{
    BTM(p) = sel;

    outline(p, TOP(p), sel);
    mon_event.type = EVENT_SELECT;
    mon_event.info = pix2ind(p, sel);
}


/*
 * tile the levelset window
 */
static void levelset_window_tile (Window *w, Point *p_size, Boolean New)
{
    Point	 size; 
    Pane	*p;

    size	= *p_size;
    w->border	= makeRect(Origin, size);

    if ( New ) {
	w->border_width	= 0;
	p = newPane(w, sm_vanilla_get_index(), Origin, size, 0);
	recolorPane(p, NULL_COLOR, NULL_COLOR, NULL_COLOR, false, true, false);
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
short sm_levelset_get_index ()
{
    return ( getScreenModuleIndex(&scr_mod_levelset) );
}


/*
 * Calculate the min pixel size to display an array of size arraysize
 */
Point sm_levelset_pane_size (Point a_size)
/*Point	a_size;			number of data pts in array plane */
{
   int x, y;
   double xpix_per, ypix_per, pix_per;

   xpix_per = ((double)IDEAL_WIDTH)  / ((double)a_size.x);
   ypix_per = ((double)IDEAL_HEIGHT) / ((double)a_size.y);
   pix_per  = Fmin(xpix_per, ypix_per);
   if ( pix_per > 3 )
	pix_per = floor(pix_per);

    x  = (int)(pix_per * (double)(a_size.x - 1));
    x += BORDER_WIDTH + BORDER_WIDTH;

    y  = (int)(pix_per * (double)(a_size.y - 1));
    y += BORDER_WIDTH + BORDER_WIDTH;

    return makePoint(x, y);
}


/*
 * start up a new graph
 *
 *	the items to graph are in value[num_vals.x][num_vals.y]
 */
void sm_levelset_new_values (Pane *p, Point num_vals, double **value, Point top, 
                             Point btm, Boolean rescale)
{
    Boolean firsttime;

    firsttime	= BOOL(NUM_V(p).x == 0);

    if ( firsttime ) {
	D_ORG(p) = Origin;
	NUM_D(p) = num_vals;
	LINE(p)  = true;
	PAINT(p) = false;
	FIELD(p) = false;
	AUTO(p)  = true;
	STEP(p)  = 1.0;
	BASE(p)  = 0.0;
    } 

    NUM_V(p)	= num_vals;
    VAL(p)	= value;

    if ( rescale || firsttime )
	sm_levelset_adjust_zoom(p, D_ORG(p), NUM_D(p), top, btm);

    if ( firsttime ) {
    	FMIN(p) = DMIN(p);
    	FMAX(p) = DMAX(p);
    }
}


void sm_levelset_paint (Pane *p)
{
    levelset_paint(p);
}


/*
 * set the contour spacing information
 */
void sm_levelset_set_contours (Pane *p, Boolean line, Boolean paint, Boolean field, 
                               Boolean Auto, double base, double step, 
                               Boolean auto_contrast)
{
    LINE(p)	= line;
    PAINT(p)	= paint;
    FIELD(p)	= field;
    AUTO(p)	= Auto;
    BASE(p)	= base;
    STEP(p)	= step;
    AUTO_CONTRAST(p) = auto_contrast;
}


/*
 * retrieve the contour spacing information
 */
void sm_levelset_get_contours (Pane *p, Boolean *line, Boolean *paint, Boolean *field,
                               Boolean *Auto, double *base, double *step, 
                               Boolean *auto_contrast)
{
    *line     = LINE(p);
    *paint    = PAINT(p);
    *field    = FIELD(p);
    *Auto     = AUTO(p);
    *base     = BASE(p);
    *step     = STEP(p);
    *auto_contrast = AUTO_CONTRAST(p);
}


/*
 * change the .ul of the elements to be displayed to be
 *	VAL(p)[display_origin.x][display_origin.y]
 *
 * and the number of elements to display to be num_display
 */
void  sm_levelset_adjust_zoom (Pane *p, Point display_origin, Point num_display, 
                               Point top, Point btm)
{
    int		i, j;
    double	v, v01, v10, v11, maxdif, dmin, dmax;
    double    **value;

    D_ORG(p)	= display_origin;
    NUM_D(p)	= num_display;
    TOP(p)	= ind2pix(p, top);
    BTM(p)	= ind2pix(p, btm);

    /* establish DMIN, DMAX */
	value = VAL(p);
	dmin  = value[display_origin.x][display_origin.y];
	dmax  = dmin;
	for ( i = display_origin.x; i < num_display.x + display_origin.x; i++ ) {
	    for ( j = display_origin.y; j < num_display.y + display_origin.y; j++ ) {
	        v = value[i][j];
		if ( v < dmin ) dmin = v;
		if ( v > dmax ) dmax = v;
	    }
	}
	DMIN(p) = dmin;
	DMAX(p) = dmax;

    /* establish MAXDIF */
	maxdif = 0.0;
	for ( i = display_origin.x; i < num_display.x + display_origin.x - 1; i++ ) {
	    for ( j = display_origin.y; j < num_display.y + display_origin.y - 1; j++ ) {
	        v   = value[i  ][j  ];
		v01 = value[i  ][j+1];
		v10 = value[i+1][j  ];
		v11 = value[i+1][j+1];
		maxdif = Fmax(maxdif, fabs(v   - v01));
		maxdif = Fmax(maxdif, fabs(v   - v10));
		maxdif = Fmax(maxdif, fabs(v   - v11));
		maxdif = Fmax(maxdif, fabs(v01 - v10));
	    }
	}
	MAXDIF(p) = maxdif;
}


/*
 *  return the indices of the top and bottom selected elements
 */
void sm_levelset_get_selection (Pane *p, Point *p_tl, Point *p_br)
{
    int		top, btm, left, right;
    Point	tl, br;

    left  = min(TOP(p).x, BTM(p).x);
    right = max(TOP(p).x, BTM(p).x);
    top   = min(TOP(p).y, BTM(p).y);
    btm   = max(TOP(p).y, BTM(p).y);

    tl	  = makePoint(left,  top);
    br	  = makePoint(right, btm);

    *p_tl = pix2ind(p, tl);
    *p_br = pix2ind(p, br);
}


/*
 *  return the smallest function value in the data
 */
double sm_levelset_get_min (Pane *p)
{
    return FMIN(p);
}


/*
 *  return the largest function value in the data
 */
double sm_levelset_get_max (Pane *p)
{
    return FMAX(p);
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
 *      |<- top.x ->|
 *      |<-        btm.x        ->|
 *
 *      +------------------------------------+  --- --- ---
 *      |                                    |   |   |  BORDER_WIDTH
 *      |   +----------------------------+   |   |   |  ---
 *      |   |\                           |   | top.y |
 *      |   | \                          |   |   |   |
 *      |   |  ul of disp area D_AREA(p) |   |   |   |
 *      |   |     (of size SIZE(p))      |   |   |   |
 *      |   |       +-------------+      |   |  --- btm.y
 *      |   |       |             |      |   |       |
 *      |   |       |             |      |   |       |
 *      |   |       |             |      |   |       |
 *      |   |       |             |      |   |       |
 *      |   |       +-------------+      |   |      ---
 *      |   |        \                   |   |
 *      |   |         \                  |   |
 *      |   |          \                 |   |
 *      |   |           rect to draw     |   |
 *      |   |                            |   |
 *      |   +----------------------------+   |  ---
 *      |                                    |  BORDER_WIDTH
 *      +------------------------------------+  ---
 *
 *
 */
static void outline (Pane *p, Point top, Point btm)
{
    Rectangle	 R;
    Window	*w;
    Pane	*subp;
    int		 t, b, l, r;

    l = min(top.x, btm.x);
    r = max(top.x, btm.x);
    t = min(top.y, btm.y);
    b = max(top.y, btm.y);

    /* set up */
	w	= p->child[FRST_NEXT];
	subp	= (Pane *)w->window_information;

    if ( CROSS(p).x != -1 ) {
	R.ul.x = CROSS(p).x;
	R.ul.y = BORDER_WIDTH;
	R.lr.x = CROSS(p).x;
	R.lr.y = BORDER_WIDTH + SIZE(p).y;
	ghostBoxInPane(subp, R, 1, false, false);

	R.ul.x = BORDER_WIDTH;
	R.ul.y = CROSS(p).y;
	R.lr.x = BORDER_WIDTH + SIZE(p).x;
	R.lr.y = CROSS(p).y;
	ghostBoxInPane(subp, R, 1, false, false);
    }

    if ( SBOX(p).x != -1 ) {
	R.ul.x = min(CROSS(p).x, SBOX(p).x);
	R.ul.y = SBOX(p).y;
	R.lr.x = max(CROSS(p).x, SBOX(p).x);
	R.lr.y = SBOX(p).y;
	ghostBoxInPane(subp, R, 1, false, false);

	R.ul.x = SBOX(p).x;
	R.ul.y = min(CROSS(p).y, SBOX(p).y);
	R.lr.x = SBOX(p).x;
	R.lr.y = max(CROSS(p).y, SBOX(p).y);
	ghostBoxInPane(subp, R, 1, false, false);

	SBOX(p).x = -1;
    }

    /* draw in the cross-hairs */
	R.ul.x = btm.x;
	R.ul.y = BORDER_WIDTH;
	R.lr.x = btm.x;
	R.lr.y = BORDER_WIDTH + SIZE(p).y;
	ghostBoxInPane(subp, R, 1, false, false);

	R.ul.x = BORDER_WIDTH;
	R.ul.y = btm.y;
	R.lr.x = BORDER_WIDTH + SIZE(p).x;
	R.lr.y = btm.y;
	ghostBoxInPane(subp, R, 1, false, false);

	CROSS(p) = btm;

    /* draw in the rest of the rectangle */
	R.ul.x = l;
	R.ul.y = top.y;
	R.lr.x = r;
	R.lr.y = top.y;
	ghostBoxInPane(subp, R, 1, false, false);

	R.ul.x = top.x;

	R.ul.y = t;
	R.lr.x = top.x;
	R.lr.y = b;
	ghostBoxInPane(subp, R, 1, false, true);

	SBOX(p) = top;

    touchPane(subp);
}

# define	g00	g[0]
# define	g01	g[1]
# define	g10	g[2]
# define	g11	g[3]
static		double	g[4];

/*
 * redisplay the graph
 */
static void levelset_paint (Pane *p)
{
  int		 i, j;
  double	 dmin, dmax, cd, base, threshold;
  
  double	 v00, v01, v10, v11, v_avg, g_avg;
  double	 v90, v09;
  int		 i00, i01, i10, i11, imin, imax, level, min_level, max_level;
  int		 lmax, lmin;
  double	 levels_per_color;
  int		 Case;
  
  int		 small_d;
  
  Rectangle	 R;
  Window	*w;
  Pane	*subp;
  Boolean	 outline_pts;
  Point	 lt, rb;
  Point	 point[6];
  
  Point	 center, delta, head;
  double	 deltax, deltay;
  Rectangle	 r;
  Boolean	 can_display_field;
  
  if ( NUM_V(p).x < 2 )
    return;		/* not ready yet */
  
  beginComputeBound();
  
  /* set up */
  w	= p->child[FRST_NEXT];
  subp	= (Pane *)w->window_information;

  /* clear the display area */
  R = makeRect(makePoint(0,0), makePoint(SIZE(p).x + 2 * BORDER_WIDTH,
					 SIZE(p).y + 2 * BORDER_WIDTH));
  ColorPaneWithPatternColor(subp, R, NULL_BITMAP, Origin, false,
			    paneBackground(subp), NULL_COLOR);
  
  SBOX(p)  = makePoint(-1,-1);
  CROSS(p) = makePoint(-1,-1);
  touchPane(subp);
  
  /* avoid division by zero when scaling */
  dmin	= DMIN(p);
  dmax	= DMAX(p);
  
  if ( dmin == dmax ) {	/* flat fn--there aren't any contour lines! */
    if ( TOP(p).x != -1 )
      outline(p, TOP(p), BTM(p));
    endComputeBound();
    return;
  }
  
  /* determine contour interval distance */
  if ( AUTO(p) ) {
    base	= dmin;
    cd		= MAXDIF(p);
    small_d	= min(SIZE(p).x, SIZE(p).y);
    threshold	= (double) small_d / 24.0;
    while ( (dmax - dmin) / cd > threshold )
      cd *= 1.5;
    
    BASE(p)	= (dmin < 0.0 && dmax > 0.0 ? 0.0 : (dmin+dmax)/2.0);
    base	= BASE(p);
    STEP(p)	= cd;
  } else {
    base	= BASE(p);
    cd		= STEP(p);
  }
  if ( AUTO_CONTRAST(p) ) {
    min_level	   = floor((DMIN(p) - base) / cd);
    max_level	   = floor((DMAX(p) - base) / cd);
    levels_per_color = (floor((DMAX(p) - base) / cd)
			- floor((DMIN(p) - base) / cd) + 1) / 9;
  } else {
    min_level	   = floor((FMIN(p) - base) / cd);
    max_level	   = floor((FMAX(p) - base) / cd);
    levels_per_color = (floor((FMAX(p) - base) / cd)
			- floor((FMIN(p) - base) / cd) + 1) / 9;
  }
  
  /*
   * if there is enough room, then put a circle around the points
   * 
   *     "enough room" means the measure of how many pixels/point
   *				is bigger than some threshhold (15.0)
   */
  outline_pts = BOOL( 
		     ((double)SIZE(p).x-1.0)/((double)NUM_D(p).x-1.0) >15.0
		     && ((double)SIZE(p).y-1.0)/((double)NUM_D(p).y-1.0) >15.0
		     );
  can_display_field = BOOL( 
			   ((double)SIZE(p).x-1.0)/((double)NUM_D(p).x-1.0) >7.0
			   && ((double)SIZE(p).y-1.0)/((double)NUM_D(p).y-1.0) >7.0
			   );
  
  /* paint in the levelset */
  for ( i = D_ORG(p).x; i < NUM_D(p).x +D_ORG(p).x - 1; i++ ) {
    for ( j = D_ORG(p).y; j < NUM_D(p).y + D_ORG(p).y - 1; j++ ) {
      v00 = VAL(p)[i  ][j  ];	/*			*/
      v01 = VAL(p)[i  ][j+1];	/*	v00	v10	*/
      v10 = VAL(p)[i+1][j  ];	/*	v01	v11	*/
      v11 = VAL(p)[i+1][j+1];	/*			*/
      
      g00 = (v00 - base) / cd;
      g01 = (v01 - base) / cd;
      g10 = (v10 - base) / cd;
      g11 = (v11 - base) / cd;
      
      i00 = floor(g00); imin = i00;            imax = i00;
      i01 = floor(g01); imin = min(imin, i01); imax = max(imax, i01);
      i10 = floor(g10); imin = min(imin, i10); imax = max(imax, i10);
      i11 = floor(g11); imin = min(imin, i11); imax = max(imax, i11);
      
      lmax = (int)floor( (imax - min_level) / levels_per_color);
      lmin = (int)floor( (imin - min_level) / levels_per_color);
      
      lt = ind2pix(p, makePoint(i,   j));
      rb = ind2pix(p, makePoint(i+1, j+1));
      R  = makeRect(lt, rb);
      
      if ( PAINT(p) && (lmin == lmax || imin < imax - 1)) {
	/* paint entire rect the same color */
	point[0] = lt;
	point[1] = makePoint(rb.x, lt.y);
	point[2] = rb;
	point[3] = makePoint(lt.x, rb.y);
	colorPoly(p, subp, 4, (Point**)point, R, (imax + imin) / 2,
		  min_level, max_level, levels_per_color);
      }

      if ( FIELD(p)  &&  can_display_field  &&  i > 0  &&  j > 0 ) {
	/* put a "gradient" vector at 00 based on 8 neighbors */
	v09 = VAL(p)[i  ][j-1];
	v90 = VAL(p)[i-1][j  ];
	
#			ifdef FOUR_POINT
	center = d_ind2pix(p, 0.5 +(double)i,
			   0.5 +(double)j);
	deltax = (v11 + v10 - v00 - v01)
	  / (4 * MAXDIF(p));
	deltay = (v11 + v01 - v00 - v10)
	  / (4 * MAXDIF(p));
#			else  /* 5 POINT */
	center = lt;
	deltax = (v10 - v90) / (4 * MAXDIF(p));
	deltay = (v01 - v09) / (4 * MAXDIF(p));
#			endif /* FOUR_POINT */
	
	/* scale delta to pixel coordinates */
	deltax *= SIZE(p).x;
	deltax /= NUM_D(p).x;
	deltax += 0.5;
	
	deltay *= SIZE(p).y;
	deltay /= NUM_D(p).y;
	deltay += 0.5;
	
	delta = makePoint((int)deltax, (int)deltay);
	
	/*
	 *	o		o		o
	 *			v09
	 *
	 *			       + (head)
	 *			      /
	 *			     /
	 *			    /
	 *			   /
	 *			  /
	 *			 /
	 *	o		o		o
	 *	v90		v00		v10
	 *			(center)
	 *
	 *
	 *
	 *
	 *
	 *
	 *
	 *	o		o		o
	 *			v01
	 */
	head = transPoint(center, delta);
	
	ex_draw_plus(subp, head, D_AREA(p));
	r = linePane(subp, center, head, 1, line_style_solid);
	
	touchPaneRect(subp, r);
      }
      
      for ( level = imin + 1; level <= imax; level++ ) {
	if ( i00 >= level ) {
	  Case  = (i10 < level) << 2;
	  Case |= (i01 < level) << 1;
	  Case |= (i11 < level);
	} else {
	  Case  = (i10 >= level) << 2;
	  Case |= (i01 >= level) << 1;
	  Case |= (i11 >= level);
	}
	
	switch ( Case ) {
	case 0:     /* no lines--all the same level */
	  /* can't happen because of for  */
	  break;
	case 1:     /* line from btm to right edge  */
	  point[0] = Endpoint(p, subp, i, j, Bottom, level);
	  point[1] = Endpoint(p, subp, i, j, Right,  level);
	  if ( PAINT(p) && imin == imax - 1 ) {
	    point[2] = makePoint(rb.x, lt.y); /* rt */
	    point[3] = lt;
	    point[4] = makePoint(lt.x, rb.y); /* lb */
	    colorPoly(p, subp, 5, (Point**)point, R, i00,
		      min_level, max_level, levels_per_color);
	    
	    point[2] = rb;
	    colorPoly(p, subp, 3, (Point**)point, R, i11,
		      min_level, max_level, levels_per_color);
	  }
	  if ( LINE(p) )
	    drawline(p, subp, point[0], point[1], level);
	  break;
	case 2:     /* line from left to btm edge   */
	  point[0] = Endpoint(p, subp, i, j, Left,   level);
	  point[1] = Endpoint(p, subp, i, j, Bottom, level);
	  if ( PAINT(p) && imin == imax - 1 ) {
	    point[2] = rb;
	    point[3] = makePoint(rb.x, lt.y); /* rt */
	    point[4] = lt;
	    colorPoly(p, subp, 5, (Point**)point, R, i00,
		      min_level, max_level, levels_per_color);
	    
	    point[2] = makePoint(lt.x, rb.y); /* lb */
	    colorPoly(p, subp, 3, (Point**)point, R, i01,
		      min_level, max_level, levels_per_color);
	  }
	  if ( LINE(p) )
	    drawline(p, subp, point[0], point[1], level);
	  break;
	case 3:     /* line from left to right edge */
	  point[0] = Endpoint(p, subp, i, j, Left,  level);
	  point[1] = Endpoint(p, subp, i, j, Right, level);
	  if ( PAINT(p) && imin == imax - 1 ) {
	    point[2] = makePoint(rb.x, lt.y); /* rt */
	    point[3] = lt;
	    colorPoly(p, subp, 4, (Point**)point, R, i00,
		      min_level, max_level, levels_per_color);
	    
	    point[2] = rb;
	    point[3] = makePoint(lt.x, rb.y); /* lb */
	    colorPoly(p, subp, 4, (Point**)point, R, i11,
		      min_level, max_level, levels_per_color);
	  }
	  if ( LINE(p) )
	    drawline(p, subp, point[0], point[1], level);
	  break;
	case 4:     /* line from top to right edge  */
	  point[0] = Endpoint(p, subp, i, j, Top,   level);
	  point[1] = Endpoint(p, subp, i, j, Right, level);
	  if ( PAINT(p) && imin == imax - 1 ) {
	    point[2] = rb;
	    point[3] = makePoint(lt.x, rb.y); /* lb */
	    point[4] = lt;
	    colorPoly(p, subp, 5, (Point**)point, R, i00,
		      min_level, max_level, levels_per_color);
	    
	    point[2] = makePoint(rb.x, lt.y); /* rt */
	    colorPoly(p, subp, 3, (Point**)point, R, i10,
		      min_level, max_level, levels_per_color);
	  }
	  if ( LINE(p) )
	    drawline(p, subp, point[0], point[1], level);
	  break;
	case 5:     /* line from top to btm edge    */
	  point[0] = Endpoint(p, subp, i, j, Top,    level);
	  point[1] = Endpoint(p, subp, i, j, Bottom, level);
	  if ( PAINT(p) && imin == imax - 1 ) {
	    point[2] = makePoint(lt.x, rb.y); /* lb */
	    point[3] = lt;
	    colorPoly(p, subp, 4, (Point**)point, R, i00,
		      min_level, max_level, levels_per_color);
	    
	    point[2] = rb;
	    point[3] = makePoint(rb.x, lt.y); /* rt */
	    colorPoly(p, subp, 4, (Point**)point, R, i11,
		      min_level, max_level, levels_per_color);
	  }
	  if ( LINE(p) )
	    drawline(p, subp, point[0], point[1], level);
	  break;
	case 6:
	  v_avg = (v00 + v01 + v10 + v11) / 4;
	  g_avg = (v_avg - base) / cd;
	  if (   i00 >= level && floor(g_avg) >= level
	      || i00 < level  && floor(g_avg) <  level ) {
	    /* line from top to right edge  */
	    /* + line from left to btm edge */
	    point[0] = Endpoint(p, subp, i, j, Top,    level);
	    point[1] = Endpoint(p, subp, i, j, Right,  level);
	    point[2] = rb;
	    point[3] = Endpoint(p, subp, i, j, Bottom, level);
	    point[4] = Endpoint(p, subp, i, j, Left,   level);
	    point[5] = lt;
	    if ( PAINT(p) && imin == imax - 1 ) {
	      colorPoly(p, subp, 6, (Point**)point, R, i00,
			min_level, max_level, levels_per_color);
	      
	      point[2] = makePoint(rb.x, lt.y);
	      colorPoly(p, subp, 3, (Point**)point, R, i10,
			min_level, max_level, levels_per_color);
	      
	      point[5] = makePoint(lt.x, rb.y);
	      colorPoly(p, subp, 3, (Point**)&point[3], R, i01,
			min_level, max_level, levels_per_color);
	    }
	  } else {
	    /* line from left to top edge   */
	    /* + line from btm to right edge*/
	    point[0] = Endpoint(p, subp, i, j, Left,   level);
	    point[1] = Endpoint(p, subp, i, j, Top,    level);
	    point[2] = makePoint(rb.x, lt.y);/* rt */
	    point[3] = Endpoint(p, subp, i, j, Right,  level);
	    point[4] = Endpoint(p, subp, i, j, Bottom, level);
	    point[5] = makePoint(lt.x, rb.y);/* lb */
	    if ( PAINT(p) && imin == imax - 1 ) {
	      colorPoly(p, subp, 6, (Point**)point, R, i10,
			min_level, max_level, levels_per_color);
	      
	      point[2] = lt;
	      colorPoly(p, subp, 3, (Point**)point, R, i00,
			min_level, max_level, levels_per_color);
	      
	      point[5] = rb;
	      colorPoly(p, subp, 3, (Point**)&point[3], R, i11,
			min_level, max_level, levels_per_color);
	    }
	  }
	  if ( LINE(p) ) {
	    drawline(p, subp, point[0], point[1], level);
	    drawline(p, subp, point[3], point[4], level);
	  }
	  break;
	case 7:     /* line from left to top edge   */
	  point[0] = Endpoint(p, subp, i, j, Left, level);
	  point[1] = Endpoint(p, subp, i, j, Top,  level);
	  if ( PAINT(p) && imin == imax - 1 ) {
	    point[2] = makePoint(rb.x, lt.y); /* rt */
	    point[3] = rb;
	    point[4] = makePoint(lt.x, rb.y); /* lb */
	    colorPoly(p, subp, 5, (Point**)point, R, i11,
		      min_level, max_level, levels_per_color);
	    
	    point[2] = lt;
	    colorPoly(p, subp, 3, (Point**)point, R, i00,
		      min_level, max_level, levels_per_color);
	  }
	  if ( LINE(p) )
	    drawline(p, subp, point[0], point[1], level);
	  break;
	}
      }
      if ( outline_pts && NOT(FIELD(p)) ) {
	ex_draw_point (subp, ind2pix(p, makePoint(i, j)), D_AREA(p));
	if ( i == NUM_D(p).x +D_ORG(p).x - 2)
	  ex_draw_point (subp, ind2pix(p, makePoint(i+1, j)), D_AREA(p));
	if ( j == NUM_D(p).y +D_ORG(p).y - 2)
	  ex_draw_point (subp, ind2pix(p, makePoint(i, j+1)), D_AREA(p));
      }
      if ( PAINT(p) )
	touchPaneRect(subp, R);
    }
  }
  if ( outline_pts && NOT(FIELD(p)) )
    ex_draw_point (subp, ind2pix(p, 
				 makePoint(NUM_D(p).x +D_ORG(p).x - 1,
					   NUM_D(p).y +D_ORG(p).y - 1)),
		   D_AREA(p));
  
  if ( TOP(p).x != -1 )
    outline(p, TOP(p), BTM(p));
  else
    touchPane(subp);
  
  endComputeBound();
  
}


/*
 *  
 */
/*ARGSUSED*/
static Point Endpoint (Pane *p, Pane *s, int ix, int iy, Edge e, int l)
{
			/*	Top,	Bottom,	Left,	Right	*/
    static int	 firstx [4] = {	0,	0,	0,	1};
    static int	 firsty [4] = {	0,	1,	0,	0};
    static int	 secondx[4] = {	1,	1,	0,	1};
    static int	 secondy[4] = {	0,	1,	1,	1};
    static int	 gval_1[4]  = {	0,	1,	0,	2};
    static int	 gval_2[4]  = {	2,	3,	1,	3};
    Rectangle	 r;
    double	 ex1, ey1, ex2, ey2;
    double	 x, y, f1, f2, scale;
    LineStyle    *style;

    /* map the function differences on the edge e into a index point x1, y1 */
	ex1 = (double)(ix + firstx[(int)e]);
	ey1 = (double)(iy + firsty[(int)e]);
	ex2 = (double)(ix + secondx[(int)e]);
	ey2 = (double)(iy + secondy[(int)e]);

	f1 = g[gval_1[(int)e]];
	f2 = g[gval_2[(int)e]];

	scale = (((double)l) - f1) / (f2 - f1);
	x = ex1 + scale * (ex2 - ex1);
	y = ey1 + scale * (ey2 - ey1);

    /* convert indices to pixels */
	return d_ind2pix(p, x, y);
}


/*
 *  
 */
/*ARGSUSED*/
static void drawline (Pane *p, Pane *s, Point p1, Point p2, int l)
{
  Rectangle	 r;
  LineStyle     *style;

  if ( PAINT(p) ) {
    r = linePaneColor(s, p1, p2, 1, line_style_solid,
		      (l < 0 ? paneBackground(s) :
		       paneForeground(s)));
  } else {
    style = (l % 2 == 0 ? line_style_solid : line_style_dotted);
    r = linePane(s, p1, p2, (l == 0 ? 2 : 1), style);
  }
  touchPaneRect(s, r);
}


/*
 *  put the "point" p in (real fat) in pane p (w/ clipping rectangle r)
 */
void ex_draw_point (Pane *s, Point p, Rectangle r)
{
  static	Point	 pt_outline[] = {
    { -1, -1 }, {  0, -1 }, {  1, -1 },
    { -1,  0 },             {  1,  0 },
    { -1,  1 }, {  0,  1 }, {  1,  1 }
  };

    if ( NOT(pointInRect(p, r)) ) return;

    polyPointPane(s, p, 8, pt_outline, r);
    pointPaneColor(s, p, r, paneBackground(s));

    touchPaneRect(s, makeRect(makePoint(p.x - 1, p.y - 1),
			      makePoint(p.x + 1, p.y + 1) ));
}


/*
 *  put a "+" at point p in pane p (w/ clipping rectangle r)
 */
static void ex_draw_plus (Pane *s, Point p, Rectangle r)
{
static	Point	 pt_outline[] = {
			            {  0, -1 },
			{ -1,  0 },             {  1,  0 },
			            {  0,  1 }
		 };

    if ( NOT(pointInRect(p, r)) ) return;

    polyPointPane(s, p, 4, pt_outline, r);

    touchPaneRect(s, makeRect(makePoint(p.x - 1, p.y - 1),
			      makePoint(p.x + 1, p.y + 1) ));
}


/*
 *  map a pixel value (in the *pane*, not the D_AREA) into an element index
 */
static Point pix2ind (Pane *p, Point P)
{
    double dx, dy;
    int    indx, indy;

    if ( P.x == -1 )
	return UnusedPoint;

    dx  = (double)P.x - BORDER_WIDTH;
/*    dx /= ((double)SIZE(p).x  - 1.0); */
    dx /= ((double)SIZE(p).x);
    dx *= ((double)NUM_D(p).x - 1.0);
    dx += 0.5;				/* round off */

    dy  = (double)P.y - BORDER_WIDTH;
/*    dy /= ((double)SIZE(p).y  - 1.0); */
    dy /= ((double)SIZE(p).y);
    dy *= ((double)NUM_D(p).y - 1.0);
    dy += 0.5;				/* round off */

    indx  = (int) dx;
    indx  = max(indx, 0);
    indx  = min(indx, NUM_D(p).x - 1);
    indx += D_ORG(p).x;

    indy  = (int) dy;
    indy  = max(indy, 0);
    indy  = min(indy, NUM_D(p).y - 1);
    indy += D_ORG(p).y;

    return makePoint(indx, indy);
}


/*
 *  map an element index into a pixel value (in the *pane*, not the D_AREA)
 */
static Point ind2pix (Pane *p, Point P)
{
    return d_ind2pix(p, (double)P.x, (double)P.y);
}


/*
 *  map an element index into a pixel value
 */
static Point d_ind2pix (Pane *p, double x, double y)
{
    if ( x < 0 || y < 0 )
	return makePoint(-1, -1);

    x -= (double)D_ORG(p).x;
    x  = Fmax(x, 0.0);
    x  = Fmin(x, (double)NUM_D(p).x - 1);

    y -= (double)D_ORG(p).y;
    y  = Fmax(y, 0.0);
    y  = Fmin(y, (double)NUM_D(p).y - 1);

/*    x *= ((double)SIZE(p).x  - 1.0);  */
    x *= ((double)SIZE(p).x); 
    x /= ((double)NUM_D(p).x - 1.0);
    x += BORDER_WIDTH;

/*    y *= ((double)SIZE(p).y  - 1.0);  */
    y *= ((double)SIZE(p).y); 
    y /= ((double)NUM_D(p).y - 1.0);
    y += BORDER_WIDTH;

    x += 0.5;
    y += 0.5;

    return makePoint((int)x, (int)y);
}


static double Fmax (double d1, double d2)
{
    return ( d1 > d2 ? d1 : d2 );
}


static double Fmin (double d1, double d2)
{
    return ( d1 < d2 ? d1 : d2 );
}


static void colorPoly (Pane *p, Pane *subp, int n, Point **pt, Rectangle R, 
                       int lev, int min_l, int max_l, double lev_per_gray)
{
  static int	 max_color = 65535;
  int		 r, g, b;
  double	 fn_val;

  if ( monochrome_screen ) {
    polygonPane(subp, Origin, n, (Point*)pt, 
		cover[(int)floor((lev - min_l) / lev_per_gray)],
		Origin, R);
  } else {
    fn_val  = (double)((lev - min_l) / ((double)(max_l - min_l + 1)));

    r  = LOW_R(p);
    r += (int)((HIGH_R(p) - LOW_R(p)) * fn_val);

    g  = LOW_G(p);
    g += (int)((HIGH_G(p) - LOW_G(p)) * fn_val);

    b  = LOW_B(p);
    b += (int)((HIGH_B(p) - LOW_B(p)) * fn_val);

    polygonPaneColor(subp, Origin, n, (Point*)pt, NULL_BITMAP, Origin, R,
		     getColorFromRGB(r, g, b), NULL_COLOR);
  }
}
