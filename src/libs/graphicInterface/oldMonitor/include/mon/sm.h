/* $Id: sm.h,v 1.12 1997/06/25 14:46:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 			  sm.h				*/
		/*	       Screen module use include file		*/
		/* 		    (root/scr_mod.c)			*/
		/********************************************************/


#ifndef sm_h
#define sm_h

	/* Structure Definitions Needed For This .h File	*/

#include <libs/graphicInterface/support/graphics/point.h>
#include <libs/graphicInterface/support/graphics/rect.h>
#include <libs/graphicInterface/oldMonitor/include/mon/gfx.h>
#include <libs/graphicInterface/support/graphics/rect_list.h>
#include <libs/graphicInterface/oldMonitor/include/mon/font.h>

	/* WINDOW/PANE/SCREEN_MODULE DEFINITIONS */

#define DIRS		2			/* the number of entry directions	*/
#define	FRST_NEXT	0			/* the first/next entry on the list	*/
#define	LAST_PREV	1			/* the last/previous entry on the list	*/
#define OTHER_DIR(x)	(!x)			/* the other direction			*/

struct pane;
struct window;

struct	pane	{				/* GENERAL PANE STRUCTURE		*/
	struct	window	*parent;		/* the enclosing window			*/
	struct	pane	*sibling[DIRS];		/* the next & prev panes on list	*/
	struct	window	*child[DIRS];		/* the front & back of the window list	*/
	struct	pane	*slave_child[DIRS];	/* the front & back of slave pane list	*/
	Point		position;		/* coord of ul corner in encl_window	*/
	Point		size;			/* the size in pixels of the pane	*/
	short		border_width;		/* the width of the border		*/
	/* Assigned pane colors: may be NULL_COLOR, representing inheritance		*/
	Color		foreground;		/* the color used for objects		*/
	Color		background;		/* the color used behind objects	*/
	Color		border_color;		/* the color of the pane's border	*/
	/* Effective pane colors: may be assigned color or inherited from ancestor	*/
	/* Invariant: These fields always contain color values, not NULL_COLOR.		*/
	Color		e_fg;			/* the color used for objects		*/
	Color		e_bg;			/* the color used behind objects	*/
	Color		e_bc;			/* the color of the pane's border	*/
	MouseCursor	cursor;			/* cursor to display in this pane	*/
	Bitmap		pattern;		/* the pane's background design		*/
	short		scr_mod_num;		/* the screen module for this pane	*/
	struct	pane	*owner;			/* the owner pane of this screen module	*/
	Generic		pane_information;	/* pane status information		*/
		};
typedef	struct	pane	Pane;
#define	NULL_PANE	((Pane *) 0)		/* empty pane */

/* If one changes the window structure, be sure to update mon/root/root.c		*/
struct	window	{				/* AN ARBITRARY COLLECTION OF PANES	*/
	struct	pane	*parent;		/* the enclosing pane			*/
	struct	window	*sibling[DIRS];		/* the next & prev windows on list	*/
	struct	pane	*child[DIRS];		/* the front & back of pane list	*/
	Rectangle	border;			/* position in enclosing pane		*/
	short		border_width;		/* the width of the enclosing border	*/
	/* Assigned window colors: may be NULL_COLOR, representing inheritance		*/
	Color		foreground;		/* the color used for objects		*/
	Color		background;		/* the color used behind objects	*/
	Color		border_color;		/* the color of the window's border	*/
	/* Effective window colors: may be assigned color or inherited from ancestor	*/
	/* Invariant: These fields always contain color values, not NULL_COLOR.		*/
	Color		e_fg;			/* the color used for objects		*/
	Color		e_bg;			/* the color used behind objects	*/
	Color		e_bc;			/* the color of the window's border	*/
	Pixmap		image;			/* the window's pixmap			*/
	Boolean		exists;			/* true if image has been constructed	*/
	Boolean		showing;		/* true if the window is showing	*/
	Generic		window_information;	/* window status information		*/
		};
typedef	struct	window	Window;
#define	NULL_WINDOW	((Window *) 0)		/* empty window				*/

typedef FUNCTION_POINTER(void, sm_start_func, (void));
/* Takes no parameters.  Called exactly once prior to all other screen module calls.	*/

typedef FUNCTION_POINTER(void, sm_finish_func, (void));
/* takes no parameters.  Called exactly once after all other screen module calls.	*/

