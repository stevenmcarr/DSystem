/* $Id: gfx.h,v 1.13 1997/06/25 14:46:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 			gfx.h				*/
		/*	   Low level graphics routines (gfx.c)		*/
		/*	  Machine dependent graphics (mach_*.c)		*/
		/* 							*/
		/********************************************************/


#ifndef gfx_h
#define gfx_h


#include <libs/graphicInterface/support/graphics/rect.h>

	/****** COLOR DEFINITIONS ******/

typedef Generic		Color;			/* Color instance type			*/

#define NULL_COLOR	((Color) 0)
	/* Use when drawing with the default colors, usually specified by the pane.	*/

#define UNUSED_COLOR    ((Color) -1)
        /* Not equal to any valid Color value.                                          */

extern Color		default_foreground_color; /* foreground Color of root window	*/
extern Color		default_background_color; /* background Color of root window	*/
extern Color		default_border_color;	/* border color of root window		*/

extern Boolean		monochrome_screen;	/* true if should use only black & white*/


EXTERN(Color, getColorFromName, (char *name));
/* returns Color instance		*/
/* Takes one parameter:  (char *name) which may be:					*/
/*	1. resource name with the form <object name>.<color specification>, e.g.,	*/
/*	   "scrollBar.arrowColor"; the color returned reflects the user's color		*/
/*	   preference for the object.							*/
/*	2. color name, e.g., "white," "black"  (Any color listed in /usr/lib/X11/rgb.txt*/
/*	   may be used.)                                                                */
/*	3. red, green, blue specification of the form #RRRRGGGGBBBB, where R, G, and B	*/
/*	   are hexadecimal numbers.  See also getColorFromRGB.				*/
/* The Color returned corresponds to the parameter.					*/

EXTERN(Color, getColorFromRGB, (int red, int green, int blue));
/* returns Color instance		*/
/* Takes three parameters:  (int red, green, blue) specify the pixel values for the	*/
/* desired color.  Only the least sixteen bits of each input is used.			*/

EXTERN(void, getRGBFromName, (char *name, int *red, int *green, int *blue));
/* places integers into the last three arguments	*/
/* Takes four parameters:  (char *name) which may be:   */
/*	1. resource name with the form <object name>.<color specification>, e.g.,	*/
/*	   "scrollBar.arrowColor"; the color returned reflects the user's color		*/
/*	   preference for the object.							*/
/*	2. color name, e.g., "white," "black"  (Any color listed in /usr/lib/X11/rgb.txt*/
/*	   may be used.)                                                                */
/*	3. red, green, blue specification of the form #RRRRGGGGBBBB, where R, G, and B	*/
/*	   are hexadecimal numbers.  See also getColorFromRGB.				*/
/* (int *red, *green, *blue), whose initial values are ignored, receive the red, green,	*/
/* and blue numbers, such that getColorFromRGB(red, green, blue) =			*/
/* getColorFromName(name).  The routine places 0 into red, green, and blue if the	*/
/* requested color does not exist or the hexadecimal string given as "name" is		*/
/* incorrectly formattted.								*/


	/****** PIXMAP DEFINITIONS ******/

/* Pixmaps, storing a rectangular image of colored pixels, are associated with each	*/
/* window.  They may be combined using BLTColorCopy() and maybe modified using		*/
/* ColorWithPattern().  The screen is represented by screen_pixmap.  The NULL_PIXMAP	*/
/* may be used in calls which ignore the pixmap.					*/

/* Only the monitor should directly change Pixmaps so please use the Pane calls in	*/
/* sm.h. */

typedef	Generic		Pixmap;			/* The pixmap type (private)		*/
typedef Generic		Bitmap;			/* the Bitmap type (private)		*/

#define	NULL_PIXMAP	((Pixmap) 0)		/* the empty (black) pixmap		*/
extern	Pixmap		screen_pixmap;		/* the screen pixmap			*/


EXTERN(Pixmap, makePixmap, (Point size, char *who));
/* create a Pixmap			*/
/* Takes two parameters:  (Point size) the size of the area and (char *who) the 	*/
/* identifying string.  Creates and returns a Pixmap for regular use.  Must be freed	*/
/* using freePixmap().									*/

