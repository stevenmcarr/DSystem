/* $Id: mach_x.ansi.c,v 1.19 1997/06/25 14:53:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 		       mach_x.c				*/
		/*              Handles X specific calls.		*/
		/* 							*/
		/********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sgtty.h>
#include <signal.h>
#include <string.h>
#include <memory.h>
#include <sys/param.h>
#include <unistd.h>

#include <include/bstring.h>
#include <libs/support/misc/general.h>

#ifdef OSF1
EXTERN(void, gtty, (int, struct sgttyb *));
#endif


#include <libs/graphicInterface/oldMonitor/monitor/keyboard/kb.h>
#include <libs/graphicInterface/support/graphics/point.h>
#include <libs/graphicInterface/support/graphics/rect.h>
#include <libs/graphicInterface/support/graphics/rect_list.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/support/tables/cNameValueTable.h>
#include <libs/support/strings/rn_string.h>

#include <libs/graphicInterface/oldMonitor/include/mon/gfx.h>
#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#include <libs/graphicInterface/oldMonitor/include/mon/event_codes.h>
#include <libs/graphicInterface/oldMonitor/include/mon/event.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/mach.h>

#define	Window		XWindow		/* rename X's window type		*/
#define	Pixmap		XPixmap		/* rename X's pixmap type		*/
#define	Bitmap		XBitmap		/* rename X's bitmap type		*/
#define	Cursor		XCursor		/* rename X's cursor type		*/
#define	Display		XDisplay	/* rename X's display type		*/
#define	GC		XGC		/* rename X's GC type			*/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>

#undef	CURSOR				/* get rid of x's CURSOR definition	*/
#undef  Window				/* use our Window definition now	*/
#undef	Pixmap				/* use our Pixmap definition now	*/
#undef	Bitmap				/* use our Bitmap definition now	*/
#undef	Cursor				/* use noone's Cursor definition now	*/
#undef	Display				/* use noone's Display definition now	*/
#undef	GC				/* use noone's GC definition now	*/


	/* GLOBAL COLOR DEFINITIONS */

Color			default_foreground_color; /* foreground Color of root window	*/
Color			default_background_color; /* background Color of root window	*/
Color			default_border_color;	/* border color of root window		*/

Boolean			monochrome_screen;	/* true if should use only black & white*/

	/* LOCAL COLOR DEFINITIONS */

typedef XrmDatabase	ColorDB;		/* user color preference database type	*/
static ColorDB		colorDB = NULL; 	/* user color preference database	*/
extern char*		D_resource_identifier;	/* color preference program name	*/
extern char*		D_color_defaults_file; /* default color preferences files	*/

#define COLOR_PIXEL(c)	(((XColor *) (c))->pixel) /* pixel value, used in implementation*/

static cNameValueTable colorHashTable;		/* hash table containing all colors	*/
#define INITIAL_NUMBER_COLORS 24		/* guess regarding number of used colors*/

STATIC (int, colorHashTableCompareFunc, (Generic name1, Generic name2));
						/* yields 0 if two color strings same	*/
STATIC (int, colorHashTableHashFunc, (Generic name, unsigned int ht_size));
						/* maps name to number in [0, ht_size)	*/
STATIC (void, destroyColor, (Generic name, Generic value, Generic extra_arg));
						/* works on each (name, value) pair	*/

STATIC (Color, color, (char * name)); /* forward declarations		*/
STATIC (ColorDB, getColorDB, (char *colorsFile));


#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

	/* LOCAL PIXMAP DEFINITIONS */

typedef	struct		{			/* PRIVATE PIXMAP STRUCTURE		*/
	Point		size;			/* the size of the image		*/
	Drawable	drawable;		/* the x drawable			*/
			} PPixmap;
static	PPixmap		p_screen_pixmap;	/* the screen pixmap (assume depth > 1)	*/
Pixmap			screen_pixmap = (Pixmap) &p_screen_pixmap;




	/* BITMAP DEFINITIONS */

typedef struct		{			/* PRIVATE BITMAP STRUCTURE		*/
	Point		size;			/* the size of the bitmap			*/
	Drawable	drawable;		/* the X drawable			*/
			} PBitmap;
#define	BITMAPM_BYTE_ORDER	MSBFirst	/* Rn bitmap byte order			*/
#define	BITMAPM_BIT_ORDER	MSBFirst		/* Rn bitmap bit order			*/


	/* WINDOW EVENT INFORMATION */

static  XDisplay  	*x_display;		/* the X display structure		*/
static	XGC		x_gc_bitmap;		/* the graphics context for bitmaps	*/
static	XGC		x_gc_window;		/* the screen graphics context		*/
static	int		x_screen;		/* X screen index number		*/
static	XWindow		x_window;		/* the root window			*/
static	short		inhibit_flush = 0;	/* nonzero when we cannot flush events	*/
#define	INHIBIT_FLUSH()	inhibit_flush++		/* prevent a flush events		*/
#define OK_TO_FLUSH()	inhibit_flush--		/* allow events to be flushed again	*/
ResizeFunc root_resizer;	/* resize the root window		*/
RedrawFunc root_redrawer;	/* redraw the root window		*/
CharHandlerFunc char_handler;	/* handle a new character		*/

static	Boolean		outside_window = true;	/* mouse is outside the window		*/
static	short		button_pushed = 0;	/* the button number that is down	*/
static	Boolean		need_exit_event = false;/* need to give a faked exit event	*/
static	Boolean		need_release_event = false;/* need to give a faked btn release	*/
Boolean			flushScreenEvents();	/* forget any accumulated screen events	*/
#define	PIXMAP_DEPTH	(DefaultDepth(x_display,x_screen))
						/* depth of our pixmaps			*/
#define BITMAP_DEPTH	(1 << 0)		/* depth of our bitmaps			*/
#define	FLAT_PLANE_BITMAP	(1L << 0)		/* plane bitmap for flat pixmaps		*/

typedef FUNCTION_POINTER(void,SignalHandler,(int));

	/* CURSOR STUFF */

typedef struct {
	XCursor		cursor;			/* X's cursor representation		*/
	Color		foreground;		/* cursor's color			*/
	Color		background;		/* color behind the cursor		*/
} MouseCursorStruct, * MouseCursorP;

static	Point		max_cursor_size;	/* largest allowed cursor size		*/
static	MouseCursor	current_cur = 0;	/* the current cursor number		*/


	/* GRAPHICS OPERATIONS */

static	int		pix_Op[] = {		/* the x graphics ops			*/
				GXclear,			/* BITMAP_CLR		*/
				GXnor,				/* BITMAP_NOR		*/
				GXandInverted,			/* BITMAP_INV_AND		*/
				GXcopyInverted,			/* BITMAP_INV_COPY	*/
				GXandReverse,			/* BITMAP_AND_INV		*/
				GXinvert,			/* BITMAP_INV		*/
				GXxor,				/* BITMAP_XOR		*/
				GXnand,				/* BITMAP_NAND		*/
				GXand,				/* BITMAP_AND		*/
				GXequiv,			/* BITMAP_EQ		*/
				GXnoop,				/* BITMAP_NOOP		*/
				GXorInverted,			/* BITMAP_INV_OR		*/
				GXcopy,				/* BITMAP_COPY		*/
				GXorReverse,			/* BITMAP_OR_INV		*/
				GXor,				/* BITMAP_OR		*/
				GXset				/* BITMAP_SET		*/
			};
#define	BLACK_SRC(op)	((op >> 2) * 5)		/* assume a black source		*/
#define INV_DST(op)	(((op & 10) >> 1 | (op & 5) << 1) ^ 15) /* inverted destination	*/
#define INV_SRC(op)	((op & 12) >> 2 | (op & 3) << 2)	/* inverted source	*/


	/* OTHER X-WINDOWS STUFF */

static XRectangle rn2xRect(Rectangle r);


	/* KEYBINDINGS */

struct	key		{			/* X KEYFUNCTIONS AND CORRESPONDING KEYS*/
	unsigned char		code;		/* the X code number (in order)		*/
	KbChar			kb;		/* the corresponding kb character	*/
			};
