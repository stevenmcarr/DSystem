/* $Id: move.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/tedprivate.h>

/*
 * Most, if not all, of the functions in this file are quite useful for and usable by CPs.
 */


/*
 * sm_ted_page_forward - page forward some sizeable fraction of the number
 *	of lines on the screen.  Many things probably need to be refigured
 *	after this.
 */
void sm_ted_page_forward(Pane *p)
{
	short numlines;
	short total;
	short nv;

	/* Calculate the number of usable screen lines */
	total = SIZE(p).y - TOPSKIP(p) - BOTSKIP(p);
	
	/* Number of lines to page forward */
	numlines = total * PAGE_FRAC(p);

	if ( sm_ted_at_eob(p) )
	{
		sm_ted_inform( p, "Bottom of buffer" );
		return;
	}
	if (total == 1)
	{
		(void) sm_ted_goto_line_rel (p, 1);
		PREFER(p) = 0;
		sm_ted_damaged_prefer (p);
	}
	else if (DOT_LOC(p).y >= numlines)
	{/* The dot will stay at the same offset */
		DOT_LOC(p).y -= numlines;
	}
	else
	{
		nv = total + (1 - PAGE_FRAC(p)) * total -1;
		if ((total > 2) & (total % 2) != 0)
			nv++;
		(void) sm_ted_goto_line_rel (p, nv - DOT_LOC(p).y);	
		if ((total % 2) != 0)
			DOT_LOC(p).y = total/2;
		else	DOT_LOC(p).y = total/2 - 1;
		PREFER(p) = 0;
		sm_ted_damaged_prefer (p);
	}
	sm_ted_damaged_win (p);
}

/*
 * sm_ted_page_back - page back some sizeable fraction of the number of
 *	lines on the screen.  Many things probably need to be refigured
 *	after this.
 */
void sm_ted_page_back(Pane *p)
{
	short numlines;
	short total;
	int   nv;
	int   actual;

	/* Calculate the number of usable screen lines */
	total = SIZE(p).y - TOPSKIP(p) - BOTSKIP(p);
	
	/* Number of lines to page back */
	numlines = total * PAGE_FRAC(p);

	if ( sm_ted_at_bob(p) )
	{
		/* can't page back */
		sm_ted_inform( p, "Top of file");
		return;
	}
	if (total == 1)
	{
		sm_ted_goto_line_rel (p, -1);
		PREFER(p) = 0;
		sm_ted_damaged_prefer (p);
	}
	else if (DOT_LOC(p).y < (total - numlines))
	{/* The dot will stay at the same offset */
		DOT_LOC(p).y += numlines;
	}
	else 
	{
		nv = - total - (1 - PAGE_FRAC(p)) * total;
		nv = nv + (total - 1 - DOT_LOC(p).y);
		if (total == 2)
			nv = -1;
		actual = sm_ted_goto_line_rel (p, nv);
		if (actual == nv)
		{
			if ((total % 2) != 0)
				DOT_LOC(p).y = total/2;
			else	DOT_LOC(p).y = total/2 - 1;
		}
		else    DOT_LOC(p).y = 0;
		PREFER(p) = 0;
		sm_ted_damaged_prefer (p);
	}
	sm_ted_damaged_win (p);
}


/*
 * sm_ted_line_to_top_of_window - shift the text up so that the current
 *	line will be the top line on the screen.
 */ 
void sm_ted_line_to_top_of_window(Pane *p)
{
	DOT_LOC(p).y = TOPSKIP(p);

	sm_ted_damaged_win(p);
}

/*
 * sm_ted_top_of_window_to_line - shift the text down so that what was
 *	the first line of text on the screen is now at DOT_LOC().y
 */
void sm_ted_top_of_window_to_line(Pane *p)
{
	if ( DOT_LOC(p).y == TOPSKIP(p) )
		return;

	(void) sm_ted_goto_line_rel(p, - DOT_LOC(p).y);
	sm_ted_damaged_win(p);
	sm_ted_damaged_dot_col(p);
}

/*
 * sm_ted_scroll_up - shift the text up one line
 */
