/* $Id: display.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/tedprivate.h>
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/damage.h>


/*
 * sm_ted_put_char_simple - Writes ch to the array of TextChars that will be handed to the optim layer.
 *	If the col is on screen, write the character.  If col+1 (the next character position) is on the
 *	screen, then return true, else false.
 */
static Boolean sm_ted_put_char_simple(register Pane *p, register short *col, 
                                      register TextChar tch, register short maxcols)
{
	register TextChar *tcp = OPTIM_LINE(p);

	if (*col < maxcols)
	{/* Write ch, increment the column index, and tell them to send more.  */
		tcp[(*col)++] = tch;
		return true;
	}
	else if (*col == maxcols)
	{/* Write the ch, increment the column, but tell them not to send any more chars. */
		tcp[(*col)++] = tch;
		return false;
	}
	else
	{/* Clipped, tell them not to send any more chars. */
		message("TED SM internal error 1");
		return false;
	}
}


static Boolean sm_ted_put_char_octal(register Pane *p, register short *col, 
                                     TextChar tch, register short maxcols)
{
	unsigned char c1 = (char)('0' + (tch.ch >> 6));		/* bits 7-6 of ch (high two bits) */
	unsigned char c2 = (char)('0' + ((tch.ch >> 3) & 07) );	/* bits 5-3 of ch */
	unsigned char c3 = (char)('0' + (tch.ch & 07));		/* bits 2-0 of ch (low three bits) */

	if (! sm_ted_put_char_simple(p, col, makeTextChar('\\',tch.style), maxcols ) )
		return false;
	if (! sm_ted_put_char_simple(p, col, makeTextChar(c1,tch.style), maxcols ) )
		return false;
	if (! sm_ted_put_char_simple(p, col, makeTextChar(c2,tch.style), maxcols ) )
		return false;

	return sm_ted_put_char_simple(p, col, makeTextChar(c3,tch.style), maxcols );
}

static Boolean sm_ted_put_char_caret(register Pane *p, register short *col, 
                                     TextChar tch, register short maxcols)
{
	if (! sm_ted_put_char_simple(p, col, makeTextChar('^',tch.style), maxcols ) )
		return false;

	/* yes, this is too simple, but it's okay for now */
	return sm_ted_put_char_simple(p, col, makeTextChar(tch.ch+'@',tch.style), maxcols );
}

static Boolean sm_ted_put_tab(register Pane *p, register short *col, TextChar tch, 
                              register short maxcols)
{
	register short i;
	register short move = sm_ted_tab_size(p,*col);

	for (i = 0;i < move; i++)
		if (! sm_ted_put_char_simple(p,col, tch, maxcols) )
			return false;
	return true;
}

static Boolean sm_ted_put_char_ctrl(register Pane *p, short *col, TextChar tch,
                                    short maxcols)
{
	switch (CTRL_STYLE(p))
	{
	case OCTAL:
		return sm_ted_put_char_octal( p, col, tch, maxcols );
	case CARET:
		return sm_ted_put_char_caret( p, col, tch, maxcols );
	case VERBATIM:
		return sm_ted_put_char_simple(p, col, tch, maxcols );
	default:
		abort();
	}
	return false;
}

static Boolean sm_ted_put_char(register Pane *p, short *col, TextChar tch, 
                               short maxcols)
{
	register char 		ch;
	register TextChar 	*tcp = OPTIM_LINE(p);

	switch( char_tab[tch.ch] )
	{
	case TAB:
		return sm_ted_put_tab(p,col,makeTextChar(' ',tch.style),maxcols);
	case LF:
		message("TED_SM internal error 2");
		return false;

	case CTRL:	case META:	case MTAB:	case MLF:
	case MSPACE:	case MCTRL:	case MDIGIT:	case MPUNC:
	case MUPPER:	case MLOWER:
		return sm_ted_put_char_ctrl(p,col,tch,maxcols);
	default:
		return sm_ted_put_char_simple(p,col,tch,maxcols);
	}
}

