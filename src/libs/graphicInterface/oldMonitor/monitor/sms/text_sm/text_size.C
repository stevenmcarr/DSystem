/* $Id: text_size.C,v 1.1 1997/06/25 15:00:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		       text_size.c			*/
		/*	        Font related text sm calls.		*/
		/* 							*/
		/********************************************************/

#include <libs/graphicInterface/oldMonitor/monitor/sms/text_sm/text.i>


/* Return the size of the character grid for a text pane.				*/
Point
sm_text_size(Pane* p)
{
	return MAP_SIZE(p);
}


/* Return the height of the character grid for a text pane.				*/
short
sm_text_height(Pane* p)
{
	return MAP_SIZE(p).y;
}


/* Returns the width of the character grid for a text pane.				*/
short
sm_text_width(Pane* p)
{
	return MAP_SIZE(p).x;
}


/* Return the next character position after this positon.				*/
Point
sm_text_char_pos_next(Pane* p, Point pos)
   /* pos - the current position			*/
{
	if (++pos.x >= MAP_SIZE(p).x)
	{/* there is no more room in the x direction */
		pos.x = 0;
		if (++pos.y >= MAP_SIZE(p).y)
		{/* there is no more room in the y direction */
			pos.y = 0;
		}
	}
	return pos;
}


/* Return the previous character position after this position.				*/
Point
sm_text_char_pos_prev(Pane* p, Point pos)
   /* pos - the current position			*/
{
	if (--pos.x < 0)
	{/* there is no more room left in the x direction */
		pos.x = MAP_SIZE(p).x - 1;
		if (--pos.y < 0)
		{/* there is no more room in the y direction */
			pos.y = MAP_SIZE(p).y - 1;
		}
	}
	return pos;
}


/* Convert a pixel coordinate to a character coordinate (optionally clip within map).	*/
Point
sm_text_point_pg(Pane* p, Point loc, Boolean clip)
   /* loc - the pane relative pixel coordinate	*/
   /* clip - clip based on the character map	*/
{
	loc.x = (loc.x - MAP_ORIGIN(p).x) / GLYPH_SIZE(p).x - (loc.x < MAP_ORIGIN(p).x);
	loc.y = (loc.y - MAP_ORIGIN(p).y) / GLYPH_SIZE(p).y - (loc.y < MAP_ORIGIN(p).y);
	if (clip)
	{/* clip the coordinate to be in the bitmap */
		loc.x = max(0, min(MAP_SIZE(p).x - 1, loc.x));
		loc.y = max(0, min(MAP_SIZE(p).y - 1, loc.y));
	}
	return loc;
}


/* Convert a character rectangle the coresponding pixel rectangle.			*/
Rectangle
sm_text_rect_gp(Pane* p, Rectangle r)
   /* r - the text rectangle			*/
{
  register short mox, moy, gsx, gsy;

	mox = MAP_ORIGIN(p).x;
	moy = MAP_ORIGIN(p).y;
	gsx = GLYPH_SIZE(p).x;
	gsy = GLYPH_SIZE(p).y;

	r.ul.x = mox + gsx * r.ul.x;
	r.ul.y = moy + gsy * r.ul.y;
	r.lr.x = mox + gsx * (r.lr.x + 1) - 1;
	r.lr.y = moy + gsy * (r.lr.y + 1) - 1;

	return r;
}
