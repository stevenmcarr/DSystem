/* $Id: find.C,v 1.1 1997/06/25 14:47:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*				    find.c				*/
/*			      find/replace dialog			*/
/*									*/
/************************************************************************/

#include <stdio.h>

#include <libs/graphicInterface/oldMonitor/include/mon/dialog_def.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/message.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/find.h>
#include <libs/graphicInterface/oldMonitor/include/items/text.h>
#include <libs/graphicInterface/oldMonitor/include/items/button.h>
#include <libs/graphicInterface/oldMonitor/include/items/radio_btns.h>
#include <libs/graphicInterface/oldMonitor/include/items/check_box.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>


    /* item id's */
#define	F_FIND_STRING		1	/* find dialog: find string	*/
#define	F_FIND_BUTTON		2	/* find dialog: find button	*/
#define	F_DIRECTION		3	/* find dialog: direction	*/
#define	F_CASE_FOLD		4	/* find dialog: case fold	*/

#define	R_FIND_STRING		5	/* replace dialog: find string	*/
#define	R_FIND_BUTTON		6	/* replace dialog: find button	*/
#define	R_DIRECTION		7	/* replace dialog: direction	*/
#define	R_CASE_FOLD		8	/* replace dialog: case fold	*/
#define	R_REPLACE_STRING	9	/* replace dialog: repl. string	*/
#define	R_REPLACE_BUTTON	10	/* replace dialog: repl. button	*/
#define	R_REPLACE_REMAINDER	11	/* replace dialog: repl. remain.*/
#define	R_REPLACE_ALL		12	/* replace dialog: repl. all	*/

struct frdia;

typedef FUNCTION_POINTER(Boolean,FinderFunc,(Generic,struct frdia *,char *,
					     Boolean,Boolean));
typedef FUNCTION_POINTER(void,ReplacerFunc,(Generic,struct frdia*,char*,
					    Boolean,char*));
typedef FUNCTION_POINTER(int,GlobalReplacerFunc,(Generic,struct frdia*,char*,
						 Boolean,Boolean,Boolean,char*));

struct	frdia	{			/* find/replace dialog structure*/
    char	*find_string;		/* the find string		*/
    char	*replace_string;	/* the replace string		*/
    Boolean	can_replace;		/* true if can replace now	*/
    Generic	direction;		/* the direction		*/
    Boolean	case_fold;		/* case fold flag		*/
    FinderFunc		finder;			/* the finder routine		*/
    ReplacerFunc		replacer;		/* the replacer routine		*/
    GlobalReplacerFunc		global_replacer;	/* the globalreplacer routine	*/
    Generic	owner;			/* the owner id			*/
    Dialog	*find_dialog;		/* the find dialog		*/
    Dialog	*replace_dialog;	/* the replace dialog		*/
		};


/* Handle help to the find/replace dialog.				*/
/*ARGSUSED*/
static
void
find_dialog_helper(Dialog *di, aFRDia *frd, Generic item)
{
    switch(item)
    {/* figure what to do based on the item of the help request */
	case F_FIND_STRING:
	case R_FIND_STRING:
	    message("Edit the string to be found here.");
	    break;

	case F_DIRECTION:
	case R_DIRECTION:
	    message("Select the direction of the search.");
	    break;

	case F_CASE_FOLD:
	case R_CASE_FOLD:
	    message("Select whether case will be significant in the search.");
	    break;

	case F_FIND_BUTTON:
	case R_FIND_BUTTON:
	    message("Find the next occurence of the string to be found.");
	    break;

	case R_REPLACE_STRING:
	    message("Edit the string to be replaced here.");
	    break;

	case R_REPLACE_BUTTON:
	    message("Replace the currently selected string with the string for replacement.");
	    break;

	case R_REPLACE_REMAINDER:
	    message("Replace all occurences of the find string with the replace string\nfrom the current position to the end (depending on the direction).");
	    break;

	case R_REPLACE_ALL:
	    message("Replace all occurences of the find string with the replace string.");
	    break;

	default:
	    message("Use this dialog to find or replace text.");
	    break;
    }
}


