/* $Id: check_sm.h,v 1.4 1997/03/11 14:33:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	check_sm.h							*/
/*	Check mark screen module include file.				*/
/*									*/
/*	The check screen module provides a superset of the vanilla	*/
/*	screen module.  It allows a selection indicator to be placed at	*/
/*	the upper left corner (character coordinates) of the pane.	*/
/*	This indicator serves as a display of some boolean status	*/
/*	associated with the pane.  This selection indicator can be of	*/
/*	one of three styles.						*/
/*									*/
/*	Last edited: May 31, 1990 at 2:16 am				*/
/*									*/
/************************************************************************/

#ifndef check_sm_h
#define check_sm_h

typedef enum checkStyleEnum {
	csmNoChange,		/* Don't change the check style		*/
	csmPlain,		/* vanilla: No check marks in the pane	*/
	csmWithBox,		/* Place a box in the pane		*/
	csmWithCheck		/* Place a check mark in the pane	*/
} ToggleType ;

/* Justification of text in panes */
#define csmCentered	false	/* The text should be centered		*/
#define csmLeft		true	/* The text should be left justified	*/

/* Background color */
#define csmBackgroundNormal	false	/* Pane is not inverted		*/
#define csmBackgroundInvert	true	/* Pane is normally inverted	*/

/* Tracking inversion */
#define csmTrackNormal		false	/* No inversion for mouse	*/
#define csmTrackInvert		true	/* Invert when mouse present	*/

EXTERN(short, sm_check_get_index, (void));
/* get a unique index for csm	*/
/* Takes no parameters.  Returns the installed screen module index	*/
/* for the check screen module.						*/

EXTERN(Point, sm_check_pane_size, (char *s, short font_id,
 ToggleType flavor));
/* find size of a csm pane	*/
/* Takes three parameters (char *s) the string for which to compute the	*/
/* size, (short fond_id) the font in which the string will appear, and	*/
/* (ToggleType flavor) the type of mark that will be placed in the	*/
/* pane.  Returns the dimensions (in pixels) of such a pane.		*/

EXTERN(void, sm_check_set_text, (Pane *p, char *text, short font,
 unsigned char style, Boolean justify, ToggleType flavor, Boolean checked));
/* set contents of a csm pane	*/
/* Takes seven parameters (Pane *p) the pane to operate on,		*/
/* (char *text) the string to set in the pane (uses the old string	*/
/* if 0), (short font) the font to use, (unsigned char style) the	*/
/* style of the text, (Boolean justify) which is csmCentered if the	*/
/* string is to be centered and csmLeft if the text is to be left	*/
/* justified, (ToggleType flavor) the type of selection indicator to	*/
/* place in the pane, and (Boolean checked) indicating whether or not	*/
/* the indicator in the pane should be selected.			*/

EXTERN(void, sm_check_change_text, (Pane *p, char *text,
 unsigned char style));
/* change a csm pane's text	*/

EXTERN(Boolean, sm_check_change_check, (Pane *p, Boolean checked));
/* change a csm's check mark	*/

EXTERN(void, sm_check_invert, (Pane *p));
/* invert a csm pane		*/
/* Takes one parameter (Pane *p) the pane to be inverted.  The normal	*/
/* inversion status is toggled.						*/

EXTERN(void, sm_check_set_inversion, (Pane *p, Boolean normal,
 Boolean track));
/* set a csm's inversion status	*/
/* Takes three parameters (Pane *p) the pane to operate on,		*/
/* (Boolean background) which is csmBackgroundInvert if the pane is	*/
/* inverted or csmBackgroundNormal if the pane is not, (Boolean track)	*/
/* which is csmTrackInvert if the pane should invert while the mouse is	*/
/* in it and csmTrackNormal otherwise.  The defaults are 'normal'.	*/

EXTERN(void, sm_check_set_help, (Pane *p, char *help));
/* set a check sm's help text	*/
/* Takes two parameters (Pane *p) the check pane, and (char *help) the	*/
/* new help string to be given when help is requested.  No help is	*/
/* given and help events are returned to the owner if help == 0.  The	*/
/* default value is 0.							*/

#endif 