EXTERN(void, freePixmap, (Pixmap b));
/* free a Pixmap			*/
/* Takes one parameter:  (Pixmap b) the Pixmap to be freed.				*/

EXTERN(Point, getPixmapSize, (Pixmap b));
/* get the size of a Pixmap		*/
/* Takes one parameter:  (Pixmap b) the Pixmap of interest.  Returns the image size.	*/

EXTERN(void, BLTColorCopy, (Pixmap src, Pixmap dst, Rectangle srcArea, Point dstOrigin, Bitmap m, Point offset, Boolean clip));
/* copy src area filtered by bitmap	*/
/* Takes seven parameters: (Pixmap src) the source image, (Pixmap dst) the image to	*/
/* receive the copy, (Rectangle srcArea) the area to copy from the source and the size	*/
/* of the affected area in dst, (Point dstOrigin) the upper left corner of the		*/
/* affected area in dst, (Bitmap m) bitmap controlling the copy, (Point offset) the upper	*/
/* left corner relative to dst's upper left corner, where m should start to be tiled,	*/
/* and (Boolean clip) true if the							*/
/* affected area is not guaranteed to be contained within dst.  Conceptually the bitmap	*/
/* is repeatedly tiled over dst.  Only those pixels covered by a '1' bit in the tiling	*/
/* and falling within dst's affected area receive new pixel values from src.  When m is	*/
/* NULL_BITMAP, the tile consists of all '1's so dst's entire affected area receives the	*/
/* copy.										*/

EXTERN(void, ColorWithPattern, (Pixmap dst, Rectangle dstArea, Bitmap pattern,
  Point offset, Boolean clip, Color foreground, Color background));
/* change area using bit pattern	*/
/* Takes seven parameters: (Pixmap dst) the image to receive the copy, (Rectangle	*/
/* dstArea) the area within dst receiving the pattern, (Bitmap pattern) one bit deep	*/
/* pattern controlling the image placed into dstArea, (Point offset) the location of	*/
/* the upper left corner of the pattern relative to the destination's upper left corner	*/
/* where pattern should	start to be tiled, (Boolean clip) true if the affected		*/
/* area is not guaranteed to be within dst, (Color foreground), if not NULL_COLOR, the	*/
/* color to place in dst's affected area for a '1' in the pattern, (Color background),	*/
/* if not NULL_COLOR, the color to place in dst's affected area for a '0' in the	*/
/* pattern.  Conceptually the pattern is repeatedly tiled over dst, with its upper left	*/
/* corner at offset.  For example, to create a background use a pattern and two colors,	*/
/* one for the foreground and one for the background.  To color a region with a solid	*/
/* color, use NULL_BITMAP as the pattern and supply a foreground color.			*/

EXTERN(Bitmap, flattenPixmap, (Pixmap b, Rectangle r, Color foreground, Color background));
/* obtain a bit plane from a Pixmap	*/
/* Takes four parameters: (Pixmap b) the source of the bit plane, (Rectangle r) the	*/
/* area within b to use, (Color foreground) all pixels with this color will have a	*/
/* corresponding '1' in the bitmap, (Color background) all pixels with this color will a	*/
/* corresponding '0' in the bitmap.  No guarantees are made regarding bits corresponding	*/
/* to other colors.  The rectangle is clipped within b.					*/

EXTERN(void, invertPixmap, (Pixmap dst, Rectangle dstArea, Boolean clip));
/* attempts to invert a pixmap's colors	*/
/* Takes three parameters:  (Pixmap dst) the destination Pixmap, (Rectangle dstArea)	*/
/* the area in the destination where the colors should be inverted, and (Boolean clip)	*/
/* true if dstArea is not guaranteed to be within dst.  All calls to invertPixmap()	*/
/* should come in pairs to ensure the colors are returned to the original state.	*/
/* The implementation does not guarantee the inverted colors will appear to be the	*/
/* color inverses.  They are the color pixel inverse.					*/