#ifdef distribution
static	struct	key	ibm_keys[] = {		/* IBM keyboard keys (PS/2)		*/
#else
static	struct	key	ibm_keys[] = {		/* IBM keyboard keys (AIX, PS/2)	*/
#endif /* distribution */
				{  83,	KB_left(1)	},	/* Insert		*/
				{  87,	KB_ArrowL	},	/* Left			*/
				{  88,	KB_left(2)	},	/* Home			*/
				{  89,	KB_left(4)	},	/* End			*/
				{  91,	KB_ArrowU	},	/* Up			*/
				{  92,	KB_ArrowD	},	/* Down			*/
				{  93,	KB_left(3)	},	/* Page Up		*/
				{  94,	KB_left(5)	},	/* Page Down		*/
				{  97,	KB_ArrowR	},	/* Right		*/
				{  99,	KB_right(4)	},	/* Num-7		*/
				{ 100,	KB_right(7)	},	/* Num-4		*/
				{ 101,	KB_right(10)	},	/* Num-1		*/
				{ 103,	KB_right(1)	},	/* Num-/		*/
				{ 104,	KB_right(5)	},	/* Num-8		*/
				{ 105,	KB_right(8)	},	/* Num-5		*/
				{ 106,	KB_right(11)	},	/* Num-2		*/
				{ 107,	KB_right(13)	},	/* Num-0		*/
				{ 108,	KB_right(2)	},	/* Num-*		*/
				{ 109,	KB_right(6)	},	/* Num-9		*/
				{ 110,	KB_right(9)	},	/* Num-6		*/
				{ 111,	KB_right(12)	},	/* Num-3		*/
				{ 112,	KB_right(14)	},	/* Num-.		*/
				{ 113,	KB_right(3)	},	/* Num--		*/
				{ 114,	KB_right(16)	},	/* Num-+		*/
				{ 116,	KB_right(15)	},	/* Num-Enter		*/
				{ 120,	KB_top(1)	},	/* F1			*/
				{ 121,	KB_top(2)	},	/* F2			*/
				{ 122,	KB_top(3)	},	/* F3			*/
				{ 123,	KB_top(4)	},	/* F4			*/
				{ 124,	KB_top(5)	},	/* F5			*/
				{ 125,	KB_top(6)	},	/* F6			*/
				{ 126,	KB_top(7)	},	/* F7			*/
				{ 127,	KB_top(8)	},	/* F8			*/
				{ 128,	KB_top(9)	},	/* F9			*/
				{ 129,	KB_top(10)	},	/* F10			*/
				{ 130,	KB_top(11)	},	/* F11			*/
				{ 131,	KB_top(12)	},	/* F12			*/
			};
#ifndef distribution
static	struct	key	ibmRT_keys[] = {	/* IBM RT keyboard keys			*/
				{   7,	KB_top(1)	},	/* F1			*/
				{  15,	KB_top(2)	},	/* F2			*/
				{  23,	KB_top(3)	},	/* F3			*/
				{  31,	KB_top(4)	},	/* F4			*/
				{  39,	KB_top(5)	},	/* F5			*/
				{  47,	KB_top(6)	},	/* F6			*/
				{  55,	KB_top(7)	},	/* F7			*/
				{  63,	KB_top(8)	},	/* F8			*/
				{  71,	KB_top(9)	},	/* F9			*/
				{  79,	KB_top(10)	},	/* F10			*/
				{  86,	KB_top(11)	},	/* F11			*/
				{  94,	KB_top(12)	},	/* F12			*/
				{  96,	KB_ArrowD	},	/* Down			*/
				{  97,	KB_ArrowL	},	/* Left			*/
				{  99,	KB_ArrowU	},	/* Up			*/
				{ 101,	KB_left(4)	},	/* End			*/
				{ 103,	KB_left(1)	},	/* Insert		*/
				{ 105,	KB_right(10)	},	/* Num-1		*/
				{ 106,	KB_ArrowR	},	/* Right		*/
				{ 107,	KB_right(7)	},	/* Num-4		*/
				{ 108,	KB_right(4)	},	/* Num-7		*/
				{ 109,	KB_left(5)	},	/* Page Down		*/
				{ 110,	KB_left(2)	},	/* Home			*/
				{ 111,	KB_left(3)	},	/* Page Up		*/
				{ 112,	KB_right(13)	},	/* Num-0		*/
				{ 113,	KB_right(14)	},	/* Num-.		*/
				{ 114,	KB_right(11)	},	/* Num-2		*/
				{ 115,	KB_right(8)	},	/* Num-5		*/
				{ 116,	KB_right(9)	},	/* Num-6		*/
				{ 117,	KB_right(5)	},	/* Num-8		*/
				{ 119,	KB_right(1)	},	/* Num-/		*/
				{ 121,	KB_right(15)	},	/* Num-Enter		*/
				{ 122,	KB_right(12)	},	/* Num-3		*/
				{ 124,	KB_right(16)	},	/* Num-+		*/
				{ 125,	KB_right(6)	},	/* Num-9		*/
				{ 126,	KB_right(2)	},	/* Num-*		*/
				{ 132,	KB_right(3)	},	/* Num--		*/
			};
#endif /* distribution */
static	struct	key	ibmSG_keys[] = {	/* IBM keyboard (Silicon Graphics) keys	*/
				{  65,	KB_right(10)	},	/* Num-1		*/
				{  66,	KB_right(13)	},	/* Num-0		*/
				{  70,	KB_right(7)	},	/* Num-4		*/
				{  71,	KB_right(11)	},	/* Num-2		*/
				{  72,	KB_right(12)	},	/* Num-3		*/
				{  73,	KB_right(14)	},	/* Num-.		*/
				{  74,	KB_right(4)	},	/* Num-7		*/
				{  75,	KB_right(5)	},	/* Num-8		*/
				{  76,	KB_right(8)	},	/* Num-5		*/
				{  77,	KB_right(9)	},	/* Num-6		*/
				{  80,	KB_ArrowL	},	/* Left			*/
				{  81,	KB_ArrowD	},	/* Down			*/
				{  82,	KB_right(6)	},	/* Num-9		*/
				{  83,	KB_right(3)	},	/* Num--		*/
				{  87,	KB_ArrowR	},	/* Right		*/
				{  88,	KB_ArrowU	},	/* Up			*/
				{  89,	KB_right(15)	},	/* Num-Enter		*/
				{  94,	KB_top(1)	},	/* F1			*/
				{  95,	KB_top(2)	},	/* F2			*/
				{  96,	KB_top(3)	},	/* F3			*/
				{  97,	KB_top(4)	},	/* F4			*/
				{  98,	KB_top(5)	},	/* F5			*/
				{  99,	KB_top(6)	},	/* F6			*/
				{ 100,	KB_top(7)	},	/* F7			*/
				{ 101,	KB_top(8)	},	/* F8			*/
				{ 102,	KB_top(9)	},	/* F9			*/
				{ 103,	KB_top(10)	},	/* F10			*/
				{ 104,	KB_top(11)	},	/* F11			*/
				{ 105,	KB_top(12)	},	/* F12			*/
				{ 109,	KB_left(1)	},	/* Insert		*/
				{ 110,	KB_left(2)	},	/* Home			*/
				{ 111,	KB_left(3)	},	/* Page Up		*/
				{ 112,	KB_left(4)	},	/* End			*/
				{ 113,	KB_left(5)	},	/* Page Down		*/
				{ 115,	KB_right(1)	},	/* Num-/		*/
				{ 116,	KB_right(2)	},	/* Num-*		*/
				{ 117,	KB_right(16)	},	/* Num-+		*/
			};
static	struct	key	sun3_keys[] = {		/* sun 3 keyboard keys			*/
				{   8,	KB_left(1)	},	/* L1			*/
				{  10,	KB_left(2)	},	/* L2			*/
				{  12,	KB_top(1)	},	/* F1			*/
				{  13,	KB_top(2)	},	/* F2			*/
				{  15,	KB_top(3)	},	/* F3			*/
				{  17,	KB_top(4)	},	/* F4			*/
				{  19,	KB_top(5)	},	/* F5			*/
				{  21,	KB_top(6)	},	/* F6			*/
				{  23,	KB_top(7)	},	/* F7			*/
				{  24,	KB_top(8)	},	/* F8			*/
				{  25,	KB_top(9)	},	/* F9			*/
				{  26,	KB_top(10)	},	/* Break		*/
				{  28,	KB_right(1)	},	/* R1			*/
				{  29,	KB_right(2)	},	/* R2			*/
				{  30,	KB_right(3)	},	/* R3			*/
				{  32,	KB_left(3)	},	/* L3			*/
				{  33,	KB_left(4)	},	/* L4			*/
				{  52,	KB_right(4)	},	/* R4			*/
				{  53,	KB_right(5)	},	/* R5			*/
				{  54,	KB_right(6)	},	/* R6			*/
				{  56,	KB_left(5)	},	/* L5			*/
				{  58,	KB_left(6)	},	/* L6			*/
				{  75,	KB_right(7)	},	/* R7			*/
				{  76,	KB_ArrowU	},	/* Up			*/
				{  77,	KB_right(9)	},	/* R9			*/
				{  79,	KB_left(7)	},	/* L7			*/
				{  80,	KB_left(8)	},	/* L8			*/
				{  98,	KB_ArrowL	},	/* Left			*/
				{  99,	KB_right(11)	},	/* R11			*/
				{ 100,	KB_ArrowR	},	/* Right		*/
				{ 102,	KB_left(9)	},	/* L9			*/
				{ 104,	KB_left(10)	},	/* L10			*/
				{ 118,	KB_Linefeed	},	/* LF			*/
				{ 119,	KB_right(13)	},	/* R13			*/
				{ 120,	KB_ArrowD	},	/* Down			*/
				{ 121,	KB_right(15)	},	/* R15			*/
			};
static	struct	key	sparc_keys[] = {	/* sun 4 keyboard keys			*/
				{   8,	KB_left(1)	},	/* L1/Stop		*/
				{  10,	KB_left(2)	},	/* L2/Again		*/
				{  12,	KB_top(1)	},	/* F1			*/
				{  13,	KB_top(2)	},	/* F2			*/
				{  14,	KB_top(10)	},	/* F10			*/
				{  15,	KB_top(3)	},	/* F3			*/
				{  16,	KB_top(11)	},	/* F11			*/
				{  17,	KB_top(4)	},	/* F4			*/
				{  18,	KB_top(12)	},	/* F12			*/
				{  19,	KB_top(5)	},	/* F5			*/
				{  21,	KB_top(6)	},	/* F6			*/
				{  23,	KB_top(7)	},	/* F7			*/
				{  24,	KB_top(8)	},	/* F8			*/
				{  25,	KB_top(9)	},	/* F9			*/
				{  28,	KB_right(1)	},	/* R1/Pause		*/
				{  29,	KB_right(2)	},	/* R2/PrSc		*/
				{  30,	KB_right(3)	},	/* R3/Scroll Lock/Break	*/
				{  32,	KB_left(3)	},	/* L3/Props		*/
				{  33,	KB_left(4)	},	/* L4/Undo		*/
				{  52,	KB_right(4)	},	/* R4/num-=		*/
				{  53,	KB_right(5)	},	/* R5/num-/		*/
				{  54,	KB_right(6)	},	/* R6/num-*		*/
				{  56,	KB_left(5)	},	/* L5/Front		*/
				{  57,	KB_right(19)	},	/* Del/num-.		*/
				{  58,	KB_left(6)	},	/* L6/Copy		*/
				{  75,	KB_right(7)	},	/* R7/Home/num-7	*/
				{  76,	KB_ArrowU	},	/* R8/Up/num-8		*/
				{  77,	KB_right(9)	},	/* R9/PgUp/num-9	*/
				{  78,	KB_right(16)	},	/* num--		*/
				{  79,	KB_left(7)	},	/* L7/Open		*/
				{  80,	KB_left(8)	},	/* L8/Paste		*/
				{  97,	KB_right(20)	},	/* Enter		*/
				{  98,	KB_ArrowL	},	/* R10/Left/num-4	*/
				{  99,	KB_right(11)	},	/* R11/num-5		*/
				{ 100,	KB_ArrowR	},	/* R12/Right/num-6	*/
				{ 101,	KB_right(18)	},	/* Ins/num-0		*/
				{ 102,	KB_left(9)	},	/* L9/Find		*/
				{ 104,	KB_left(10)	},	/* L10/Cut		*/
				{ 118,	KB_Linefeed	},	/* LF			*/
				{ 119,	KB_right(13)	},	/* R13/End/num-1	*/
				{ 120,	KB_ArrowD	},	/* R14/Down/num-2	*/
				{ 121,	KB_right(15)	},	/* R15/PgDn/num-3	*/
				{ 125,	KB_left(11)	},	/* Help			*/
				{ 132,	KB_right(17)	},	/* num-+		*/
			};
static	struct	key	*the_keys;		/* the key list we're actually using	*/
static	int		num_keys;		/* the number of entries in the key list*/
	

/* getColorFromName() yields a color instance corresponding to the resource name provided.
 *
 * input <- specified resource of the form <object name>.<color specification> xor
 *          color name xor
 *	     hexadecimal encoding with the form #RGB, #RRGGBB, #RRRGGGBBB,
 *            xor #RRRRGGGGBBBB
 * output -> color object corresponding to the resource xor color name
 *           the color object for grey50 is returned if no resource xor color matches
 *
 * Assumptions: The user preference database ColorDB colorDB is available.
 */
Color
getColorFromName(char *name)
{
char			res_name[1024];		/* full name of resource		*/
char			*repr_type;		/* representation type of returned value*/
XrmValue		val;			/* value returned			*/

    (void) sprintf(res_name, "%s.", D_resource_identifier);	/* construct resource's name	*/
    (void) strcat(res_name, name);
    (void) strcat(res_name, monochrome_screen ? ".monochrome" : ".color");

    if (!XrmGetResource((XrmDatabase) colorDB, res_name,
			(char *) NULL, /* ignore class name */
			&repr_type, &val))
        return color(name);
    else
        return color(val.addr);
}


/* Allocate a color using red, green, and blue specifications.
 *
 * input <- three sixteen bit numbers specifying red, green, and blue pixel values
 *          only the least significant sixteen bits of each input is used
 * output -> color object corresponding to the given pixel values
 *           the grey50 color object is returned if no space in the colormap is available
 *           NULL_COLOR is returned in the rare case that no more colors can be allocated
 *
 * Assumptions:  Global variables x_display and x_screen are set.
 */
Color
getColorFromRGB(int red, int green, int blue)
{
unsigned r = red & 0xFFFF;			/* use least significant sixteen bits	*/
unsigned g = green & 0xFFFF;
unsigned b = blue & 0xFFFF;
char RGB_specification[1+4+4+4+1];		/* RGB string has form #RRRRGGGGBBBB	*/

        (void) sprintf(RGB_specification, "#%.4X%.4X%.4X", r, g, b);
        return color(RGB_specification);
}


/* Return RGB values for a color.
 *
 * input <- specified resource of the form <object name>.<color specification> xor
 *          color name xor
 *	     hexadecimal encoding with the form #RGB, #RRGGBB, #RRRGGGBBB,
 *            xor #RRRRGGGGBBBB
 *	    pointers to three integers
 * output -> none
 * side effect:	the locations pointed to by red, green, and blue receive the
 *		RGB color values associated with the specified color,
 *		such that, getColorFromRGB(red, green, blue) = getColorFromName(name)
 *		If the specified color does not reside in the color database or the
 *		hexadecimal string does has the wrong format, red, green, and blue are
 *		assigned 0, 0, 0.
 */
void
getRGBFromName(char *name, int *red, int *green, int *blue)
{
char			res_name[1024];		/* full name of resource		*/
char			*repr_type;		/* representation type of returned value*/
XrmValue		val;			/* value returned			*/
XColor			col;			/* value returned			*/
int			status;			/* success status of function call	*/

    (void) sprintf(res_name, "%s.", D_resource_identifier);	/* construct resource's name	*/
    (void) strcat(res_name, name);
    (void) strcat(res_name, monochrome_screen ? ".monochrome" : ".color");

    status = XrmGetResource((XrmDatabase) colorDB, res_name,
			(char *) NULL, /* ignore class name */
			&repr_type, &val);
    if ((status = XParseColor(x_display, DefaultColormap(x_display, x_screen),
			      status ? val.addr : name, &col))) {
		*red = col.red;
		*green = col.green;
		*blue = col.blue;
	}
    else
	{
		*red = *green = *blue = 0;
	}
}


/* color() yields a color instance corresponding to the color name provided.
 *
 * input <- color name or hexadecimal encoding with the form #RGB, #RRGGBB, #RRRGGGBBB,
 *            xor #RRRRGGGGBBBB
 * output -> color object corresponding to the color name
 *           the color object for grey50 is returned if no resource xor color matches
 *           NULL_COLOR is returned in the rare case that no more colors can be allocated
 *
 * Assumptions:  Global variables x_display and x_screen are set.
 */
static
Color
color(char *name)
{
XColor			col;			/* value returned			*/
int			status;			/* success status of function call	*/
Generic			c;			/* color in hash table			*/

	/* Check for an already allocated color.					*/
	if (NameValueTableQueryPair(colorHashTable, (Generic) name, &c))
		return (Color) c;
	else {
		/* Allocate color and add to the hash table.				*/
	        status = XParseColor(x_display, DefaultColormap(x_display, x_screen),
				     name, &col);
		if ( status )  {
		  status = XAllocColor(x_display, DefaultColormap(x_display, x_screen),
				       &col);
		  if ( status )  {
		  	/* save the XColor */
		  	char* cname = (char*)get_mem(strlen(name)+1, "color(%s) name", name);
			Generic xcol = (Generic)get_mem(sizeof(XColor), "color (%s) XColor",
                                                        name);
			strcpy(cname, name);
			bcopy((const char*)&col, (char*)xcol, sizeof(XColor));
			NameValueTableAddPair(colorHashTable, (Generic)cname, xcol, (Generic*)0);
			return (Color)xcol;
		  }
		  else
		    return NULL_COLOR;
		}
		else
		  return color("grey50");
	}
}


/* destroyColor() deallocates color pair in colorHashTable.
 *
 * input <- color string name, XColor for name, ignored argument
 *
 * The colorHashTable contains pairs (name, XColor), where name is an allocated
 * color string and XColor is its associated X color object.  This pair is
 * is removed from the colorHashTable.
 */
static
void
destroyColor(Generic name, Generic value, Generic ignored)
{
Generic			cname;			/* color name in hash table		*/
Generic			cvalue;			/* XColor object in table, same as value*/

	if (NameValueTableDeletePair(colorHashTable, name, &cname, &cvalue))  {
		free_mem((void*)name);
		free_mem((void*)cvalue);
	}
	else
		die_with_message("destroyColor() called with name (%s) not in colorHashTable", (char *) name);

	return;
}


/* getColorDB() yields a ColorDB database, to be used with getColorFromName(),
 * which yields the user's choice for the specified object.
 *
 * input <- colorsFile, name of file specified using config.c
 * output -> ColorDB database, assigned to colorDB.
 *
 * Different locations for user supplied defaults are searched, according
 * to the order specified by X11R4.
 *    application specific file "D_color_defaults_file"
 *       specified in config.c
 *    window manager's property xor .Xdefaults file
 *    XENVIRONMENT file xor .Xdefaults-<host> file
 * Note that command line arguments are not used.
 *
 * XrmInitialize() must be called before calling this function.
 * XDisplay *x_display should point to the window's display structure.
 * D_color_defaults_file should contain default colors for all uses of
 * color in the environment.
 *
 * Code based on X11R4/contrib/examples/OReilly/Xlib/basecalc/basecalc.c/-
 * GetUsersDatabase().
 */

static
ColorDB
getColorDB(char *colorsFile)
{
XrmDatabase		rDB;			/* database with color preferences      */
char			*environment;		/* name of defaults file for ParaScope  */
char			filename[1024+MAXHOSTNAMELEN];
						/* name of defaults file                */

    /* Load the application specific file, usually "D_color_defaults_file." */
    rDB = XrmGetFileDatabase(colorsFile);

    if (rDB == NULL)
    {
      fprintf(stderr,
	      "Cannot file color default definitions file %s.\n", colorsFile);
      fprintf(stderr,
	      "Please install a default file, or set the location of the \n");
      fprintf(stderr,
	      "using your ~/.dsystemrc file.\n");
      exit(-1);
    }

    /* MERGE server defaults, these are created by xrdb, loaded as a
     * property of the root window when the server initializes, and
     * loaded into the display structure on XOpenDisplay.  If not defined,
     * use ~/.Xdefaults  */
    if (XResourceManagerString(x_display) != NULL) {
        XrmMergeDatabases(XrmGetStringDatabase(XResourceManagerString(x_display)),
			  &rDB);
    }
    else if (getenv("HOME") != (char *) NULL)  {
        /* Open ~/.Xdefaults file and merge into existing data base */
	(void) strcpy(filename, getenv("HOME"));
	(void) strcat(filename, "/.Xdefaults");
	XrmMergeDatabases(XrmGetFileDatabase(filename), &rDB);
    }

    /* Open XENVIRONMENT file, or if not defined, the ~/.Xdefaults-<hostname>,
     * and merge into existing data base */
    if ((environment = getenv("XENVIRONMENT")) == (char *) NULL ||
	!strlen(environment)) {
        if ((environment = getenv("HOME")) != (char *) NULL &&
	    (int)strlen(environment) > 0)  {
	    int len;
	    (void) strcpy(filename, environment);
	    environment = strcat(filename, "/.Xdefaults-");
	    len = strlen(environment);
	    if (gethostname(environment+len, 1024+MAXHOSTNAMELEN-len))
	        die_with_message("getColorDB(): gethostname() failed!");
	}
	else
	  environment = (char *) NULL;
    }
    XrmMergeDatabases(XrmGetFileDatabase(environment), &rDB);

    return (ColorDB) rDB;
}


/* Compare two strings.									*/
static
int
colorHashTableCompareFunc(Generic name1, Generic name2)
{
  return strcmp((char *) name1, (char *) name2);
}


/* Convert color name into hash number.							*/
static
int
colorHashTableHashFunc(Generic name, unsigned int ht_size)
{
  return hash_string((char *) name, (int) ht_size);
}


/* Create a new white pixmap of size 'size'.						*/
Pixmap
makePixmap(Point size, char *who)
{
PPixmap			*pb;			/* the new pixmap			*/

	pb = (PPixmap *) get_mem(sizeof(PPixmap), "new pixmap by %s", who);
	pb->size = size;
	INHIBIT_FLUSH();
	pb->drawable = XCreatePixmap(x_display, x_window, size.x, size.y, PIXMAP_DEPTH);
	ColorWithPattern((Pixmap) pb, makeRectFromSize(Origin, size), NULL_BITMAP, Origin, false, default_background_color, default_background_color);
	OK_TO_FLUSH();
	return ((Pixmap) pb);
}


/* Free a created pixmap 'b'.								*/
void
freePixmap(Pixmap b)
{
PPixmap			*pb = (PPixmap *) b;	/* the pixmap				*/

	INHIBIT_FLUSH();
	XFreePixmap(x_display, pb->drawable);
	OK_TO_FLUSH();
	free_mem((void*) pb);
}


/* Obtain a Bitmap (bit plane) from screen depth pixmap.					*/
Bitmap
flattenPixmap(Pixmap b, Rectangle r, Color foreground, Color background)
{
PPixmap			*pb = (PPixmap *) b;	/* private pixmap type			*/
Bitmap			m;			/* photo bitmap to return			*/
PBitmap			*pm;			/* private bitmap type, contains bits	*/
Point			r_size;			/* size of region to copy		*/
XGC			gc;			/* the correct graphics context		*/
unsigned long		plane_bitmap;		/* bit specifying plane to copy		*/
unsigned long		color_diff;		/* pixel bits differing in fg and bg	*/

	color_diff = COLOR_PIXEL(foreground) ^ COLOR_PIXEL(background);
	if (!color_diff)
		die_with_message("flattenPixmap(): must specify different fg and bg colors");

	/* Determine the area to copy.	*/
	r = interRect(r, makeRectFromSize(Origin, getPixmapSize(b)));
	if (NOT(positiveArea(r)))
		return NULL_BITMAP;
	else
		r_size = sizeRect(r);

	/* Create the bitmap photo.	*/
	m = makeBitmap(r_size, "flattenPixmap()");
	BLTBitmap(NULL_BITMAP, m, makeRectFromSize(Origin, r_size), Origin, BITMAP_CLR, false);
	pm = (PBitmap *) m;

	/* Determine the plane to copy. */
	for (plane_bitmap = 1; !(plane_bitmap & color_diff); plane_bitmap <<= 1)
		;	/* guaranteed to terminate because color_diff != 0 */

	gc = x_gc_bitmap;
	XSetFunction(x_display, gc, pix_Op[BITMAP_COPY]);
	XSetClipMask(x_display, gc, None);
	XCopyPlane(x_display, pb->drawable, pm->drawable, gc, r.ul.x, r.ul.y,
		   r_size.x, r_size.y, 0, 0, plane_bitmap);

	return (Bitmap) pm;
}


/* Get the size of the pixmap 'b'.							*/
Point
getPixmapSize(Pixmap b)
{
PPixmap			*pb = (PPixmap *) b;	/* the pixmap				*/

	if (b == NULL_PIXMAP)
	{/* bogus request */
		die_with_message("getPixmapSize():  null source size requested");
	}
	return pb->size;
}


/* Copy a region from source Pixmap to destination modulo a bitmap.			*/
void
BLTColorCopy(Pixmap src, Pixmap dst, Rectangle srcArea, Point dstOrigin, 
             Bitmap m, Point offset, Boolean clip)
{
PPixmap			*psrc = (PPixmap *) src;/* private source pixmap type		*/
PPixmap			*pdst = (PPixmap *) dst;/* private destination pixmap type	*/
PBitmap			*pm = (PBitmap *) m;	/* private bitmap type			*/
static PBitmap		*pclip = (PBitmap *) NULL;
						/* clip bitmap containing 1's and 0's	*/
Point			srcSize;		/* the size of the source area		*/
Point			save;			/* saved difference between origins	*/
XGC			gc;			/* the correct graphics context		*/

	save = subPoint(dstOrigin, srcArea.ul);

	/* -1. Check for invalid actual arguments. */
	if (src == NULL_PIXMAP)
	{/* cannot copy from the null pixmap */
		die_with_message("BLTCopyColor():  null source");
	}
	else if (dst == NULL_PIXMAP)
	{/* cannot copy to the null pixmap */
		die_with_message("BLTCopyColor():  null destination");
	}

	/* 0. Prepare the region to affect. */
	if (clip)
	{/* clip against the source */
		srcArea = interRect(srcArea, makeRectFromSize(Origin, psrc->size));
		dstOrigin = transPoint(save, srcArea.ul);
	 /* clip the destination */
		srcArea = interRect(srcArea, makeRectFromSize(makePoint(-save.x, -save.y), pdst->size));
		dstOrigin = transPoint(save, srcArea.ul);
	}

	srcSize = sizeRect(srcArea);
	if ((srcSize.x <= 0) || (srcSize.y <= 0))
	{/* we have no work to do */
		return;
	}

	/* Area to affect has origin dstOrigin and size srcSize. */

	INHIBIT_FLUSH();

	if (m != NULL_BITMAP)  {
		/* 1. Produce a clip bitmap the size of the area to copy. */
		if (pclip == (PBitmap *) NULL ||
		    NOT(equalPoint(pclip->size, srcSize)))  {
			/* Create a correctly sized pclip. */
			freeBitmap((Bitmap) pclip);
			pclip = (PBitmap *) makeBitmap(makePoint(srcSize.x, srcSize.y),
						   "mon/mach_x.c/BLTColorCopy() clip bitmap");
		}

		gc = x_gc_bitmap;
		XSetFunction(x_display, gc, pix_Op[BITMAP_COPY]);
		XSetFillStyle(x_display, gc, FillOpaqueStippled);
		XSetStipple(x_display, gc, pm->drawable);
		XSetTSOrigin(x_display, gc, offset.x, offset.y);

		XFillRectangle(x_display, pclip->drawable, gc, 0,0, srcSize.x, srcSize.y);

		/* 2. Copy from the source to the destination through the clip bitmap. */
		gc = x_gc_window;
		XSetFunction(x_display, gc, GXcopy);
		XSetPlaneMask(x_display, gc, (unsigned long) AllPlanes);
		XSetClipMask(x_display, gc, pclip->drawable);
		XSetClipOrigin(x_display, gc, dstOrigin.x, dstOrigin.y);

		XCopyArea(x_display, psrc->drawable, pdst->drawable, gc, srcArea.ul.x,
			  srcArea.ul.y, srcSize.x, srcSize.y, dstOrigin.x, dstOrigin.y);
	}
	else  {	/** m == NULL_BITMAP */
		gc = x_gc_window;
		XSetFunction(x_display, gc, GXcopy);
		XSetPlaneMask(x_display, gc, (unsigned long) AllPlanes);
		XSetClipMask(x_display, gc, None);
		XCopyArea(x_display, psrc->drawable, pdst->drawable, gc, srcArea.ul.x,
			  srcArea.ul.y, srcSize.x, srcSize.y, dstOrigin.x, dstOrigin.y);
	}

	if (dst == screen_pixmap)
		XFlush(x_display);		/* display BLT to screen quickly	*/
	OK_TO_FLUSH();

	return;
}


/* Cover an area with a pattern.	*/
void
ColorWithPattern(Pixmap dst, Rectangle dstArea, Bitmap pattern, Point offset, 
                 Boolean clip, Color foreground, Color background)
{
PPixmap			*pdst = (PPixmap *) dst;/* private destination pixmap type	*/
PBitmap			*ppattern = (PBitmap *) pattern;
						/* private bitmap type			*/
Point			dstSize;		/* the size of the destination area	*/
XRectangle		xrectangle;		/* screen clipping rectangle (x)	*/
XGC			gc;			/* the correct graphics context		*/


	if (dst == NULL_PIXMAP)
	{/* cannot copy to the null pixmap */
		die_with_message("ColorWithPattern():  null destination");
	}

	if (clip)	/** Is the clipping correct? */
	{/* clip the destination */
		dstArea = interRect(dstArea, makeRectFromSize(Origin, pdst->size));
	}

	dstSize = sizeRect(dstArea);
	if ((dstSize.x <= 0) || (dstSize.y <= 0))
	{/* we have no work to do */
		return;
	}

	xrectangle.x      = dstArea.ul.x;
	xrectangle.y      = dstArea.ul.y;
	xrectangle.width  = dstSize.x;
	xrectangle.height = dstSize.y;

	INHIBIT_FLUSH();
	gc = x_gc_window;
	XSetClipRectangles(x_display, gc, 0, 0, &xrectangle, 1 /* count */, Unsorted);
	XSetPlaneMask(x_display, gc, (unsigned long) AllPlanes);
	XSetFunction(x_display, gc, GXcopy);
	XSetTSOrigin(x_display, gc, offset.x, offset.y);

	if (foreground == NULL_COLOR)  {
		if (background == NULL_COLOR || pattern == NULL_BITMAP)
			return;	/* no work to do */
		else  {
			XSetFillStyle(x_display, gc, FillStippled);
			XSetForeground(x_display, gc, COLOR_PIXEL(background));
			BLTBitmap(NULL_BITMAP, pattern, dstArea, Origin, BITMAP_INV, false);
			XSetStipple(x_display, gc, ppattern->drawable);
			XFillRectangle(x_display, pdst->drawable, gc, dstArea.ul.x,
				       dstArea.ul.y, dstSize.x, dstSize.y);
			BLTBitmap(NULL_BITMAP, pattern, dstArea, Origin, BITMAP_INV, false);
						/* return the pattern to original form	*/
		}
	}
	else  { /* foreground != NULL_COLOR */
		if (pattern == NULL_BITMAP)
			XSetFillStyle(x_display, gc, FillSolid);
		else if (background == NULL_COLOR)
			XSetFillStyle(x_display, gc, FillStippled);
		else
			XSetFillStyle(x_display, gc, FillOpaqueStippled);
		XSetForeground(x_display, gc, COLOR_PIXEL(foreground));
		if (pattern != NULL_BITMAP)
			XSetStipple(x_display, gc, ppattern->drawable);
		if (background != NULL_COLOR)
			XSetBackground(x_display, gc, COLOR_PIXEL(background));
		XFillRectangle(x_display, pdst->drawable, gc, dstArea.ul.x,
			       dstArea.ul.y, dstSize.x, dstSize.y);
	}

	if (dst == screen_pixmap)
		XFlush(x_display);		/* display BLT to screen quickly	*/
	OK_TO_FLUSH();

	return;
}


/* Attempt to invert the colors in a pixmap.	*/
void
invertPixmap(Pixmap dst, Rectangle dstArea, Boolean clip)
{
PPixmap			*pdst = (PPixmap *) dst;/* private destination pixmap type	*/
Point			dstSize;		/* the size of the source area		*/
XRectangle		xrectangle;		/* screen clipping rectangle (x)	*/
XGC			gc;			/* the correct graphics context		*/

	if (dst == NULL_PIXMAP)
	{/* cannot BLT to the null pixmap */
		die_with_message("BLT():  null destination");
	}
	else if (clip)
	{/* clip the destination */
		dstArea = interRect(dstArea, makeRectFromSize(Origin, pdst->size));
	}

	dstSize = sizeRect(dstArea);
	if ((dstSize.x <= 0) || (dstSize.y <= 0))
	{/* we have no work to do */
		return;
	}
	xrectangle.x      = dstArea.ul.x;
	xrectangle.y      = dstArea.ul.y;
	xrectangle.width  = dstSize.x;
	xrectangle.height = dstSize.y;

	INHIBIT_FLUSH();
	gc = x_gc_window;
	XSetClipRectangles(x_display, gc, 0, 0, &xrectangle, 1 /* count */, Unsorted);
	XSetFunction(x_display, gc, GXinvert);
	XCopyArea(x_display, pdst->drawable, pdst->drawable, gc,
		  dstArea.ul.x, dstArea.ul.y, dstSize.x, dstSize.y,
		  dstArea.ul.x, dstArea.ul.y);

	if (dst == screen_pixmap)
	{/* an op to the screen should show up quickly */
		XFlush(x_display);
	}
	OK_TO_FLUSH();
}


/* Create a new Bitmap with all bits set.	*/
Bitmap
makeBitmap(Point size, char *who)
{
PBitmap			*pm;			/* the new bitmap				*/

	pm = (PBitmap *) get_mem(sizeof(PBitmap), "new bitmap by %s", who);
	pm->size = size;
	INHIBIT_FLUSH();
	pm->drawable = XCreatePixmap(x_display, x_window, size.x, size.y, BITMAP_DEPTH);
	BLTBitmap(NULL_BITMAP, (Bitmap) pm, makeRectFromSize(Origin, size), Origin, BITMAP_SET, false);
	OK_TO_FLUSH();
	return ((Bitmap) pm);
}


/* Create a new Bitmap containing the specified bits.	*/
Bitmap
makeBitmapFromData(Point size, BITMAPM_UNIT *data, char *who)
{
PBitmap			*pm = (PBitmap *) NULL;	/* the private bitmap type, bitmap returned	*/
XGC			gc = x_gc_bitmap;		/* graphics context to use		*/
XRectangle		xrectangle;		/* pixmap clipping rectangle (x)	*/
XImage			*image;			/* dummy image structure for our data	*/

	INHIBIT_FLUSH();
	pm = (PBitmap *) get_mem(sizeof(PBitmap), "new bitmap by %s", who);
	pm->size = size;
	pm->drawable = XCreatePixmap(x_display, x_window, MAX(1, size.x), MAX(1, size.y), BITMAP_DEPTH);

	/* Prepare the image.	*/
	image = XCreateImage(x_display, DefaultVisual(x_display, x_screen), BITMAP_DEPTH, XYPixmap, 0, (char *) data, size.x, size.y, BITMAPM_BITS_PER_UNIT, 0);
	image->byte_order       = BITMAPM_BYTE_ORDER;
	image->bitmap_unit      = BITMAPM_BITS_PER_UNIT;
	image->bitmap_bit_order = BITMAPM_BIT_ORDER;
	xrectangle.x      = 0;
	xrectangle.y      = 0;
	xrectangle.width  = size.x;
	xrectangle.height = size.y;
	XSetClipRectangles(x_display, gc, 0, 0, &xrectangle, 1 /* count */, Unsorted);

	XSetFunction(x_display, gc, pix_Op[BITMAP_COPY]);
	XPutImage(x_display, pm->drawable, gc, image, 0, 0, 0, 0, size.x, size.y);

	image->data = NULL; 	/* XDestroyImage will free our data if we don't do this */
	XDestroyImage(image);
	OK_TO_FLUSH();

	return (Bitmap) pm;
}


/* Free a created bitmap m.								*/
void
freeBitmap(Bitmap m)
{
PBitmap			*pm = (PBitmap *) m;	/* the bitmap				*/

	if (m == NULL_BITMAP)
		die_with_message("freeBitmap():  may not free NULL_BITMAP");

	INHIBIT_FLUSH();
	XFreePixmap(x_display, pm->drawable);
	OK_TO_FLUSH();
	free_mem((void*) pm);
}


/* Get the size of the bitmap 'b'.							*/
Point
getBitmapSize(Bitmap m)
{
PBitmap			*pm = (PBitmap *) m;	/* the bitmap				*/

	if (m == NULL_BITMAP)
	{/* bogus request */
		die_with_message("getBitmapSize():  null source size requested");
	}
	return pm->size;
}


/* Get the data from the bitmap m.							*/
void
getBitmapData(Bitmap m, BITMAPM_UNIT *data)
{
PBitmap			*pm = (PBitmap *) m;	/* the bitmap				*/
XImage			*tmp_image;		/* the temporary image			*/
int			src_bytes_per_line;	/* the number of bytes in a source line	*/
int			dst_bytes_per_line;	/* the number of bytes in a dest line	*/
char			*src_ptr, *dst_ptr;	/* source & destination ptrs for coping	*/
char			*ptr, c;		/* the current pointer			*/
int			y;			/* the current line being copied	*/
int			x;			/* the current byte being inverted	*/
static	char		rev4[] = {		/* the reversal of 4 bits		*/
				0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE,
				0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF,
			};

	if (m == NULL_BITMAP)
	{/* bogus request */
		die_with_message("getBitmapData():  null source data requested");
	}

	INHIBIT_FLUSH();
	tmp_image = XGetImage(x_display, pm->drawable, 0, 0, pm->size.x, pm->size.y, FLAT_PLANE_BITMAP, XYPixmap);
	OK_TO_FLUSH();

	src_bytes_per_line = tmp_image->bytes_per_line;
	src_ptr = tmp_image->data;
	dst_bytes_per_line = BITMAPM_UNITS_PER_LINE(pm->size.x) * sizeof(BITMAPM_UNIT);
	dst_ptr = (char *) data;
	for (y = 0; y < tmp_image->height; y++)
	{/* copy each line, doing any necessary twiddling */
		if (tmp_image->byte_order != tmp_image->bitmap_bit_order && tmp_image->bitmap_unit == 16)
		{/* swap pairs of bytes in this row */
			for (x = 0, ptr = src_ptr; x < src_bytes_per_line; x += 2, ptr += 2)
			{/* swap this byte with the next */
				c = ptr[1]; ptr[1] = ptr[0]; ptr[0] = c;
			}
		}
		else if (tmp_image->byte_order != tmp_image->bitmap_bit_order && tmp_image->bitmap_unit == 32)
		{/* reverse four bytes in this row */
			for (x = 0, ptr = src_ptr; x < src_bytes_per_line; x += 4, ptr += 4)
			{/* reverse the order of the next four bytes */
				c = ptr[3]; ptr[3] = ptr[0]; ptr[0] = c;
				c = ptr[2]; ptr[2] = ptr[1]; ptr[1] = c;
			}
		}
		if (tmp_image->bitmap_bit_order != BITMAPM_BIT_ORDER)
		{/* reverse each byte in this row of data */
			for (ptr = src_ptr, x = 0; x < src_bytes_per_line; x++, ptr++)
			{/* reverse this byte */
				*ptr = rev4[*ptr >> 4] | rev4[*ptr & 0xF] << 4;
			}
		}
		(void) bcopy(src_ptr, dst_ptr, MIN(src_bytes_per_line, dst_bytes_per_line));
		src_ptr += src_bytes_per_line;
		dst_ptr += dst_bytes_per_line;
	}

	INHIBIT_FLUSH();
	XDestroyImage(tmp_image);
	OK_TO_FLUSH();
}


/* Set the data into the bitmap m.							*/
void
setBitmapData(Bitmap m, BITMAPM_UNIT *data)
{
PBitmap			*pm = (PBitmap *) m;	/* the bitmap				*/
XRectangle		xrectangle;		/* bitmap clipping rectangle (x)		*/
XImage			*image;			/* dummy image structure for our data	*/
XGC			gc;			/* graphics context			*/

	if (m == NULL_BITMAP)
	{/* bogus request */
		die_with_message("setBitmapData():  null source data requested to be set")
;
	}

	INHIBIT_FLUSH();
	image = XCreateImage(x_display, DefaultVisual(x_display, x_screen), BITMAP_DEPTH, XYBitmap
, 0, (char *) data, pm->size.x, pm->size.y, BITMAPM_BITS_PER_UNIT, 0);
	image->byte_order	= BITMAPM_BYTE_ORDER;
	image->bitmap_unit	= BITMAPM_BITS_PER_UNIT;
	image->bitmap_bit_order = BITMAPM_BIT_ORDER;
	xrectangle.x	  = 0;
	xrectangle.y	  = 0;
	xrectangle.width  = pm->size.x;
	xrectangle.height = pm->size.y;

	gc = x_gc_bitmap;
	XSetClipRectangles(x_display, gc, 0, 0, &xrectangle, 1 /* count */, Unsorted);
	/* Dump the data directly into the bitmap. */
	XSetFunction(x_display, gc, pix_Op[BITMAP_COPY]);
	XPutImage(x_display, pm->drawable, gc, image, 0, 0, 0, 0, pm->size.x, pm->size.y);

	image->data = NULL;/* XDestroyImage() will free our data if we don't do this!!! */
	XDestroyImage(image);
	OK_TO_FLUSH();
}


/* Modify the destination Bitmap using the source Bitmap and the graphics operation.	*/
void
BLTBitmap(Bitmap src, Bitmap dst, Rectangle srcArea, Point dstOrigin, 
          int op, Boolean clip)
{
PBitmap			*psrc = (PBitmap *) src;	/* private source bitmap type		*/
PBitmap			*pdst = (PBitmap *) dst;	/* private destination bitmap type	*/
Point			save;			/* saved difference between origins	*/
Point			srcSize;		/* the size of the source area		*/
XRectangle		xrectangle;		/* screen clipping rectangle (x)	*/
XGC			gc;			/* the correct graphics context		*/

	save = subPoint(dstOrigin, srcArea.ul);

	if (src != NULL_BITMAP && clip)
	{/* clip against the source */
		srcArea = interRect(srcArea, makeRectFromSize(Origin, psrc->size));
		dstOrigin = transPoint(save, srcArea.ul);
	}

	if (dst == NULL_BITMAP)
	{/* cannot BLT to the null pixmap */
		die_with_message("BLTBitmap():  null destination");
	}
	else if (clip)
	{/* clip the destination */
		srcArea = interRect(srcArea, makeRectFromSize(makePoint(-save.x, -save.y), pdst->size));
		dstOrigin = transPoint(save, srcArea.ul);
	}

	srcSize = sizeRect(srcArea);
	if (srcSize.x <= 0 || srcSize.y <= 0 || op == BITMAP_NOOP)
	{/* we have no work to do */
		return;
	}

	xrectangle.x      = dstOrigin.x;
	xrectangle.y      = dstOrigin.y;
	xrectangle.width  = srcSize.x;
	xrectangle.height = srcSize.y;

	INHIBIT_FLUSH();
	gc = x_gc_bitmap;
	XSetClipRectangles(x_display, gc, 0, 0, &xrectangle, 1 /* count */, Unsorted);

	if (src == NULL_BITMAP)
	{/* null to pixmap */
		XSetFunction(x_display, gc, pix_Op[BLACK_SRC(op)]);
		XSetFillStyle(x_display, gc, FillSolid);
		XFillRectangles(x_display, pdst->drawable, gc, &xrectangle, 1 /* count */);
	}
	else
	{/* pixmap to pixmap */
		XSetFunction(x_display, gc, pix_Op[op]);
		XCopyArea(x_display, psrc->drawable, pdst->drawable, gc,
			  srcArea.ul.x, srcArea.ul.y, srcSize.x, srcSize.y,
			  dstOrigin.x, dstOrigin.y);
	}

	OK_TO_FLUSH();
}

/*  I added trash_data and trash_data2 since the format of the function  */
/*  call had to match the prototype (which had to match the form for     */
/*  mach_none.                              --curetonk                   */

/* Create a brand new MouseCursor from the information provided.			*/
MouseCursor
makeMouseCursorFromData(Point size, 
                        BITMAPM_UNIT *single_data, short single_op, 
                        BITMAPM_UNIT *shape_data, short trash_data,
                        BITMAPM_UNIT *bitmap_data, short trash_data2,
                        Point hot_spot, char *who)
{
  return makeColorMouseCursorFromData(size, single_data, single_op, shape_data, 
                                      bitmap_data, hot_spot, 
                                      getColorFromName("cursor.foreground"), 
                                      getColorFromName("cursor.background"), who);
}


/*ARGSUSED*/
/* Create a brand new colored MouseCursor from the information provided.		*/
MouseCursor
makeColorMouseCursorFromData(Point size, 
                             BITMAPM_UNIT *single_data, short single_op, 
                             BITMAPM_UNIT *shape_data, 
                             BITMAPM_UNIT *bitmap_data, 
                             Point hot_spot, Color fg, Color bg, char *who)
{
MouseCursorP		mc;			/* the return cursor			*/
Rectangle		r;			/* the maximum cursor region rectangle	*/
Rectangle		cursor_rect;		/* the actual cursor region rectangle	*/
Bitmap			src;			/* Bitmap with bits defining cursor shape	*/
Bitmap			bitmap;			/* cursor bits actually displayed	*/
PBitmap			*psrc;			/* private bitmap type			*/
PBitmap			*pbitmap;			/* private bitmap type			*/

	mc = (MouseCursorP) get_mem(sizeof(MouseCursorStruct), "new cursor by (%s)", who);

	r = makeRectFromSize(Origin, max_cursor_size);
	cursor_rect = interRect(r, makeRectFromSize(Origin, size));

	/* Create Bitmaps. */
	src = makeBitmapFromData(sizeRect(cursor_rect), (BITMAPM_UNIT *) shape_data,
			       "makeColorMouseCursorFromData(): src");
	bitmap = makeBitmapFromData(sizeRect(cursor_rect), (BITMAPM_UNIT *) bitmap_data,
			       "makeColorMouseCursorFromData(): src");
	psrc = (PBitmap *) src;
	pbitmap = (PBitmap *) bitmap;

	mc->foreground = (fg == NULL_COLOR) ? getColorFromName("cursor.foreground") : fg;
	mc->background = (bg == NULL_COLOR) ? getColorFromName("cursor.background") : bg;
	mc->cursor = XCreatePixmapCursor(x_display, psrc->drawable, pbitmap->drawable, (XColor *) mc->foreground, (XColor *) mc->background, MAX(0, MIN(hot_spot.x, max_cursor_size.x - 1)), MAX(0, MIN(hot_spot.y, max_cursor_size.y - 1)));

	freeBitmap(src);
	freeBitmap(bitmap);

	return (MouseCursor) mc;
}


/* change color of cursor								*/
void
recolorMouseCursor(MouseCursor mc, Color fg, Color bg)
{
MouseCursorP		cursor = (MouseCursorP) mc;

	if (fg != NULL_COLOR)
		cursor->foreground = fg;
	if (bg != NULL_COLOR)
		cursor->background = bg;
	XRecolorCursor(x_display, cursor->cursor, 
                       (XColor*)cursor->foreground, (XColor*)cursor->background);

	return;
}


/* Free a created MouseCursor 'mc'.							*/
void
freeMouseCursor(MouseCursor mc)
{
MouseCursorP		mcp = (MouseCursorP) mc;/* the cursor				*/

	INHIBIT_FLUSH();
	XFreeCursor(x_display, mcp->cursor);
	OK_TO_FLUSH();
	free_mem((void*) mc);
}


/* Install a new cursor and return the old cursor.					*/
MouseCursor
CURSOR(MouseCursor New)
{
MouseCursor		previous;		/* the previous cursor number		*/
MouseCursorP		nw = (MouseCursorP) New;/* the new cursor			*/

	if (New == current_cur)
	{/* the cursor hasn't really changed */
		return (New);
	}
	else
	{/* change the cursor */
		INHIBIT_FLUSH();
		XDefineCursor(x_display, x_window, nw->cursor);
		XFlush(x_display);
		OK_TO_FLUSH();

		previous = current_cur;
		current_cur  = New;
		return (previous);
	}
}


/* Plot a colored point in a pixmap.							*/
void
pointColor(Pixmap dst, Point p, Rectangle clipper, Boolean clip, Color c)
{
PPixmap			*pdst = (PPixmap *) dst;/* private destination pixmap type	*/
XRectangle		xrectangle;		/* screen clipping rectangle (x)	*/
XGC			gc;			/* the correct graphics context		*/

	if (clip)
	{/* clip based on what's given us */
		xrectangle = rn2xRect(clipper);
	}
	else
	{/* clip based on the outer border of the drawable */
		xrectangle.x      = 0;
		xrectangle.y      = 0;
		xrectangle.width  = pdst->size.x - 1;
		xrectangle.height = pdst->size.y - 1;
	}
	INHIBIT_FLUSH();
	gc = x_gc_window;
	XSetClipRectangles(x_display, gc, 0, 0, &xrectangle, 1 /* count */, Unsorted);
	XSetForeground(x_display, x_gc_window, COLOR_PIXEL(c == NULL_COLOR ? default_foreground_color : c));
	XSetFunction(x_display, gc, pix_Op[BITMAP_COPY]);
	XDrawPoint(x_display, pdst->drawable, gc, p.x, p.y);

	if (dst == screen_pixmap)
		XFlush(x_display);		/* display BLT to screen quickly	*/

	OK_TO_FLUSH();
	return;
}


/* Plot a set of colored points in a pixmap.						*/
void
polyColorPoint(Pixmap dst, Point origin, short n, Point *pp, 
               Rectangle clipper, Boolean clip, Color c)
{
PPixmap			*pdst = (PPixmap *) dst;/* private destination pixmap type	*/
XRectangle		xrectangle;		/* screen clipping rectangle (x)	*/
XGC			gc;			/* the correct graphics context		*/
XPoint			*points;		/* the new x point list			*/
short			i;			/* point index				*/
long			maxPoints;		/* maximum number of points X can draw	*/

	if (clip)
	{/* clip based on what's given us */
		xrectangle = rn2xRect(clipper);
	}
	else
	{/* clip based on the outer border of the drawable */
		xrectangle.x      = 0;
		xrectangle.y      = 0;
		xrectangle.width  = pdst->size.x - 1;
		xrectangle.height = pdst->size.y - 1;
	}

	points = (XPoint *) get_mem(sizeof(XPoint) * n, "polyColorPoint()");
	for (i = 0; i < n; i++)
	{/* coerce a ParaScope point to an X point */
		points[i].x = pp[i].x + origin.x;
		points[i].y = pp[i].y + origin.y;
	}

	maxPoints = XMaxRequestSize(x_display) - 3;
	INHIBIT_FLUSH();
	gc = x_gc_window;
	XSetClipRectangles(x_display, gc, 0, 0, &xrectangle, 1 /* count */, Unsorted);
        XSetForeground(x_display, x_gc_window, COLOR_PIXEL(c == NULL_COLOR ? default_foreground_color : c));
        XSetFunction(x_display, gc, GXcopy);
	for (i = 0; i < n; i += maxPoints)
		/* can draw at most maxPoints at one time */
		XDrawPoints(x_display, pdst->drawable, gc, points + i,
			    MIN(((int) maxPoints), n - i), CoordModeOrigin);

        if (dst == screen_pixmap)
                XFlush(x_display);              /* display BLT to screen quickly        */

	OK_TO_FLUSH();
	free_mem((void*) points);
}


/* Plot a fancy colored line in a pixmap.						*/
void
lineColor(Pixmap dst, Point p1, Point p2, short width, LineStyle *style, 
          Rectangle clipper, Boolean clip, Color c)
{
PPixmap			*pdst = (PPixmap *) dst;/* private destination pixmap type	*/
XRectangle		xrectangle;		/* screen clipping rectangle (x)	*/
XGC			gc;			/* the correct graphics context		*/

	if (clip)
	{/* clip based on what's given us */
		xrectangle = rn2xRect(clipper);
	}
	else
	{/* clip based on the outer border of the drawable */
		xrectangle.x      = 0;
		xrectangle.y      = 0;
		xrectangle.width  = pdst->size.x - 1;
		xrectangle.height = pdst->size.y - 1;
	}
	INHIBIT_FLUSH();
	gc = x_gc_window;

	XSetClipRectangles(x_display, gc, 0, 0, &xrectangle, 1 /* count */, Unsorted);
	if (style == line_style_solid)
	{/* set up for a solid line */
		XSetLineAttributes(x_display, gc, width, LineSolid, CapButt, JoinMiter);
	}
	else
	{/* set up for a dashed line */
		XSetLineAttributes(x_display, gc, width, LineDoubleDash, CapButt, JoinMiter);
		XSetDashes(x_display, gc, style->offset, style->pattern, style->entries);
	}
	XSetForeground(x_display, x_gc_window, COLOR_PIXEL(c == NULL_COLOR ? default_foreground_color : c));
	XSetFillStyle(x_display, gc, FillSolid);
	XSetFunction(x_display, gc, GXcopy);
	XDrawLine(x_display, pdst->drawable, gc, p1.x, p1.y, p2.x, p2.y);

        if (dst == screen_pixmap)
                XFlush(x_display);              /* display BLT to screen quickly        */

	OK_TO_FLUSH();
}


/* Plot a fancy colored polyline in a pixmap.						*/
void
polyColorLine(Pixmap dst, Point origin, short n, Point *pp, short width, 
              LineStyle *style, Rectangle clipper, Boolean clip, Color c)
{
PPixmap			*pdst = (PPixmap *) dst;/* private destination pixmap type	*/
XRectangle		xrectangle;		/* screen clipping rectangle (x)	*/
XGC			gc;			/* the correct graphics context		*/
XPoint			*points;		/* the new x point list			*/
short			i;			/* point index				*/
long			maxLines;		/* maximum number of lines X can draw	*/

	if (clip)
	{/* clip based on what's given us */
		xrectangle = rn2xRect(clipper);
	}
	else
	{/* clip based on the outer border of the drawable */
		xrectangle.x      = 0;
		xrectangle.y      = 0;
		xrectangle.width  = pdst->size.x - 1;
		xrectangle.height = pdst->size.y - 1;
	}
	points = (XPoint *) get_mem(sizeof(XPoint) * n, "polyColorLine()");
	for (i = 0; i < n; i++)
	{/* coerce an Rn point to an X point */
		points[i].x = pp[i].x + origin.x;
		points[i].y = pp[i].y + origin.y;
	}
	maxLines = (XMaxRequestSize(x_display) - 3) / 2;
	INHIBIT_FLUSH();
	gc = x_gc_window;
	XSetClipRectangles(x_display, gc, 0, 0, &xrectangle, 1 /* count */, Unsorted);
	if (style == line_style_solid)
	{/* set up for a solid line */
		XSetLineAttributes(x_display, gc, width, LineSolid, CapButt, JoinMiter);
	}
	else
	{/* set up for a dashed line */
		XSetLineAttributes(x_display, gc, width, LineDoubleDash, CapButt, JoinMiter);
		XSetDashes(x_display, gc, style->offset, style->pattern, style->entries);
	}
	XSetForeground(x_display, gc, COLOR_PIXEL(c == NULL_COLOR ? default_foreground_color : c));
	XSetFillStyle(x_display, gc, FillSolid);
	XSetFunction(x_display, gc, GXcopy);
	for (i = 0; i < n; i += maxLines)
		XDrawLines(x_display, pdst->drawable, gc, points + i,
			   MIN(((int) maxLines), n-i), CoordModeOrigin);

	if (dst == screen_pixmap)
		XFlush(x_display);		/* display to screen quickly		*/

	OK_TO_FLUSH();
	free_mem((void*) points);
}


/* Cover the region specified by a colored closed polyline.				*/
void
polygonColor(Pixmap dst, Point origin, short n, Point *pp, Bitmap pattern, 
             Point offset, Rectangle clipper, Boolean clip, Color fg, Color bg)
{
PPixmap			*pdst = (PPixmap *) dst;/* private destination pixmap type	*/
PBitmap			*ppattern = (PBitmap *) pattern;
						/* private bitmap type			*/
XRectangle		xrectangle;		/* screen clipping rectangle (x)	*/
XGC			gc;			/* the correct graphics context		*/
XPoint			*points;		/* the new x point list			*/
short			i;			/* point index				*/

	if (clip)
	{/* clip based on what's given us */
		xrectangle = rn2xRect(clipper);
	}
	else
	{/* clip based on the outer border of the drawable */
		xrectangle.x      = 0;
		xrectangle.y      = 0;
		xrectangle.width  = pdst->size.x - 1;
		xrectangle.height = pdst->size.y - 1;
	}
	points = (XPoint *) get_mem(sizeof(XPoint) * n, "polygonColor()");
	for (i = 0; i < n; i++)
	{/* coerce an Rn point to an X point */
		points[i].x = pp[i].x + origin.x;
		points[i].y = pp[i].y + origin.y;
	}
	INHIBIT_FLUSH();
	gc = x_gc_window;
	XSetClipRectangles(x_display, gc, 0, 0, &xrectangle, 1 /* count */, Unsorted);
	XSetFunction(x_display, gc, GXcopy);
	if (pattern == NULL_BITMAP)
	{
		XSetFillStyle(x_display, gc, FillSolid);
		XSetForeground(x_display, gc, COLOR_PIXEL(fg == NULL_COLOR ? default_foreground_color : fg));
		XFillPolygon(x_display, pdst->drawable, gc, points, n, Complex, CoordModeOrigin);
	}
	else if (fg == NULL_COLOR && bg == NULL_COLOR)
	{
		XSetFillStyle(x_display, gc, FillOpaqueStippled);
		XSetForeground(x_display, gc, COLOR_PIXEL(default_foreground_color));
		XSetBackground(x_display, gc, COLOR_PIXEL(default_background_color));
		XSetStipple(x_display, gc, ppattern->drawable);
		XSetTSOrigin(x_display, gc, offset.x, offset.y);
		XFillPolygon(x_display, pdst->drawable, gc, points, n, Complex, CoordModeOrigin);
	}
	else if (fg == NULL_COLOR)
	{	/* Invert pattern. */
		XSetForeground(x_display, gc, COLOR_PIXEL(bg));
		XSetFillStyle(x_display, gc, FillStippled);
		XSetTSOrigin(x_display, gc, offset.x, offset.y);
		BLTBitmap(NULL_BITMAP, pattern,
			makeRectFromSize(Origin, getBitmapSize(pattern)), Origin,
			BITMAP_INV, false);
		XSetStipple(x_display, gc, ppattern->drawable);
		XFillPolygon(x_display, pdst->drawable, gc, points, n, Complex, CoordModeOrigin);
		BLTBitmap(NULL_BITMAP, pattern,
			makeRectFromSize(Origin, getBitmapSize(pattern)), Origin,
			BITMAP_INV, false);
	}
	else
	{
		XSetForeground(x_display, gc, COLOR_PIXEL(fg));
		XSetStipple(x_display, gc, ppattern->drawable);
		XSetTSOrigin(x_display, gc, offset.x, offset.y);
		if (bg == NULL_COLOR)
		{
			XSetFillStyle(x_display, gc, FillStippled);
		}
		else
		{
			XSetFillStyle(x_display, gc, FillOpaqueStippled);
			XSetBackground(x_display, gc, COLOR_PIXEL(bg));
		}
		XFillPolygon(x_display, pdst->drawable, gc, points, n, Complex, CoordModeOrigin);
	}		

	if (dst == screen_pixmap)
		XFlush(x_display);

	OK_TO_FLUSH();
	free_mem((void*) points);
}


#define	XANGLE(a)	((int)(a * 64 + ((a > 0) ? 0.5 : -0.5)))

/* Draw an arc of an ellipse in the specified pixmap.					*/
void
arcLine(Pixmap dst, Rectangle *r, double a1, double a2, short width, 
        LineStyle *style, Rectangle clipper, Boolean clip, Color c)
{
PPixmap			*pdst = (PPixmap *) dst;/* private destination pixmap type	*/
XRectangle		xrectangle;		/* screen clipping rectangle (x)	*/
XGC			gc;			/* the correct graphics context		*/

	if (clip)
	{/* clip based on what's given us */
		xrectangle = rn2xRect(clipper);
	}
	else
	{/* clip based on the outer border of the drawable */
		xrectangle.x      = 0;
		xrectangle.y      = 0;
		xrectangle.width  = pdst->size.x - 1;
		xrectangle.height = pdst->size.y - 1;
	}

	INHIBIT_FLUSH();
	gc = x_gc_window;
	XSetClipRectangles(x_display, gc, 0, 0, &xrectangle, 1 /* count */, Unsorted);
	if (style == line_style_solid)
	{/* set up for a solid line */
		XSetLineAttributes(x_display, gc, width, LineSolid, CapButt, JoinMiter);
	}
	else
	{/* set up for a dashed line */
		XSetLineAttributes(x_display, gc, width, LineDoubleDash, CapButt, JoinMiter);
		XSetDashes(x_display, gc, style->offset, style->pattern, style->entries);
	}

	XSetFunction(x_display, gc, GXcopy);
	XSetForeground(x_display, gc, COLOR_PIXEL(c == NULL_COLOR ? default_foreground_color : c));
	XDrawArc(x_display, pdst->drawable, gc, r->ul.x, r->ul.y, r->lr.x - r->ul.x + 1, r->lr.y - r->ul.y + 1, XANGLE(a1), XANGLE(a2));

	if (dst == screen_pixmap)
		XFlush(x_display);

	OK_TO_FLUSH();
}


/* Pattern an arc's pie piece in the specified pixmap.					*/
void
arcColor(Pixmap dst, Rectangle *r, double a1, double a2, Bitmap pattern, 
         Point offset, Rectangle clipper, Boolean clip, Color fg, Color bg)
{
PPixmap			*pdst = (PPixmap *) dst;/* private destination pixmap type	*/
PBitmap			*ppattern = (PBitmap *) pattern;
						/* private bitmap type			*/
XRectangle		xrectangle;		/* screen clipping rectangle (x)	*/
XGC			gc;			/* the correct graphics context		*/

	if (clip)
	{/* clip based on what's given us */
		xrectangle = rn2xRect(clipper);
	}
	else
	{/* clip based on the outer border of the drawable */
		xrectangle.x      = 0;
		xrectangle.y      = 0;
		xrectangle.width  = pdst->size.x - 1;
		xrectangle.height = pdst->size.y - 1;
	}
	INHIBIT_FLUSH();
	gc = x_gc_window;
	XSetClipRectangles(x_display, gc, 0, 0, &xrectangle, 1 /* count */, Unsorted);

	if (pattern == NULL_BITMAP)
	{
		XSetFillStyle(x_display, gc, FillSolid);
		XSetForeground(x_display, gc, COLOR_PIXEL(fg == NULL_COLOR ? default_foreground_color : fg));
		XFillArc(x_display, pdst->drawable, gc, r->ul.x, r->ul.y, r->lr.x - r->ul.x + 1, r->lr.y - r->ul.y + 1, XANGLE(a1), XANGLE(a2));
	}
	else if (fg == NULL_COLOR && bg == NULL_COLOR)
	{
		XSetFillStyle(x_display, gc, FillOpaqueStippled);
		XSetForeground(x_display, gc, COLOR_PIXEL(default_foreground_color));
		XSetBackground(x_display, gc, COLOR_PIXEL(default_background_color));
		XSetStipple(x_display, gc, ppattern->drawable);
		XSetTSOrigin(x_display, gc, offset.x, offset.y);
		XFillArc(x_display, pdst->drawable, gc, r->ul.x, r->ul.y, 
			 r->lr.x - r->ul.x + 1, r->lr.y - r->ul.y + 1, XANGLE(a1), XANGLE(a2));
	}
	else if (fg == NULL_COLOR)
	{	/* Invert pattern. */
		XSetForeground(x_display, gc, COLOR_PIXEL(bg));
		XSetFillStyle(x_display, gc, FillStippled);
		XSetTSOrigin(x_display, gc, offset.x, offset.y);
		BLTBitmap(NULL_BITMAP, pattern,
			makeRectFromSize(Origin, getBitmapSize(pattern)), Origin,
			BITMAP_INV, false);
		XSetStipple(x_display, gc, ppattern->drawable);
		XFillArc(x_display, pdst->drawable, gc, r->ul.x, r->ul.y, r->lr.x - r->ul.x + 1, r->lr.y - r->ul.y + 1, XANGLE(a1), XANGLE(a2));
		BLTBitmap(NULL_BITMAP, pattern,
			makeRectFromSize(Origin, getBitmapSize(pattern)), Origin,
			BITMAP_INV, false);
	}
	else
	{
		XSetForeground(x_display, gc, COLOR_PIXEL(fg));
		XSetStipple(x_display, gc, ppattern->drawable);
		XSetTSOrigin(x_display, gc, offset.x, offset.y);
		if (bg == NULL_COLOR)
		{
			XSetFillStyle(x_display, gc, FillStippled);
		}
		else
		{
			XSetFillStyle(x_display, gc, FillOpaqueStippled);
			XSetBackground(x_display, gc, COLOR_PIXEL(bg));
		}
		XFillArc(x_display, pdst->drawable, gc, r->ul.x, r->ul.y, r->lr.x - r->ul.x + 1, r->lr.y - r->ul.y + 1, XANGLE(a1), XANGLE(a2));
	}		

	if (dst == screen_pixmap)
		XFlush(x_display);

	OK_TO_FLUSH();
	return;
}


