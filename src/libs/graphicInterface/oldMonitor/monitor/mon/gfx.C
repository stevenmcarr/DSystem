/* $Id: gfx.C,v 1.1 1997/06/25 14:53:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
 		/********************************************************/
		/* 			gfx.c				*/
		/*   		Low level graphics functions.		*/
		/* 							*/
		/********************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <sys/file.h>
#include <unistd.h>

#include <libs/support/misc/general.h>
#include <libs/graphicInterface/support/graphics/point.h>
#include <libs/graphicInterface/support/graphics/rect.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/graphicInterface/oldMonitor/include/mon/gfx.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/inlines.h>


	/* LineStyles */

LineStyle		*line_style_solid = 	/* the solid line style			*/
				(LineStyle *) 0;

static	char		dotted_pattern[] =	/* dotted pattern			*/
				{ 1, 1 };
static	LineStyle	dotted =		/* dotted line style structure		*/
				{ 2, dotted_pattern, 0 };
LineStyle		*line_style_dotted =	/* the dotted line style		*/
				&dotted;

static	char		dashed_pattern[] =	/* dashed pattern			*/
				{ 4, 4 };
static	LineStyle	dashed =		/* dashed line style structure		*/
				{ 2, dashed_pattern, 0 };
LineStyle		*line_style_dashed =	/* the dashed line style		*/
				&dashed;


	/* Bitmaps */

Bitmap			gray_bitmap;		/* 50% gray cover, use with BITMAP_AND	*/


	/* MouseCursors */

MouseCursor		standard_cursor;	/* standard pointer for general use	*/
MouseCursor		moving_cursor;		/* hand for use in window movement	*/
MouseCursor		sizing_cursor;		/* corner for use in window sizing	*/
MouseCursor		invisible_cursor;	/* no cursor at all			*/
MouseCursor		wait_cursor;		/* watch cursor for long waits		*/


/* Write a bitmap to a named image file.						*/
Boolean
fileBitmap(Bitmap src, char *name)
{
Point			srcSize;		/* the size of the source bitmap		*/
int			fd;			/* the file descriptor to read from	*/
ifHeader		header;			/* the header of the image file		*/
int			bytes;			/* the number of bytes in the image	*/
BITMAPM_UNIT		*data;			/* the data of the image		*/

	if ((fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644)) != -1)
	{/* the file was opened and the area was interesting */
		srcSize = getBitmapSize(src);
		bytes = BITMAPM_BYTES_PER_IMAGE(srcSize.x, srcSize.y);
		data = (BITMAPM_UNIT *) get_mem(bytes, "fileBitmap()");
		getBitmapData(src, data);

		/* write the bitmap (tmp) */
			header.if_magic     = IF_MAGIC;
			header.if_width     = srcSize.x;
			header.if_height    = srcSize.y;
			header.if_depth     = 1;
			header.if_bytes     = bytes;
			header.if_type      = 1;
			header.if_cm_type   = 0;
			header.if_cm_length = 0;
			(void) write(fd, (char *) &header, sizeof(ifHeader));
			(void) write(fd, (char *) data, bytes);

		free_mem((void*) data);
		(void) close(fd);
		return (true);
	}
	else
	{/* the request was bogus or the file could not be opened */
		return (false);
	}
}


/* Load a previously saved image file into a bitmap.  (The bitmap must be later freed.)	*/
Bitmap
loadBitmap(char *name)
{
int			fd;			/* the file descriptor to read from	*/
ifHeader		header;			/* the header of the image file		*/
BITMAPM_UNIT		*data;			/* the data of the image		*/
Bitmap			m = (Bitmap) 0;		/* the returned bitmap			*/

	if ((fd = open(name, O_RDONLY, 0)) != -1)
	{/* the file could opened */
		if (
			(read(fd, (char *) &header, sizeof(ifHeader)) == sizeof(ifHeader)) &&
			(header.if_magic == IF_MAGIC) &&
			(header.if_width > 0) &&
			(header.if_height > 0) &&
			(header.if_depth == 1) &&
			(header.if_bytes == BITMAPM_BYTES_PER_IMAGE(header.if_width, header.if_height)) &&
			(header.if_type == 1) &&
			(header.if_cm_type == 0) &&
			(header.if_cm_length == 0)
		)
		{/* the header was valid */
			data = (BITMAPM_UNIT *) get_mem(header.if_bytes, "loadBitmap()");
			if (read(fd, (char *) data, header.if_bytes) == header.if_bytes)
			{/* the image was read */
				m = makeBitmapFromData(makePoint(header.if_width, header.if_height), data, name);
			}
			else
				die_with_message("loadBitmap(): failed to create a Bitmap");

			free_mem((void*) data);
		}
		else
			die_with_message("loadBitmap(): failed to find the file (%s)",name);
			(void) close(fd);
	}
	else
		die_with_message("loadBitmap(): failed to open the file (%s)", name);

	return m;
}


