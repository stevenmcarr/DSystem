/* $Id: rect_list.h,v 1.1 1997/06/02 19:40:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		    rect_list.h				*/
		/* 	     Rectangle list information			*/
		/* 		 (root/rect_list.c)			*/
		/********************************************************/

#ifndef rect_list_h
#define rect_list_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

struct	rect_node {				/* A RECTANGLE NODE			*/
	Rectangle	r;			/* the rectangle			*/
struct	rect_node	*next;			/* the next rectangle node		*/
		};

typedef
struct	rect_node	*RectList;		/* A RECTANGLE LIST			*/

EXTERN(void, startRectLists, (void));
/* initialize the abstraction		*/
/* Takes no parameters.  Initializes the RectList abstraction.				*/

EXTERN(void, finishRectLists, (void));
/* finalize the abstraction		*/
/* Takes no parameters.  Finalizes the RectList abstraction.				*/

EXTERN(void, initializeRectList, (RectList *rlp));
/* initializes a RectList		*/
/* Takes one parameter (RectList *rlp) a pointer to the RectList to be initialized.	*/

EXTERN(void, freeRectList, (RectList *rlp));
/* free the rectangles of a rect list	*/
/* Takes one parameter (RectList *rlp) the RectList pointer to be freed.  *rlp is then	*/
/* made empty.										*/

EXTERN(Boolean, emptyRectList, (RectList rl));
/* returns true if a RectList is empty	*/
/* Takes one parameter (RectList rl) the RectList to be checked.  Returns true if rl	*/
/* is empty.										*/

EXTERN(RectList, singletonRectList, (Rectangle r));
/* return a singleton RectList		*/
/* Takes one parameter (Rectangle r) the rectangle.  Returns a RectList containing r.	*/

EXTERN(void, pushRectList, (Rectangle r, RectList *rlp));
/* add a rectangle to a RectList	*/
/* Takes two parameters (Rectangle r) the rectangle to be pushed and (RectList *rlp)	*/
/* the pointer to the RectList to which to add the rectangle.				*/

EXTERN(Rectangle, popRectList, (RectList *rlp));
/* delete a rectangle from a RectList	*/
/* Takes one parameter (RectList *rlp) the pointer to the RectList from which a		*/
/* rectangle should be deleted.  The rectangle is returned.				*/

EXTERN(RectList, joinRectLists, (RectList rl1, RectList rl2));
/* joins two rectangle lists		*/
/* Takes two parameters (RectList rl1, rl2) the rectangle lists to be joined.  The	*/
/* joined lists are returned.								*/

EXTERN(RectList, copyRectList, (RectList rl));
/* return a duplicate of a RectList	*/
/* Takes one parameter (RectList rl) the list to be duplicated.  Returns a copy.	*/

EXTERN(void, transRectList, (RectList rl, Point p));
/* shift a list of rectangles		*/
/* Takes two parameters (RectList rl) the list of rectangles to be moved and (Point p)	*/
/* the distance to move it.  The list is altered by this call.				*/

EXTERN(void, subRectList, (RectList rl, Point p));
/* unshift a list of rectangles		*/
/* Takes two parameters (RectList rl) the list of rectangles to be unshifted and	*/
/* (Point p) the distance to move it.  The list is altered by this call.		*/

EXTERN(RectList, interRectList, (Rectangle clipper, RectList head));
/* clip a RectList within a rectangle	*/
/* Takes two parameters (Rectangle r) the clipping rectangle and (RectList rl) the list	*/
/* of rectangles to be clipped.  The resulting rectangle list is returned, which may be	*/
/* empty.  All rectangles in the returned list are of positive size.  The original 	*/
/* RectList is lost.									*/

EXTERN(void, removeFromRectList, (Rectangle omit, RectList *rlp));
/* remove a rectangle from a RectList	*/
/* Takes two parameters (Rectangle r) the rectangle to remove and (RectList *rlp) the	*/
/* RectList pointer.  rlp is modified to be a RectList pointer which does not include	*/
/* the area of r.  The original RectList is lost.					*/

EXTERN(RectList, partitionRectList, (Rectangle partition, RectList *rlp));
/* partition a RectList around a rect.	*/
/* Takes two parameters (Rectangle r) the dividing rectangle and (RectList *rlp) the 	*/
/* pointer to the source RectList.  The RectList is partitioned around r.  The RectList	*/
/* which is inside r is returned and the RectList which is not inside r is set into the	*/
/* pointer.  The original RectList is no longer available.				*/

#endif