/* Handle changes to the find/replace dialog.				*/
/*ARGSUSED*/
static
Boolean
find_dialog_handler(Dialog *di, aFRDia *frd, Generic item)
{
int		num;			/* number of entries replaced	*/
char		mes[50];		/* output message string	*/
Boolean		success;		/* find succeeded		*/

    switch(item)
    {/* figure what to do based on the item modified */
	case DIALOG_CANCEL_ID:	    /* cancel action (quit) */
	    return DIALOG_QUIT;

	case F_FIND_STRING:
	case R_FIND_STRING:	    /* propagate & check the value */
            if (di == frd->find_dialog)
	    {/* modify the replace dialog */
		dialog_item_modified(frd->replace_dialog, R_FIND_STRING);
	    }
	    else
	    {/* modify the find dialog */
		dialog_item_modified(frd->find_dialog,    F_FIND_STRING);
	    }
	    frd->can_replace = false;
	    dialog_item_ability(
		frd->replace_dialog,
		R_REPLACE_BUTTON,
		DIALOG_DISABLE
	    );

	    dialog_item_ability(
		    frd->find_dialog,
		    F_FIND_BUTTON,
		    (*frd->find_string) ? DIALOG_ENABLE : DIALOG_DISABLE
	    );
	    dialog_item_ability(
		    frd->replace_dialog,
		    R_FIND_BUTTON,
		    (*frd->find_string) ? DIALOG_ENABLE : DIALOG_DISABLE
	    );
	    break;

	case R_REPLACE_STRING:	    /* nothing */
	    break;

	case F_DIRECTION:
	case R_DIRECTION:	    /* propagate the value */
            if (di == frd->find_dialog)
	    {/* modify the replace dialog */
		dialog_item_modified(frd->replace_dialog, R_DIRECTION);
	    }
	    else
	    {/* modify the find dialog */
		dialog_item_modified(frd->find_dialog,    F_DIRECTION);
	    }
	    break;

	case F_CASE_FOLD:
	case R_CASE_FOLD:	    /* propagate the value */
            if (di == frd->find_dialog)
	    {/* modify the replace dialog */
		dialog_item_modified(frd->replace_dialog, R_CASE_FOLD);
	    }
	    else
	    {/* modify the find dialog */
		dialog_item_modified(frd->find_dialog,    F_CASE_FOLD);
	    }
	    break;

	case DIALOG_DEFAULT_ID:
	case F_FIND_BUTTON:
	case R_FIND_BUTTON:	    /* perform the find operation */
	    success = (frd->finder)(
		    frd->owner,
		    frd,
		    frd->find_string,
		    BOOL(frd->direction),
		    frd->case_fold
	    );
	    if (success && (frd->replacer != (ReplacerFunc) 0))
	    {/* turn on the replace capability */
		frd->can_replace = true;
		dialog_item_ability(
			frd->replace_dialog,
			R_REPLACE_BUTTON,
			DIALOG_ENABLE
		);
	    }
	    else if (NOT(success))
	    {/* put out message */
	        dialog_message(di, "Not found.");
	    }
	    break;

	case R_REPLACE_BUTTON:	    /* do a single replacement */
	    (frd->replacer)(
		    frd->owner,
		    frd,
		    frd->find_string,
		    frd->case_fold,
		    frd->replace_string
	    );
	    break;

	case R_REPLACE_REMAINDER:   /* do the replacement */
	case R_REPLACE_ALL:	    /* do the replacement */
	    num = (frd->global_replacer)(
		    frd->owner,
		    frd,
		    frd->find_string,
		    BOOL(item == R_REPLACE_ALL),
		    BOOL(frd->direction),
		    frd->case_fold,
		    frd->replace_string
	    );
	    (void) sprintf(mes, "Replaced %d occurences.", num);
	    dialog_message(frd->replace_dialog, mes);
	    break;
    }
    return DIALOG_NOMINAL;
}