typedef FUNCTION_POINTER(void, sm_create_func, (Pane *p));
/* Takes one parameter, (Pane *p) the pane being created.  Set local information and	*/
/* default values in the pane structure (border width, cursor, cover).			*/

typedef FUNCTION_POINTER(void, sm_resize_func, (Pane *p));
/* Takes one parameter, (Pane *p) the pane being resized/repositioned.  The size and	*/
/* position have been installed into the pane structure.  The background will be drawn	*/
/* before and the border will be drawn after this call.  The pane's image should be	*/
/* drawn into the parent's image but not propagated.					*/

typedef FUNCTION_POINTER(RectList, sm_propagate_change_func, 
			 (Pane *p, Window *w, RectList rl));
/* Takes three parameters (Pane *p) the pane of the change, (Window *w) the window which*/
/* has changed--NULL_WINDOW if the area is to be completely repainted, (RectList rl) the*/
/* pane relative list of rectangles completely within p which have been changed.  The	*/
/* image mentioned should be drawn to the parent image of p and touched.  Note:  in the	*/
/* case that the parent of the modified window is itself a slave pane, the master pane	*/
/* will get the propagate call.  This means that w's parent may not be p.  The areas of	*/
/* p which could not be drawn (or accounted for) are returned.				*/

typedef FUNCTION_POINTER(void, sm_destroy_func, (Pane *p));
/* Takes one parameter, (Pane *p) the pane being destroyed.  Destroy local information.	*/
/* Remaining child windows will be freed afterwards.					*/

typedef FUNCTION_POINTER(void, sm_input_func, (Pane *p, Rectangle r));
/* Takes two parameters, (Pane *p) the pane for which the input is ready, and (Rectangle*/
/* r) a visible rectangle that the first event is in.					*/

typedef FUNCTION_POINTER(void, sm_window_tile_func, (Window *w, Generic info, Boolean isnew));
/* Takes three parameters, (Window *w) the window (installed), (Generic info) the	*/
/* information passed from the caller, and (Boolean isnew) true if this is a new window.*/
/* The window border width and border must be set.					*/

typedef FUNCTION_POINTER(void, sm_window_destroy_func, (Window *w));
/* Takes one parameter (Window *w) the window to be destroyed.  The local information	*/
/* should be freed.  Remaining panes in the window and the window structure itself will	*/
/* be destroyed after the call.								*/

typedef	struct	{				/* SCREEN MODULE FUNCTIONS		*/
	char			*name;		/* the name of the screen module	*/
	sm_start_func		start;		/* start the screen module		*/
	sm_finish_func		finish;		/* finish the screen module		*/
	sm_create_func		create;		/* create an instance of the pane	*/
	sm_resize_func		resize;		/* resize/reposition and redraw a pane	*/
	sm_propagate_change_func
				propagate_change;/* propagate changes in a subwindow	*/
	sm_destroy_func		destroy;	/* destroy the pane of this type	*/
	sm_input_func		input;		/* handle an input event		*/
	sm_window_tile_func	window_tile;	/* handle a window tiling		*/
	sm_window_destroy_func	window_destroy;	/* handle the destruction of a window	*/
		} aScreenModule;

EXTERN(short, getScreenModuleIndex, (aScreenModule *sm));
/* Takes one parameter (aScreenModule *sm) a pointer to a screen module structure	*/
/* which is looked up or installed in the screen module array and the index is returned.*/
#define	MAX_SMs		128			/* the maximum number of screen modules	*/

EXTERN(void, standardStart, (void));
/* This is to be attached to the screen module structure in the start slot for those	*/
/* screen modules with no special startup work.						*/

EXTERN(void, standardFinish, (void));
/* This is to be attached to the screen module structure in the finish slot for those	*/
/* screen modules with no special finish work.						*/

EXTERN(RectList, standardNoSubWindowPropagate, (Pane *p, Window *w,
 RectList rl));
/* This is to be attached into a screen module structure in the propagate pane slot to	*/
/* handle the case of no subwindows.							*/

EXTERN(RectList, standardPropagate, (Pane *p, Window *w, RectList rl));
/* This is to be attached into a screen module structure in the propagate pane slot to	*/
/* handle the case of standard windows.							*/

EXTERN(void, standardDestroyPane, (Pane *p));
/* This is to be attached into a screen module structure in the destroy pane slot to	*/
/* handle the case of no local storage in the pane.					*/

EXTERN(void, standardTileNoWindow, (Window *w, Generic info, Boolean newWin));
/* This is to be attached into a screen module structure in the tile window slot to	*/
/* handle the case of no children windows for the pane.					*/

