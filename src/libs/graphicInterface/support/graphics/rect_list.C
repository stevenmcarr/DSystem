/* $Id: rect_list.C,v 1.2 1997/06/25 15:02:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		      rect_list.c			*/
		/* 		  Rectangle list calls.			*/
		/* 							*/
		/********************************************************/

#include <libs/support/memMgmt/mem.h>
#include <libs/graphicInterface/support/graphics/point.h>
#include <libs/graphicInterface/support/graphics/rect.h>
#include <libs/graphicInterface/support/graphics/rect_list.h>

static	RectList	free_list;		/* the list of freed rectangle nodes	*/

STATIC(RectList, newRectListNode, (void));

/* Initialize the free list of rectangles.						*/
void startRectLists(void)
{
	free_list = (RectList) 0;
}


/* Free the free list of rectangles.							*/
void finishRectLists(void)
{
register RectList	current;		/* the current node being removed	*/

	while (free_list)
	{/* there is something to delete */
		current = free_list;
		free_list = free_list->next;
		free_mem((void*) current);
	}
}


/* Get a new rectangle node entry.							*/
static RectList newRectListNode(void)
{
register RectList	newList;	       	/* the new rectangle list pointer	*/

	if (free_list)
	{/* the free list has an entry */
		newList = free_list;
		free_list = free_list->next;
	}
	else
	{/* allocate a new rectangle list entry */
		newList = (RectList) get_mem(sizeof(struct rect_node), "rect_list.c:  newRectListNode() new node");
	}
	return (newList);
}


/* Initialize a RectList.								*/
void initializeRectList(RectList* rlp)
{
	*rlp = (RectList) 0;
}


/* Return the storage used by a rect list.						*/
void freeRectList(RectList* rlp)
{
register RectList	current;		/* the current node			*/

	if (*rlp)
	{/* there is something to delete */
		for (current = *rlp; current->next; current = current->next)
		{/* walk until current is the end of the list */
		}
		current->next = free_list;
		free_list = *rlp;
		*rlp = (RectList) 0;
	}
}


/* Return true if a RectList is empty.							*/
Boolean emptyRectList(RectList rl)
{
	return (NOT(rl));
}


/* Return a singleton RectList made from r.						*/
RectList singletonRectList(Rectangle r)
{
RectList		newList;	     /* the return RectList			*/

	newList = newRectListNode();
	newList->r = r;
	newList->next = (RectList) 0;
	return (newList);
}


/* Push a rectangle onto a rectangle list.						*/
void pushRectList(Rectangle r, RectList* rlp)
{
register RectList	n;			/* the new rectangle node		*/

	n = newRectListNode();
	n->r    = r;
	n->next = *rlp;
	*rlp = n;
}


/* Pop a rectangle off a RectList.							*/
Rectangle popRectList(RectList* rlp)
{
register RectList	head;			/* the freeing rectangle node		*/
Rectangle		r;			/* the rectangle to return		*/

	if (!*rlp)
	{/* we can't pop from an empty list */
		die_with_message("rect_list.c: popRectList() tried to pop from an empty list.");
	}
	head = *rlp;
	r    = head->r;
	*rlp = head->next;
	head->next = free_list;
	free_list = head;
	return (r);
}


/* Join two rectangle lists and return the result.					*/
RectList joinRectLists(RectList rl1, RectList rl2)
{
register RectList	current;		/* the current node being moved		*/

	while (rl2)
	{/* there is something left--move rn from rl2 to rl1 */
		current = rl2;
		rl2 = rl2->next;
		current->next = rl1;
		rl1 = current;
	}
	return (rl1);
}


/* Duplicate a rectangle list.								*/
RectList copyRectList(RectList rl)
{
register RectList	current;		/* the current node			*/
register RectList	copy;			/* the duplicate of current		*/
register RectList	dup = (RectList) 0;	/* the dupilcate rectangle list		*/

	for (current = rl; current; current = current->next)
	{/* copy this rectangle node */
		copy = newRectListNode();
		copy->r = current->r;
		copy->next = dup;
		dup = copy;
	}
	return (dup);
}


/* Translate all the rectangles in a RectList.						*/
void transRectList(RectList rl, Point p)
{
register Rectangle	*rp;			/* pointer to the current rectangle	*/

	while (rl)
	{/* there is something left to translate */
		rp = &(rl->r);

		rp->ul.x += p.x;
		rp->ul.y += p.y;
		rp->lr.x += p.x;
		rp->lr.y += p.y;

		rl = rl->next;
	}
}


/* Move all the rectangles in a RectList by an amount (-).				*/
void subRectList(RectList rl, Point p)
{
register Rectangle	*rp;			/* pointer to the current rectangle	*/

	while (rl)
	{/* there is something left to move */
		rp = &(rl->r);

		rp->ul.x -= p.x;
		rp->ul.y -= p.y;
		rp->lr.x -= p.x;
		rp->lr.y -= p.y;

		rl = rl->next;
	}
}


