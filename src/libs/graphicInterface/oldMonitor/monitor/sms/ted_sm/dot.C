/* $Id: dot.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/tedprivate.h>

void
sm_ted_set_dot(Pane *p, int nd)
{
	int lines;
	int od = DOT(p);

	ASSERT( IN_RANGE( 0, nd, flex_size (STORE(p)) ) );

	lines = flex1_count_occurrences(STORE(p),
			min(od,nd),
			max(od,nd),
			'\n');
	DOTLINE(p) += (nd > od) ? lines : -lines;
	DOT(p) = nd;
	
}

/*
 * sm_ted_goto_next_line - places dot at beginning of next line.  Returns false if 
 * there is no next line in the buffer.
 */
Boolean
sm_ted_goto_next_line(Pane *p)
{
	int offset;

	offset = flex1_index(STORE(p),DOT(p),'\n');

	if (offset >= 0 && offset+1 <= flex_size(STORE(p)))
	{
		DOT(p) = offset+1;
		DOTLINE(p) += 1;
		return true;
	}
	else
	{/* No next line */
		return false;
	}
}

/*
 * sm_ted_goto_prev_line - places dot at the end of the previous line.
 * Returns false if there is no previous line in the buffer.
 */
Boolean
sm_ted_goto_prev_line(Pane *p)
{
	int offset;

	offset = flex1_rindex(STORE(p), DOT(p), '\n');

	if (offset >= 0)
	{
		DOT(p) = offset;
		DOTLINE(p) -= 1;
		return true;
	}
	else
	{
		return false;
	}
}

void
sm_ted_set_dot_relative(Pane *p, int del)
{
	sm_ted_set_dot(p,DOT(p)+del);
}

/*
 *  sm_ted_goto_bol - moves dot to beginning of current line
 */
void
sm_ted_goto_bol(Pane *p)
{
	int offset;

	offset = flex1_rindex(STORE(p),DOT(p),'\n');

	if (offset < 0)
		sm_ted_set_dot(p,0);
	else
		sm_ted_set_dot(p,offset+1);
}

/*
 *  sm_ted_whereis_bol - returns the offset of the beginning of the line containing offset
 */
int
sm_ted_whereis_bol(Pane *p, int offset)
{
	offset = flex1_rindex(STORE(p),offset,'\n');

	if (offset < 0)
		return 0;
	else
		return offset + 1;
}
/*
 *  sm_ted_at_bol - returns true iff dot is at the beginning of a line
 */
int
sm_ted_at_bol(Pane *p)
{
	int offset = DOT(p) - 1;
	return (DOT(p) == 0) || (sm_ted_buf_get_char(p, offset) == '\n');
}


/*
 *  sm_ted_goto_eol - moves dot to end of current line
 */
void
sm_ted_goto_eol(Pane *p)
{
	int offset = DOT(p);

	offset = flex1_index(STORE(p),DOT(p),'\n');

	if (offset < 0)
		sm_ted_goto_eob(p);
	else
		sm_ted_set_dot(p,offset);
}
/*
 *  sm_ted_whereis_eol - returns offset of the end of the line containing offset
 */
int
sm_ted_whereis_eol(Pane *p, int offset)
{
	offset = flex1_index(STORE(p),offset,'\n');

	if (offset < 0)
	{
		return flex_size(STORE(p));
	}
	else
		return offset;
}
/*
 *  sm_ted_at_eol - returns true iff dot is at the end of a line
 */
int
sm_ted_at_eol(Pane *p)
{
	int offset = DOT(p);
	return (DOT(p) == flex_size (STORE(p))) || sm_ted_buf_get_char(p, offset) == '\n';
}

/*
 *  sm_ted_goto_bob - puts dot at beginning of the buffer.  This is very simple and is mostly here for orthogonality.
 */
void
sm_ted_goto_bob(Pane *p)
{
	sm_ted_set_dot(p,0);
}
/*
 *  sm_ted_at_bob - returns true iff dot is at the beginning of the buffer
 */
int
sm_ted_at_bob(Pane *p)
{
	return (DOT(p) == 0);
}

/*
 *  sm_ted_goto_eob - very simple, similar in purpose to goto_bob
 */
void
sm_ted_goto_eob(Pane *p)
{
	sm_ted_set_dot(p,flex_size (STORE(p)));
}
/*
 *  sm_ted_at_eob - returns true iff dot is at the end of the buffer
 */