/* Return a new rotated bitmap based on the original bitmap.				*/
Bitmap
rotateBitmap(Bitmap src, short amount)
{
Point			srcSize;		/* the size of the source bitmap		*/
Bitmap			dst;			/* the destination rotated bitmap		*/
Point			pt;			/* the current point in tmp being copied*/

	if (src == NULL_BITMAP)
		die_with_message("rotateBitmap(): src may not be NULL_BITMAP!");

	amount &= 3;
	srcSize = getBitmapSize(src);
	dst = makeBitmap((amount & 1) ? makePoint(srcSize.y, srcSize.x) : srcSize, "rotateBitmap(): rotated result image");
	BLTBitmap(NULL_BITMAP, dst, makeRectFromSize(Origin, srcSize), Origin, BITMAP_CLR, false);
	if (amount)
	{/* do it the hard way */
		for (pt.y = 0; pt.y < srcSize.y; pt.y++)
		{/* walk down the lines of the original bitmap */
			for (pt.x = 0; pt.x < srcSize.x; pt.x++)
			{/* walk down the current line of the original bitmap */
				switch (amount)
				{/* put the pixel in the proper place */
					case 1:	/* rotate 90 degrees */
						BLTBitmap(src, dst, makeRect(pt, pt),
							makePoint(srcSize.y - 1 - pt.y, pt.x                ),
							BITMAP_COPY, false);
						break;
					case 2:	/* rotate 180 degrees */
						BLTBitmap(src, dst, makeRect(pt, pt),
							makePoint(srcSize.x - 1 - pt.x, srcSize.y - 1 - pt.y),
							BITMAP_COPY, false);
						break;
					case 3:	/* rotate 270 degrees */
						BLTBitmap(src, dst, makeRect(pt, pt),
							makePoint(pt.y,                 srcSize.x - 1 - pt.x),
							BITMAP_COPY, false);
						break;
				}
			}
		}
	}
	else
	{/* do the simple case the easy way */
		BLTBitmap(src, dst, makeRectFromSize(Origin, srcSize), Origin, BITMAP_COPY, false);
	}
	return (dst);
}


/* Draw a box of the given width within rectangle r in pixmap dst.			*/
void
boxColor(Pixmap dst, Rectangle r, short width, Bitmap pattern, Point offset, 
         Rectangle clipper, Boolean clip, Color foreground, Color background)
{
/* The box looks like this:
 *
 *  Rectangle r
 * ------------------------------
 * |YYXXXXXXXXXXXXXXXXXXXXXXXXYY|	pattern begins according to upper left corner
 * |YYXXXXXXXXXXXXXXXXXXXXXXXXYY|	dst, not the Rectangle r
 * |XX			      XX|
 * |XX			      XX|	If no hole in the middle, produce entire box.
 * |XX			      XX|	Else:	1. produce top X's and ul Y's
 * |XX			      XX|		2. produce bottom X's and lr Y's
 * |XX			      XX|		3. produce left X's and ll Y's
 * |YYXXXXXXXXXXXXXXXXXXXXXXXXYY|		4. produce right X's and ur Y's
 * |YYXXXXXXXXXXXXXXXXXXXXXXXXYY|	(X's and Y's represent the same pattern)
 * ------------------------------
 */
	if( NOT(clip) )  clipper = MaximumRect;

	if (width == 0 || (foreground == NULL_COLOR && background == NULL_COLOR))
	{/* there is no border to draw */
		return;
	}
	else if (MIN(r.lr.x - r.ul.x, r.lr.y - r.ul.y) >= width + width)
	{/* normal box--do four COVERs */
		/* top X's and upper left corner Y's */
		ColorWithPattern(dst,
				 interRect(clipper, makeRect(r.ul, makePoint(r.lr.x-width, r.ul.y+width-1))),
				 pattern,
				 offset,
				 clip,
				 foreground,
				 background);

		/* bottom X's and lower right corner Y's */
		ColorWithPattern(dst,
				 interRect(clipper, makeRect(makePoint(r.ul.x+width, r.lr.y-width+1), r.lr)),
				 pattern,
				 offset,
				 clip,
				 foreground,
				 background);

		/* left X's and lower left corner Y's */
		ColorWithPattern(dst,
				 interRect(clipper, makeRect(makePoint(r.ul.x, r.ul.y+width),
							     makePoint(r.ul.x+width-1, r.lr.y))),
				 pattern,
				 offset,
				 clip,
				 foreground,
				 background);

		/* right X's and upper left corner Y's */
		ColorWithPattern(dst,
				 interRect(clipper, makeRect(makePoint(r.lr.x-width+1, r.ul.y),
							     makePoint(r.lr.x, r.lr.y-width))),
				 pattern,
				 offset,
				 clip,
				 foreground,
				 background);
	}
	else
	{/* box with no inside not-patterned area */
		ColorWithPattern(dst, interRect(clipper, r), pattern, offset, clip, foreground, background);
	}
}


