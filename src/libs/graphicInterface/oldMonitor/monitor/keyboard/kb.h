/* $Id: kb.h,v 1.7 1997/03/11 14:33:36 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 			  kb.h				*/
		/*            Keyboard input include file.		*/
		/* 							*/
		/********************************************************/


/* Note:  A KbChar in intended to be a superset of simple characters.  By the addition	*/
/* of 8 more bits in a KbChar, room is made for machine independent keycodes for	*/
/* function and arrow keys.  Regular characters map to KbChars by a simple cast.	*/

#ifndef kb_h
#define kb_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

typedef	unsigned short	KbChar;			/* KEYBOARD CHARACTER DEFINITION	*/

#define	toKbChar(ch)	((KbChar) ch)		/* make a KbChar from a char		*/
/* Takes one parameter: (char ch) the character to be converted.  Returns the		*/
/* corresponding KbChar.								*/

EXTERN(KbChar, makeKbChar, (unsigned char base, unsigned char top));
/* Make a general KbChar		*/
/* Takes two parameters:  (unsigned char base, top) the two constituents.  The KbChar	*/
/* made from these pieces is returned.							*/


	/* useful KbChar constants, and (normalized) symbolic name arrays */
#define	KB_first	toKbChar(0x0000)	/* lowest KbChar			*/
#define	KB_last		toKbChar(0xffff)	/* highest KbChar			*/

#define	KB_first_ascii	toKbChar(0x0000)	/* lowest char (coerced to KbChar)	*/
#define KB_num_ascii	256			/* number of ascii characters		*/
#define KB_last_ascii	toKbChar(0x00ff)	/* highest char (coerced to KbChar)	*/
#define	KB_first_print	toKbChar(0x0020)	/* lowest printable KbChar		*/
#define	KB_last_print	toKbChar(0x007e)	/* highest printable KbChar		*/
#define KB_Nul		toKbChar(0x0000)	/* the nul KbChar			*/
#define KB_Bell		toKbChar(0x0007)	/* the bell KbChar			*/
#define KB_Backspace	toKbChar(0x0008)	/* the backspace KbChar			*/
#define KB_Tab		toKbChar(0x0009)	/* the tab KbChar			*/
#define KB_Linefeed	toKbChar(0x000a)	/* the line feed KbChar			*/
#define KB_Formfeed	toKbChar(0x000b)	/* the form feed KbChar			*/
#define KB_Enter	toKbChar(0x000d)	/* the enter KbChar			*/
#define KB_Escape	toKbChar(0x001b)	/* the escape KbChar			*/
#define KB_Space	toKbChar(0x0020)	/* the space KbChar			*/
#define KB_Delete	toKbChar(0x007f)	/* the delete KbChar			*/

#define	KB_first_right	toKbChar(0x0100)	/* first right keypad KbChar		*/
#define	KB_num_right	25			/* number of defined right keypad keys	*/
#define KB_right(x)	toKbChar(KB_first_right+x)/* KbChars of the right keypad	*/

#define	KB_first_left	toKbChar(0x0200)	/* first left keypad KbChar		*/
#define	KB_num_left	25			/* number of defined left keypad keys	*/
#define KB_left(x)	toKbChar(KB_first_left+x)/* KbChars of the left keypad		*/

#define KB_first_top	toKbChar(0x0300)	/* first top keypad KbChar		*/
#define	KB_num_top	25			/* number of defined top keypad keys	*/
#define KB_top(x)	toKbChar(KB_first_top+x)/* KbChars of the top keypad		*/

#define KB_first_arrow	toKbChar(0x0400)	/* first arrow keypad KbChar		*/
#define	KB_num_arrow	4			/* number of defined arrow keypad keys	*/
#define KB_arrow(x)	toKbChar(KB_first_arrow+x)/* KbChars of the arrow keypad	*/
#define KB_ArrowU	KB_arrow(0)		/* KbChar of the up arrow		*/
#define KB_ArrowD	KB_arrow(1)		/* KbChar of the down arrow		*/
#define KB_ArrowL	KB_arrow(2)		/* KbChar of the left arrow		*/
#define KB_ArrowR	KB_arrow(3)		/* KbChar of the right arrow		*/


	/* KbString Manipulation */

/* Keyboard strings are the way to refer to strings of key hits on the keyboard.	*/

typedef	struct	_kbs	{			/* KEYBOARD STRING DEFINITION		*/
	short		num_kc;			/* the number of KbChars		*/
	KbChar		*kc_ptr;		/* the KbChar array			*/
	Boolean		ephemeral;		/* "kc_ptr should be freed on use"	*/
			} KbString;

EXTERN(KbString, getKbString, (short len, char *s));
/* make a KbString of a specified length*/
/* Takes two parameters:  (short len) the the length of the KbString and (char *s) the	*/
/* identification string (a la get_mem).  Returns the made non-ephemeral KbString with	*/
/* the num_kc field initialized.  This KbString must later be freed using		*/
/* freeKbString().									*/

EXTERN(KbString, makeKbString, (char *str, char *s));
/* make a KbString from a char *	*/
/* Takes two parameters:  (char *str) the null-terminated string to be converted and	*/
/* (char *s) the identification string (a la get_mem).  Returns the correpsonding	*/
/* ephemeral KbString.  This KbString must later be freed using freeKbString().		*/