static Boolean sm_ted_put_str(Pane *p, register int start, register int end,
                              register short lineno, register short maxcol)
	/*register short maxcol; The maximum column to write characters (SIZE(p).x-1) */
{
	short col = 0;			/* The column on which we are currently drawing */
	TextChar tch;			/* The character we are currently drawing */
	Boolean on_screen = true;	/* True iff the next character will be on the screen at all */
	int i;

	register char 		ch;
	register TextChar 	*tcp = OPTIM_LINE(p);


	tch.style = STYLE(p);

	/*
	 * While we haven't written the whole string, and we are still on the screen.
	 */
	while (start < end && on_screen)
	{/* "Write" a character to the screen. */
		tch.ch = sm_ted_buf_get_char(p, start);
		start++;

		switch( char_tab[tch.ch] )
		{
		case TAB:
			on_screen = sm_ted_put_tab(p,&col,makeTextChar(' ',tch.style),maxcol);
			break;
		case LF:
			message("TED_SM internal error 2");
			on_screen = false;
			break;
		case CTRL:	case META:	case MTAB:	case MLF:
		case MSPACE:	case MCTRL:	case MDIGIT:	case MPUNC:
		case MUPPER:	case MLOWER:
			on_screen = sm_ted_put_char_ctrl(p,&col,tch,maxcol);
			break;
		default:
			if (col < maxcol)
				on_screen = true;
			else if (col == maxcol)
				on_screen = false;
			else 
			{
				message("TED_SM internal error 1");
				on_screen = false;
			}
			tcp[col++] = tch;
			break;
		}
	}

	if ( NOT(on_screen) )
	{/* We clipped at the right edge.  Write a '+' in the last column, so they'll know there's more. */
		tcp[maxcol] = makeTextChar('+',STYLE(p)^ATTR_INVERSE);
	}

#ifdef DEBUG
	/*
	 * Make sure everything is as we think it should be
	 * before we hand the line off to the optim layer.
	 */
	for (i = 0;i<=maxcol;i++)
	{
		char ch    = OPTIM_LINE(p)[i].ch;

		ASSERT(!IS_CONTROL(ch));
		ASSERT(!IS_META(ch));

		ASSERT(0 <= lineno);
		ASSERT(lineno < SIZE(p).y);
	}
#endif /* DEBUG */

	OPTIM_LINE(p) = sm_optim_putline( OP(p), lineno, OPTIM_LINE(p) );
	return on_screen;
}

/*
 *  sm_ted_rewrite_line rewrites only the current line.
 */
void
sm_ted_rewrite_line(register Pane *p, Point loc)
{
	register int bol = sm_ted_whereis_line_rel( p, loc.y-DOT_LOC(p).y, DOT(p) );
	register int eol;
	short maxcols = sm_optim_width(OP(p)) - 1;

	/* if there weren't that many lines, just use BOF */
	if (bol == -1)
	{
		bol = 0;
		DOT_LOC(p).y = DOTLINE(p);
	}
	eol = sm_ted_whereis_eol(p,bol);

	(void) sm_ted_put_str(p,bol,eol,loc.y,maxcols);
}


/*
 *  sm_ted_rewrite_line_to_bot - redraws the current line,
 *	and everything below it.
 */
void
sm_ted_rewrite_line_to_bot(register Pane *p, short lineno)
{
	register int bol;
	register int eol;
	short maxcols =  sm_optim_width(OP(p)) - 1;

	/*  This code is definitely for nonwrapping display. */

	bol = sm_ted_whereis_line_rel( p, lineno-DOT_LOC(p).y, DOT(p) );

	/* if there weren't that many lines, just use BOF */
	if (bol == -1)
	{
		bol = 0;
		DOT_LOC(p).y = DOTLINE(p);
	}

	eol = sm_ted_whereis_eol(p,bol);

	while ( lineno < sm_optim_height(OP(p)) - BOTSKIP(p))
	{
		(void) sm_ted_put_str( p, bol, eol, lineno, maxcols );

		bol = sm_ted_whereis_next_line(p,eol);
		lineno++;
		if (bol == -1)
			break;
		eol = sm_ted_whereis_eol(p,bol);
	}

	LASTLINE(p) = lineno - 1;
	
	while ( lineno < sm_optim_height(OP(p)) - BOTSKIP(p))
	{
		sm_optim_clear_eol( OP(p), makePoint(0,lineno) );
		lineno++;
	}
}


