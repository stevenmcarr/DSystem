/* $Id: point.h,v 1.1 1997/06/02 19:40:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 			 point.h			*/
		/* 		  coordinate utilities			*/
		/* 		      (util/point.c)			*/
		/********************************************************/

#ifndef point_h
#define point_h

typedef	struct	{				/* A COORDINATE				*/
	int		x;			/* x-coordinate				*/
	int		y;			/* y-coordinate				*/
		} Point;
extern	Point		Origin;			/* The (0,0) coordinate			*/
extern	Point		UnusedPoint;		/* The (UNUSED, UNUSED) coordinate	*/

EXTERN(int, xPoint, (Point p));
/* get the x coordinate of a point	*/
/* Takes one parameter (Point p) and returns the x coordinate.				*/

EXTERN(int, yPoint, (Point p));
/* get the y coordinate of a point	*/
/* Takes one parameter (Point p) and returns the y coordinate.				*/

EXTERN(Point, makePoint, (int x, int y));
/* makes a point from two coordinates	*/
/* Takes two parameters (int x, y) and returns the point made from them.		*/

EXTERN(Point, transPoint, (Point p, Point inc));
/* return the sum of two points		*/
/* Takes two parameters (Point p1, p2) and returns p1 + p2.				*/

EXTERN(Point, subPoint, (Point p, Point diff));
/* return the difference of two points	*/
/* Takes two parameters (Point p1, p2) and returns p1 - p2.				*/

EXTERN(Boolean, equalPoint, (Point p1, Point p2));
/* return true iff two points are equal	*/
/* Takes two parameters (Point p1, p2) and returns true if p1 and p2 are equal.		*/

#endif