EXTERN(void, standardDestroyWindow, (Window *w));
/* This is to be attached into a screen module structure in the destroy window slot to	*/
/* handle the case of no local window information.					*/


	/* PANE HANDLING ROUTINES  */

EXTERN(Pane *, newPane, (Window *w, short scr_mod_num, Point position, Point size, short border));
/* Takes five parameters, (Window *w) is the parent window, (short scr_mod_num) the	*/
/* number of the screen module that will be assigned to the new pane, (Point position,	*/
/* size) the position and size of the pane, and (short border) the border width of the	*/
/* pane.  The pane will be put into the beginning of the list of panes for the enclosing*/
/* window.  The return value is a pointer to the pane.					*/

EXTERN(Pane *, newColorPane, (Window *w, short scr_mod_num, Point position, Point size, short border, Color foreground, Color background, Color borderColor));
/* Takes eight parameters, (Window *w) is the parent window, (short scr_mod_num) the	*/
/* number of the screen module that will be assigned to the new pane, (Point position,	*/
/* size) the position and size of the pane, and (short border) the border width of the	*/
/* pane.  The foreground, background, and border colors determine the default colors of */
/* objects in the pane, the color behind these objects, and the color of the pane's	*/
/* border, respectively.  The pane will be put into the beginning of the list of panes  */
/* for the enclosing window.  The return value is a pointer to the pane.		*/

EXTERN(Pane *, newSlavePane, (Pane *owner, short scr_mod_num, Point position,
 Point size, short border));
/* Takes five parameters, (Pane *owner) is the owner pane, (short scr_mod_num) the	*/
/* number of the screen module that will be assigned to the new pane, (Point position,	*/
/* size) the position and size of the pane, (short border) the pane's border.  The pane	*/
/* will not be put into the list of panes for the enclosing window.  The return value is*/
/* a pointer to the pane.  The pane should be later destroyed with destroyPane.  It will*/
/* NOT be done automatically.								*/

EXTERN(Pane *, newColorSlavePane, (Pane *owner, short scr_mod_num, Point position,
 Point size, short border, Color foreground, Color background, Color borderColor));
/* Takes eight parameters, (Pane *owner) is the owner pane, (short scr_mod_num) the	*/
/* number of the screen module that will be assigned to the new pane, (Point position,	*/
/* size) the position and size of the pane, (short border) the pane's border.  The	*/
/* foreground, background, and border colors determine the default colors of		*/
/* objects in the pane, the color behind these objects, and the color of the pane's	*/
/* border, respectively.  The pane will not be put into the list of panes for the	*/
/* enclosing window.  The return value is a pointer to the pane.  The pane should be	*/
/* later destroyed with destroyPane.  It will NOT be done automatically.		*/

EXTERN(void, resizePane, (Pane *p, Point position, Point size));
/* Takes three parameters (Pane *p) the pane to be redrawn, (Point position) the new	*/
/* pane position, and (Point size) the new pane size.  The pane is redrawn on its parent*/
/* window.  Returns nothing.								*/

EXTERN(void, recolorPane, (Pane *p, Color foreground, Color background, Color borderColor, Boolean invert, Boolean useParentColors, Boolean display));
/* Takes seven parameters: (Pane *p) the pane whose color should be changed, the new	*/
/* foreground, background, and border colors, and three Booleans.  These colors deter-	*/
/* mine the default colors of objects in the pane, the color behind these objects, and	*/
/* the colors of the pane's border, respectively.  If Boolean useParentColors is true, 	*/
/* the colors become the same as the parent's colors.  If Boolean invert is true, the	*/
/* foreground color changes to the current background color, while the background and	*/
/* border colors change to the current foreground color.  If both Booleans are true,	*/
/* the colors become the inverse of the parent's colors.  If any color actual is	*/
/* (Color *) 0, all the color actuals are ignored.  If (Boolean display) is true, the	*/
/* pane is redisplayed after the color changes have occurred.				*/

EXTERN(void, drawWindowsInPane, (Pane *p));
/* Takes one parameter (Pane *p) the pane to be drawn.  Draws the subwindows inside the	*/
/* pane's border.									*/

EXTERN(RectList, propagateSlavePane, (Pane *p, Window *w, RectList rl));
/* Takes three parameters (Pane *p) the slave pane, (Window *w) the window of the change*/
/* and (RectList rl) the owner pane relative rectangle list that has been damaged.  The	*/
/* list is adjusted for the slave and the slave's image is reconstructed from the	*/
/* information.  Note:  this call is meant to be made from the master of the slave pane	*/
/* with the same w and rl parameters that the master was called with.  Both will be	*/
/* adjusted accordingly in the propagate call to the slave.				*/

