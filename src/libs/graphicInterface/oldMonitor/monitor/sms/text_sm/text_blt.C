/* $Id: text_blt.C,v 1.1 1997/06/25 15:00:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		       text_blt.c			*/
		/* Writing, movement and clearing related text sm calls.*/
		/* 							*/
		/********************************************************/

#include <libs/graphicInterface/oldMonitor/monitor/sms/text_sm/text.i>


/* Clear out the text pane.								*/
void
sm_text_pane_clear(Pane* p)
{
	ColorPaneWithPattern(p, makeRectFromSize(Origin, p->size), p->pattern, Origin, false);
}


/* Insert a blank line at lineno.  Shift following lines down.				*/
void
sm_text_line_insert(Pane* p, short lineno)
{
	sm_text_lines_copy (p, lineno, MAP_SIZE(p).y - 2, lineno + 1);
	sm_text_lines_clear(p, lineno, lineno);
}


/* Delete a line at lineno.  Shift following lines up.					*/
void
sm_text_line_delete(Pane* p, short lineno)
{
	sm_text_lines_copy (p, lineno + 1, MAP_SIZE(p).y - 1, lineno);
	sm_text_lines_clear(p, MAP_SIZE(p).y - 1, MAP_SIZE(p).y - 1);
}


/* Clear out a block of characters.							*/
void
sm_text_block_clear(Pane* p, Rectangle src)
/* src - the affected glyph rectangle		*/
{
	sm_text_block_clear_color(p, src, NULL_COLOR, NULL_COLOR);
}


/* Clear out a block of characters.							*/
void
sm_text_block_clear_color(Pane* p, Rectangle src, Color fg, Color bg)
/* src - the affected glyph rectangle		*/
/* fg - the foreground color to clear to	*/
/* bg - the background color to clear to	*/
{
	ColorPaneWithPatternColor(p, sm_text_rect_gp(p, src), p->pattern, Origin, true,
				  (fg == NULL_COLOR ? paneForeground(p) : fg),
				  (bg == NULL_COLOR ? paneBackground(p) : bg));
}


/* Non-destructively copy a block of characters from one position to another.		*/
void
sm_text_block_copy(Pane* p, Rectangle src, Point dst_ul, Boolean clip)
/* src - the source glyph rectangle		*/
/* dst_ul - the ul destination glyph coord	*/
/* clip - true if clipping is to be done	*/
{
Point			rel;			/* the pixel distance to move the block	*/
Point			dst;			/* the destination point in pixels	*/
Rectangle		border;			/* the pane clipping rectangle in pixels*/

	if (p->parent->exists)
	{/* safe to do the copy */
		rel = subPoint(dst_ul, src.ul);

		/* change cordinates */
			src = sm_text_rect_gp(p, src);
			rel.x *= GLYPH_SIZE(p).x;
			rel.y *= GLYPH_SIZE(p).y;

		if (clip)
		{/* clip the copy */
			border = makeRectFromSize(Origin, p->size);
			border = clipRectWithBorder(border, border, p->border_width);
			/* clip the pick-up image */
				src = interRect(src, border);
			/* clip the put-down image */
				src = subRect(interRect(transRect(src, rel), border), rel);
		}

		/* compute the destination in pixels */
			dst = transPoint(src.ul, rel);

		/* perform the BLT */
			BLTColorCopyPane(p, p, src, dst, NULL_BITMAP, Origin, true);
	}
}


/* Clear out a line group.								*/
void
sm_text_lines_clear(Pane* p, short start, short end)
/* start - the first line			*/
/* end - the last line			*/
{
	ColorPaneWithPattern(p,
			     sm_text_rect_gp(p, makeRect(makePoint(0, start),
							 makePoint(MAP_SIZE(p).x - 1, end))),
			     p->pattern, Origin, true);
}


/* Copy a group of lines to another position.						*/
void
sm_text_lines_copy(Pane* p, short start, short end, short dst)
/* start - the first line to be copied		*/
/* end - the last line to be copied		*/
/* dst - the new position of the first line	*/
{
	sm_text_block_copy(
			p,
			makeRect(makePoint(0, start), makePoint(MAP_SIZE(p).x - 1, end)),
			makePoint(0, dst),
			TSM_NO_CLIP
	);
}


/* Put a TextChar in a pane at a given location (immediate).  Return its success.	*/
void
sm_text_char_put(Pane* p, Point loc, TextChar tch, Boolean now)
/* loc - Place to put the char (glyph coords)	*/
/* tch - The text character to put		*/
/* now - true if we should propagate		*/
{
	sm_text_char_put_color(p, loc, tch, now, NULL_COLOR, NULL_COLOR);
	return;
}