int
sm_ted_at_eob(Pane *p)
{
	return (DOT(p) == flex_size (STORE(p)));
}

/*
 * sm_ted_whereis_next_line - returns offset of beginning of next line after offset.
 *	Returns -1 if there is no next line in the buffer.
 */
int
sm_ted_whereis_next_line(Pane *p, int offset)
{
	offset = flex1_index(STORE(p),offset,'\n');

	if (offset >= 0 && offset+1 <= flex_size(STORE(p)))
	{
		return offset+1;
	}
	else
	{/* No next line after offset */
		return -1;
	}
}


/*
 * sm_ted_whereis_prev_line - returns offset of end of the previous line.
 *	Returns -1 if there is no previous line in the buffer.
 */
int
sm_ted_whereis_prev_line(Pane *p, int offset)
{
	offset = flex1_rindex(STORE(p),offset,'\n');

	if (offset >= 0)
		return offset;
	else
	{
		return -1;
	}
}

/*
 * sm_ted_goto_line_rel - moves to the beginning of the line which is 
 *	delta lines offset from the current line
 */
int
sm_ted_goto_line_rel(Pane *p, int delta)
{
	int i = 0;

	if (delta == 0)
	{
		sm_ted_goto_bol(p);
	}
	else if (delta > 0)
	{
		while ( i < delta && sm_ted_goto_next_line(p) )
			i++;
	}
	else if (delta < 0)
	{
		while ( i < -delta && sm_ted_goto_prev_line(p) )
			i++;
		sm_ted_goto_bol(p);
		i = -i;
	}
	return i;
}


/*
 * sm_ted_whereis_line_rel - returns offset of the beginning of the line
 *	which is delta lines offset from "offset"
 */
int
sm_ted_whereis_line_rel(Pane *p, int delta, int offset)
{
	int i = 0;

	if (delta == 0)
	{
		offset = sm_ted_whereis_bol(p,offset);
	}
	else if (delta > 0)
	{
		while ( i < delta  &&  (offset = sm_ted_whereis_next_line(p,offset)) != -1 )
			i++;
	}
	else if (delta < 0)
	{
		while ( i < -delta  &&  (offset = sm_ted_whereis_prev_line(p,offset)) != -1 )
			i++;

		if (offset != -1)
			offset = sm_ted_whereis_bol(p,offset);
	}
	return offset;
}

/*
 *  sm_ted_line_number - returns the number (ordinal) of the line containing the cursor
 */
int
sm_ted_line_number(Pane *p)
{
	return DOTLINE(p) + 1;
}

/*
 *  sm_ted_lines - returns the number of the lines in the buffer
 */
int
sm_ted_lines(Pane *p)
{
	register int offset;
	register int line;

	offset = flex_length(STORE(p));
	line = 0;
	while ( offset >= 0 )
	{
		line++;
		offset = flex1_rindex(STORE(p),offset,'\n');
	}
	return line;
}


int sm_ted_tab_size(Pane *p, short col)
{
	return TAB_SIZE(p) - (col % TAB_SIZE(p));
}

/*ARGSUSED*/
int sm_ted_ctrl_size(Pane *p, short col)
{
	switch (CTRL_STYLE(p))
	{
		case VERBATIM:
			return 1;
		case OCTAL:
			return 4;
		case CARET:
			return 2;
		default:
			abort();
			return 1;	/* lint */
	}
}

/*
 *  sm_ted_width_of_text gives the width in character positions of a blob
 *	of text.  The size of the blob is fixed.  The width of the blob
 *	is returned as the result.
 */
short
sm_ted_width_of_text(Pane *p, int offset, register short coloff, register short bytes)
	/*int offset;		   start of text whose width is to be calculated */
	/*register short coloff;   initial column offset (often 0) */
	/*register short bytes;	   number of bytes of text which satisfy prefer_col */
{
	register short pos = coloff;
	int end = offset + bytes;

	while ( offset < end )
	{
		switch( char_tab[sm_ted_buf_get_char(p, offset++)])
		{
		case TAB:
			pos += sm_ted_tab_size(p,pos);
			break;
		case CTRL:
		case META:
			pos += sm_ted_ctrl_size(p,pos);
			break;
		default:
			pos++;
		}
	}
	return pos;
}

/*
 *  sm_ted_width_of_text_preferred gives the width in character positions of a blob of text.  The size of the blob
 *  is adjusted in an attempt to satisfy prefer_col.  This routine assumes that there are no newlines in the
 *  blob of text.
 */