EXTERN(void, invertBox, (Pixmap dst, Rectangle r, short width, Boolean clip));
/* attempt to invert colors within a box*/
/* Takes four parameters:  (Pixmap dst) the destination Pixmap, (Rectangle r) the box's	*/
/* outer border, (short width) the width of the border, and (Boolean clip) true if the	*/
/* box is not guaranteed to be within dst.  All calls to invertBox() should come in	*/
/* pairs to ensure the colors are returned to a coherent state.  The inverted colors	*/
/* are color pixel inverses, not color inverses.					*/


	/****** BITMAP DEFINITIONS ******/

/* Bitmaps, one dimensional bit arrays, are used to when modifying Panes and Pixmaps.	*/
/* With its upper left corner at the specified point, the bitmap is conceptually		*/
/* repeatedly tiled over the destination.  Thus, an array of 1's and 0's cover the	*/
/* destination.  The use of the 1's and 0's varies according to their use.		*/

/* Bitmaps may be created using static data, and existing bitmaps may be combined using	*/
/* BLTBitmap() and a graphics operation.  The NULL_BITMAP represents a pattern containing	*/
/* all one bits, while using inverse_cover with BITMAP_INV will yield a bitmap with all	*/
/* bits inverted.									*/

#define	BITMAP_CLR	0			/* make dest white			*/
#define	BITMAP_NOR	1			/* nor of src and dest			*/
#define	BITMAP_INV_AND	2			/* not src and dest			*/
#define	BITMAP_INV_COPY	3			/* copy from src to dest inverting	*/
#define	BITMAP_AND_INV	4			/* and of src and not dest		*/
#define	BITMAP_INV	5			/* inverse of dest			*/
#define	BITMAP_XOR	6			/* exclusive or of src and dest		*/
#define BITMAP_NAND	7			/* nand of src and dest			*/
#define	BITMAP_AND	8			/* and of src and dest			*/
#define BITMAP_EQ		9			/* eq of scr and dest			*/
#define	BITMAP_NOOP	10			/* do nothing				*/
#define	BITMAP_INV_OR	11			/* not src or dest			*/
#define	BITMAP_COPY	12			/* copy the image from src to dest	*/
#define	BITMAP_OR_INV	13			/* or of src and not dest		*/
#define	BITMAP_OR		14			/* or of src and dest			*/
#define	BITMAP_SET	15			/* make dest black			*/

#define NULL_BITMAP	((Bitmap) 0)		/* Bitmap with all bits on		*/

	/* General Bitmap data representation */

#define	BITMAPM_UNIT				/* the unit of storage of memory bitmap*/		unsigned short
#define BITMAPM_BITS_PER_UNIT			/* the number of bits in a unit		*/	(8 * sizeof(BITMAPM_UNIT))
#define BITMAPM_UNITS_PER_LINE(width)		/* the number of units per line of data	*/	((width + BITMAPM_BITS_PER_UNIT - 1) / BITMAPM_BITS_PER_UNIT)
#define BITMAPM_UNITS_PER_IMAGE(width, height)	/* the number of units in an image	*/	(height * BITMAPM_UNITS_PER_LINE(width))
#define BITMAPM_BYTES_PER_IMAGE(width, height)	/* the number of bytes in an image	*/	(BITMAPM_UNITS_PER_IMAGE(width, height) * sizeof(BITMAPM_UNIT))


EXTERN(Bitmap, makeBitmap, (Point size, char *who));
/* create a new Bitmap with all bits set	*/
/* Takes two parameters:  (Point size) the size of the area and (char *who) the		*/
/* identifying string.  Returns a Bitmap which must later be freed with freeBitmap().	*/

EXTERN(Bitmap, makeBitmapFromData, (Point size, BITMAPM_UNIT *data, char *who));
/* create a new Bitmap with specified bits*/
/* Takes three parameters:  (Point size) the size of the image, (BITMAPM_UNIT *data)	*/
/* the data of the image, and (char *who) the identifying string.  Returns a Bitmap which	*/
/* must later be freed with freeBitmap().  The data is not used after creation.  The	*/
/* bitmap's contents is unpredictable is the incorrect amount of data is provided.	*/

