/* $Id: rect.h,v 1.1 1997/06/02 19:40:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 			rect.h				*/
		/* 		 Rectangle information			*/
		/* 		     (root/rect.c)			*/
		/********************************************************/

#ifndef rect_h
#define rect_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif


typedef	struct	{				/* A RECTANGLE				*/
	Point		ul;			/* upper left corner			*/
	Point		lr;			/* lower right corner			*/
		} Rectangle;

extern Rectangle	EmptyRect;		/* an empty rectangle			*/
extern Rectangle	UnusedRect;		/* the unused rectangle			*/
extern Rectangle	MaximumRect;		/* the maximum-sized rectangle		*/


EXTERN(Rectangle, makeRect, (Point ul, Point lr));
/* makes a rect from ul and lr points	*/
/* Takes two parameters (Point ul, lr) and returns the rectangle made from them.	*/

EXTERN(Point, sizeRect, (Rectangle r));
/* return the rectangle's size		*/
/* Takes one parameter (Rectangle r) and returns the size of the rectangle.		*/

EXTERN(Rectangle, makeRectFromSize, (Point ul, Point size));
/* make a rectangle from offset & size	*/
/* Takes two parameters (Point ul, size) and returns the rectangle with the upper left	*/
/* corner at 'ul' with size 'size'.							*/

EXTERN(Boolean, positiveArea, (Rectangle r));
/* true if the rectangle has pos area.	*/
/* Takes one parameter (Rectangle r) and returns true if the rectangle has a positive	*/
/* area.										*/

EXTERN(Rectangle, transRect, (Rectangle r, Point p));
/* return a shifted rectangle		*/
/* Takes two parameters (Rectangle r) the rectangle to be moved and (Point p) the	*/
/* distance to move it.  Returns the shifted rectangle.					*/

EXTERN(Rectangle, subRect, (Rectangle r, Point p));
/* return an unshifted rectangle	*/
/* Takes two parameters (Rectangle r) the rectangle to be moved and (Point p) the	*/
/* distance to move it.  Returns the unshifted rectangle.				*/

EXTERN(Rectangle, relocRect, (Rectangle r, Point newOrigin));
/* move a rectangle to a new place	*/
/* Takes two parameters (Rectangle r) the rectangle to be operated on and		*/
/* (Point newOrigin) the new origin of the rectangle.  Returns the shifted rectangle.	*/

EXTERN(Boolean, pointInRect, (Point p, Rectangle r));
/* return true if the point is in rect	*/
/* Takes two parameters (Point p) and (Rectangle r) and returns true if the point is in	*/
/* the rectangle (border included).							*/

EXTERN(Rectangle, clipRectWithBorder, (Rectangle r, Rectangle border, short bw));
/* clip a rectangle within a border	*/
/* Takes three parameters (Rectangle r) the rectangle to be clipped, (Rectangle border)	*/
/* the border in which to clip it, and (short bw) the width of the border.  The		*/
/* resulting rectangle is returned which may have zero (or negative) size in either	*/
/* dimension.										*/

EXTERN(Boolean, rectEq, (Rectangle r1, Rectangle r2));
/* returns true if two rectangles equal	*/
/* Takes two parameters (Rectangle r, s) and returns true if they are equal.		*/

EXTERN(Boolean, intersect, (Rectangle r, Rectangle s));
/* determine if two rectangles intercect*/
/* Takes two parameters (Rectangle r, s) and returns true if they intersect.		*/

EXTERN(Rectangle, interRect, (Rectangle r1, Rectangle r2));
/* return the intersection of two rects	*/
/* Takes two parameters (Rectangle r1, r2) and returns their intersection.  If they do	*/
/* not intersect, a rectangle of zero area is returned.					*/

EXTERN(Rectangle, unionRect, (Rectangle r1, Rectangle r2));
/* return the union of two rects	*/
/* Takes two parameters (Rectangle r1, r2) and returns their union.  The union is the	*/
/* smallest rectangle which includes both constituents.					*/

EXTERN(Rectangle, centerRectInRect, (Rectangle r, Rectangle base));
/* return a rectangle centered in a brdr*/
/* Takes two parameters (Rectangle r) the rectangle to be adjusted and (Rectangle base)	*/
/* base reference rectangle.  The result of r centered in base is returned.		*/

EXTERN(Rectangle, bounceRectInRect, (Rectangle r, Rectangle base));
/* return a rect shown in another rect	*/
/* Takes two parameters (Rectangle r) the rectangle to be adjusted and (Rectangle ref)	*/
/* the rectangle in which the first is to appear (if possible).  The size of the first	*/
/* rectangle is not changed.								*/

#endif