short
sm_ted_width_of_text_preferred(Pane *p, int offset, int maxbytes, short coloff, 
                               int prefer_col, int *bytes)
	/*int offset;		start of text whose width is to be calculated */
	/*int maxbytes;		maximum amount of text to be scanned */
	/*short coloff;		initial column offset (often 0) */
	/*int prefer_col;	prefered width of text */
	/*int *bytes;		number of bytes of text which satisfy prefer_col */
{
	short pos = coloff;
	short oldpos = coloff;
	int end = offset + maxbytes;

	/*
	 * While we haven't looked at more than maxbytes characters, and while the column #
	 *	is still less than the preferred column, look at one more character and
	 *	increase the column # by the width of the character.
	 */
	while ( offset < end && pos < prefer_col )
	{
		oldpos = pos;
		(*bytes)++;
		switch( char_tab[sm_ted_buf_get_char(p, offset++)])
		{
		case TAB:
			pos += sm_ted_tab_size(p,pos);
			break;
		case CTRL:
		case META:
			pos += sm_ted_ctrl_size(p,pos);
			break;
		default:
			pos++;
		}
	}

	/*
	 * If we had enough characters to exceed prefer_col...
	 */
	if ( pos > prefer_col )
	{/* Back up one character, and decrease the column # by the width of that char	. */
		pos = oldpos;
		offset--;
		(*bytes)--;
	}

	return pos;
}


/*
 *  find_dot_using_prefer sets the x position of the cursor of the screen, and sets the position of the dot
 *  on the current line based on the values of cw->prefer_col and cw->size.x.  Tabs and possibly control
 *  characters must be accounted for in width calculations.
 */
void
sm_ted_find_dot_using_prefer(Pane *p)
{
	int bol = sm_ted_whereis_bol(p,DOT(p));
	int eol = sm_ted_whereis_eol(p,DOT(p));
	int nb = 0;
	int width;

	width = sm_ted_width_of_text_preferred( p, bol, (short)(eol-bol), 0, PREFER(p), &nb );

	sm_ted_set_dot(p,bol+nb);		/* number of bytes in text */
	DOT_LOC(p).x = 
		MAX(0,MIN(width,SIZE(p).x-1));	/* width of text ( width <= prefer_col ) */ 
}

void
sm_ted_find_prefer_using_doffset(Pane *p)
{
	int bol = sm_ted_whereis_bol (p,DOT(p));
	int eol = sm_ted_whereis_eol (p, DOT(p));
	int width;

	width = sm_ted_width_of_text( p, bol, 0, DOT(p) - bol );
	if ((DOT(p) == eol) & (eol != bol))
		PREFER(p) = EOL;
	else
	{
		PREFER(p) = width;		/* number of bytes in text */
	}
	DOT_LOC(p).x = 
		MAX(0,MIN(width,SIZE(p).x-1));	/* width of text (width <= prefer_col ) */
}

void
sm_ted_goto_mark (Pane *p)
{
	sm_ted_win_goto_mark (p, 0);
}

void
sm_ted_set_mark (Pane *p)
{
	sm_ted_buf_set_mark (p, 0);
}

int sm_ted_whereis_beginning_of_word (Pane *p, int offset)
{
	char c;
	Boolean found_word = false;

	for (;offset >= 0 ; offset--)	
	{
		c = sm_ted_buf_get_char(p, offset);
		if (IS_ALPHA_NUM(c))
		{
			found_word = true;
			break;
		}
	}
	for (;offset >= 0; offset--)	
	{
		c = sm_ted_buf_get_char(p, offset);
		if (!(IS_ALPHA_NUM(c)))
		{
			break;
		}
	}
	if (found_word)
	{
		return ++offset;
	}
	else
	{
		return -1;
	}
}

int sm_ted_whereis_end_of_word (Pane *p)
{
	char c;
	int offset = DOT(p);
	int eob = flex_size (STORE(p));
	Boolean found_word = false;

	for (;offset <= eob  ; offset++)	
	{
		c = sm_ted_buf_get_char(p, offset);
		if (IS_ALPHA_NUM(c))
		{
			found_word = true;
			break;
		}
	}
	for (;offset <= eob; offset++)	
	{
		c = sm_ted_buf_get_char(p, offset);
		if (!(IS_ALPHA_NUM(c)))
		{
			break;
		}
	}
	if (found_word)
	{
		return --offset;
	}
	else
	{
		return -1;
	}
}