/*
 *  sm_ted_screenline_of - returns the screenline that a buffer offset will
 * 		lie on when the screen is displayed.  This call may only be called
 * 		after sm_ted_dot_on_screen(), since only then are things like DOT()
 * 		and DOT_LOC() consistent.  If the offset is not on the screen, -1 is
 * 		returned.
 */
int sm_ted_screenline_of(Pane *p, register int off)
{
	int line;
	int maxlines = SIZE(p).y;
	register int dot = DOT(p);	/* the dot of something we know
					 * is on the screen.
					 */
	line = DOT_LOC(p).y;
	if (off < dot)	
	{
		off = flex1_index(STORE(p),off,'\n');
		while (off < dot && off >= 0)
		{
			line--;
			if (line < 0)
				return -1;
			off = flex1_index(STORE(p),++off,'\n');
		}
	}
	else if (off > dot)
	{
		dot = flex1_index(STORE(p),dot,'\n');
		while (off > dot)
		{
			line++;
			if (line > maxlines -1)
					return -1;
			dot = flex1_index(STORE(p), ++dot,'\n');
		}
	}
	else if (! IN_RANGE(0,line,maxlines-1) )
	{
		line = -1;
	}

	return line;
}


/*
 * sm_ted_dot_on_screen - This function is useful for repairing damage to
 * 	the vertical screen location of the dot.  It ensures that the dot
 * 	(the buffer offset) is on the screen.  If the dot was not
 * 	(vertically) on the screen, this function adds appropriate damage to
 * 	the damage list, and returns false.  If the dot was on the screen,
 * 	then the dot location is updated.
 */
Boolean
sm_ted_dot_on_screen(Pane *p)
{
	int lineno;
	int lines;
	int row;
	int maxlineno = SIZE(p).y - 1;
	int save_dot;
	Point save_dot_loc;

	save_dot = DOT(p);
	save_dot_loc	= DOT_LOC(p);

	/*
	 * Here we set things up to fool sm_ted_screenline_of() into using the
	 * old dot and dot location to find the new dot location.
 	 */
	DOT(p) = OLD_DOT(p);
	DOT_LOC(p) = OLD_DOT_LOC(p);

	lineno = sm_ted_screenline_of(p,save_dot);


	if (lineno == -1)
	{/* The new dot is off the old screen.  Center it and redisplay. */
		lines = maxlineno - TOPSKIP(p) - BOTSKIP(p);	/* # of useable lines */
		row = (lines >> 1) + TOPSKIP(p);
		DOT_LOC(p).y = row;
		DOT_LOC(p).x = save_dot_loc.x;
		DOT(p) = save_dot;

		sm_ted_damaged_win(p);
		return false;
	}

	DOT_LOC(p).y = lineno;
	DOT_LOC(p).x = save_dot_loc.x;
	DOT(p) = save_dot;
	return true;
}



/*
 * sm_ted_same_line - returns true iff off1 and off2 are on same line
 */
Boolean sm_ted_same_line(Pane *p, int off1, int off2)
{
	if (off1 > off2)
	{
		off2 = flex1_index(STORE(p),off2,'\n');
		if (off2 > off1 || off2 < 0)
			return false;
	}
	else
	{
		off1 = flex1_index(STORE(p),off1,'\n');
		if (off1 > off2 || off1 < 0)
			return false;
	}
	return true;
}





static void
sm_ted_damage_add(Pane *p,DamageType type, int loc)
{
	UtilList *damagelist = DAMAGE(p);
	UtilNode *node;
	WinChange *temp;

	temp = (WinChange *)get_mem(sizeof(WinChange), "sm_ted_damage_add");
	temp->change	= type;
	temp->loc	= loc;

	/* Allocate a node, make it point to temp, and push onto the damage list */
	node = util_node_alloc( (Generic)temp, "sm_ted_damage_add");
	util_append( damagelist, node);
}

WinChange *Damage_of(UtilNode *node)
{
	return (WinChange *)util_node_atom(node);
}

static void
sm_ted_damage_free(Pane *p)
{
	util_free_nodes_and_atoms(DAMAGE(p));
}