EXTERN(void, freeBitmap, (Bitmap m));
/* free a previously created Bitmap	*/
/* Takes one parameter:  (Bitmap bitmap) the bitmap to free.  The bitmap may not be NULL_BITMAP.	*/

EXTERN(Point, getBitmapSize, (Bitmap m));
/* get the size of a Bitmap		*/
/* Takes one parameter:  (Bitmap m) the Bitmap of interest.  Returns the image size.  The	*/
/* bitmap may not be NULL_BITMAP.								*/

EXTERN(void, getBitmapData, (Bitmap m, BITMAPM_UNIT *data));
/* obtain data within the Bitmap		*/
/* Takes two parameters:  (Bitmap m) the Bitmap to observe and (BITMAPM_UNIT *data)	*/
/* the address in which to write the data.  The amount of data copied depends on the	*/
/* size of the Bitmap and the memory Bitmap definitions, above.  The bitmap may not be	*/
/* NULL_BITMAP.										*/

EXTERN(void, setBitmapData, (Bitmap m, BITMAPM_UNIT *data));
/* set the data for a Bitmap		*/
/* Takes two parameters:  (Bitmap m) the Bitmap to reset and (BITMAPM_UNIT *data)		*/
/* the address from which to copy the data.  The amount of data copied depends on the	*/
/* size of the Bitmap and the memory Bitmap definitions, above.  The bitmap may not be	*/
/* NULL_BITMAP.										*/

EXTERN(Boolean, fileBitmap, (Bitmap src, char *name));
/* save a Bitmap to a file		*/
/* Takes two parameters:  (Bitmap src) the source Bitmap and (char *name) the path name	*/
/* of the file to be written.  The success of the call is returned.  The call will fail	*/
/* if the file could not be opened/created or the area was not positive.  Note: the	*/
/* image file format is compatable with SUN rasterfile.  The bitmap may not be NULL_BITMAP.	*/

EXTERN(Bitmap, loadBitmap, (char *name));
/* load a created Bitmap from a file	*/
/* Takes one parameter:  (char *name) the name of the file to be loaded.  The file	*/
/* should be of the format that fileBitmap() produces.  A new Bitmap is created for the	*/
/* image and returned.  If there was an error, the returned value is 0.  A valid	*/
/* returned Bitmap must be later freed with freeBitmap().					*/

EXTERN(Bitmap, rotateBitmap, (Bitmap src, short amount));
/* create a new, rotated copy of a Bitmap	*/
/* Takes two parameters:  (Bitmap src) the Bitmap to be copied and (short amount) the 	*/
/* multiple of 90 to rotate the Bitmap.  The original Bitmap is not modified.  The new	*/
/* rotated copy of src is returned and must later be freed using freeBitmap().  src may	*/
/* not be NULL_BITMAP.									*/

EXTERN(void, BLTBitmap, (Bitmap src, Bitmap dst, Rectangle srcArea, Point dstOrigin, int op, Boolean clip));
/* modify destination Bitmap using source Bitmap and the graphics operation	*/
/* Takes six parameters: (Bitmap src) the source bitmap, (Bitmap dst) the bitmap to be		*/
/* modified, (Rectangle srcArea) the area to copy from the source, (Point dstOrigin)	*/
/* the location of the upper left corner of srcArea, relative to dst's upper left	*/
/* corner, where srcArea should be tiled, (int op) the graphics operation used to	*/
/* combine src and dst, and (Boolean clip) true if the affected area is not guaranteed	*/
/* to be contained within dst.  If op is BITMAP_CLR, BITMAP_NOOP, BITMAP_INV, or BITMAP_SET,	*/
/* the source bitmap may be NULL_BITMAP.  The destination bitmap may not be NULL_BITMAP.	*/


	/****** CURSOR DEFINITIONS ******/

/* Note:  Cursors are highly machine dependent.  The major variation is the availablity	*/
/* of two BLT cursors vs. one BLT cursors.  Two BLT cursors usually allow two images	*/
/* to be written with different graphics ops and one BLT cursors only allow one.  The	*/
/* machine will try to use the two BLT definition if it can but both should be defined.	*/