/* Create a find/replace dialog.					*/
aFRDia *
find_dialog_create(char *find_title, char *replace_title, char *find_string, 
                   char *replace_string, Boolean direction, Boolean case_fold,
		   FinderFunc finder, ReplacerFunc replacer, GlobalReplacerFunc global_replacer, Generic owner)
{
aFRDia		*frd;			/* the find/replace dialog	*/

    /* create and initialize the dialog */
	frd = (aFRDia *) get_mem(sizeof(aFRDia), "find/replace dialog");
	frd->find_string     = ssave(find_string);
	frd->replace_string  = ssave(replace_string);
	frd->direction	     = (Generic) direction;
	frd->case_fold       = case_fold;
	frd->finder          = finder;
	frd->replacer        = replacer;
	frd->global_replacer = global_replacer;
	frd->owner           = owner;
	frd->find_dialog     = dialog_create(
		find_title, 
		(dialog_handler_callback)find_dialog_handler,
		(dialog_helper_callback)find_dialog_helper,
		(Generic) frd,
		dialog_desc_group(
			DIALOG_VERT_CENTER,
			2,
			item_text(
				F_FIND_STRING,
				"Find:",
				DEF_FONT_ID,
				&frd->find_string,
				30
			),
			dialog_desc_group(
				DIALOG_HORIZ_BOTTOM,
				3,
				item_button(
					F_FIND_BUTTON,
					"find",
					DEF_FONT_ID,
					true
				),
				item_radio_buttons(
					F_DIRECTION,
					DIALOG_NO_TITLE,
					DEF_FONT_ID,
					&frd->direction,
					2, 1,
					    "forward ", (Generic) FRD_FORWARD,
					    "backward", (Generic) FRD_BACKWARD
				),
				item_check_box(
					F_CASE_FOLD,
					"case fold",
					DEF_FONT_ID,
					&frd->case_fold
				)
			)
		)
	);
	frd->replace_dialog  = dialog_create(
		replace_title, 
		(dialog_handler_callback)find_dialog_handler,
		(dialog_helper_callback)find_dialog_helper,
		(Generic) frd,
		dialog_desc_group(
			DIALOG_VERT_CENTER,
			4,
			item_text(
				R_FIND_STRING,
				"Replace:",
				DEF_FONT_ID,
				&frd->find_string,
				30
			),
			item_text(
				R_REPLACE_STRING,
				"With:   ",
				DEF_FONT_ID,
				&frd->replace_string,
				30
			),
			dialog_desc_group(
				DIALOG_HORIZ_BOTTOM,
				3,
				item_button(
					R_FIND_BUTTON,
					"find",
					DEF_FONT_ID,
					true
				),
				item_radio_buttons(
					R_DIRECTION,
					DIALOG_NO_TITLE,
					DEF_FONT_ID,
					&frd->direction,
					2, 1,
					    "forward ", (Generic) FRD_FORWARD,
					    "backward", (Generic) FRD_BACKWARD
				),
				item_check_box(
					R_CASE_FOLD,
					"case fold",
					DEF_FONT_ID,
					&frd->case_fold
				)
			),
			dialog_desc_group(
				DIALOG_HORIZ_BOTTOM,
				3,
				item_button(
					R_REPLACE_BUTTON,
					"replace\ncurrent",
					DEF_FONT_ID,
					false
				),
				item_button(
					R_REPLACE_REMAINDER,
					"replace\nremainder",
					DEF_FONT_ID,
					false
				),
				item_button(
					R_REPLACE_ALL,
					"replace\nall",
					DEF_FONT_ID,
					false
				)
			)
		)
	);

    /* set initial conditions */
	frd->can_replace = false;
	dialog_item_ability(
		frd->replace_dialog,
		R_REPLACE_BUTTON,
		DIALOG_DISABLE
	);
        if (!*frd->find_string)
	{/* cannot search for an empty string */
	    dialog_item_ability(
		    frd->find_dialog,
		    F_FIND_BUTTON,
		    DIALOG_DISABLE
	    );
	    dialog_item_ability(
		    frd->replace_dialog,
		    R_FIND_BUTTON,
		    DIALOG_DISABLE
	    );
	}
        if (frd->global_replacer == (GlobalReplacerFunc) 0)
	{/* cannot do global replaces */
	    dialog_item_ability(
		    frd->replace_dialog,
		    R_REPLACE_REMAINDER,
		    DIALOG_DISABLE
	    );
	    dialog_item_ability(
		    frd->replace_dialog,
		    R_REPLACE_ALL,
		    DIALOG_DISABLE
	    );
	}
	
    return (frd);
}


/* Run a find dialog.							*/
void
find_dialog_run_find(aFRDia *frd)
{
    dialog_modeless_show(frd->find_dialog);
}


/* Run a replace dialog.						*/
void
find_dialog_run_replace(aFRDia *frd)
{
    dialog_modeless_show(frd->replace_dialog);
}


/* Set the important values by a client in a find/replace dialog.	*/
void
find_dialog_set_values(aFRDia *frd, char *find_string, char *replace_string, 
                       Boolean direction, Boolean case_fold)
{
    if (strcmp (find_string, frd->find_string) != 0)
    {/* modify the find_string */
    	free_mem ((void*) frd->find_string);
	frd->find_string     = ssave(find_string);
    	dialog_item_modified (frd->find_dialog, F_FIND_STRING);
    	dialog_item_modified (frd->replace_dialog, R_FIND_STRING);
    }    
    if (strcmp (replace_string, frd->replace_string) != 0)
    {
    	free_mem ((void*) frd->replace_string);
	frd->replace_string  = ssave(replace_string);
    	dialog_item_modified (frd->replace_dialog, R_REPLACE_STRING);
    }
    if (direction != BOOL(frd->direction))
    {
    	frd->direction = (Generic) direction;
    	dialog_item_modified (frd->find_dialog, F_DIRECTION);
    	dialog_item_modified (frd->replace_dialog, R_DIRECTION);
    }
    if (case_fold != frd->case_fold)
    {
	frd->case_fold = case_fold;
    	dialog_item_modified (frd->find_dialog, F_CASE_FOLD);
    	dialog_item_modified (frd->replace_dialog, R_CASE_FOLD);
    }
}

/* Get the important values from a find/replace dialog.			*/
void
find_dialog_get_values(aFRDia *frd, char **find_string, char **replace_string, 
                       Boolean *direction, Boolean *case_fold)
{
    *find_string    = frd->find_string;
    *replace_string = frd->replace_string;
    *direction      = BOOL(frd->direction);
    *case_fold      = frd->case_fold;
}


/* Note that the found string is no longer found.			*/
void
find_dialog_dirty(aFRDia *frd)
{
    if( frd->can_replace )
    {/* turn off the replace button */
	dialog_item_ability(
		frd->replace_dialog,
		R_REPLACE_BUTTON,
		DIALOG_DISABLE
	);
	frd->can_replace = false;
    }
}


/* Destroy a find/replace dialog.					*/
void
find_dialog_destroy(aFRDia *frd)
{
    dialog_destroy(frd->find_dialog);
    dialog_destroy(frd->replace_dialog);
    free_mem((void*) frd->find_string);
    free_mem((void*) frd->replace_string);
    free_mem((void*) frd);
}
