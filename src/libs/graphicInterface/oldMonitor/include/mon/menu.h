/* $Id: menu.h,v 1.6 1997/03/11 14:33:18 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/* 									*/
/*		 	         menu.h					*/
/*		          Menu use include file				*/
/* 									*/
/* Menus allow the user to select from a list of items.  Menus may be	*/
/* reusable or disposable.  Reusable menus are much more flexible but	*/
/* disposable menus are easier to use.  Menus may also either be binding*/
/* or non-binding with binging meaning that the user must select an	*/
/* entry from the menu.							*/
/* 									*/
/* Reusable menus may have choices (buttons) whose options (titles) can	*/
/* change and have the ability to set the selectedness/selectability	*/
/* status of each choice.  Reusable menus also provide a help facility	*/
/* for each of the option for a choice as well as a client-defined	*/
/* return codes.  Finally, reusable menus provide for the display of a	*/
/* keyboard shorthand to be displayed with menu choices.  The shorthands*/
/* are accepted when the menu is up, too.				*/
/* 									*/
/************************************************************************/

#ifndef menu_h
#define menu_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

/************************ REUSABLE MENUS ********************************/

	/* REUSABLE MENU BUILDING STRUCTURES */

typedef struct option_def {		/* OPTION DEFINITION STRUCTURE	*/
	char		*displayed_text;/* the displayed option text	*/
	char		*help_text;	/* the help request text	*/
					/* use (char *) 0 for no help	*/
			} anOptionDef;

typedef	struct choice_def {		/* CHOICE DEFINITION STRUCTURE	*/
	Generic		id;		/* the choice return value	*/
					/* must be unique within a menu	*/
	KbChar		kb_code;	/* kb code which selects this	*/
					/* use toKbChar(0) if no kb	*/
	short		num_options;	/* number of entries above	*/
	anOptionDef	*option_list;	/* possible options for choice	*/
			} aChoiceDef;

typedef	struct	menu_def {		/* MENU DEFINITION STRUCTURE	*/
	char		*title;		/* title of the menu		*/
	Point		size;		/* size of the menu in choices	*/
	Generic		def;		/* default (non-selection value)*/
					/* must be unique from other ids*/
	aChoiceDef	*choice_list;	/* list of choices (row major)	*/
			} aMenuDef;

typedef	struct	_menu	aMenu;		/* menu definition		*/

#define		NULL_MENU ((aMenu *) 0)	/* the null menu		*/

	/* REUSABLE MENU CALLS */

EXTERN(aMenu*, create_menu, (aMenuDef *md));
/* create a menu for later use	*/
/* Takes one parameter (aMenuDef *md) the menu definition.  Returns a	*/
/* handle to the menu for later use.  Note: the menu should be freed	*/
/* with destroy_menu() below.						*/

EXTERN(aMenu*, make_menu, (char *title, Generic def, 
                            int rows, int columns, ...));
/* alternate menu creation	*/
/* Takes an argument list.  Returns a handle to the menu for later use.	*/
/* Note:  the menu should be free with destroy_menu() below.  This	*/
/* function will be prefered to the create_menu() function above if the	*/
/* menu specification is completely fixed.				*/
/* The menu is specified by arguments as follows:			*/
/*	(char    *title)	title of the menu			*/
/*	(Generic    def)	The default (non-selection) menu return	*/
/* 				value (must also be unique)		*/
/*	(int       rows)	# of rows of choices			*/
/*	(int       cols)	# of columns of choices			*/
/*	The choices are specified in row major order:			*/
/*		(Generic      id) 	id (return value) of the choice	*/
/* 					(must be unique)		*/
/* 		(KbChar	 kb_code)	The kb code which selects this	*/
/* 					entry or toKbChar(0) for none.	*/
/*		(int num_options)	# of options for this choice	*/
/*		The options for a choice are given in sequence:		*/
/*			(char   *displayed_text)	Option title	*/
/*			(char   *help_text)		Help text or 0	*/

EXTERN(void, destroy_menu, (aMenu *menu));
/* destroy a menu		*/
/* Takes one parameter (aMenu *menu) the previously created menu.  The	*/
/* information associated with the menu is freed.			*/

EXTERN(Generic, select_from_menu, (aMenu *menu,
 Boolean binding));
/* ask the user to make a choice*/
/* Takes two parameters (aMenu *menu) the menu to choose from, and	*/
/* (Boolean binding) true if one of the menu choices must be made and	*/
/* false if the user may select off the menu.  The id associated with	*/
/* the choice is returned.  Note that a binding menu is potentially	*/
/* dangerous with no choices or all choices unselectable.		*/

EXTERN(void, modify_menu_choice, (aMenu *menu, Generic id,
 Boolean selectable, Boolean selected));
/* change status of a choice	*/
/* Takes four parameters, (aMenu *menu) the menu handle, (Generic id)	*/
/* the id of the choice to modify, (Boolean selected) true if the option*/
/* is selected, and (Boolean selectable) true if the option is		*/
/* selectable.  These values are false and true initially.		*/

EXTERN(void, default_menu_choices, (aMenu *menu));
/* reset all choice statuses	*/
/* Takes one parameter, (aMenu *menu) the menu to set defaults.  Set	*/
/* all choices to be the default (selectable, unselected) status.  Note:*/
/* this is done automatically on creation.				*/

EXTERN(void, switch_menu_option, (aMenu *menu, Generic id,
 short num));
/* changes a choice's option	*/
/* Takes three parameters, (aMenu *menu) the menu handle, (Generic id)	*/
/* the id of the choice to modify, and (short num) the new option index	*/
/* for the choice.  The original index is 0 (first option).  Option	*/
/* indices which are out of range cause death.				*/

EXTERN(void, toggle_menu_option, (aMenu *menu, Generic id));
/* toggles a choice's option	*/
/* Takes two parameters, (aMenu *menu) the menu handle, and		*/
/* (Generic id) the id of the choice to toggle.  The next defined option*/
/* (modulo the number of options) is made current.  The starting option	*/
/* is 0, the first defined option for a choice.				*/


/********************* DISPOSABLE MENU INFORMATION **********************/

EXTERN(Point, general_menu_select, (char *title, Point size,
 char *labels[], Boolean binding));
/* Takes four parameters:  (char *title) the title of the menu,		*/
/* (Point dimensions) the number of buttons in either direction,	*/
/* (char *labels[]) the one dimensional list of names of the selectable	*/
/* items, and (Boolean binding) true if an item must be selected.	*/
/* Returns the item number selected or (UNUSED, UNUSED).		*/

EXTERN(short, menu_select, (char *title, short num, char *labels[]));
/* Takes three parameters:  (char *title) the title of the menu,	*/
/* (short num) the number of items in the list, and (char *labels[]) the*/
/* list of item labels.  Returns the label selected or UNUSED if nothing*/
/* was selected.							*/

EXTERN(short, binding_menu_select, (char *title, short num,
 char *labels[]));
/* Takes three parameters:  (char *title) the title of the menu,	*/
/* (short num) the number of items in the list, and (char *labels[]) the*/
/* list of item labels.  Returns the label selected.  A label must be	*/
/* selected.								*/

#endif