EXTERN(void, destroyPane, (Pane *p));
/* Takes one parameter (Pane *p) the pane to be destroyed.  Returns nothing.		*/

EXTERN(void, handlePane, (Pane *p));
/* Takes one parameter (Pane *p) the new pane to begin interpretation.			*/

EXTERN(Color, paneForeground, (Pane *p));
/* Takes one parameter:  (Pane *p) the pane whose current foreground color should be	*/
/* returned.  NULL_COLOR should never be returned.					*/

EXTERN(Color, paneBackground, (Pane *p));
/* Takes one parameter:  (Pane *p) the pane whose current background color should be	*/
/* returned.  NULL_COLOR should never be returned.					*/

EXTERN(Color, paneBorderColor, (Pane *p));
/* Takes one parameter:  (Pane *p) the pane whose current border color should be	*/
/* returned.  NULL_COLOR should never be returned.					*/


	/* WINDOW HANDLING ROUTINES */

EXTERN(Window *, createWindow, (Pane *p, Generic info));
/* Takes three parameters (Pane *p) the parent pane, (Point position) the position of	*/
/* upper left corner of the window, and (Generic info) the information to be passed to	*/
/* the window tiling routine.  Calls the screen module's window tiling routine to set up*/
/* the panes within the window and set up its size. New window is put at the head of the*/
/* list and its pointer is returned.  The window is not shown.				*/

EXTERN(Window *, createColorWindow, (Pane *p, Generic info, Color foreground, Color background, Color borderColor));
/* Takes five parameters (Pane *p) the parent pane and (Generic info) the information to*/
/* be passed to the window tiling routine.  The foreground, background, and border	*/
/* colors determine the default colors of objects in the window, the color behind the	*/
/* objects, and the color of the window's pane, respectively.  Calls the screen module's*/
/* window tiling routine to set up the panes within the window and set up its size. New */
/* window is put at the head of the list and its pointer is returned.  The window is	*/
/* shown.										*/

EXTERN(void, showWindow, (Window *w));
/* Takes one parameter (Window *w) the window to be shown.  The change is propagated	*/
/* appropriately.									*/

EXTERN(void, hideWindow, (Window *w));
/* Takes one parameter (Window *w) the window to be hidden.  The change is propagated	*/
/* appropriately.									*/

EXTERN(void, modifyWindow, (Window *w, Generic info));
/* Takes two parameters (Window *w) the window to be retiled and (Generic info) the	*/
/* information parameter passed to the window tiling routine.				*/

EXTERN(void, recolorWindow, (Window *w, Color foreground, Color background, Color borderColor, Boolean invert, Boolean useParentColors));
/* Takes six parameters, (Window *w) the window whose color should be changed, the new	*/
/* foreground, background, and border colors, and two Booleans.  These colors determine */
/* the default colors of objects in the window, the color behind these objects, and the	*/
/* colors of the window's border, respectively.  If Boolean useParentColors is true, the*/
/* colors become the same as the parent's colors.  If Boolean invert is true, the	*/
/* foreground color changes to the current background color, while the background and	*/
/* border colors change to the current foreground color.  If both Booleans are true,	*/
/* the colors become the inverse of the parent's colors.				*/

EXTERN(void, destroyWindow, (Window *w));
/* Takes one parameter (Window *w) the window to be destroyed.  Redraws appropriately.	*/

EXTERN(void, toEndWindow, (Window *w, short dir));
/* Takes two parameters (Window *w) the window to be adjusted and (short dir) the 	*/
/* list end.  The window is moved to the indicated end of the enclosing pane's window	*/
/* list and all is redrawn appropriately.						*/

EXTERN(Color, windowForeground, (Window *w));
/* Takes one parameter:  (Window *w) the window whose current foreground color should 	*/
/* be returned.  NULL_COLOR should never be returned.					*/

EXTERN(Color, windowBackground, (Window *w));
/* Takes one parameter:  (Window *w) the window whose current background color should 	*/
/* be returned.  NULL_COLOR should never be returned.					*/

EXTERN(Color, windowBorderColor, (Window *w));
/* Takes one parameter:  (Window *w) the window whose current border color should 	*/
/* be returned.  NULL_COLOR should never be returned.					*/


	/* GRAPHICS/GEOMETRY ROUTINES */