/* Put a colored TextChar in a pane at given location (immediate).  Return its success.	*/
void
sm_text_char_put_color(Pane* p, Point loc, TextChar tch, Boolean now, Color fg, Color bg)
/* loc - Place to put the char (glyph coords)	*/
/* tch - The text character to put		*/
/* now - true if we should propagate		*/
/* fg - color of the character		*/
/* bg - color behind the character		*/
{
static	TextString	ts = {1, 0, false};	/* the text string to write		*/
Rectangle		dst;			/* destination window rectangle		*/

	if (p->parent->exists)
	{/* safe to do a fontWritePane */
#ifdef DEBUG
		if (!pointInRect(loc, makeRectFromSize(Origin, MAP_SIZE(p))))
		{/* the image could not be written */
			die_with_message("sm_text_char_put():  location out of bounds");
		}
#endif /* DEBUG */

		ts.tc_ptr = &tch;
		dst = sm_text_rect_gp(p, makeRect(loc, loc));
		fontWritePaneColor(
				   p,
				   dst.ul,
				   dst,
				   ts,
				   FONT(p),
				   fg == NULL_COLOR ? paneForeground(p) : fg,
				   bg == NULL_COLOR ? paneBackground(p) : bg
		);
		if (now)
		{/* go ahead and propagate the write */
			sm_text_pixels_touch(p, dst);
		}
	}
}


/* Put a TextString in a pane starting at a location (immediate).  Return its success.	*/
void
sm_text_string_put(Pane* p, Point start, TextString ts, Boolean now)
/* start - Place the string should start	*/
/* ts - The text string to put		*/
/* now - true if we should propagate		*/
{
	sm_text_string_put_color(p, start, ts, now, NULL_COLOR, NULL_COLOR);
	return;
}


/* Put a TextString in a pane starting at a location (immediate).  Return its success.	*/
void
sm_text_string_put_color(Pane* p, Point start, TextString ts, Boolean now, Color fg, Color bg)
/* start - Place the string should start	*/
/* ts - The text string to put		*/
/* now - true if we should propagate		*/
/* fg - color of the character		*/
/* bg - color behind the character		*/
{
Rectangle		dst;			/* destination window rectangle		*/

	if (p->parent->exists)
	{/* safe to do a fontWritePane */
#ifdef DEBUG
		if ((start.x < 0) || (start.y < 0) || (start.x + ts.num_tc >= MAP_SIZE(p).x) || (start.y >= MAP_SIZE(p).y))
		{/* the image could not be written */
			die_with_message("sm_text_string_put():  location out of bounds");
		}
#endif /* DEBUG */

		dst = sm_text_rect_gp(p, makeRectFromSize(start, makePoint(ts.num_tc, 1)));
		fontWritePaneColor(
			      p,
			      dst.ul,
			      dst,
			      ts,
			      FONT(p),
			      fg == NULL_COLOR ? paneForeground(p) : fg,
			      bg == NULL_COLOR ? paneBackground(p) : bg
		);
		if (now)
		{/* go ahead and propagate the write */
			sm_text_pixels_touch(p, dst);
		}
	}
}


/* Invert the character and touch the affected area if necessary.			*/
void
sm_text_cursor_invert(Pane* p, Point loc, Boolean on, Boolean now)
/* loc - Place to invert the character	*/
/* on - true if cursor should appear		*/
/* now - true if we should propagate		*/
{
Rectangle		dst;			/* destination window rectangle		*/

	if (p->parent->exists)
	{/* safe to do a boxPaneColor() */
#ifdef DEBUG
		if (!pointInRect(loc, makeRectFromSize(Origin, MAP_SIZE(p))))
		{/* the image could not be written */
			die_with_message("sm_text_char_invert():  location out of bounds");
		}
#endif /* DEBUG */

		dst = sm_text_rect_gp(p, makeRect(loc, loc));
		boxPaneColor(p, dst, sizeRect(dst).x, NULL_BITMAP, Origin, true,
			     on ? paneForeground(p) : paneBackground(p), NULL_COLOR);
		/* The correct solution involves remembering the most recent character
		 * and printing it again under a cursor, using fontWritePane().
		 * Currently, I am just erasing the character under the cursor since
		 * the shells seem to ignore the character anyway.  Jeffrey D.
		 * Oldham.
		 */
		if (now)
		{/* go ahead and propagate the write */
			sm_text_pixels_touch(p, dst);
		}
	}
}
