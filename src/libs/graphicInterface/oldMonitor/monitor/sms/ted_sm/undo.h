/* $Id: undo.h,v 1.4 1997/03/11 14:34:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef undo_h
#define undo_h

#define HIDE -1
#define CONTINUOUS 1
#define DISJOINT  -1
#define NOCURVE   -2

typedef struct UndoInfo {
	int dot;		/* the dot before the operation */
	int adot;		/* where the dot goes after the operation */
	int amount;		/*  0 for dot only
				 *  x for insertion of buffer size x,
				 * -x for deletion of x */
	int   location;		/* where the insertion or deletion occurs */
	short curvature;	/* if this is part of several editing operations
				 * which were done as a unit
				 */
	char *buffer;		/* the text to be inserted */
} Undo;

#endif
