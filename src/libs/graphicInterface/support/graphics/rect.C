/* $Id: rect.C,v 1.1 1997/06/02 19:40:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 			   rect.c			*/
		/* 		    Rectangle information.		*/
		/* 							*/
		/********************************************************/

#include <libs/support/misc/general.h>
#include <limits.h>
#include <libs/graphicInterface/support/graphics/point.h>
#include <libs/graphicInterface/support/graphics/rect.h>


Rectangle	EmptyRect   = {{0,0},{-1,-1}};				/* an empty rectangle	*/
Rectangle	UnusedRect  = {{UNUSED,UNUSED},{UNUSED,UNUSED}};	/* the unused rectangle	*/
Rectangle	MaximumRect = {{INT_MIN,INT_MIN},{INT_MAX,INT_MAX}};	/* the maximum rectangle*/


/* Make a rectangle from its upper left and lower right corners.			*/
Rectangle
makeRect(Point ul, Point lr)
/* ul: the upper left corner		*/
/* lr: the lower right corner		*/
{
Rectangle		n;			/* the new rectangle			*/

	n.ul = ul;
	n.lr = lr;
	return (n);
}


/* Return the size of a rectangle.							*/
Point
sizeRect(Rectangle r)
{
Point			size;			/* the size of the rectangle		*/

	size.x = r.lr.x - r.ul.x + 1;
	size.y = r.lr.y - r.ul.y + 1;
	return (size);
}


/* Make a rectangle from an upper left corner and a size.				*/
Rectangle
makeRectFromSize(Point ul, Point size)
{
Rectangle		r;			/* the rectangle being made		*/

	r.ul = ul;
	r.lr.x = ul.x + size.x - 1;
	r.lr.y = ul.y + size.y - 1;
	return (r);
}


/* Return true if the rectangle has positive area (could be visible).			*/
Boolean
positiveArea(Rectangle r)
{
	return (BOOL(r.ul.x <= r.lr.x && r.ul.y <= r.lr.y));
}


/* Move a rectangle by an amount (+).							*/
Rectangle
transRect(Rectangle r, Point p)
/* p: the distance to move it		*/
{
	r.ul.x += p.x;
	r.ul.y += p.y;
	r.lr.x += p.x;
	r.lr.y += p.y;
	return (r);
}


/* Move a rectangle by an amount (-).							*/
Rectangle
subRect(Rectangle r, Point p)
{
	r.ul.x -= p.x;
	r.ul.y -= p.y;
	r.lr.x -= p.x;
	r.lr.y -= p.y;
	return (r);
}


/* Return a rectangle relocated to a new origin.					*/
Rectangle
relocRect(Rectangle r, Point newOrigin)
{
	r.lr.x += newOrigin.x - r.ul.x;
	r.lr.y += newOrigin.y - r.ul.y;
	r.ul = newOrigin;
	return (r);
}


/* Return true if the point is in the rectangle.					*/
Boolean
pointInRect(Point p, Rectangle r)
{
	return (BOOL( p.x >= r.ul.x && p.x <= r.lr.x && p.y >= r.ul.y && p.y <= r.lr.y ));
}


/* Adjust the coordinates of r so that they reside in the border adjusted for bw.	*/
Rectangle
clipRectWithBorder(Rectangle r, Rectangle border, register short bw)
{
int			temp;			/* temporary for max/min calculation	*/
int			limit;			/* upper/lower bound on temp		*/

	temp  = r.ul.x;
	limit = border.lr.x - bw + 1;
	temp  = MIN(limit, temp);
	limit = border.ul.x + bw;
	r.ul.x = MAX(limit, temp);

	temp  = r.ul.y;
	limit = border.lr.y - bw + 1;
	temp  = MIN(limit, temp);
	limit = border.ul.y + bw;
	r.ul.y = MAX(limit, temp);

	temp  = r.lr.x;
	limit = border.lr.x - bw;
	temp  = MIN(limit, temp);
	limit = border.ul.x + bw - 1;
	r.lr.x = MAX(limit, temp);

	temp  = r.lr.y;
	limit = border.lr.y - bw;
	temp  = MIN(limit, temp);
	limit = border.ul.y + bw - 1;
	r.lr.y = MAX(limit, temp);

	return (r);
}


/* Return true if two rectangles are equal.						*/
Boolean
rectEq(Rectangle r1, Rectangle r2)
{
	return (BOOL( r1.lr.x == r2.lr.x && r1.lr.y == r2.lr.y && r1.ul.x == r2.ul.x && r1.ul.y == r2.ul.y ));
}


/* Return true if two rectangles intercect.						*/
/* Adapted from Pike's code in ACM ToG 2, 2 (April 1983).				*/
Boolean
intersect(Rectangle r, Rectangle s)
{
	return (BOOL( r.ul.x <= s.lr.x &&
		      s.ul.x <= r.lr.x &&
                      r.ul.y <= s.lr.y &&
		      s.ul.y <= r.lr.y ));
}


/* Return the intersection of two rectangles.  The result may have zero area.		*/
Rectangle
interRect(Rectangle  r1, Rectangle r2)
{
        r1.ul.x = MAX(r1.ul.x, r2.ul.x);
        r1.ul.y = MAX(r1.ul.y, r2.ul.y);
        r1.lr.x = MIN(r1.lr.x, r2.lr.x);
        r1.lr.y = MIN(r1.lr.y, r2.lr.y);
        return (r1);
}


/* Return the union (smallest rectangle which includes both) of two rectangles.		*/
Rectangle
unionRect(Rectangle r1, Rectangle r2)
{
	r1.ul.x = MIN(r1.ul.x, r2.ul.x);
	r1.ul.y = MIN(r1.ul.y, r2.ul.y);
	r1.lr.x = MAX(r1.lr.x, r2.lr.x);
	r1.lr.y = MAX(r1.lr.y, r2.lr.y);
	return (r1);
}


/* Return the border of a rectangle which has been centered in another.			*/
Rectangle
centerRectInRect(Rectangle r, Rectangle base)
/* r: the rectangle to be moved		*/
/* base: the reference rectangle		*/
{
Point			offset;			/* the new offset of r			*/

	offset.x = (base.ul.x + base.lr.x + r.ul.x - r.lr.x) / 2;
	offset.y = (base.ul.y + base.lr.y + r.ul.y - r.lr.y) / 2;
	r.lr.x += offset.x - r.ul.x;
	r.lr.y += offset.y - r.ul.y;
	r.ul = offset;
	return (r);
}


/* Return the border of a rectangle which has been bounced off the edges of another.	*/
Rectangle
bounceRectInRect(Rectangle r, Rectangle base)
{
	return (
		transRect(
			r,
			makePoint(
				MAX(MIN(0, base.lr.x - r.lr.x), base.ul.x - r.ul.x),
				MAX(MIN(0, base.lr.y - r.lr.y), base.ul.y - r.ul.y)
			)
		)
	);
}