void sm_ted_damaged_buffer (Pane *p)
{
	TedWin *save_win = WIN(p);	
	UtilNode *wnode;
	
	wnode = util_head (WIN_LIST(p));
	while (wnode != NULL)
	{
		WIN(p) = (TedWin *) util_node_atom (wnode);
		if (save_win != WIN(p))
		{
			sm_ted_damage_add (p, damaged_to_bottom, 0);
			
			/*up dates the dot in other windows, not always necessary */
			sm_ted_damage_add (p, damaged_prefer, 0);
		}
		wnode = util_next (wnode);
	}
	WIN(p) = save_win;
}

void sm_ted_damaged_win(Pane *p)
{
	sm_ted_damage_add(p,damaged_to_bottom,0);
}

void sm_ted_damaged_win_to_end(Pane *p, int loc)
{
	sm_ted_damage_add(p,damaged_to_bottom,loc);
}

void sm_ted_damaged_line_to_end(Pane *p, int loc)
{
	sm_ted_damage_add(p,damaged_to_end,loc);
}

void sm_ted_damaged_dot_row(Pane *p)
{
	sm_ted_damage_add(p,damaged_dot_row,0);
}

void sm_ted_damaged_dot_col(Pane *p)
{
	sm_ted_damage_add(p,damaged_dot_col,0);
}
void sm_ted_damaged_prefer (Pane *p)
{
	sm_ted_damage_add (p, damaged_prefer, 0);
}

/* remove the old cursor */
void sm_ted_remove_cursor(Pane *p)
{
	TextChar tch;

	tch = sm_optim_getchar (OP(p), OLD_DOT_LOC(p), SM_OPTIM_DESIRED );
	tch.style &= ~ATTR_CURSOR;
	sm_optim_putchar( OP(p), OLD_DOT_LOC(p), tch );
}

/* place the new cursor */
void sm_ted_place_cursor(Pane *p)
{
	TextChar tch;

	if (ACTIVE(p))
	{
		tch = sm_optim_getchar (OP(p), DOT_LOC(p), SM_OPTIM_DESIRED);
		tch.style |= ATTR_CURSOR;
		sm_optim_putchar (OP(p), DOT_LOC(p), tch);
	}
}

/*
 * sm_ted_repair - fix all changes of the display since last call to this routine.
 */