EXTERN(void, BLTColorCopyPane, (Pane *src, Pane *dst, Rectangle srcArea, Point dstOrigin, Bitmap m, Point offset, Boolean clip));
/* copy src area filtered by bitmap	*/
/* Takes seven parameters: (Pane *src) the source image, (Pane *dst) the image to	*/
/* receive the copy, (Rectangle srcArea) the area to copy from the source and the size	*/
/* of the affected area in dst, (Point dstOrigin) the upper left corner of the		*/
/* affected area in dst, (Bitmap m) bitmap controlling the copy, (Point offset) the upper	*/
/* left corner where m should start to be tiled, and (Boolean clip) true if the		*/
/* affected area is not guaranteed to be contained within dst.  Conceptually the bitmap	*/
/* is repeatedly tiled over dst.  Only those pixels covered by a '1' bit in the tiling	*/
/* and falling within dst's affected area receive new pixel values from src.  When m is	*/
/* NULL_BITMAP, the tile consists of all '1's so dst's entire affected area receives the	*/
/* copy.										*/

EXTERN(void, ColorPaneWithPattern, (Pane *dst, Rectangle dstArea, Bitmap pattern,
  Point offset, Boolean clip));
/* change area using bit pattern	*/
/* Takes five parameters the same as for ColorPaneWithPatternColor.  Uses the destina-	*/
/* tion pane's default colors.								*/

EXTERN(void, ColorPaneWithPatternColor, (Pane *dst, Rectangle dstArea,
  Bitmap pattern, Point offset, Boolean clip, Color foreground, Color background));
/* change area using bit pattern	*/
/* Takes seven parameters: (Pane *dst) the image to receive the copy, (Rectangle	*/
/* dstArea) the area within dst receiving the pattern, (Bitmap pattern) one bit deep	*/
/* pattern controlling the image placed into dstArea, (Point offset) the upper left	*/
/* corner where pattern should start to be tiled, (Boolean clip) true if the affected	*/
/* area is not guaranteed to be within dst, (Color foreground), if not NULL_COLOR, the	*/
/* color to place in dst's affected area for a '1' in the pattern, (Color background),	*/
/* if not NULL_COLOR, the color to place in dst's affected area for a '0' in the	*/
/* pattern.  Conceptually the pattern is repeatedly tiled over dst, with its upper left	*/
/* corner at offset.  For example, to create a background use a pattern and two colors,	*/
/* one for the foreground and one for the background.  To color a region with a solid	*/
/* color, use NULL_BITMAP as the pattern and supply a foreground color.  If both color	*/
/* arguments are NULL_COLOR, the pane's default foreground and background colors are	*/
/* used.										*/

EXTERN(void, ghostBoxInPane, (Pane *p, Rectangle r, short width, Boolean clip, Boolean redisplay));
/* Takes five parameters: (Rectangle r) the rectangle to be boxed (pane relative), 	*/
/* (Pane *p) the pane it is relative to, (short width) the width of the box's border,	*/
/* (Boolean clip) true if the								*/
/* box is not guaranteed to be within p, and (Boolean redisplay) true if the pane	*/
/* should be copied to the screen after producing the box.  The caller should ensure	*/
/* calls to ghostBoxInPane are paired: one to draw the box and one to erase the box.	*/
/* The caller should also call touchPane(p) for the changes to appear.			*/

EXTERN(Bitmap, flattenPane, (Pane *p));
/* obtain a Bitmap from a part of a Pane	*/
/* Takes one parameter as for flattenPaneColor().  Uses the pane's foreground and	*/
/* background colors.									*/

EXTERN(Bitmap, flattenPaneColor, (Pane *p, Color foreground, Color background));
/* obtain a Bitmap from a part of a Pane	*/
/* Takes three parameters: (Pane *p) the source of the bit plane, (Color foreground)	*/
/* all pixels with this color will have a corresponding '1' in the bitmap, (Color		*/
/* background) all pixels with this color will a corresponding '0' in the bitmap.  No	*/
/* guarantees are made regarding bits corresponding to other colors.  To use the pane's	*/
/* default foreground and background colors, use foreground and background actual	*/
/* arguments of NULL_COLOR.								*/

EXTERN(void, flashPane, (Pane *p));
/* Takes one parameter (Pane *p) the pane to be flashed.				*/

EXTERN(Rectangle, pointPane, (Pane *p, Point p1, Rectangle clipper));
/* Takes three parameters as described in pointPaneColor().  Uses the pane's foreground	*/
/* color.										*/