typedef	Generic		MouseCursor;		/* The MouseCursor type (private)	*/

extern	MouseCursor	standard_cursor;	/* usual cursor				*/
extern	MouseCursor	moving_cursor;		/* cursor for use in moving		*/
extern	MouseCursor	sizing_cursor;		/* cursor for use in sizing		*/
extern	MouseCursor	invisible_cursor;	/* invisible cursor			*/
extern	MouseCursor	wait_cursor;		/* "wait for computation" cursor	*/

EXTERN(MouseCursor, makeMouseCursorFromData, (Point size,
                                            BITMAPM_UNIT *single_data, short single_op, 
                                            BITMAPM_UNIT *primary_data, short primary_op, 
                                            BITMAPM_UNIT *secondary_data, short secondary_op, 
                                            Point hot_spot, char *who));
/* create a new MouseCursor		*/
/* Takes seven parameters:  (Point size) the size of all of the MouseCursor's images,	*/
/* (BITMAPM_UNIT *single_data) the data for the single BLT image, (int single_op)	*/
/* graphics function for the single BLT image, (BITMAPM_UNIT *shape_data) bits		*/
/* defining the shape of the cursor, (BITMAPM_UNIT *bitmap_data) the bits of the	*/
/* cursor which are actually displayed, (Point hot_spot) the position in the image	*/
/* where the pointer will reside, and (char *who) the identifying string.  Returns a	*/
/* MouseCursor which must be freed using freeMouseCursor().				*/

EXTERN(MouseCursor, makeColorMouseCursorFromData, (Point size,
                           BITMAPM_UNIT *single_data, short single_op, 
                           BITMAPM_UNIT *shape_data, 
                           BITMAPM_UNIT *bitmap_data, 
                           Point hot_spot, Color foreground, Color background, char *who));
/* create a new MouseCursor		*/
/* Takes nine parameters:  (Point size) the size of all of the MouseCursor's images,	*/
/* (BITMAPM_UNIT *single_data) the data for the single BLT image, (int single_op)	*/
/* graphics function for the single BLT image, (BITMAPM_UNIT *shape_data) bits		*/
/* defining the shape of the cursor, (BITMAPM_UNIT *bitmap_data) the bits of the	*/
/* cursor which are actually displayed, (Point hot_spot) the position in the image	*/
/* where the pointer will reside, (Color foreground) the cursor's color, and (Color	*/
/* background) the color behind the cursor, and (char *who) the identifying string.	*/
/* Returns a MouseCursor which must be freed using freeMouseCursor().			*/

EXTERN(void, recolorMouseCursor, (MouseCursor mc, Color foreground, Color background));
/* change cursor's colors */
/* Takes three parameters:  (MouseCursor mc) the cursor in question, (Color foreground)	*/
/* the cursor's color, (Color background) the color behind the cursor.  The cursor's	*/
/* colors are changed.									*/

EXTERN(void, freeMouseCursor, (MouseCursor mc));
/* free a previously created MouseCursor*/
/* Takes one parameter:  (MouseCursor cursor) the MouseCursor to free.			*/

EXTERN(MouseCursor, CURSOR, (MouseCursor cursor));
/* install a new MouseCursor		*/
/* Takes one parameter:  (MouseCursor cursor) a pointer to the new MouseCursor.  Returns*/
/* the previous MouseCursor.								*/


	/****** GRAPHICS FUNCTIONS ******/

EXTERN(void, pointColor, (Pixmap dst, Point p, Rectangle clipper, Boolean clip, Color c));
/* Please use pointPane in sm.h. */
/* draw a colored point */
/* Takes five parameters:  (Pixmap dst) the destination Pixmap, (Point p) the point to	*/
/* modify, (Rectangle clipper) the clipping rectangle, (Boolean clip) true if clipping	*/
/* should be performed against the clipping rectangle, and (Color c) the color of the	*/
/* point.										*/

