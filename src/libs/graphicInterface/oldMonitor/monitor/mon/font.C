/* $Id: font.C,v 1.1 1997/06/25 14:53:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 			font.c				*/
		/*   		    Font functions.			*/
		/* 							*/
		/********************************************************/


#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>
#include <stdio.h>

#include <include/bstring.h>
#include <libs/support/misc/general.h>

#include <libs/graphicInterface/support/graphics/point.h>
#include <libs/graphicInterface/support/graphics/rect.h>
#include <libs/support/memMgmt/mem.h>

#include <libs/graphicInterface/oldMonitor/include/mon/gfx.h>
#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/mon/font.h>

extern	unsigned char	UNDERLINE_CH;		/* the underline character		*/
extern	unsigned char	CURSOR_CH;		/* the cursor character			*/
extern	char		*D_font_dirs[];	/* the standard Rn font directory	*/
	short		DEF_FONT_ID;		/* the default font index		*/
TextString		emptyTextString = 	/* an empty text string			*/
				{ 0, 0, false};
extern	char		*DEF_FONT_NAME;		/* the default font name		*/
static	Bitmap		half_bitmap;		/* the 'half' intensity bitmap		*/
						/* use with op BLT_AND			*/


	/* FONT INFORMATION */

typedef	struct	font	{			/* FONT FILE STRUCTURE			*/
	char		*name;			/* the font name			*/
	Point		glyph_size;		/* the size of each glyph		*/
	Rectangle	glyph_area;		/* the Origin based area of a glyph	*/
	short		baseline;		/* the baseline, origin distance	*/
	Bitmap		scratch_bitmap;		/* workspace				*/
	Bitmap		glyphs[STYLE_TYPES];	/* the character glyphs for each style	*/
	short		ref_count;		/* the number of users of this font	*/
			} RnFont;
#define	NULL_FONT	(RnFont *) 0		/* a non-open font			*/
#define	MAX_FONTS	50			/* the maximum fonts allowed		*/
static	RnFont		*open_fonts[MAX_FONTS];	/* the fonts that have been opened	*/
static	short		num_open_fonts = 0;	/* number of open fonts			*/


/* Convert a char and style into the corresponding TextChar				*/
TextChar
makeTextChar(unsigned char ch, unsigned char style)
{
TextChar		tch;			/* the returned TextChar		*/

	tch.ch    = ch;
	tch.style = style;
	return (tch);
}


/* Return true if two TextChar's are exactly the same and false otherwise.		*/
/* Boolean
 * equalTextChar(tc1, tc2)
 * TextChar		tc1;
 * TextChar		tc2;
 * {
 *	return (BOOL(tc1.ch == tc2.ch && tc1.style == tc2.style));
 * }
 */

/* Return an ephemeral text string representing a regular string and a style.		*/
TextString
makeTextString(char *s, unsigned char style, char *who)
{
TextString		ts;			/* the text string being made		*/
register short		i;			/* the current character		*/

	ts.num_tc    = strlen(s);
	ts.tc_ptr    = (TextChar *) ((ts.num_tc == 0) ? 0 : get_mem(ts.num_tc * sizeof(TextChar), "%s (makeTextString)", who));
	ts.ephemeral = true;
	for (i = 0; i < ts.num_tc; i++)
	{/* transfer a character to a TextChar */
		ts.tc_ptr[i].ch    = s[i];
		ts.tc_ptr[i].style = style;
	}
	return ts;
}


/* Return an ephemeral text string representing a regular string and a style.		*/
TextString
makePartialTextString(char *s, unsigned char style, short len, char *who)
{
TextString		ts;			/* the text string being made		*/
register short		i;			/* the current character		*/

	ts.num_tc    = len;
	ts.tc_ptr    = (TextChar *) ((len == 0) ? 0 : get_mem(len * sizeof(TextChar), "%s (makePartialTextString)", who));
	ts.ephemeral = true;
	for (i = 0; i < len; i++)
	{/* transfer a character to a TextChar */
		ts.tc_ptr[i].ch    = s[i];
		ts.tc_ptr[i].style = style;
	}
	return ts;
}


/* Make a TextString of a known size.							*/
TextString
createTextString(short size, char *who)
{
TextString		ts;			/* the resulting TextString		*/

	ts.num_tc    = size;
	ts.ephemeral = false;
	ts.tc_ptr    = (TextChar *) get_mem(size * sizeof(TextChar), "createTextString for %s", who);
	return ts;
}


/* Make a duplicate of a TextString.							*/
TextString
copyTextString(TextString ts)
{
	return subTextString(ts, 0, ts.num_tc);
}


/* Duplicate a portion of a TextString.							*/
TextString
subTextString(TextString ts, short start, short count)
{
TextString		result;			/* the resulting TextString		*/
short			bytes;			/* the number of bytes involved		*/

	result.num_tc = count;
	result.ephemeral = false;
	if (count == 0)
	{/* don't bother allocating space */
		result.tc_ptr = (TextChar *) 0;
	}
	else
	{/* allocate space and copy */
		bytes = count * sizeof(TextChar);
		result.tc_ptr = (TextChar *) get_mem(bytes,"subTextString");
		bcopy((char *) (ts.tc_ptr + start), (char *) result.tc_ptr, bytes);
	}

	return result;
}


