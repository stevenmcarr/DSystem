/* $Id: button_sm.h,v 1.6 1997/03/11 14:33:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		       button_sm.h			*/
		/* 	    Button screen module include file.		*/
		/* 							*/
		/********************************************************/

/* This screen module is used for defining and using button layouts.  Button layouts	*/
/* are associated with a created button pane and multiple layouts may be defined for	*/
/* a button pane but only one is visible at a time.  Buttons within a layout have an id	*/
/* and multiple faces.  Faces are pairs of button lables and help strings.  The help	*/
/* string is given on a help reqest, and the id is returned with each selection.	*/
/* Each button in a layout may be modified by changing its selected or selectability	*/
/* status and the faces may be swapped within a button.					*/

/* Returned events are as follows:							*/
/* EVENT_SELECT  events are returned with the button coordinate in info and with the id	*/
/*		 parameter in msg.							*/
/* EVENT_HELP    events are returned for buttons with active faces without defined help	*/
/* 		 strings the info and msg parameters are defined as above.		*/

#ifndef button_sm_h
#define button_sm_h

#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>

EXTERN(short, sm_button_get_index, (void));
/* gets the button s. m. index number	*/
/* Takes no parameters.  Returns the installed screen module index number for the	*/
/* button screen module.								*/

typedef	struct	face_def {			/* BUTTON FACE DEFINTION STRUCTURE	*/
	char		*displayed_text;	/* the text displayed			*/
	char		*help_text;		/* the help text for the face		*/
			} aFaceDef;

typedef	struct	button_def {			/* BUTTON DEFINITION STRUCTURE		*/
	Generic		id;			/* the button id (returned in msg field)*/
						/* These must be unique within a layout.*/
	aFaceDef	*face_list;		/* the possible faces for the button	*/
	short		num_faces;		/* the number of entries in face_list	*/
			} aButtonDef;

typedef	struct	layout_def {			/* BUTTON LAYOUT DEFINITION		*/
	Point		size;			/* the size of the layout		*/
	aButtonDef	*buttons;		/* the buttons of the layout (row major)*/
			} aLayoutDef;

EXTERN(Point, sm_button_layout_size, (aLayoutDef *layout, short font));
/* Takes two parameters (aLayoutDef *layout) the btn layout definition and (short font)	*/
/* the font in which to display the buttons.  The pixel size of the minimum sized button*/
/* pane which will display the layout is returned.					*/

EXTERN(Generic, sm_button_layout_create, (Pane *p, aLayoutDef *layout, short font, Boolean fill));
/* Takes four parameters (Pane *p) the button pane, (aLayoutDef *layout) the btn layout	*/
/* definition, (short font) the font to use, and (Boolean fill) true if the buttons are	*/
/* to fill the entire pane [cp's should use false for this parameter].  The button	*/
/* layout is created, but not displayed, and its handle is returned.			*/

#define	NULL_LAYOUT	0			/* the empty button layout		*/

EXTERN(void, sm_button_layout_destroy, (Pane *p, Window *layout));
/* Takes two parameters (Pane *p) the button pane and (Generic layout) the previously	*/
/* created layout to destroy.  Note that button layouts need not be destroyed by the	*/
/* client as they are destroyed by the pane when its window is destroyed.  The call is	*/
/* provided so that clients who create button layouts without bound can free the memory	*/
/* associated with an old layout.							*/

EXTERN(void, sm_button_layout_show, (Pane *p, Window *layout));
/* Takes two parameters (Pane *p) the button pane, (Generic layout) the previously 	*/
/* created layout (or NULL_LAYOUT for no buttons).  The new layout is shown in the	*/
/* button pane.  The previous layout is hidden in the process.				*/

EXTERN(Boolean, sm_button_visible, (Pane *p));
/* Takes one parameter (Pane *p) the button pane.  Returns true if the current layout	*/
/* is completely visible.								*/

EXTERN(void, sm_button_modify_all, (Pane *p, Window *layout, Boolean selected, Boolean selectable));
/* Takes four parameters (Pane *p) the button pane, (Generic layout) the button layout	*/
/* to modify, (Boolean selected) the new selected status of all buttons, and (Boolean	*/
/* selectable) the new selectablity status of all buttons.				*/

EXTERN(void, sm_button_modify_button, (Pane *p, Window *layout, Generic id, Boolean selected, Boolean selectable));
/* Takes five parameters (Pane *p) the button pane, (Generic layout) the button layout	*/
/* to modify, (Generic id) the id of the button to modify, (Boolean selected) true if	*/
/* the button is selected, and (Boolean selectable) true if the button is selectable.	*/
/* The layout is modified accordingly.  The default values are "selectable" and "not	*/
/* selected".  A button which is not selectable will not return EVENT_SELECTED events.	*/

EXTERN(void, sm_button_switch_face, (Pane *p, Window *layout, Generic id, short num));
/* Takes four parameters (Pane *p) the button pane, (Generic layout) the layout which	*/
/* contains the button, (Generic id) the id of the button, and (short num) the defined	*/
/* face index to display in this button.  The default value is 0 (first defined face).	*/

EXTERN(void, sm_button_toggle_face, (Pane *p, Window *layout, Generic id));
/* Takes three parameters (Pane *p) the button pane, (Generic layout) the layout which	*/
/* contains the button, and (Generic id) the id of the button.  The next defined face	*/
/* (modulo the number of faces) for the button is put into place.  The starting value	*/
/* is zero (the first defined face).							*/


/******** THE FOLLOWING CALLS ARE OBSOLETE--DO NOT USE THEM FOR NEW DEVELOPMENT *********/
/**************** MIXING OLD AND NEW CALLS WILL GIVE UNEXPECTED RESULTS *****************/

EXTERN(Point, sm_button_pane_size, (Point dim, char **labels, short font));
/* Takes three parameters (Point dim) the button layout dimensions, (char *labels[])	*/
/* the labels for the buttons, and (short font) the font to use.  Returns the minimum   */
/* size in pixels of a pane necessary to show all of the buttons.			*/

EXTERN(void, sm_button_create_btns, (Pane *p, Point dim, char **labels, short font, Boolean fill));
/* Takes five parameters (Pane *p) the pane to be operated on, (Point dim) the number	*/
/* number of buttons to create in each direction, (char *labels[]) the (one dimensional)*/
/* list of button labels in row major order, (short font) the font that the buttons	*/
/* are to be shown in, and (Boolean fill) true if the buttons are to fill the entire	*/
/* pane and false if the buttons should be of minimum size.				*/

#endif
