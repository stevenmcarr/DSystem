/* $Id: char.h,v 1.2 1997/03/27 20:30:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/*
 *
 * char.h
 *
 *	structure declarations for characters, used
 *	to interpret character expressions and
 *	generate character moves
 *
*/

/* change */
/* added #defines for CharDesc->description */
#define CHAR_NORMAL         (1<<0)
#define CHAR_FUNCTION       (1<<1)
#define CHAR_UNKNOWN_LEN    (1<<2)
/* and one for CharacterAssignment */
#define MAX_CONCATS         100
/* end change */

/* change */
/* following structure re-worked */
struct CharDesc
{
	int		addr;    /* iloc reg that holds str addr */
	int 		misc;    /* depends on description -- might be */
	                         /* CHAR_NORMAL:  actual length */
	                         /* CHAR_FUNCTION: node pointer into AST */
	                         /* CHAR_UNKNOWN_LENGTH:  length register */
	int             description;
};
/* end change */

#define END_OF_CHAR_LIST -1
#define BLANK_CHARACTER 32 	/* ascii space in decimal */

/* change 5/22/91
 * added #define's
*/
#define STAR_LEN -1000
#define DEFAULT_CHAR_LENGTH 1
/* end change */
