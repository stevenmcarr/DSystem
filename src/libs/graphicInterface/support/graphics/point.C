/* $Id: point.C,v 1.1 1997/06/02 19:40:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 			 point.c			*/
		/* 							*/
		/*		 Point (pair) utilities.		*/
		/* 							*/
		/********************************************************/


#include <libs/support/misc/general.h>
#include <libs/graphicInterface/support/graphics/point.h>

Point			Origin = { 0, 0 };	/* the origin coordinate		*/
Point			UnusedPoint = { UNUSED, UNUSED };/* the unused coordinate	*/


/* Get the x coordinate of a point.							*/
int
xPoint(Point p)
{
	return p.x;
}


/* Get the y coordinate of a point.							*/
int
yPoint(Point p)
{
	return p.y;
}


/* Return a point made from its coordinates.						*/
Point
makePoint(int x, int y)
{
Point			to_make;			/* The new point			*/

	to_make.x = x;
	to_make.y = y;
	return to_make;
}


/* Return p + inc.									*/
Point
transPoint(Point p, Point inc)
/*Point			p;			the point to be adjusted		*/
/*Point			inc;			the amount to be adjusted by		*/
{
	p.x += inc.x;
	p.y += inc.y;
	return p;
}


/* Return p - diff.									*/
Point
subPoint(Point p, Point diff)
/*Point			p;			the point to be adjusted		*/
/*Point			diff;			the amount to adjust by		*/
{
	p.x -= diff.x;
	p.y -= diff.y;
	return (p);
}


/* Return true iff p1 and p2 are equal.							*/
Boolean
equalPoint(Point p1, Point p2)
{
	return (BOOL(p1.x == p2.x && p1.y == p2.y));
}