EXTERN(KbString, copyKbString, (KbString ks));
/* create an ephemeral copy of KbString	*/
/* Takes one parameters:  (KbString ks) the string to return a copy of.  Returns	*/
/* a new ephemeral copy.								*/

EXTERN(KbString, regetKbString, (KbString *ks, int nl));
/* resize a KbString			*/
/* Takes two parameters:  (KbString *ks) the KbString to resize and (int nl) the new	*/
/* length.  Ben, I think, wrote this and it doesn't seem to do anything very sensible	*/
/* unless the input pointer is an ephemeral KbString, in which case it acts properly.	*/

EXTERN(void, freeKbString, (KbString kbs));
/* free a KbString			*/
/* Takes one parameter:  (KbString ks) and returns the storage associated with it.	*/

EXTERN(void, convertKbString, (KbString kbs, char *buf));
/* convert a KbString to a char *	*/
/* Takes two parameters:  (KbString ks) the KbString to convert and (char *buf) the	*/
/* buffer in which to write the converted string.  The KbString is converted into a	*/
/* null-terminated C style string by ignoring the high bits of each KbChar.  The	*/
/* KbString is freed if it was ephemeral.						*/


	/* Symbolic Names */

/* The symbolic keymapping provides a machine-independent, human readable way to refer	*/
/* to refer to key bindings.  These names are suitable for text files of key bindings.	*/
/* Note:  every KbChar in the ascii set and ranges defined above have a symbolic name.	*/

EXTERN(KbString, symbolicToKbString, (char *sym));
/* convert a symbolic string to KbString*/
/* Takes one parameter:  (char *sym) a null-terminated string containing a symbolic	*/
/* representation of a KbString.  The corresponding ephemeral KbString is returned.	*/
/* If the string is not a real symbolic string, the call returns a KbString with length	*/
/* field of UNUSED--this KbString should not be freed in this instance.			*/

EXTERN(char *, symbolicFromKbChar, (KbChar kbc));
/* convert a KbChar to a symbolic str.	*/
/* Takes one parameter:  (KbChar kbc) the KbChar to convert to its symbolic equivalent.	*/
/* If kbc does not have a symbolic equivalent, the resulting string will be		*/
/* KB_bogus_name.  The resulting symbolic string should not be freed.			*/

EXTERN(char *, symbolicFromKbString, (KbString kbs));
/* convert a KbString to a symbolic str.*/
/* Takes one parameter:  (KbString kbs) the KbString to convert to its symbolic		*/
/* representation.  If the KbString is ephemeral, it will be freed.  The resulting	*/
/* symbolic string must be freed.							*/


	/* Actual Names */

/* The actual names are the machine-specific, human readable names for each possible	*/
/* key on the current keyboard.								*/

extern	char		*KB_bogus_name;		/* the name of the unknown KbChar	*/

EXTERN(char *, actualFromKbChar, (KbChar kbc));
/* returns the actual name of a KbChar	*/
/* Takes one parameter:  (KbChar kbc) the KbChar to find the actual name of.  The	*/
/* resulting name is suitable for user output.  If kbc does not have a keyboard		*/
/* equivalent, the resulting string will be KB_bogus_name.  The resulting string should	*/
/* not be freed.									*/

EXTERN(char *, actualFromKbString, (KbString kbs));
/* returns the actual name of a KbString*/
/* Takes one parameter:  (KbString kbs) the KbString to find the actual name of.  The	*/
/* resulting name is suitable for user output.  If one of the KbChars in the KbString	*/
/* does not have a keyboard equivalent, the entire string will be KB_bogus_name.  If the*/
/*  KbString is ephemeral, it will be freed.  The resulting string must be freed.	*/


	/* Keyboard Information */

/* Keyboard ID and keybindings for the keyboard.					*/

extern	short		kb_keyboard_id;		/* the keyboard index			*/
#define			KB_DEFAULT	0	/* the default keyboard (unknown)	*/
#define			KB_IBM		1	/* IBM keyboard				*/
#define			KB_SUN_3	2	/* Sun 3 Keyboard			*/
#define			KB_SPARC	3	/* Sun 4 (Sparc) Keyboard		*/
#define			KB_LAST_ID	3	/* the last keyboard index used		*/

extern	char		*kb_names[];		/* keyboard names by ID-zero terminated	*/
extern	Boolean		kb_swap_bs_del;		/* true if we should swap bs & del fcns.*/

EXTERN(char *, inputFromKbChar, (KbChar kbc));
/* returns the input string for a KbChar*/
/* Takes one parameter (KbChar kbc) the keyboard character of interest.  Returns the	*/
/* character sequence generated by that key (or "" if the key doesn't exist).  The	*/
/* returned string should NOT be freed.							*/


	/* Initialization / Finalization */

EXTERN(void, startKb, (void));
/* start the keyboard abstraction	*/
/* Takes no parameters.  Initializes the symbolic keyboard conversion.			*/

EXTERN(void, finishKb, (void));
/* finish the keyboard abstraction	*/
/* Takes no parameters.  Finalized the symbolic keyboard conversion.			*/

#endif