EXTERN(Rectangle, pointPaneColor, (Pane *p, Point p1, Rectangle clipper, Color c));
/* Takes four parameters:  (Pane *p) the pane of interest, (Point p1) the point to	*/
/* modify, (Rectangle clipper) the clipping rectangle, and (Color c) the point's color.	*/
/* The point will be clipped to the pane's						*/
/* boundaries.  The affected rectangle will be returned.  Changes are not propagated	*/
/* through all enclosing windows.  Use c = NULL_COLOR to use the pane's default		*/
/* foreground color.									*/

EXTERN(Rectangle, polyPointPane, (Pane *p, Point origin, short n, Point *pp, Rectangle clipper));
/* Takes five parameters as described in polyPointPaneColor().  Uses the pane's		*/
/* foreground color.									*/

EXTERN(Rectangle, polyPointPaneColor, (Pane *p, Point origin, short n, Point *pp, Rectangle clipper, Color c));
/* Takes six parameters:  (Pane *p) the pane of interest, (Point origin) the amount	*/
/* added to each of the points in *pp, (short n) the number of points in *pp,		*/
/* (Point *pp) the array of points to modify, (Rectangle clipper) the clipping		*/
/* rectangle, and (Color c) the color of the points.  The point array will be clipped	*/
/* to the pane's boundaries.  The affected rectangle will be returned.  Changes are not	*/
/* propagated through all enclosing windows.						*/

EXTERN(Rectangle, linePane, (Pane *p, Point p1, Point p2, short width,
 LineStyle *style));
/* draw a colored line in a pane	*/
/* Takes five parameters as described in linePaneColor().  Uses the pane's foreground	*/
/* color.										*/

EXTERN(Rectangle, linePaneClipped, (Pane *p, Point p1, Point p2, short width,
 LineStyle *style, Rectangle clipper));
/* draw a colored line in a pane	*/
/* Takes six parameters as described in linePaneColorClipped().  Uses the pane's foreground	*/
/* color.										*/

EXTERN(Rectangle, linePaneColor, (Pane *p, Point p1, Point p2, short width,
 LineStyle *style, Color c));
/* draw a colored line in a pane	*/
/* Takes six parameters: (Pane *p) the pane of interest, (Point p1, p2) the line's two	*/
/* endpoints (pane relative coordinates), (short width) the width of the line,		*/
/* (LineStyle *style) the line style, and (Color c) the line's color.  See the LINE()	*/
/* call for more information.  A clipped line is drawn and the affected rectangle is	*/
/* returned.  Changes are NOT propagated through all enclosing windows.  Use c =	*/
/* NULL_COLOR to use the pane's default foreground color.				*/

EXTERN(Rectangle, linePaneColorClipped, (Pane *p, Point p1, Point p2, short width,
 LineStyle *style, Color c, Rectangle clipper));
/* draw a colored line in a pane	*/
/* Same as linePaneColor except that output is clipped to 'clipper'.			*/

EXTERN(void, boxPane, (Pane *dst, Rectangle r, short width, Bitmap pattern, Point offset, Boolean clip));
/* draw a box in a Pane	*/
/* Takes six parameters as described in boxPaneColor.  Uses the pane's foreground and	*/
/* background colors.									*/

EXTERN(void, boxPaneColor, (Pane *dst, Rectangle r, short width, Bitmap pattern, Point offset, Boolean clip, Color foreground, Color background));
/* draw a box in a Pane	*/
/* Takes eight parameters: (Pane *dst) the Pane in which to draw the box, (Rectangle r)	*/
/* the box's outer border, (short width) the width of the border, (Bitmap pattern) the	*/
/* pattern to use, (Point offset) the pattern offset from the ul corner of dst,		*/
/* (Boolean clip) true if the box is not guaranteed to be within the pixmap boundary of	*/
/* the destination, and (Color foreground) and (Color background) the colors for the	*/
/* 1's and 0's in the pattern.  To color the box with the foreground color, use		*/
/* pattern = NULL_BITMAP.  To use the pane's default colors, set both foreground and	*/
/* background to NULL_COLOR.  To color only the 1's xor the 0's, set one color actual	*/
/* argument to NULL_COLOR.  A rectangular box, possibly empty inside with border width	*/
/* "width," is produced.								*/