/* Destroy the storage associated with a TextString.					*/
void
moveTextString(TextString src, TextString *dst)
{
	dst->num_tc = src.num_tc;
	bcopy((char *) src.tc_ptr, (char *) dst->tc_ptr, src.num_tc * sizeof(TextChar));
}


/* Destroy the storage associated with a TextString.					*/
void
destroyTextString(TextString ts)
{
	if (ts.num_tc)
	{/* the pointer had been allocated */
		free_mem((void*) ts.tc_ptr);
	}
}


/* Open and set the DEF_FONT_ID.							*/
void
fontsInitialize()
{
static  BITMAPM_UNIT    half_data[]   = {       /* a 'half' intensity bit pattern       */
                                0xB5B5, 0x6B6B, 0xB6B6, 0x5B5B, 0xB5B5, 0x6B6B, 0xB6B6, 0x5B5B,
                                0xB5B5, 0x6B6B, 0xB6B6, 0x5B5B, 0xB5B5, 0x6B6B, 0xB6B6, 0x5B5B
                        };
#if 0  /** DEBUG 04 Mar 14.0 */
int			font_index;		/* loop variable			*/

	for (font_index = 0; font_index < MAX_FONTS; font_index++)
		open_fonts[font_index] = (RnFont *) 0;
#endif  /** 0 DEBUG  04 Mar 14.0 */

        half_bitmap = makeBitmapFromData(makePoint(16, 16), half_data, "font.c:  fontsInitialize()");
	if ((DEF_FONT_ID = fontOpen(DEF_FONT_NAME)) == UNUSED)
	{/* the original font could not be opened */
		die_with_message("The default font '%s' could not be opened.", DEF_FONT_NAME);
	}
}


/* Free the default font and check to make sure all fonts have been closed.		*/
void
fontsFinalize()
{
	freeBitmap(half_bitmap);
	fontClose(DEF_FONT_ID);
	if (num_open_fonts)
	{/* not all fonts were closed */
		die_with_message("Not all opened fonts were closed.");
	}
}


#define BITS_PER_BYTE	8			/* the number of bits in a byte		*/

/* Return the identifier associated with a font.  Return UNUSED if it is not available.	*/
short
fontOpen(char *name)
{
short			i;			/* font index				*/
char			filename[100];		/* dummy font file name			*/
int			fd;			/* the font file descriptor		*/
char			**dir;			/* directory list			*/
register RnFont		*cf;			/* the current font being created	*/
register short		style;			/* the style index			*/
short			x, y;			/* size coordinates for reading		*/
Point			bitmap_size;		/* the style bitmap size			*/
BITMAPM_UNIT		*data;			/* the bits of the font bitmap		*/

	/* check to see if it is already opened */
		if (num_open_fonts)
		{/* there is a possibility of duplication */
			for (i = 0; i < MAX_FONTS; i++)
			{/* check the i'th font slot for a duplication */
				if ((open_fonts[i] != NULL_FONT) && (strcmp(open_fonts[i]->name, name) == 0))
				{/* the font was found--return its index */
					open_fonts[i]->ref_count++;
					return (i);
				}
			}
		}

	/* check to see if we can open another */
		if (num_open_fonts == MAX_FONTS)
		{/* the maximum number of fonts has been reached */
			return (UNUSED);
		}

	/* open the font file */
		fd = -1;
		for (dir = D_font_dirs; *dir && (fd == -1); dir++)
		{/* try each directory in turn until there is a match */
			(void) sprintf(filename, "%s%s", *dir, name);
			fd = open(filename, O_RDONLY);
		}
		if (fd == -1)
		{/* the font file could not be found */
			return (UNUSED);
		}

	/* create and initialize a new font */
		cf = (RnFont *) get_mem(sizeof(RnFont), "font.c: font structure");
		(void) read(fd, (char *) &x,            sizeof(short));
		(void) read(fd, (char *) &y,            sizeof(short));
		(void) read(fd, (char *) &cf->baseline, sizeof(short));
		cf->name         = (char *) get_mem(strlen(name) + 1, "font.c: font name");
		cf->glyph_size	 = makePoint(x, y);
		cf->glyph_area	 = makeRectFromSize(Origin, cf->glyph_size);
		cf->scratch_bitmap = makeBitmap(cf->glyph_size, "font.c: scratch bitmap");
		cf->ref_count    = 1;
		(void) strcpy(cf->name, name);

	/* read in the bitmap for each style */
		bitmap_size = FONT_BITMAP_SIZE(cf->glyph_size);
		data = (BITMAPM_UNIT *) get_mem(BITMAPM_BYTES_PER_IMAGE(bitmap_size.x, bitmap_size.y), "temporary font bitmap storage");
		for (style = STYLE_NORMAL; style < STYLE_TYPES; style++)
		{/* get bitmaps for the current style */
			(void) read(fd, (char *) data, BITMAPM_BYTES_PER_IMAGE(bitmap_size.x, bitmap_size.y));
			cf->glyphs[style] = makeBitmapFromData(bitmap_size, data, "font.c: new style glyph");
		}
		free_mem((void*) data);

	/* close the font file */
		(void) close(fd);

	/* install and return the new font */
		for (i = 0; i < MAX_FONTS; i++)
		{/* look for an empty space */
			if (open_fonts[i] == NULL_FONT)
			{/* this is an empty space */
				break;
			}
		}
		open_fonts[i] = cf;
		num_open_fonts++;
		return (i);
}


