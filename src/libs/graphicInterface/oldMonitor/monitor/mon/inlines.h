/* $Id: inlines.h,v 1.3 1997/03/11 14:33:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		   point.inline.h			*/
		/* 							*/
		/********************************************************/

#define xPoint(p)	p.x
#define yPoint(p)	p.y
#define equalPoint(p1, p2) \
	(BOOL(p1.x == p2.x && p1.y == p2.y))

#define positiveArea(r) \
	(BOOL(r.ul.x <= r.lr.x && r.ul.y <= r.lr.y))

#define pointInRect(p, r) \
	(BOOL(p.x>=r.ul.x&&p.x<=r.lr.x&&p.y>=r.ul.y&&p.y<=r.lr.y))

#define rectEq(r1, r2) \
	(BOOL(r1.lr.x==r2.lr.x&&r1.lr.y==r2.lr.y&&r1.ul.x==r2.ul.x&& \
		r1.ul.y==r2.ul.y))

#define intersect(r, s) \
	(BOOL(r.ul.x<=s.lr.x&&s.ul.x<=r.lr.x&&r.ul.y<=s.lr.y&& \
		s.ul.y<=r.lr.y))

#define bounceRectInRect(r, base) \
	(transRect(r,makePoint(MAX(MIN(0,base.lr.x-r.lr.x),base.ul.x-r.ul.x),\
	 MAX(MIN(0,base.lr.y-r.lr.y),base.ul.y-r.ul.y))))

