/* $Id: damage.h,v 1.5 1997/03/11 14:34:10 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef damage_h
#define damage_h 

typedef enum {
	damaged_dot_row,		/* the dot has changed lines */
	damaged_dot_col,		/* the dot should be found based on prefer_col */
	damaged_prefer,			/* prefer should be found based on the dot */
	damaged_to_end,			/* a line has been changed */
	damaged_to_bottom		/* the bottom portion of the pane has been changed */
} DamageType;

typedef struct {
	DamageType	change;		/* type of change */
	int		loc;		/* the location of the change */
} WinChange;

/*EXTERN(WinChange, *Damage_of,(UtilNode *node));*/

#endif
