/* $Id: font.h,v 1.7 1997/03/11 14:33:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 			font.h				*/
		/*	   Font opening and TextString calls.		*/
		/* 							*/
		/********************************************************/


	/* TEXT-CHARACTER/TEXT-STRING MAKING INFORMATION -- General use.	*/

#ifndef font_h
#define font_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef point_h
#include <libs/graphicInterface/support/graphics/point.h>
#endif

	/* The following font styles are mutually exclusive */
#define STYLE_NORMAL	 0			/* the normal font style		*/
#define STYLE_BOLD	 1			/* the emboldened font style		*/
#define STYLE_ITALIC	 2			/* the italic font style		*/
#define STYLE_BOLDITALIC 3			/* the bold italic font style		*/
#define STYLE_TYPES	 4			/* the number of style types above	*/
#define STYLE_MASK	 3			/* mask of style bits			*/

	/* The following font attributes are independent of each other */
#define ATTR_UNDERLINE	 4			/* underlined characters		*/
#define ATTR_INVERSE	 8			/* inverted characters			*/
#define	ATTR_HALF	16			/* half intensity characters		*/
#define ATTR_CURSOR	32			/* cursor character			*/

typedef	struct	{				/* A TEXT CHARACTER (for writing)	*/
	unsigned char	ch;			/* the actual character			*/
	unsigned char	style;			/* the style parameter			*/
		}	TextChar;

typedef	struct	{				/* A TEXT STRING (for writing)		*/
	short		num_tc;			/* the number of text characters	*/
	TextChar	*tc_ptr;		/* the text character array		*/
	Boolean		ephemeral;		/* "tc_ptr should be freed on write"	*/
		}	TextString;
extern TextString	emptyTextString;	/* an empty text string			*/

EXTERN(TextChar, makeTextChar, (unsigned char ch, unsigned char style));
/* create a text char from its parts	*/
/* Takes two parameters (unsigned char ch) the character and (unsigned char style) the	*/
/* style to use for the TextChar.							*/

#define 		equalTextChar(t1, t2) 	/* compare two TextChars		*/  BOOL(t1.ch == t2.ch && t1.style == t2.style)
/* Takes two parameters (TextChar tc1, tc2), two TextChars to compare.  Returns true	*/
/* iff the two are identical.								*/

EXTERN(TextString, makeTextString, (char *s, unsigned char style, char *who));
/* create an ephemeral text string	*/
/* Takes two parameters (char *s) the string to use and (unsigned char style) the style	*/
/* index for the whole string.  An ephemeral text string is created and returned.	*/

EXTERN(TextString, makePartialTextString, (char *s, unsigned char style,
 short len, char *who));
/* create a limited eph. text string	*/
/* Takes three parameters (char *s) the starting character position of the TextString	*/
/* source, (unsigned char style) the style index of the whole string, and (short len)	*/
/* the number of characters to use in making up the string.				*/

EXTERN(TextString, createTextString, (short size, char *who));
/* create a fixed size text string	*/
/* Takes two parameters:  (short size) the size of the text string to make and		*/
/* (char *who) the memory identification string.  A non-ephemeral TextString of the	*/
/* appropriate size is returned and the TextChars are not set to anything.		*/

EXTERN(TextString, copyTextString, (TextString ts));
/* duplicate a TextString		*/
/* Takes one parameter:  (TextString src) the text string to copy.  Returns a non-	*/
/* ephemeral duplicate of the original.							*/

EXTERN(TextString, subTextString, (TextString ts, short start, short count));
/* duplicate a portion of a TextString	*/
/* Takes three parameters:  (TextString src) the source text string, (short start, len)	*/
/* the position to begin copying and the number of TextChars to copy.  Returns a non-	*/
/* ephemeral partial duplicate of the original.						*/

EXTERN(void, moveTextString, (TextString src, TextString *dst));
/* Copy the contents of a TextString	*/
/* Takes two parameters:  (TextString src) the source TextString and (TextString *dest)	*/
/* the destination TextString pointer.  The TextChars of the source are copied into the	*/
/* destination.  Note:  the destination must have a tc_ptr allocated big enough.	*/

EXTERN(void, destroyTextString, (TextString ts));
/* free the storage of a TextString	*/
/* Takes one parameter:  (TextString ts) the TextString to free.  If a tc_ptr was	*/
/* allocated, it will be freed.								*/


	/* FONT INITIALIZATION/FINALIZATION -- System use.			*/

EXTERN(void, fontsInitialize, (void));
/* intitialize the font module		*/
/* Takes no parameters.  Defines DEF_FONT_ID based on DEF_FONT_NAME.			*/

EXTERN(void, fontsFinalize, (void));
/* finalize the font module		*/
/* Takes no parameters.  Checks to make sure all open fonts have been closed.		*/


	/* FONT OPENING AND CLOSING -- General use.				*/

EXTERN(short, fontOpen, (char *name));
/* open an Rn font file			*/
/* Takes one parameter (char *name) the name of the font file.  The id for this font is	*/
/* returned.  Returns UNUSED if the file doesn't exist, the file is the wrong format or	*/
/* too many fonts have been opened.  A font opened with this call must later be closed	*/
/* with fontClose() below.  Note:  opening a previously opened font is an inexpensive	*/
/* operation because of reference counting.						*/

EXTERN(void, fontClose, (short id));
/* close a previously opened font	*/
/* Takes one parameter (short font) the index of a previously opened font.  This call	*/
/* will decrement the reference count for the font and free the resources if necessary.	*/

extern	short		DEF_FONT_ID;		/* the default font			*/


	/* FONT GRAPHICS DEFINITIONS -- To be used only by screen modules and	*/
	/* stand alone programs.						*/

#define	FONT_NUM_CHARS				/* the number of characters in a font	*/	256
#define FONT_BITMAP_SIZE(glyph_size)		/* the size of the style pixmap		*/	makePoint(glyph_size.x * FONT_NUM_CHARS, glyph_size.y)
#define	FONT_CHAR_AREA(size, ch)		/* region of a glyphs pixmap for a char	*/	makeRectFromSize(makePoint((short) (ch) * size.x, 0), size)

EXTERN(Point, fontSize, (short id));
/* return the character size of a font	*/
/* Takes one parameter (short font) font to use. Returns the size in pixels of a	*/
/* character in that font.								*/

EXTERN(short, fontBaseline, (short id));
/* return the baseline value of a font	*/
/* Takes one parameter (short font) font to use. Returns the uld to baseline distance	*/
/* of the font.										*/

#ifdef gfx_h
EXTERN(void, fontColorWrite, (Pixmap dst, Point p, Rectangle clip, TextString ts, short font, Color foreground, Color background));
/* fontColorWrite is an implementation function.  Please call fontWritePane.	*/
#endif  /* gfx_h */

#endif
