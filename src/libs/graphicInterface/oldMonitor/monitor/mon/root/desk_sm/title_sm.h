/* $Id: title_sm.h,v 1.4 1997/03/11 14:33:52 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*			       title_sm.h				*/
/*		    standard window title screen module			*/
/*									*/
/************************************************************************/

#ifndef title_sm_h
#define title_sm_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

EXTERN(short, sm_title_get_index,(void));
/* get the index of the title sm*/
/* Takes no parameters.  Returns the installed screen module index	*/
/* number for the title screen module.					*/

EXTERN(Point, sm_title_pane_size,(char *title, short font));	
/* get the desired pane size	*/
/* Takes two parameters:  (char *title) the title to use and (short	*/
/* font) the font in which to show the title.  Returns the minimum	*/
/* pane size necessary to show said title.				*/

EXTERN(void, sm_title_initialize,(Pane *p, short font, Boolean resizable));	
/* set up the title pane	*/
/* Takes three parameters:  (Pane *p) the title pane to initialize,	*/
/* (short font) the font to use for the title, (Boolean resizable) true	*/
/* if the window can be resized.					*/

EXTERN(void, sm_title_display,(Pane *p, char *title));	
/* show a new title		*/
/* Takes two parameters:  (Pane *p) the title pane and (char *title)	*/
/* the new title.  The title is displayed in the desired font.		*/

EXTERN(void, sm_title_active,(Pane *p, Boolean active));	
/* show/hide current title	*/
/* Takes two parameters:  (Pane *p) the title pane and (Boolean act)	*/
/* true if the window should be shown as active.			*/

#endif