void sm_ted_repair(Pane *p)
{
    UtilList	*damagelist;			/* a list of damage to a pane since last repair */
    UtilNode	*node;				/* a node on damagelist */
    UtilNode	*wnode;				/* the window where there is damage */
    TedWin	*save_win;			/* the current window */
    WinChange	*damage;			/* the damage associated with node */
    int		win_damage;			/* offset of where large scale damage begins */
    int		line_damage;			/* offset of where small scale damage begins */
    Boolean	win_damaged 	= false;	/* true iff large scale damage has occurred */
    Boolean	line_damaged 	= false;	/* true iff a line has been damaged */
    Boolean	dot_col_damaged = false;	/* true iff the column of the dot needs fixing */
    Boolean	dot_row_damaged = false;	/* true iff the row of the dot needs fixing */
    Boolean	prefer_damaged 	= false;	/* true iff the dot prefer col needs fixing */
    short	lineno;				/* a screen line number */
    
    /*
    Keep lint quiet
    */
    win_damage = flex_size (STORE(p));
    line_damage = flex_size (STORE(p));

    /*
    Look first for damage to the screen location of the dot.
    */
    save_win = WIN(p);
    wnode = util_head (WIN_LIST(p));
    while (wnode != NULL)
    {
	WIN(p) = (TedWin *)util_node_atom(wnode);

	
	if (SIZE(p).y != 0 && SIZE(p).x != 0) ; 
	{/* only do something if the screen is large enough to write on */
	    damagelist = DAMAGE(p);
	    
	    for (node = util_head(damagelist); node != NULLNODE; node = util_next(node) )
	    {
		damage = Damage_of(node);
		switch (damage->change)
		{
		    case damaged_to_end:
			/* damaged from damage.loc to end of line */
			if (win_damaged)
			{/* window already damaged, convert this line damage to window damage */
			    win_damage = min(damage->loc,win_damage);
			}
			else
			{
			    if (line_damaged)
			    {/* We've already seen some line damage. */
				if (sm_ted_same_line(p,damage->loc,line_damage))
				{/* Find the first damage on this line */
				    line_damage = min(line_damage,damage->loc);
				}
				else
				{/* Line damage on two distinct lines!  Convert to window damage. */
				    win_damaged = true;
				    damage->loc = min(damage->loc,line_damage);
				    win_damage = min(damage->loc,win_damage);
				}
			    }
			    else
			    {/* first (and probably only) line damage seen */
				line_damaged= true;
				line_damage = damage->loc;
			    }
			}
			break;
	
		    case damaged_to_bottom:
		    /* damaged from damage->loc to bottom of pane */
			if (win_damaged)
			{
			    win_damage = min(damage->loc,win_damage);
			}
			else
			{/* This is the first window damage we've seen */
			    win_damaged = true;
			    if (line_damaged)
			    {/* convert the line damage into window damage */
				damage->loc = min(damage->loc,line_damage);
			    }
			    win_damage = damage->loc;
			}
			break;
	
		    case damaged_dot_col:
			if (!dot_col_damaged)
			{
			    sm_ted_find_dot_using_prefer(p);
			    dot_col_damaged = true;
			}
			break;
		    case damaged_dot_row:
			if (!dot_row_damaged)
			{
			    /* This may add more window damage, if the new dot is off-screen. */
			    (void) sm_ted_dot_on_screen(p);
			    dot_row_damaged = true;
			}
			break;
		    case damaged_prefer:
			if (!prefer_damaged)
			{
			    sm_ted_find_prefer_using_doffset (p);
			    prefer_damaged = true;
			}
			break;		
		    default:abort();
		}
	    }
	    
	    /* repair the damage that we found */
	    
	    if (win_damaged)
	    {/* redraw large portion of window */
		sm_ted_rewrite_line_to_bot(p,0);
		sm_ted_scroll_reset(p);
	
		/* place the new cursor */
		sm_ted_place_cursor(p);
	
		/* propagate changes to the display */
		sm_optim_touch(OP(p));

		win_damaged = false;
	    }
	    else if (line_damaged)
	    {/* repair damage on one line, check cursor movement */
	    
		lineno = sm_ted_screenline_of(p,line_damage);
	
		if (lineno >= 0)
		    sm_ted_rewrite_line( p, makePoint(damage->loc,lineno) );
	
		sm_ted_scroll_reset(p);

		sm_ted_remove_cursor(p);
		sm_ted_place_cursor(p);
	

		/* propagate changes to the display */
	
		if (lineno >= 0)
		    sm_optim_touch_line( OP(p), lineno );
	
		/*
		If old cursor isn't on the same line as the damaged line, we'll
		have to explicitly touch the line containing the old cursor.
		*/
		if ( OLD_DOT_LOC(p).y != lineno )
		    sm_optim_touch_line( OP(p), OLD_DOT_LOC(p).y );
	
		if ( DOT_LOC(p).y != lineno && DOT_LOC(p).y != OLD_DOT_LOC(p).y )
		{/* New cursor not on the damaged line or old cursor line */
		    sm_optim_touch_line( OP(p), DOT_LOC(p).y );
		}

		line_damaged = false;
	    }
	    else
	    {/* check cursor movement only */
		if (NOT(equalPoint( OLD_DOT_LOC(p), DOT_LOC(p) )))
		{
		    sm_ted_remove_cursor(p);
		    sm_ted_place_cursor(p);
		    if (OLD_DOT_LOC(p).y != DOT_LOC(p).y)
		    {
			sm_optim_touch_line( OP(p), OLD_DOT_LOC(p).y );
		    }
		    sm_optim_touch_line( OP(p), DOT_LOC(p).y );
		}
	    }
	}
	OLD_DOT_LOC(p) = DOT_LOC(p);
	OLD_DOT(p) = DOT(p);
	
	dot_col_damaged = false;
	dot_row_damaged = false;
	prefer_damaged  = false;

	/* flush damage list */
	util_free_nodes_and_atoms(DAMAGE(p)); /* was "sm_ted_damage_free(p);" */
	wnode = util_next (wnode);	
    }
    WIN(p) = save_win;	/* restore the window */
}