/* Attempt to invert colors within a box.	*/
void invertBox(Pixmap dst, Rectangle r, short width, Boolean clip)
{
/* The box looks like this:
 *
 *  Rectangle r
 * ------------------------------
 * |YYXXXXXXXXXXXXXXXXXXXXXXXXYY|	pattern begins according to upper left corner
 * |YYXXXXXXXXXXXXXXXXXXXXXXXXYY|	dst, not the Rectangle r
 * |XX			      XX|
 * |XX			      XX|	If no hole in the middle, produce entire box.
 * |XX			      XX|	Else:	1. produce top X's and ul Y's
 * |XX			      XX|		2. produce bottom X's and lr Y's
 * |XX			      XX|		3. produce left X's and ll Y's
 * |YYXXXXXXXXXXXXXXXXXXXXXXXXYY|		4. produce right X's and ur Y's
 * |YYXXXXXXXXXXXXXXXXXXXXXXXXYY|	(X's and Y's represent the same pattern)
 * ------------------------------
 */
	if (width == 0)
	{/* there is no border to draw */
		return;
	}
	else if (MIN(r.lr.x - r.ul.x, r.lr.y - r.ul.y) >= width + width)
	{/* normal box--do four COVERs */
		/* top X's and upper left corner Y's */
		invertPixmap(dst,
			     makeRect(r.ul, makePoint(r.lr.x-width, r.ul.y+width-1)),
			     clip);

		/* bottom X's and lower right corner Y's */
		invertPixmap(dst,
			     makeRect(makePoint(r.ul.x+width, r.lr.y-width+1), r.lr),
			     clip);

		/* left X's and lower left corner Y's */
		invertPixmap(dst,
			     makeRect(makePoint(r.ul.x, r.ul.y+width),
				      makePoint(r.ul.x+width-1, r.lr.y)),
			     clip);

		/* right X's and upper left corner Y's */
		invertPixmap(dst,
			     makeRect(makePoint(r.lr.x-width+1, r.ul.y),
				      makePoint(r.lr.x, r.lr.y-width)),
			     clip);
	}
	else
	{/* box with no inside not-patterned area */
		invertPixmap(dst, r, clip);
	}
}