	/* WINDOW EVENT ROUTINES */

/* Signal handler for a pause event--hide the main window.				*/
static
int
handleSIGTSTP()
{
	INHIBIT_FLUSH();
	XFlush(x_display);
	OK_TO_FLUSH();
	(void) kill(getpid(), SIGSTOP);
	return 0;
}


/* Filter the event queue to check for a window creation event.				*/
/*ARGSUSED*/
static
Bool
wait_for_window(XDisplay *display, XEvent *ie, char *args)
{
	return ie->type == ConfigureNotify || ie->type == MapNotify;
}


#ifdef DEBUG
int errorHandler(XDisplay *display, XErrorEvent *err)
{
  char msg[1024];
  XGetErrorText(display, err->error_code, msg, 1024);
  (void) fprintf(stderr, "Error code %s\n", msg);

  while(true) ;					/** loop indefinitely			*/
}
#endif /** DEBUG */


/* Create the root window and fire up the root screen module, run it, and fire down.	*/
void
startScreenEvents(ResizeFunc resize, RedrawFunc redraw, CharHandlerFunc handler)
{
struct	sgttyb		terminal_status;	/* terminal keyboard status settings	*/
unsigned int		c_width, c_height;	/* best cursor width, height		*/
int			dx, dy;			/* the display width, height		*/
XEvent	 		ie;			/* an input event			*/
static	XPixmap		dummy_pixmap;		/* dummy flat pixmap for null blt	*/
static	char 		*def_argv[] = {		/* the "default" argv for this exec.	*/
				"envX",
				(char *) 0
			};
static	XSizeHints	sizehints =  {		/* the size the Rn window wants		*/
				PSize | PMinSize,
				0,   0,			/* x, y */
				800, 500,		/* Size */
				400, 250,		/* MinSize */
				0,   0,			/* MaxSize */
				1,   1,			/* ResizeInc */
				{ 0, 1},		/* MinAspect */
				{ 0, 1}			/* MaxAspect */
			};
XGCValues		gc_values;		/* the gc values template		*/
#ifdef DDEBUG  /** 09 Mar 10.9 */
int			clss = 5;		/* visual structure type		*/
XVisualInfo		vi;			/* visual structure controlling depth	*/
XSetWindowAttributes	wwAttr;			/* window attributes for root window	*/
#endif /** DDEBUG 09 Mar 10.9 */

	/* set callbacks */
		root_resizer  = resize;
		root_redrawer = redraw;
		char_handler  = handler;

	/* check for swapping DEL & BS */
		gtty(0, &terminal_status);
		kb_swap_bs_del = BOOL(terminal_status.sg_erase == '\177');

	/* open and initialize the display */
		if ((x_display = XOpenDisplay((char *) NULL)) == (XDisplay *) 0)
		{/* could not open the display */
			(void) fprintf(stderr, "Could not open the '%s' display.\n", XDisplayName((char *) 0));
			exit(-1);
		}
		add_select_mask(ConnectionNumber(x_display));
		(void) signal(SIGTSTP, (SignalHandler) handleSIGTSTP);
		x_screen = DefaultScreen(x_display);

#ifdef DEBUG
	(void) XSynchronize(x_display, True);
	(void) XSetErrorHandler(errorHandler);
#endif

	/* Decide the keyboard type based on the (gulp) display size */
		dx = DisplayWidth (x_display, x_screen);
		dy = DisplayHeight(x_display, x_screen);
#ifdef distribution
		if (	 dx == 1280 && dy == 1024       /* SG, PS/2    */
		)
#else
		if (	 dx == 1024 && dy ==  800 ||	/* AED         */
			 dx == 1024 && dy ==  768 ||	/* apa16, 8514 */
			 dx ==  640 && dy ==  350 ||	/* ega         */
			 dx == 1024 && dy == 1024 ||	/* mpel        */
			 dx ==  720 && dy ==  512 ||	/* apa8c       */
			 dx ==  640 && dy ==  480 ||	/* vga         */
			 dx ==  720 && dy ==  540 ||	/* vga720      */
                         dx == 1280 && dy == 1024       /* AIX SG PS/2 */
		)
#endif /* distribution */
                {/* Some IBM-style keyboarded display */
                        kb_keyboard_id = KB_IBM;
/*			if (strcmp(x_display->vendor, "Silicon Graphics") == 0) */
			if (strcmp(ServerVendor(x_display), "Silicon Graphics") == 0)
			{/* Silicon Graphics (IBM Style) keyboard */
				the_keys = ibmSG_keys;
				num_keys = sizeof(ibmSG_keys) / sizeof(struct key);
			}
#ifndef distribution
			else if (!(dx == 1280 && dy == 1024) && ImageByteOrder(x_display) == MSBFirst)
                        {/* RT keyboard */
                                the_keys = ibmRT_keys;
                                num_keys = sizeof(ibmRT_keys) / sizeof(struct key);
                        }
#endif /* distribution */
                        else
                        {/* standard IBM keyboard */
                                the_keys = ibm_keys;
                                num_keys = sizeof(ibm_keys) / sizeof(struct key);
                        }
                }
		else if (dx == 1152 && dy ==  768 ||	/* CG3B           */
			 dx == 1152 && dy ==  900 ||	/* BW2, CG3A, CG4 */
			 dx == 1600 && dy == 1280 ||	/* BW2 hi res     */
			 dx <   500 && dy <   500	/* suntools ?     */
		)
                {/* the keyboard is a Sun (but we can't tell what kind--assume sparc) */
                        kb_keyboard_id = KB_SPARC;
                        the_keys = sparc_keys;
                        num_keys = sizeof(sparc_keys) / sizeof(struct key);
                }
                else
                {/* not sure! */
                        kb_keyboard_id = KB_DEFAULT;
                        the_keys = (struct key *) 0;
                        num_keys = 0;
                        (void) fprintf(stderr, "Warning:  Could not determine keyboard type--using default keyboard.\n");
                }

	/* Prepare the globally available colors and monochrome_screen Boolean. */
		colorHashTable = NameValueTableAlloc(INITIAL_NUMBER_COLORS,
					(NameCompareCallback)colorHashTableCompareFunc,
					(NameHashFunctCallback)colorHashTableHashFunc);
		XrmInitialize();		/* prepare resource manager		*/
		colorDB = getColorDB(D_color_defaults_file);
		{
			char res_name[1024];	/* resource name			*/
			char *repr_type;	/* representation type of value returned*/
			XrmValue val;		/* value in database			*/

			(void) sprintf(res_name, "%s.monochrome", D_resource_identifier);
			if (XrmGetResource((XrmDatabase) colorDB, res_name,
					    (char *) NULL, /* ignore class name */
					    &repr_type, &val))
			  monochrome_screen = (!strcmp((char *) val.addr, "true") |
					       !strcmp((char *) val.addr, "True") |
					       !strcmp((char *) val.addr, "TRUE")) ? true : false;
			if (DefaultDepth(x_display, x_screen) == 1)
				monochrome_screen = true;  /* display black & white	*/
		}

	/* set up the foreground and background colors */
		default_foreground_color = getColorFromName("pane.foreground");
		default_background_color = getColorFromName("pane.background");
		default_border_color = getColorFromName("pane.borderColor");

		if (default_foreground_color == default_background_color)  {
			default_foreground_color = getColorFromName("black");
			default_background_color = getColorFromName("white");
		}

	/* set up the main window and its graphics context */
#ifdef DDEBUG  /** 09 Mar. 10.9 */
		while (!XMatchVisualInfo(x_display, x_screen, DefaultDepth(x_display, x_screen), clss--, &vi))
			;

		wwAttr.background_pixel = COLOR_PIXEL(default_background_color);
		wwAttr.border_pixel = COLOR_PIXEL(default_border_color);
		x_window = XCreateWindow(x_display, RootWindow(x_display, x_screen), sizehints.x, sizehints.y, sizehints.width, sizehints.height, 2, DefaultDepth(x_display, x_screen), InputOutput, vi.visual, CWBackPixel|CWBorderPixel, &wwAttr);  /** was InputOutput, CopyFromParent */
#else
		x_window = XCreateSimpleWindow(x_display, RootWindow(x_display, x_screen), sizehints.x, sizehints.y, sizehints.width, sizehints.height, 2, COLOR_PIXEL(default_border_color), COLOR_PIXEL(default_background_color));
#endif  /** DDEBUG  09 Mar. 10.9 */
		XSetStandardProperties(x_display, x_window, "D System", "D System", None, def_argv, 1 /* argc */, &sizehints);
		gc_values.graphics_exposures = False;
		gc_values.foreground = COLOR_PIXEL(default_foreground_color);
		gc_values.background = COLOR_PIXEL(default_background_color);
		x_gc_window = XCreateGC(x_display, x_window, GCForeground | GCBackground |GCGraphicsExposures, &gc_values);
		p_screen_pixmap.size     = UnusedPoint;
		p_screen_pixmap.drawable = x_window;

	/* set up the graphics context for Bitmaps */
		gc_values.function = GXcopy;
		gc_values.foreground = 1;
		gc_values.background = 0;
		gc_values.fill_style = FillOpaqueStippled;
		gc_values.graphics_exposures = False;
		gc_values.plane_mask = FLAT_PLANE_BITMAP;
		dummy_pixmap = XCreatePixmap(x_display, x_window, 1, 1, BITMAP_DEPTH);
		x_gc_bitmap = XCreateGC(x_display, dummy_pixmap, GCFunction | GCForeground | GCBackground | GCFillStyle | GCGraphicsExposures | GCPlaneMask, &gc_values);
		XFreePixmap(x_display, dummy_pixmap);

	/* set up for cursors; default cursor */
		c_width = c_height = 16;
		(void) XQueryBestSize(x_display, CursorShape, x_window, c_width, c_height, &c_width, &c_height);
		max_cursor_size = makePoint((int) c_width, (int) c_height);

	/* set up graphics */
		start_gfx();
		(void) CURSOR(standard_cursor);

	/* set up for events & paint the window */
		mon_event.offset = Origin;
		mon_event.loc    = Origin;
		XSelectInput(x_display, x_window, KeyPressMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | ButtonMotionMask | ExposureMask | StructureNotifyMask);
		XMapRaised(x_display, x_window);
		XPeekIfEvent(x_display, &ie, wait_for_window, (char *) 0);
		(void) flushScreenEvents();
}


/* Stop the flow of events from the window.						*/
void
stopScreenEvents()
{
	/* empty the color hash table */
		NameValueTableForAll(colorHashTable, destroyColor, (Generic) 0);
		NameValueTableFree(colorHashTable);

	/* free the cursor information */
		XUndefineCursor(x_display, x_window);

	/* fire down graphics */
		finish_gfx();

	/* free & close the display */
		(void) signal(SIGTSTP, SIG_IGN);
		remove_select_mask(ConnectionNumber(x_display));
		XFreeGC(x_display, x_gc_window);
		XDestroyWindow(x_display, x_window);
		XFlush(x_display);
		XCloseDisplay(x_display);
}


/* Return true if an X event is ready for processing.					*/
static
Boolean
readyXEvent()
{
XEvent	 		ie;			/* an input event			*/
Point			new_size;		/* the new window size			*/
Point			this_size;		/* the current window size		*/
Boolean			resized = false;	/* true if we have resized		*/
RectList		exposures;		/* the list of exposures		*/
XWindow			root;
int			x, y;
unsigned int		width, height, border_width, depth;

	initializeRectList(&exposures);
	new_size = p_screen_pixmap.size;
	XSync(x_display, False);
	while (XCheckMaskEvent(x_display, ExposureMask | StructureNotifyMask, &ie))
	{/* we have extracted a redraw/resize event--handle it */
		switch (ie.type)
		{/* handle this event type */
			case Expose:
				pushRectList(
					makeRectFromSize(
						makePoint(ie.xexpose.x,     ie.xexpose.y),
						makePoint(ie.xexpose.width, ie.xexpose.height)
					),
					&exposures
				);
				break;
			case ConfigureNotify:
				this_size = makePoint(ie.xconfigure.width, ie.xconfigure.height);
				if (NOT(equalPoint(new_size, this_size)))
				{/* the size has really changed */
					resized = true;
					new_size = this_size;
					freeRectList(&exposures);
				}
				break;
			case MapNotify:
				XGetGeometry(x_display, x_window, &root, &x, &y, &width, &height, &border_width, &depth);
				new_size = makePoint((int) width, (int) height);
				freeRectList(&exposures);
				break;
			case MappingNotify:
				XRefreshKeyboardMapping((XMappingEvent*)&ie);
				break;
		}
	}
	if (resized)
	{/* set the new size */
		p_screen_pixmap.size = new_size;
		root_resizer(new_size);
	}
	root_redrawer(exposures);
	return BOOL(XPending(x_display));
}


/* Return true if there is an event waiting.						*/
Boolean
readyScreenEvent()
{
	return BOOL(need_exit_event || need_release_event || readyXEvent());
}


/* Handle a keyboard event.								*/
static
void
handleKeyboardEvent(XKeyEvent *ke)
{
#define CHARBUFSIZE	128
char			charbuf[CHARBUFSIZE];	/* buffer for mapping a key event	*/	
char			*bptr;			/* pointer to input character stream	*/
int			len;			/* the length of input character stream	*/
unsigned char		val = ke->keycode;	/* the x value of a key event		*/
register int		high = num_keys;	/* binary search too high		*/
register int		low = -1;		/* binary search too low		*/
register int		mid;			/* binary search probe value		*/
register int		i;			/* index into charbuf			*/

	while (low + 1 < high)
	{/* divide the list */
		mid = (low + high) >> 1;
		if (the_keys[mid].code < val)
		{/* throw away the bottom of the list */
			low = mid;
		}
		else if (the_keys[mid].code > val)
		{/* throw away the top of the list */
			high = mid;
		}
		else
		{/* we have found the value--dump it into the mapper */
			for (bptr = inputFromKbChar(the_keys[mid].kb); *bptr; bptr++)
			{/* stuff the characters in to be mapped */
				char_handler(*bptr);
			}
			return;
		}
	}
	/* do the default lookup */
		len = XLookupString(ke, charbuf, CHARBUFSIZE, NULL, NULL);
		for (i = 0; i < len; i++)
		{/* stuff the characters in to be mapped */
			char_handler(charbuf[i]);
		}
}


/* Get an important mouse/keyboard event into mon_event.				*/
void
getScreenEvent()
{
XEvent			ie;			/* the X input event			*/
Boolean			important = false;	/* true if this is an important event	*/
#define			SHIFTEVENT(ie)		(!!(((XMotionEvent *) ie)->state & ShiftMask))

	if (need_exit_event)
	{/* fake an exit event */
		mon_event.type = MOUSE_EXIT;
		mon_event.msg  = 0;
		button_pushed  = 0;
		need_exit_event = false;
		need_release_event = false;
		important = true;
	}
	else if (need_release_event)
	{/* fake a release event */
		mon_event.type = MOUSE_UP;
		mon_event.info = makePoint(button_pushed, 0);
		mon_event.msg  = 0;
		button_pushed  = 0;
		need_release_event = false;
		important = true;
	}
	while (NOT(important) && readyXEvent())
	{/* convert an event from their style to our style */
		mon_event.info = Origin;
		mon_event.msg  = 0;
		XNextEvent(x_display, &ie);
		switch(ie.type)
		{/* handle the event based on the type */
			case EnterNotify:
				mon_event.type = (button_pushed) ? MOUSE_DRAG : MOUSE_MOVE;
				mon_event.info = makePoint(button_pushed, SHIFTEVENT(&ie));
				mon_event.loc = makePoint(ie.xcrossing.x, ie.xcrossing.y);
				break;
			case LeaveNotify:
				mon_event.type = MOUSE_EXIT;
				button_pushed  = 0;
				mon_event.loc = makePoint(ie.xcrossing.x, ie.xcrossing.y);
				important = true;
				break;
			case MotionNotify:
				mon_event.type = (button_pushed) ? MOUSE_DRAG : MOUSE_MOVE;
				mon_event.info = makePoint(button_pushed, SHIFTEVENT(&ie));
				mon_event.loc = makePoint(ie.xmotion.x, ie.xmotion.y);
				break;
			case ButtonPress:
				if (button_pushed == 0)
				{/* this is a button down event */
					button_pushed  = ie.xbutton.button;
					mon_event.type = MOUSE_DOWN;
					mon_event.info = makePoint(button_pushed, SHIFTEVENT(&ie));
					important = true;
				}
				else
				{/* not an interesting button */
					mon_event.type = (button_pushed) ? MOUSE_DRAG : MOUSE_MOVE;
					mon_event.info = makePoint(button_pushed, SHIFTEVENT(&ie));
				}
				mon_event.loc = makePoint(ie.xbutton.x, ie.xbutton.y);
				break;
			case ButtonRelease:
				if (ie.xbutton.button == button_pushed)
				{/* this is our button being released */
					mon_event.type = MOUSE_UP;
					mon_event.info = makePoint(button_pushed, SHIFTEVENT(&ie));
					button_pushed  = 0;
					important = true;
				}
				else
				{/* not an interesting button */
					mon_event.type = (button_pushed) ? MOUSE_DRAG : MOUSE_MOVE;
					mon_event.info = makePoint(button_pushed, SHIFTEVENT(&ie));
				}
				mon_event.loc = makePoint(ie.xbutton.x, ie.xbutton.y);
				break;
			case KeyPress:
				handleKeyboardEvent(&ie.xkey);
				mon_event.type = (button_pushed) ? MOUSE_DRAG : MOUSE_MOVE;
				mon_event.info = makePoint(button_pushed, SHIFTEVENT(&ie));
				mon_event.loc = makePoint(ie.xkey.x, ie.xkey.y);
				break;
		}
	}

	mon_event.loc = subPoint(mon_event.loc, mon_event.offset);

	if (mon_event.type == MOUSE_EXIT)
	{/* we have left the window */
		outside_window = true;
	}
	else if (mon_event.type != MOUSE_KEYBOARD)
	{/* we are inside the window */
		outside_window = false;
	}
}


/* Flush any queued screen events.							*/
Boolean
flushScreenEvents()
{
XEvent	 		ie;			/* an input event			*/

	if (inhibit_flush)
	{/* could not do flushing */
		return false;
	}
	else
	{/* flush the input queue */
		while (readyXEvent())
		{/* we have pending events--eat one */
			XNextEvent(x_display, &ie);
			switch (ie.type)
			{/* handle (flush) the event */
				case LeaveNotify:
					if (NOT(outside_window))
					{/* the mouse has left the window */
						need_exit_event = true;
					}
					if (button_pushed)
					{/* set up to release the button */
						need_release_event = true;
					}
					break;
				case ButtonPress:
				case ButtonRelease:
					if (button_pushed)
					{/* set up to release the button */
						need_release_event = true;
					}
					break;
				case KeyPress:
					handleKeyboardEvent(&ie.xkey);
					break;
			}
		}
		return true;
	}
}




static
XRectangle
rn2xRect(Rectangle r)
{
  XRectangle xr;

  if( positiveArea(r) )
    { xr.x      = r.ul.x;
      xr.y      = r.ul.y;
      xr.width  = r.lr.x - r.ul.x + 1;
      xr.height = r.lr.y - r.ul.y + 1;
    }
  else
    { xr.x      = 0;
      xr.y      = 0;
      xr.width  = 0;
      xr.height = 0;
    }

  return xr;
}