void sm_ted_scroll_up(Pane *p)
{
	short line;
	short numlines;		/* number of lines on the screen */
	short jump;		/* number of lines to jump the cursor if it
				   is pushed off the top of the screen */

	numlines = SIZE(p).y - TOPSKIP(p) - BOTSKIP(p);
	jump = numlines / 2;

	if (DOT_LOC(p).y > TOPSKIP(p))
	{/* The cursor wasn't on the top line. */
		--(DOT_LOC(p).y);
	}
	else
	{/* Need to find a new home for the cursor, in addition to scrolling */
		line = sm_ted_goto_line_rel(p,jump);
		if (line > 0)
			DOT_LOC(p).y = line - 1;
		else
			sm_ted_inform( p, "Bottom of file");

		PREFER(p) = 0;
		sm_ted_damaged_prefer (p);
	}
	sm_ted_damaged_win(p);
}


/*
 * sm_ted_scroll_down - shift the text down one line
 */
void sm_ted_scroll_down(Pane *p)
{
	short line;
	short numlines;		/* number of lines on the screen */
	short jump;		/* number of lines to jump the cursor if it
				   is pushed off the top of the screen */

	numlines = SIZE(p).y - TOPSKIP(p) - BOTSKIP(p);
	jump = numlines / 2;

	if (DOT_LOC(p).y < SIZE(p).y - 1 - BOTSKIP(p))
	{/* quicky case, screen scrolls, but dot offset is the same */
		++(DOT_LOC(p).y);
		sm_ted_damaged_win(p);
		return;
	}

	{/* The cursor will fall off the bottom of the screen.  Reposition. */

		line = sm_ted_goto_line_rel(p,-jump);

		/* line is negative, in range [-jump..0] */
		DOT_LOC(p).y = SIZE(p).y + line;

		PREFER(p) = 0;
		sm_ted_damaged_prefer (p);
	}
	sm_ted_damaged_win(p);
}

/*
 * sm_ted_prev_line - move the dot to the previous line.  Scroll the screen if necessary.
 */
void sm_ted_prev_line(Pane *p)
{
	short numlines;		/* number of lines on the screen */

	numlines = SIZE(p).y - TOPSKIP(p) - BOTSKIP(p);

	if (NOT(sm_ted_goto_prev_line(p)))
	{
		sm_ted_inform( p, "Top of file");
		return;
	}

	--(DOT_LOC(p).y);
	
	if (DOT_LOC(p).y < 0)
	{/* Prev line is off the top of the screen. */
		DOT_LOC(p).y = numlines / 2;
		sm_ted_damaged_win(p);
	}
	sm_ted_damaged_dot_col(p);
}


/*
 * sm_ted_next_line - move the dot to the next line.  Scroll the screen if necessary.
 */
void sm_ted_next_line(Pane *p)
{
	short numlines = SIZE(p).y - BOTSKIP(p) - TOPSKIP(p);

	if (NOT(sm_ted_goto_next_line(p)))
	{
		sm_ted_inform( p, "Bottom of file");
		return;
	}

	(DOT_LOC(p).y)++;
	
	if (DOT_LOC(p).y >= SIZE(p).y - BOTSKIP(p))
	{/* The next line is off the bottom of the screen.  Scroll it. */
		DOT_LOC(p).y = numlines / 2;
		sm_ted_damaged_win(p);
	}
	sm_ted_damaged_dot_col(p);
}

/*
 * sm_ted_beginning_of_line - move the dot to the beginning of the line it is on.
 */
void sm_ted_beginning_of_line(Pane *p)
{
	sm_ted_goto_bol(p);
	sm_ted_damaged_prefer(p);
}

/*
 * sm_ted_end_of_line - move the dot to the end of the line that it is on.
 */
void sm_ted_end_of_line(Pane *p)
{
	sm_ted_goto_eol(p);
	PREFER (p) = EOL;

	sm_ted_damaged_dot_col(p);
}

/*
 * sm_ted_prev_char - back up over a character, except if we're at the beginning of file
 */
void sm_ted_prev_char(Pane *p)
{
	if ( sm_ted_at_bob(p) )
	{
		sm_ted_inform( p, "Beginning of file");
		PREFER(p) = 0;
		return;
	}

	if ( sm_ted_at_bol(p) )
	{
		sm_ted_prev_line(p);
		PREFER(p) = EOL;
		sm_ted_damaged_dot_col (p);
		return;
	}

	sm_ted_set_dot_relative(p,-1);

	sm_ted_damaged_prefer (p);
}