EXTERN(void, polyColorPoint, (Pixmap dst, Point origin, short n, Point *pp, Rectangle clipper, Boolean clip, Color c));
/* Please use polyPointPane in sm.h.	*/
/* draw a set of colored points in a pixmap	*/
/* Takes seven parameters:  (Pixmap dst) the destination Pixmap, (Point origin) the	*/
/* amount added to each of the points in *pp, (short n) the number of points in *pp,	*/
/* (Point *pp) the array of points to modify, (Rectangle clipper) the clipping		*/
/* rectangle, (Boolean clip) true if clipping should be performed against the clipping	*/
/* rectangle, and (Color c) the color of the points.					*/

typedef
struct	line_style	{			/* LINE STYLE STRUCTURE			*/
	short		entries;		/* number of entries in pattern (even)	*/
	char		*pattern;		/* pointer to pattern list		*/
						/* first entry is "on" & alternates	*/
	short		offset;			/* offset into the pattern to start	*/
			} LineStyle;

extern	LineStyle	*line_style_solid;	/* a solid line style			*/
extern  LineStyle	*line_style_dotted;	/* a dotted line style (1 pixel reps)	*/
extern	LineStyle	*line_style_dashed;	/* a dashed line style (4 pixel reps)	*/

EXTERN(void, lineColor, (Pixmap dst, Point p1, Point p2, short width, 
 LineStyle *style, Rectangle clipper, Boolean clip, Color c));
/* Please use linePane in sm.h.	*/
/* draw a colored line in a Pixmap		*/
/* Takes eight parameters:  (Pixmap dst) the destination Pixmap, (Point p1, p2) the	*/
/* endpoints of the line, (short width) the width of the line, (LineStyle *style) the	*/
/* line style to use, (Rectangle clipper) the clipping rectangle, (Boolean clip) true	*/
/* if clipping should be performed, and (Color c) the color of the line.		*/

EXTERN(void, polyColorLine, (Pixmap dst, Point origin, short n, Point *pp, short width, LineStyle *style, Rectangle clipper, Boolean clip, Color c));
/* Please use polyLinePane in sm.h. */
/* draw a colored poly-line in a Pixmap	*/
/* Takes nine parameters:  (Pixmap dst) the destination Pixmap, (Point origin) the	*/
/* amount added to each of the points in *pp, (short n) the number of points in *pp,	*/
/* (Point *pp) the array of endpoints of the line segments, (short width) the width of	*/
/* the poly-line, (LineStyle *style) the line style to use, (Rectangle clipper) the	*/
/* clipping rectangle, (Boolean clip) true if clipping should be performed, and (Color	*/
/* c) the poly-line's color.  A poly-line is a sequence of line segments connected at	*/
/* common endpoints.  The poly-line will not be closed unless the first point is the	*/
/* same as the last point.  All points are relative to origin, not the previous point	*/
/* in the poly-line.									*/

EXTERN(void, boxColor, (Pixmap dst, Rectangle r, short width, Bitmap pattern, Point offset, Rectangle clipper, Boolean clip, Color foreground, Color background));
/* Please use boxPane in sm.h.	*/
/* draw a box in a Pixmap		*/
/* Takes nine parameters: (Pixmap dst) the Pixmap in which to draw the box, (Rectangle	*/
/* r) the box's outer border, (short width) the width of the border, (Bitmap pattern)	*/
/* the pattern to use, (Point offset) the pattern offset from the ul corner of dst,	*/
/* (Rectangle clipper) the clipping rectangle, (Boolean clip) true if the box is not	*/
/* guaranteed to be within the pixmap boundary of the destination, and			*/
/* (Color foreground) and (Color background) the colors for 1's and 0's in the pattern.	*/
/* To color the box with the foreground color, use pattern = NULL_BITMAP.  To avoid	*/
/* coloring either the 1's or 0's in the pattern, use NULL_COLOR.			*/
/* A rectangular box, possibly empty inside with border width "width," is produced.	*/

