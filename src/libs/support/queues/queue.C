/* $Id: queue.C,v 1.2 1997/06/26 17:28:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	q.c - general purpose queue package				*/
/*									*/
/*	Last edited: 28 March 1988					*/
/*									*/
/************************************************************************/

struct QueueStruct {
	int	eltSize;
	int	qFront;
	int	qLength;
	int	qMax;
	char	*q;
};

#include <include/bstring.h>
#include <libs/support/misc/general.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/support/queues/queue.h>

Queue qCreate(int eltSize, int initialLength)
{
	Queue q;

	q = (Queue)get_mem(sizeof(*q),"qCreate");
	q->q = (char *)get_mem(initialLength*eltSize,"qCreate");
	q->qMax		= initialLength;
	q->qLength	= 0;
	q->qFront	= 0;
	q->eltSize	= eltSize;

	return q;
}

void qDestroy(Queue q)
{
	free_mem((void*)q->q);
	free_mem((void*)q);
}

static void qGrow(Queue q, int increase)
{
	q->q = (char *)reget_mem(q->q,
			(q->qMax + increase) * q->eltSize,
			"qGrow");

	bcopy((const char *)	q->q + q->eltSize *  q->qFront,
	      (char *)	q->q + q->eltSize * (q->qFront + increase),
		q->eltSize * (q->qMax - q->qFront) );

	q->qMax += increase;
	q->qFront += increase;
	q->qFront %= q->qMax;
}

void qFlush(Queue q)
{
	q->qLength = 0;
}

Boolean qIsEmpty(Queue q)
{
	return BOOL(q->qLength == 0);
}

int qLength(Queue q)
{
	return q->qLength;
}

char *qLeave(Queue q)
{
	char *head;

	head = &(q->q[q->qFront*q->eltSize]);

	q->qFront++;
	q->qFront %= q->qMax;
	q->qLength--;

	return head;
}

char *qUp(Queue q, char *eltp)
{
	int place;

	/* Ensure space for new element */
	if (q->qLength+1 > q->qMax)
		qGrow(q,1);

	/* Find location of new element */
	place = (q->qFront + q->qLength) % q->qMax;
	place *= q->eltSize;

	/* Insert new element */
	bcopy((const char *) eltp,(char *) &(q->q[place]), q->eltSize );
	q->qLength++;

	return eltp;
}

char *qButt(Queue q, char *eltp)
{
	int place;

	/* Ensure space for new element */
	if (q->qLength+1 > q->qMax)
		qGrow(q,1);

	/* Find location of new element */
	place = (q->qFront - 1);
	place += q->qMax;
	place %= q->qMax;
	place *= q->eltSize;

	/* Insert new element */
	bcopy((const char *) eltp, (char *)&(q->q[place]), q->eltSize );
	q->qLength++;

	return eltp;
}