/*
 * sm_ted_next_char - go forward over a character, except if we're at the end of file.
 */

void sm_ted_next_char(Pane *p)
{
	if ( sm_ted_at_eob(p) )
	{
		PREFER(p) = EOL;
		sm_ted_inform( p, "End of file");
		return;
	}
	if ( sm_ted_at_eol(p) )
	{
		PREFER(p) = 0;
		sm_ted_next_line(p);
		return;
	}

	sm_ted_set_dot_relative(p,1);
	sm_ted_damaged_prefer (p);
}


void sm_ted_beginning_of_buffer(Pane *p)
{
	sm_ted_goto_bob(p);
	DOT_LOC(p) = makePoint(0,TOPSKIP(p));

	sm_ted_damaged_dot_row (p);
	sm_ted_damaged_prefer (p);
}


void sm_ted_end_of_buffer(Pane *p)
{
	short numlines = SIZE(p).y - BOTSKIP(p) - TOPSKIP(p);

	DOT_LOC(p).y = TOPSKIP(p) + numlines/2;

	sm_ted_goto_eob(p);
	PREFER(p) = EOL;
	
	sm_ted_damaged_dot_row (p);
	sm_ted_damaged_prefer(p);
}

void sm_ted_scroll(Pane *p)
{
	int amount;
	
	amount = CLICK_AT(p).y;
	if (amount < 0)
	{
		for (;amount < 0; amount++)
			sm_ted_scroll_down (p);
	}
	else
	{
		for (;amount > 0; amount--)
			sm_ted_scroll_up (p);
	}
}

void sm_ted_forward_word(Pane *p)
{
	char c;
	int line   = DOT_LOC(p).y;
	int offset = DOT(p);
	int size   = flex_size (STORE(p));
	Boolean found_word = false;

	for (;offset <= size; offset++)	
	{
		c = sm_ted_buf_get_char(p, offset);
		if (IS_ALPHA_NUM(c))
		{
			found_word = true;
			break;
		}
		if (c == '\n')
		{
			line++;
		}
	}
	for (;offset <= size; offset++)	
	{
		c = sm_ted_buf_get_char(p, offset);
		if (!(IS_ALPHA_NUM(c)))
			break;
	}
	if (found_word)
	{
		sm_ted_set_dot(p,min(size, offset));
		sm_ted_damaged_prefer(p);

		if (line >= SIZE(p).y)
			sm_ted_damaged_win (p);
		if (line != DOT_LOC(p).y)
			sm_ted_damaged_dot_row (p);
	}
	else
	{
		sm_ted_bitch(p,"No word before the end of buffer");
	}

}

void sm_ted_backward_word(Pane *p)
{
	char c;
	int line   = DOT_LOC(p).y;
	int offset = DOT(p) - 1;
	Boolean found_word = false;

	for (;offset >= 0; offset--)	
	{
		c = sm_ted_buf_get_char(p, offset);
		if (IS_ALPHA_NUM(c))
		{
			found_word = true;
			break;
		}
		if (c == '\n')
		{ /* decrements the line number */
			line--;
		}
	}
	for (;offset >= 0; offset--)	
	{
		c = sm_ted_buf_get_char(p, offset);
		if (!(IS_ALPHA_NUM(c)))
			break;
	}

	if (found_word)
	{
		sm_ted_set_dot(p, max(0, offset + 1));
		sm_ted_damaged_prefer (p);

		if (line < 0)
			sm_ted_damaged_win(p);
		if (line != DOT_LOC(p).y)
			sm_ted_damaged_dot_row(p);
	}
	else
	{
		sm_ted_bitch(p,"beginning of buffer");
	}
	
}

void sm_ted_win_goto_mark(Pane *p, int num_mark)
{
	TedMark *mark;

	if (num_mark == 0)
	{
		if (MARK(p) == NOT_SET)
		{
			sm_ted_bitch (p, "mark not set");
			return;
		}
		sm_ted_set_dot(p,MARK(p));
	}
	else
	{
		mark = (TedMark *) (Generic) flex_get_one (MARKLIST(p), num_mark);
		sm_ted_set_dot(p,mark->location);
		free_mem((void*) mark);
	}
	sm_ted_damaged_prefer(p);
	sm_ted_damaged_dot_row(p);
	sm_ted_damaged_win(p);
}