/* Close a previously opened font.							*/
void
fontClose(short id)
{
register short		style;			/* the current style index		*/

	if (--open_fonts[id]->ref_count == 0)
	{/* free the resources of this font */
		for (style = STYLE_NORMAL; style < STYLE_TYPES; style++)
		{/* free each style glyph */
			freeBitmap(open_fonts[id]->glyphs[style]);
		}
		freeBitmap(open_fonts[id]->scratch_bitmap);
		free_mem((void*) open_fonts[id]->name);
		free_mem((void*) open_fonts[id]);
		open_fonts[id] = NULL_FONT;
		num_open_fonts--;
	}
}


/* Return the glyph size of the font.							*/
Point
fontSize(short id)
{
	return (open_fonts[id]->glyph_size);
}


/* Return the ulc to basline distance of the font.					*/
short
fontBaseline(short id)
{
	return (open_fonts[id]->baseline);
}


/* Write a text string to a pixmap postion in a given font.				*/
void
fontColorWrite(Pixmap dst, Point p, Rectangle clip, TextString ts, 
               short font, Color fg, Color bg)
{
register RnFont		*cf = open_fonts[font];	/* the current font to use		*/
register TextChar	*tcp = ts.tc_ptr;	/* the pointer to the current TextChar	*/
Rectangle		r;			/* the area of the scratch pixmap to cpy*/
Rectangle		r1;

	if (dst)
	{/* there is writing to be done */
		while (ts.num_tc)
		{/* write the current TextChar */
		  if (tcp->style & 
		      (ATTR_UNDERLINE | ATTR_HALF | ATTR_CURSOR))
		  {
			/* COPY the character's glyph into the scratch area */
		    		BLTBitmap(cf->glyphs[tcp->style & STYLE_MASK],
					cf->scratch_bitmap,
					FONT_CHAR_AREA(cf->glyph_size, tcp->ch),
					Origin,
					BITMAP_COPY,
					false);

			/* MODIFY the scratch area based on the attributes */
				if (tcp->style & ATTR_UNDERLINE)
				{/* draw the underline */
					BLTBitmap(cf->glyphs[tcp->style & STYLE_MASK], cf->scratch_bitmap, FONT_CHAR_AREA(cf->glyph_size, UNDERLINE_CH), Origin, BITMAP_OR, false);
				}
				if (tcp->style & ATTR_HALF)
				{/* AND the half intensity character */
					BLTBitmap(half_bitmap, cf->scratch_bitmap, makeRectFromSize(Origin, getBitmapSize(half_bitmap)), Origin, BITMAP_AND, false);
				}
				if (tcp->style & ATTR_CURSOR)
				{/* XOR the cursor character */
					BLTBitmap(cf->glyphs[tcp->style & STYLE_MASK], cf->scratch_bitmap, FONT_CHAR_AREA(cf->glyph_size, CURSOR_CH), Origin, BITMAP_XOR, false);
				}

			/* Place the scratch area into the destination. */
				r = interRect(subRect(clip, p), cf->glyph_area);
				ColorWithPattern(dst,
						 transRect(r, p),
						 cf->scratch_bitmap,
						 transPoint(r.ul,p),
						 false,  /* already clipped */
						 (tcp->style & ATTR_INVERSE ? bg : fg),
						 (tcp->style & ATTR_INVERSE ? fg : bg));
		  }
		  else	/* at most ATTR_INVERSE */
		  {	
			r  = interRect(subRect(clip, p), cf->glyph_area); 
			r1 = FONT_CHAR_AREA(cf->glyph_size, tcp->ch);
			ColorWithPattern(dst,
					 transRect(r, p),
					 cf->glyphs[tcp->style & STYLE_MASK], /* font */
					 subPoint(transPoint(r.ul, p), r1.ul), /* font's offset */
					 false,  /* already clipped */
					 (tcp->style & ATTR_INVERSE ? bg : fg),
					 (tcp->style & ATTR_INVERSE ? fg : bg));
		  }

			/* set up for the next iteration */
				p.x += cf->glyph_size.x;
				ts.num_tc--;
				tcp++;
	      }
	}

	if (ts.ephemeral && ts.tc_ptr)
	{/* free the tc_ptr */
		free_mem((void*) ts.tc_ptr);
	}
}