EXTERN(Rectangle, polyLinePane, (Pane *p, Point origin, short n, Point *pp, short width, LineStyle *style));
/* draw a colored poly-line in a pane	*/
/* Takes six parameters as described in polyLinePaneColor().  Uses the pane's		*/
/* foreground color.									*/

EXTERN(Rectangle, polyLinePaneColor, (Pane *p, Point origin, short n, Point *pp, short width, LineStyle *style, Color c));
/* draw a colored poly-line in a pane	*/
/* Takes seven parameters: (Pane *p) the pane to contain the poly-line, (Point origin)	*/
/* the amount added to each point in pp, (short n) the number of points in pp, (Point	*/
/* *pp) the array of endpoints of the line segments, (short width) the width of the	*/
/* poly-line, (LineStyle *style) the line style to use, and (Color c) the poly-line's	*/
/* color.  A poly-line is a sequence of line segments connected at common endpoints.	*/
/* The poly-line will not be closed unless the first point is the same as the last	*/
/* point.  Use c = NULL_COLOR to use the pane's foreground color.  All points are	*/
/* to origin, not the previous point in the poly-line.					*/

EXTERN(Rectangle, polygonPane, (Pane *p, Point origin, short n, Point *pp,
 Bitmap pattern, Point offset, Rectangle clipper));
/* Takes seven parameters as described in polygonPaneColor().  Uses the pane's		*/
/* foreground and background color.							*/

EXTERN(Rectangle, polygonPaneColor, (Pane *p, Point origin, short n, Point *pp,
 Bitmap pattern, Point offset, Rectangle clipper, Color foreground, Color background));
/* Takes nine parameters: (Pane *p) the pane of interest, (Point origin) the amount to	*/
/* add to each point in *pp, (short n) the number of points in *pp, (Point *pp) the	*/
/* array of end points of the line segments of the polygon, (Bitmap pattern) the pattern*/
/* to repeat in the polygon, (Point offset) the upper left corner to begin the pattern	*/
/* relative to p's upper left corner, (Rectangle clipper) the clipping rectangle, and	*/
/* (Color foreground, background) the colors for					*/
/* 1's and 0's in the pattern.  If both foreground and background are NULL_COLOR, the	*/
/* pane's default foreground and background colors are used.  If only one color is	*/
/* NULL_COLOR, the corresponding pixels are not drawn.  The list of points is "closed"	*/
/* automatically, i.e. the first point in the list need not be the same as the last.	*/
/* The area is covered using an even-odd rule (a point is draw if an infinite ray with	*/
/* with the point as origin crosses the path an odd number of times).  The polygon will	*/
/* be clipped to the pane's boundaries.  The affected rectangle will be returned.	*/
/* Changes are not propagated through all enclosing windows.				*/

EXTERN(Rectangle, arcLinePane, (Pane *p, Rectangle *r, double a1, double a2,
				short width, LineStyle *style));
/* Takes five parameters as described in arcLinePaneColor().  Uses the pane's fore-	*/
/* ground color.									*/

EXTERN(Rectangle, arcLinePaneClipped, (Pane *p, Rectangle *r, double a1, double a2,
				       short width, LineStyle *style, Rectangle clipper));
/* Takes six parameters as described in arcLinePaneColorClipped().  Uses the pane's     */
/* foreground color.									*/

EXTERN(Rectangle, arcLinePaneColor, (Pane *p, Rectangle *r, double a1, double a2,
				     short width, LineStyle *style, Color c));
/* Takes seven parameters:  (Pane *p) the pane of interest, (Rectangle *r) the bounding	*/
/* box of the ellipse from which the arc is derived, (double a1) the starting angle of	*/
/* the arc measured in degrees from 3 o'clock counterclockwise, (double a2) the extent of*/
/* the arc measured in degrees counterclockwise from a1, (short width) the line width	*/
/* for the arc, (LineStyle *style) the line style to use for the arc, and (Color c) the	*/
/* arc line's color.  Note that the angles are "true" and ignore the aspect ratio of	*/
/* bounding box.  The arc will be clipped to the pane's boundaries.  The affected  	*/
/* rectangle will be returned.  Changes are not propagated through all enclosing	*/
/* windows.  To color the arc with the pane's foreground color, use c = NULL_COLOR.	*/
/* NOTE:  This call does nothing under Suntools.					*/

EXTERN(Rectangle, arcLinePaneColorClipped, (Pane *p, Rectangle *r, double a1, double a2,
					    short width, LineStyle *style, Color c,
					    Rectangle clipper));
/* Same as 'arcLinePaneColor' except that output is clipped to 'clipper'.		*/

