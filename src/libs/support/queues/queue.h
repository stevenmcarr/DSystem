/* $Id: queue.h,v 1.3 1997/03/11 14:37:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/****************************************************************/
/*	queue.h							*/
/*								*/
/*	Queue -- general purpose queues				*/
/*	Last edited: May 28, 1990 at 2:13pm			*/
/*								*/
/****************************************************************/

#ifndef queue_h
#define queue_h
typedef struct QueueStruct *Queue;

EXTERN(Queue, qCreate, (int eltSize, int initialLength));
/*	int eltSize		Size of a queue member in bytes
 *	int initialLength	Initial length of queue (advisory)
 *
 *	Creates a queue for items of size "eltSize".  The initial
 *	length is "initialLength", although this is dynamically
 *	extended.  The queue returned by this call should be
 *	destroyed by qDestroy() when no longer needed.
 */

EXTERN(void, qDestroy, (Queue q));
/*	Queue q;
 *
 *	Frees memory allocated by the queue package for the queue q.
 */

EXTERN(void, qFlush, (Queue q));
/*	Queue q;
 *
 *	Flushes the current contents of the queue, setting its
 *	length to zero.
 */

EXTERN(Boolean, qIsEmpty, (Queue q));
/*	Queue q;
 *
 *	Returns true is the queue q has nothing in it.
 */

EXTERN(int, qLength, (Queue q));
/*	Queue q;
 *
 *	Returns the number of items in the queue q.
 */

EXTERN(char *, qLeave, (Queue q));
/*	Queue q;
 *
 *	Removes the item at the head of the queue q, and returns
 *	a pointer to it.  The item may be overwritten by the
 *	queue package the next time anything is enqueued or
 *	dequeued.
 */

EXTERN(char *, qUp, (Queue q, char *eltp));
/*	Queue q;
 *	char *eltp;
 *
 *	Adds the item pointed to by "eltP" to the end of the
 *	queue q.  The pointer "eltP" is returned for the
 *	sake of convenience.
 */

EXTERN(char *, qButt, (Queue q, char *eltp));
/*	Queue q;
 *	char *eltP;
 *
 *	Adds the item pointed to by "eltP" to the *front* of
 *	the queue q.  The pointer "eltP" is returned for the
 *	sake of convenience.
 */

#endif