/* Remove the area of a rectangle from a list of rectangles.				*/
void removeFromRectList(Rectangle omit, RectList* rlp)
{
register RectList	res = (RectList) 0;	/* the resulting RectList		*/
register RectList	current;		/* the current rectangle being processed*/
Rectangle		r;			/* the rectangle of interest		*/
Rectangle		slice;			/* a slice off r			*/

	while (*rlp)
	{/* there is a rectangle to process */
		if (intersect((*rlp)->r, omit))
		{/* there is an intersection--clip it away */
			current = *rlp;
			r = (*rlp)->r;
			*rlp = (*rlp)->next;
			current->next = free_list;
			free_list = current;
			if (r.ul.y < omit.ul.y)
			{/* take the top off r */
				slice = r;
				slice.lr.y = omit.ul.y - 1;
				r.ul.y = omit.ul.y;
				current = newRectListNode();
				current->r = slice;
				current->next = res;
				res = current;
			}
			if (r.lr.y > omit.lr.y)
			{/* take the bottom off r */
				slice = r;
				slice.ul.y = omit.lr.y + 1;
				r.lr.y = omit.lr.y;
				current = newRectListNode();
				current->r = slice;
				current->next = res;
				res = current;
			}
			if (r.ul.x < omit.ul.x)
			{/* take the left off r */
				slice = r;
				slice.lr.x = omit.ul.x - 1;
				r.ul.x = omit.ul.x;
				current = newRectListNode();
				current->r = slice;
				current->next = res;
				res = current;
			}
			if (r.lr.x > omit.lr.x)
			{/* take the right off r */
				slice = r;
				slice.ul.x = omit.lr.x + 1;
				r.lr.x = omit.lr.x;
				current = newRectListNode();
				current->r = slice;
				current->next = res;
				res = current;
			}
		}
		else
		{/* there is no intersection--put current on the result list  */
			current = *rlp;
			*rlp = (*rlp)->next;
			current->next = res;
			res = current;
		}
	}
	*rlp = res;
}


/* Return only the entries in a rectangle list which appear in a given rectangle.	*/
RectList interRectList(Rectangle clipper, RectList head)
{
register RectList	rl;			/* the head of the returned list	*/
register RectList	orl;			/* One behind rl for purging empties	*/
register RectList	temp;			/* For freeing				*/

	rl = head;
	orl = (RectList) 0;
	while (rl)
	{/* walk down the rect list */
		rl->r = interRect(rl->r,clipper);
		if ( ! positiveArea(rl->r) )
		{/* Found an empty rect - flush it */
			if (orl == (RectList)0)
			{/* Empty rect is first on list, so advance head of the list */
				temp = rl;
				head = head->next;
				rl = head;
				temp->next = free_list;
				free_list = temp;
			}
			else
			{/* Clip the empty rect out of the middle, and free it */
				temp = rl;
				orl->next = rl->next;
				rl = rl->next;
				temp->next = free_list;
				free_list = temp;
			}
		}
		else
		{/* Move on to the next Rect.  orl tags along one behind. */
			orl = rl;
			rl = rl->next;
		}
	}
	return head;
}


/* Partition a rectangle list around a rectangle.					*/
RectList partitionRectList(Rectangle partition, RectList* rlp)
{
register RectList	inside = (RectList) 0;	/* the rectangles inside the partition	*/
register RectList	outside = (RectList) 0;	/* the rectangles outside the partition	*/
register RectList	current;		/* the current rectangle being processed*/
Rectangle		remaining;		/* the remaining rectangle of interest	*/
Rectangle		slice;			/* a slice off remaining			*/

	while (*rlp)
	{/* there is a rectangle to process */
		if (intersect((*rlp)->r, partition))
		{/* there is an intersection--clip it away */
			/* put this rectangle node on the free list */
				current = *rlp;
				remaining = (*rlp)->r;
				*rlp = (*rlp)->next;
				current->next = free_list;
				free_list = current;
			if (remaining.ul.y < partition.ul.y)
			{/* take the top off remaining */
				slice = remaining;
				slice.lr.y = partition.ul.y - 1;
				remaining.ul.y = partition.ul.y;
				current = newRectListNode();
				current->r = slice;
				current->next = outside;
				outside = current;
			}
			if (remaining.lr.y > partition.lr.y)
			{/* take the bottom off remaining */
				slice = remaining;
				slice.ul.y = partition.lr.y + 1;
				remaining.lr.y = partition.lr.y;
				current = newRectListNode();
				current->r = slice;
				current->next = outside;
				outside = current;
			}
			if (remaining.ul.x < partition.ul.x)
			{/* take the left off remaining */
				slice = remaining;
				slice.lr.x = partition.ul.x - 1;
				remaining.ul.x = partition.ul.x;
				current = newRectListNode();
				current->r = slice;
				current->next = outside;
				outside = current;
			}
			if (remaining.lr.x > partition.lr.x)
			{/* take the right off remaining */
				slice = remaining;
				slice.ul.x = partition.lr.x + 1;
				remaining.lr.x = partition.lr.x;
				current = newRectListNode();
				current->r = slice;
				current->next = outside;
				outside = current;
			}
			if (positiveArea(remaining))
			{/* the remaining rectangle should be put on the inside list */
				current = newRectListNode();
				current->r = remaining;
				current->next = inside;
				inside = current;
			}
		}
		else
		{/* there is no intersection--put current on the result list  */
			current = *rlp;
			*rlp = (*rlp)->next;
			current->next = outside;
			outside = current;
		}
	}
	*rlp = outside;
	return (inside);
}