EXTERN(Rectangle, arcCoverPane, (Pane *p, Rectangle *r, double a1, double a2,
 Bitmap pattern, Point offset));
/* Takes six parameters as in arcColorPaneColor.  Uses the pane's foreground and back-	*/
/* ground colors.									*/

EXTERN(Rectangle, arcCoverPaneColor, (Pane *p, Rectangle *r, double a1, double a2,
				      Bitmap pattern, Point offset, Color foreground,
				      Color background));
/* Takes eight parameters:  (Pane *p) the pane of interest, (Rectangle *r) the bounding */
/* box of the ellipse from which the arc is derived, (double a1) the starting angle of	*/
/* the arc measured in degrees from 3 o'clock counterclockwise, (double a2) the extent of*/
/* the arc measured in degrees counterclockwise from a1, (Bitmap pattern) the pattern to*/
/* cover the area, (Point offset) the upper left corner of the pattern relative to the	*/
/* upper left corner of p offset of the pane's Origin, and (Color foreground,		*/
/* background) the color for 1's and 0's in the pattern.  Note that the angles are	*/
/* "true" and ignore the aspect ratio of the bounding box.  The pie slice will be	*/
/* clipped to the pane's boundaries.  The affected rectangle will be returned.		*/
/* Changes are not propagated through all enclosing windows.  To color the arc with the	*/
/* foreground color, use pattern == NULL_BITMAP.  To color just the 1's xor the 0's, use*/
/* NULL_COLOR as background xor foreground.  To use the pane's default foreground and	*/
/* background colors, use NULL_COLOR for both color arguments.  NOTE:  This call does	*/
/* nothing under Suntools.								*/

EXTERN(void, touchPane, (Pane *p));
/* Takes one parameter:  (Pane *p) the pane that has been modified.  Propagates changes	*/
/* through all enclosing windows.							*/

EXTERN(void, touchPaneRect, (Pane *p, Rectangle r));
/* Takes two parameters:  (Pane *p) the pane where the change has been made and		*/
/* (Rectangle r) the region in p of the change.  The procedure updates all enclosing	*/
/* windows.										*/

EXTERN(void, touchPaneRectList, (Pane *p, RectList rl));
/* Takes two parameters:  (Pane *p) then pane where the changes have been made and	*/
/* (RectList rl) the regions in p of the the changes.  The procedure updates all	*/
/* enclosing windows.									*/

EXTERN(void, fontWritePane, (Pane *dst, Point p, Rectangle clip, TextString ts,
 short font));
/* write a text string in a pane	*/
/* Takes five parameters as in fontWritePaneColor().  Uses the pane's foreground and	*/
/* background colors.									*/

EXTERN(void, fontWritePaneColor, (Pane *dst, Point p, Rectangle clip,
 TextString ts, short font, Color foreground, Color background));
/* write a text string in a pane	*/
/* Takes seven parameters:  (Pane *dst) the destination pane, (Point p) the starting	*/
/* point of the write, (Rectangle clip) the clipping rectangle, (TextString ts) the	*/
/* text string to be printed, (short font) the font id to be used, (Color foreground)	*/
/* the text color, and (Color background) the color behind the text.  The TextString	*/
/* 'tc_ptr' is freed if 'ephemeral' is true.  If both the foreground and background	*/
/* actuals are NULL_COLOR, then the pane's default colors are used.  If exactly one is	*/
/* NULL_COLOR, either the letter or its background is not produced.			*/
/* Note:  the clipping rectangle must be within the destination pane.			*/

EXTERN(void, moveWindow, (Window *w));
/* Takes one parameter:  (Window *w) the window to be moved.  It is redrawn in the new	*/
/* position.										*/

EXTERN(void, moveWindowTo, (Window *w, Point pt));
/* Takes two parameters:  (Window *w) the window to move and (Point pt) the new origin	*/
/* for the window.  The move is made and the window is redrawn.				*/

EXTERN(void, invertPane, (Pane *p, Rectangle dstArea, Boolean clip));
/* Takes three parameters:  (Pane *p) the pane whose colors should be inverted,		*/
/* (Rectangle dstArea) the area within the pane to change, and (Boolean clip) true if	*/
/* dstArea is not guaranteed to be within dst.  All calls to invertPane() should come	*/
/* in pairs to ensure the colors are returned to their original state.  The inverted	*/
/* colors are color pixel inverses, not color inverses, so they may not appear as	*/
/* expected.										*/


#endif
