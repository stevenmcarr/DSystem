/* $Id: text_touch.C,v 1.1 1997/06/25 15:00:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		       text_touch.c			*/
		/*	     Propagation related text sm calls.		*/
		/* 							*/
		/********************************************************/

#include <libs/graphicInterface/oldMonitor/monitor/sms/text_sm/text.i>


/* Begin batch touch mode.								*/
void
sm_text_start_batch_touch(Pane* p)
{
	BATCH_TOUCH(p)++;
}


/* End batch touch mode and make the appropriate touch.					*/
void
sm_text_end_batch_touch(Pane* p)
{
	BATCH_TOUCH(p)--;
	if (BATCH_TOUCH(p) == 0)
	{/* we are exiting batch mode for the last time */
		touchPaneRectList(p, BATCH_RL(p));
		initializeRectList(&BATCH_RL(p));
	}
}


/* Propagate changes in a pane.								*/
void
sm_text_pane_touch(Pane* p)
{
	if (BATCH_TOUCH(p))
	{/* store away the rectangle */
		pushRectList(makeRectFromSize(Origin, p->size), &BATCH_RL(p));
	}
	else
	{/* make the touch now */
		touchPane(p);
	}
}


/* Propagate changes in a half pane (from a line downward).				*/
void
sm_text_half_pane_touch(Pane* p, short l)
   /* l - the first line in the region		*/
{
Rectangle		r;			/* the rectangle of the touch		*/

	r = sm_text_rect_gp(p, makeRect(makePoint(0, l), makePoint(MAP_SIZE(p).x - 1, MAP_SIZE(p).y - 1)));
	if (BATCH_TOUCH(p))
	{/* store away the rectangle */
		pushRectList(r, &BATCH_RL(p));
	}
	else
	{/* make the touch now */
		touchPaneRect(p, r);
	}
}


/* Propagate changes in a line group.							*/
void
sm_text_lines_touch(Pane* p, short start, short end)
   /* start - the first line			*/
   /* end - the last line			*/
{
Rectangle		r;			/* the rectangle of the touch		*/

	r = sm_text_rect_gp(p, makeRect(makePoint(0, start), makePoint(MAP_SIZE(p).x - 1, end)));
	if (BATCH_TOUCH(p))
	{/* store away the rectangle */
		pushRectList(r, &BATCH_RL(p));
	}
	else
	{/* make the touch now */
		touchPaneRect(p, r);
	}
}


/* Propagate changes in a character block.						*/
void
sm_text_block_touch(Pane* p, Rectangle src)
   /* src - the affected glyph rectangle		*/
{
	src = sm_text_rect_gp(p, src);
	if (BATCH_TOUCH(p))
	{/* store away the rectangle */
		pushRectList(src, &BATCH_RL(p));
	}
	else
	{/* make the touch now */
		touchPaneRect(p, src);
	}
}


/* Propagate changes in a character position.						*/
void
sm_text_char_touch(Pane* p, Point loc)
   /* loc - the affected glyph coordinate	*/
{
Rectangle		r;			/* the rectangle of the touch		*/

	r = sm_text_rect_gp(p, makeRect(loc, loc));
	if (BATCH_TOUCH(p))
	{/* store away the rectangle */
		pushRectList(r, &BATCH_RL(p));
	}
	else
	{/* make the touch now */
		touchPaneRect(p, r);
	}
}


/* Propagate changes in a pixel region.							*/
void
sm_text_pixels_touch(Pane* p, Rectangle src)
   /* src - the affected pane relative area	*/
{
	if (BATCH_TOUCH(p))
	{/* store away the rectangle */
		pushRectList(src, &BATCH_RL(p));
	}
	else
	{/* make the touch now */
		touchPaneRect(p, src);
	}
}