EXTERN(void, polygonColor, (Pixmap dst, Point origin, short n, Point *pp, Bitmap pattern, Point offset, Rectangle clipper, Boolean clip, Color foreground, Color background));
/* Please use polygonPane in sm.h.	*/
/* cover a polygonal shape with a bitmap	*/
/* Takes ten parameters:  (Pixmap dst) the destination Pixmap, (Point origin) the	*/
/* amount added to each of the points in *pp, (short n) the number of points in *pp,	*/
/* (Point *pp) the array of endpoints of the line segments of the polygon, (Bitmap	*/
/* pattern) the pattern to repeat in the polygon, (Point offset) the upper left corner	*/
/* to begin the pattern relative to dst's upper left corner, (Rectangle clipper) the	*/
/* clipping rectangle, (Boolean clip) true if clipping is to be performed, and (Color	*/
/* foreground, background) the colors for 1's and 0's in the pattern.  The list of	*/
/* points is "closed" automatically, i.e. the first point in the list need not be the	*/
/* last.  The area is covered using an even-odd rule (a point is drawn if an infinite	*/
/* ray with the point as origin crosses the path an odd number of times).  Specifying	*/
/* a color of NULL_COLOR means the corresponding bits should not be colored.		*/

EXTERN(void, arcLine, (Pixmap dst, Rectangle* r, double a1, double a2, short width,
 LineStyle *style, Rectangle clipper, Boolean clip, Color c));
/* Please use arcLinePane in sm.h. */
/* draw an arc outline in a Pixmap	*/
/* Takes nine parameters:  (Pixmap dst) the destination Pixmap to receive the arc,	*/
/* (Rectangle* r) the bounding box of the ellipse from which the arc is derived, (dbl	*/
/* a1) the starting angle of the arc measured in degrees from 3:00 counterclockwise,	*/
/* (dbl a2) the extent of the arc measured in degrees counterclockwise from a1,   	*/
/* (short width) the line width of the arc, (LineStyle *style) the line style to use	*/
/* for the arc, (Rectangle clipper) the clipping rectangle, (Boolean clip) true if	*/
/* clipping should be performed, and (Color c) the arc's color.  Note the angles are	*/
/* "true" and ignore the aspect ratio of the bounding box.  NOTE:  This call does	*/
/* nothing under Suntools.								*/

EXTERN(void, arcColor, (Pixmap dst, Rectangle* r, double a1, double a2,
 Bitmap pattern, Point offset, Rectangle clipper, Boolean clip, Color foreground,
 Color background));
/* Please use arcColorPane in sm.h.	*/
/* cover an arc pie slice in a Pixmap	*/
/* Takes eight parameters:  (Pixmap dst) the destination Pixmap, (Rectangle r) the	*/
/* bounding box of the ellipse from which the arc is derived, (float a1) the starting	*/
/* angle of the arc measured in degrees*64 from 3 o'clock counterclockwise, (float a2) 	*/
/* the extent of the arc measured in degrees*64 counterclockwise from a1, (Bitmap pattern)*/
/* the pattern to fill the arc, (Point offset) the upper left corner of the pattern's	*/
/* tiling relative to the upper left corner of dst, (Rectangle clipper) the clipping	*/
/* rectangle, (Boolean clip) true if clipping should be performed, and (Color		*/
/* foreground, background) the color for 1's and 0's in the pattern.  Note that the	*/
/* angles are "true" and ignore the aspect ratio of the bounding box.  The color just	*/
/* the 1's or the 0's in the pattern, use NULL_COLOR as the other color's argument.	*/
/* NOTE:  This call does nothing under Suntools.					*/


	/****** IMAGE FILE DEFINITIONS ******/

#define IF_MAGIC	0x59a66a95		/* an image file magic number		*/
typedef	struct		{			/* an image file header			*/
	int		if_magic;		/* the image file magic number		*/
	int		if_width;		/* the image width			*/
	int		if_height;		/* the image height			*/
	int		if_depth;		/* the image depth (1 only)		*/
	int		if_bytes;		/* the image number of bytes		*/
	int		if_type;		/* the image file type (1 only)		*/
	int		if_cm_type;		/* the image colormap type (0 only)	*/
	int		if_cm_length;		/* the image colormap length (0 only)	*/
			} ifHeader;

EXTERN(void, start_gfx,(void));
EXTERN(void,finish_gfx,(void));
#endif