/* true if p1 is above or to the left of p2 */

#define compPoint(p1,p2) (BOOL ((p1.y == p2.y) ? (p1.x < p2.x) : (p1.y < p2.y)))

/*ARGSUSED*/
static void sm_ted_figure_region (Pane *op, Pane *p, Point last, Point current, 
                                  RectList *New, RectList *fix)
	/*Pane *op;	 the optim pane */
	/*Pane *p;	 my pane */
	/*Point last;	 the last end point of the region */
	/*Point current; the current end point of the region */
	/*RectList *New; the increment of the region to be inverted */
	/*RectList *fix; the decrement of the region to be inverted */
{
	int	low;	/* low line on the screen */
	int	high;	/* high line on the screen*/
	int	y;	/* line we are looking at */
	int	xmin;	/* change in the row min  */
	int	xmax;	/* change in the row max  */
	int	length; /* length of the line     */
	int 	bol;	/* beginning offset of the line */
	int 	eol;	/* end of line */
	RectList *list; /* where the damage should go */
	
	if (equalPoint (current, last))
		return;

	low = MIN(current.y, last.y);
	high = MAX(current.y, last.y);
	
	for (y = low; y <= high; y++)
	{
		if (y > LASTLINE(p))
			break;
	
		bol = sm_ted_whereis_line_rel (p, y - DOT_LOC(p).y, DOT(p));
		eol = sm_ted_whereis_eol (p, bol);
		length = sm_ted_width_of_text (p, bol, 0, eol - bol) - 1;

		if (y == low)
		{
			if (current.y == last.y)
			{
				if (current.x > last.x)
				{
					xmin = last.x;
					xmax = MIN(length, current.x - 1);
					list = (compPoint(current, DOT_LOC(p))) ? fix : New;
				}
				else
				{
					xmin = current.x;
					xmax = MIN(length, last.x - 1);
					list = (compPoint(DOT_LOC(p), current)) ? fix : New;
					
				}
			}
			else if (current.y == y)
			{ /* get the right part of the line */
				xmin = current.x;
				xmax = length;
				list = (compPoint (current, DOT_LOC(p))) ? New : fix;
			}
			else 
			{/* get the remaining part of the line */
			 /* last.y is on this line */
				xmin = last.x;
				xmax = length;
				list = (compPoint (current, DOT_LOC(p))) ? fix : New;
			}
			pushRectList( makeRect(
					makePoint (xmin, y),
					makePoint (xmax, y) ),
				      list
			);
		}
		else if (y == high)
		{	
			if (current.y == y)
			{ /* fill in from left to right */
				xmin = 0;
				xmax = MIN(length, current.x - 1);
				list = (compPoint (current, DOT_LOC(p))) ? fix : New;
			}
			else
			{/* last.y is on this line */
				xmin = 0;
				xmax = MIN(length, last.x - 1);
				list = (compPoint (current, DOT_LOC(p))) ? New : fix;
			}
			pushRectList( makeRect(
					makePoint (xmin, y),
					makePoint (xmax, y) ),
				      list
			);
		}
		else
		{ /* get the whole line */
			pushRectList( makeRect(
					makePoint (0, y),
					makePoint (length, y) ),
				      ((y > DOT_LOC(p).y) == (current.y > last.y)) ? New : fix
			);
		}
	}
}

void
sm_ted_dot_to_click(Pane *p)
{
	Point result;

	(void) sm_ted_win_set_xy(p,CLICK_AT(p));
	sm_ted_repair (p);

	result = DOT_LOC(p);

	if (equalPoint (DOT_LOC(p), result))
	{
		sm_ted_damaged_dot_col(p);
		return;
	}
	else
	{
		sm_ted_buf_set_mark (p, 0);
		DOT_LOC(p).y += sm_ted_goto_line_rel (p, result.y - DOT_LOC(p).y);
		
		PREFER(p) = result.x;
		sm_ted_damaged_dot_col(p);
	}
}

