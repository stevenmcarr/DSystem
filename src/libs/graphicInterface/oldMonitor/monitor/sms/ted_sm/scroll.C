/* $Id: scroll.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/tedprivate.h>
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/damage.h>

void sm_ted_handle_scroll_horizontal(Pane *p, ScrollBar sb, short h_or_v, int loc)
	/*Pane *p;	my pane */
	/*ScrollBar sb;	the horizontal scrollbar */
	/*short h_or_v;	direction ignored, always SB_HORIZONTAL */
	/*int loc;	# of column to place at left edge of pane */
{
	PREFER(p) = loc;

	/* Does the change get added to the undo list? */
	sm_ted_damaged_dot_col(p);
	sm_ted_repair(p);
}

void sm_ted_handle_scroll_vertical(Pane *p,ScrollBar sb, short h_or_v, int newtop)
	/*Pane *p;	my pane */
	/*ScrollBar sb;	the vertical scrollbar */
	/*short h_or_v;	direction ignored, always SB_VERTICAL */
	/*int newtop;	# of line to place at top of pane */
{
	int curline;		/* # of line containing cursor */
	int topline;		/* # of line at top of pane */
	int scroll_amount;	/* # of lines to scroll the pane */

	curline = DOTLINE(p);
	topline = curline - DOT_LOC(p).y;

	scroll_amount = newtop - topline;

	if (scroll_amount == 0)
	{/* Do no work. */
		return;
	}

	if (IN_RANGE( 1 , scroll_amount , SIZE(p).y - 1) )
	{/* New top line is currently on screen */
		while (0 < scroll_amount)
		{
		    sm_ted_scroll_up(p);
		    scroll_amount--;
		}
	}
	else if (IN_RANGE( 1 , -scroll_amount , SIZE(p).y - 1) )
	{
		while (scroll_amount < 0)
		{
		    sm_ted_scroll_down(p);
		    scroll_amount++;
		}
	}
	else 
	{
		sm_ted_goto_line_rel(p,-DOT_LOC(p).y);
		sm_ted_goto_line_rel(p,scroll_amount);
		DOT_LOC(p).y = 0;
		PREFER(p) = 0;
		sm_ted_damaged_win(p);
		sm_ted_damaged_dot_col(p);
	}
	sm_ted_repair(p);
}

/*
 * Install a horizontal scrollbar
 */
void sm_ted_win_set_scroller_horizontal(Pane *p, ScrollBar hs)
{
	HORZ(p) = hs;

	/*
	Set the slow,fast scroll amounts
	*/
	sm_scroll_set_step(hs,1,PAGE_FRAC(p)*SIZE(p).x);

	/*
	Set the limits.
	*/
	sm_ted_scroll_reset(p);

	sm_scroll_scrollee(hs,(Generic)p,
           (sm_scroll_scrollee_callback)sm_ted_handle_scroll_horizontal);
	sm_scroll_activate(hs,ACTIVE(p));
}

/*
 * Install a vertical scrollbar
 */
void sm_ted_win_set_scroller_vertical(Pane *p, ScrollBar vs)
{
	/* Install the scrollbar */
	VERT(p) = vs;

	/* Set the slow,fast scroll amounts */
	sm_scroll_set_step(vs,1,(int)(PAGE_FRAC(p)*SIZE(p).y));

	sm_ted_scroll_reset(p);

	/* Give the scrollbar our handler */
	sm_scroll_scrollee(vs,(Generic)p,
             (sm_scroll_scrollee_callback)sm_ted_handle_scroll_vertical);

	/* Set the scrollbar's active status to be the same as ours */
	sm_scroll_activate(vs,ACTIVE(p));
}

void sm_ted_scroll_reset(Pane *p)
{
	if (HORZ(p) != (ScrollBar) UNUSED)
	{
		message("Ted has horizontal scrollbars already?");
	}

	if (VERT(p) != (ScrollBar) UNUSED)
	{
		int minline;
		int maxline;	/* # of lines not displayed in pane */
		int topline;	/* ord # of line at top of pane */

		minline = 0;
		maxline = max( 0, LINES(p) - (SIZE(p).y-1) );
		topline = DOTLINE(p) - DOT_LOC(p).y;

		sm_scroll_set(VERT(p), minline, maxline, topline, true );
	}
}
