/* $Id: vanilla_sm.h,v 1.4 1997/03/11 14:33:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		      vanilla_sm.h			*/
		/* 	   Vanilla screen module include file.		*/
		/* 							*/
		/********************************************************/

/* This screen module is used for title panes and for table headings.  Returns on a	*/
/* selection (if it is selectable) or move request.  Help requests give a message or	*/
/* return a help event based on client preference.					*/

#ifndef vanilla_sm_h
#define vanilla_sm_h

EXTERN(short, sm_vanilla_get_index, (void));
/* gets the vanilla s. m. index number	*/
/* Takes no parameters.  Returns the installed screen module index number for the	*/
/* vanilla screen module.								*/

EXTERN(Point, sm_vanilla_pane_size, (char *s, short font_id));
/* pane size needed to display a text	*/
/* Takes two parameters (char *s) the string of interest and (short font_id) the font	*/
/* that the string is to appear in.  Returns the dimensions (in pixels) of the	 	*/
/* appropriate pane.									*/

EXTERN(void, sm_vanilla_invert, (Pane *p));
/* invert a pane */
/* Takes one parameter (Pane *p) the pane to be inverted.  The normal inversion status	*/
/* is toggled.										*/

EXTERN(void, sm_vanilla_set_inversion, (Pane *p, Boolean normal, Boolean track));
/* set the pane inversion status	*/
/* Takes three parameters (Pane *p) the pane to operate on, (Boolean normal)    	*/
/* VSM_INVERT_BKGND if the pane is normally reverse video and VSM_NORMAL_BKGND if the	*/
/* pane is normal video, and (Boolean track) VSM_INVERT_TRACK if the pane inverts when	*/
/* the mouse is in it and VSM_NORMAL_TRACK otherwize.  The defaults are 'normal'.	*/
#define	VSM_INVERT_BKGND	true		/* the pane is inverted normally	*/
#define	VSM_NORMAL_BKGND	false		/* the pane is not inverted		*/
#define	VSM_INVERT_TRACK	true		/* the pane inverts with mouse		*/
#define	VSM_NORMAL_TRACK	false		/* the pane doesn't invert with mouse	*/

EXTERN(void, sm_vanilla_get_inversion, (Pane *p, Boolean *normal,
 Boolean *track));
/* find current pane inversions	*/
/* takes three parameters (Pane *p) the pane to operate on, (Boolean *normal),  	*/
/* and (Boolean *track), pointers in which the current normal-video and track values	*/
/* are returned. */

EXTERN(void, sm_vanilla_set_text, (Pane *p, char *text, short font_id,
 unsigned char style, Boolean justify));
/* set the text for the pane		*/
/* Takes five parameters (Pane *p) the pane to operate on, (char *text) the string to	*/
/* set in the pane (uses the old string if 0), (short font_id) the font id, (unsigned 	*/
/* char style) the style of the text, and (Boolean justify) which is VSM_JUSTIFY_CENTER	*/
/* if the string is to be centered and VSM_JUSTIFY_LEFT if the text is to be left	*/
/* justified. The pane is redrawn.  The default string is the empty string.		*/
#define	VSM_JUSTIFY_CENTER	false		/* the text should be centered		*/
#define VSM_JUSTIFY_LEFT	true		/* the text should be left justified	*/

EXTERN(void, sm_vanilla_set_help, (Pane *p, char *help));
/* set the help string */
/* Takes two parameters (Pane *p) the vanilla pane, and (char *help) the new help string*/
/* to be given when help is requested.  No help is given and help events are returned	*/
/* to the owner if help == 0.  The default value is 0.					*/

#endif