/* Start graphics.									*/
void
start_gfx(void)
{
static	BITMAPM_UNIT	null_data[] = {		/* a lot of nothing			*/
				0x0000
			};
static	BITMAPM_UNIT	gray_data[]  = {	/* a 50% pattern			*/
				0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555,
				0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555
			};
static	BITMAPM_UNIT	pointer_data[]  = {	/* a generic pointer pattern		*/
				0x0000, 0x4000, 0x6000, 0x7000, 0x7800, 0x7C00, 0x7E00, 0x7800,
				0x4C00, 0x0C00, 0x0600, 0x0600, 0x0300, 0x0300, 0x0180, 0x0000
			};
static	BITMAPM_UNIT	pointero_data[]  = {	/* a generic pointer outline pattern	*/
				0xC000, 0xE000, 0xF000, 0xF800, 0xFC00, 0xFE00, 0xFF00, 0xFF00,
				0xFE00, 0xDE00, 0x0F00, 0x0F00, 0x0780, 0x0780, 0x03C0, 0x03C0
			};
static	BITMAPM_UNIT	hand_data[] = {		/* a hand pattern			*/
				0x0080,	0x09C8,	0x1DDC,	0x1DDC,	0x1DDF,	0x5DDF,	0xFDDF,	0xFFFF,
				0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0x7FFE,	0x7FFE,	0x3FFC,	0x1FFC
			};
static	BITMAPM_UNIT	handin_data[] = {	/* a hand inside pattern		*/
				0x0080,	0x0948,	0x15D4,	0x1D5C,	0x1557,	0x5555,	0xB557,	0xF775,
				0x9005,	0x9001,	0x9001,	0x8001,	0x4082,	0x4002,	0x2004,	0x1004
			};
static	BITMAPM_UNIT	lrc_data[] = {		/* a lower right corner pattern		*/
				0x0400, 0x0600, 0x0600, 0x0640, 0x0660, 0xFE60, 0x7E60, 0x0060,
				0x0060, 0x1FE0, 0x0FE0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
			};
static	BITMAPM_UNIT	lrcp_data[] = {		/* a lower right corner pattern		*/
				0x0400, 0x0400, 0x0400, 0x0400, 0x0420, 0xFC20, 0x0020, 0x0020,
				0x0020, 0x0020, 0x0FE0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
			};
static	BITMAPM_UNIT	lrcs_data[] = {		/* a lower right corner pattern		*/
				0x0000, 0x0A00, 0x0A00, 0x0A00, 0x7A50, 0x0250, 0x7E50, 0x0050,
				0x0050, 0x0FD0, 0x0010, 0x0FF0, 0x0000, 0x0000, 0x0000, 0x0000
			};
static	BITMAPM_UNIT	watch_data[] = {	/* a watch pattern			*/
				0xF000, 0xFFCC, 0xF836, 0xE00A, 0x4004, 0x4C04, 0x861A, 0x8362,
				0x8182, 0x8002, 0x4004, 0x4006, 0x200F, 0x183F, 0x07DF, 0x000F
			};

static	BITMAPM_UNIT	watchbitmap_data[] = {	/* a watch bitmap pattern			*/
			      0xF000, 0xFFCC, 0xFFFE, 0xFFFE, 0x7FFC, 0x7FFC, 0xFFFE, 0xFFFE,
			      0xFFFE, 0xFFFE, 0x7FFC, 0x7FFE, 0x3FFF, 0x1FFF, 0x07DF, 0x000F
			};


	/* create the Bitmaps */
		gray_bitmap	 = makeBitmapFromData(makePoint(16, 16), gray_data, "gfx.c: start_gfx()");

	/* create the MouseCursors */
  standard_cursor  = makeMouseCursorFromData(makePoint(16, 16), 
                                             pointer_data, BITMAP_OR,   
                                             pointer_data, 0, 
                                             pointero_data, 0, 
                                             makePoint(1, 1),  "gfx.c: start_gfx()");
  moving_cursor    = makeMouseCursorFromData(makePoint(16, 16), 
                                             hand_data, BITMAP_COPY, 
                                             handin_data, 0, 
                                             hand_data, 0,  
                                             makePoint(9, 11), "gfx.c: start_gfx()");
  sizing_cursor    = makeMouseCursorFromData(makePoint(16, 16), 
                                             lrc_data, BITMAP_XOR,  
                                             lrcp_data, 0,    
                                             lrcs_data, 0,     
                                             makePoint(8, 8),  "gfx.c: start_gfx()");
  invisible_cursor = makeMouseCursorFromData(makePoint( 1,  1), 
                                             null_data, BITMAP_NOOP, 
                                             null_data, 0, 
                                             null_data, 0, 
                                             Origin, "gfx.c: start_gfx()");
  wait_cursor      = makeMouseCursorFromData(makePoint(16, 16), 
                                             watch_data, BITMAP_COPY, 
                                             watch_data, 0,   
                                             watchbitmap_data, 0,  
                                             makePoint(7, 7),  "gfx.c: start_gfx()");
}


/* Finish graphics.									*/
void
finish_gfx(void)
{

	/* free the Bitmaps */
		freeBitmap(gray_bitmap);

	/* free the MouseCursors */
		freeMouseCursor(standard_cursor);
		freeMouseCursor(moving_cursor);
		freeMouseCursor(sizing_cursor);
		freeMouseCursor(invisible_cursor);
		freeMouseCursor(wait_cursor);
}
