/* $Id: skew_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/****************************************************************/
/*													*/
/*	ped_cp/PEditorCP/dp/skew_d.c					*/
/*													*/
/*	Skew Dialog for PED								*/
/*													*/
/****************************************************************/

#include <ctype.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>
#include <libs/graphicInterface/oldMonitor/include/items/text.h>

/*--------------------------------------------------------------------

	loop skewing

*/

/* helper routine to check if a string is alpha-numeric 
(containing both characters and numbers) */

Boolean
skew_is_valid(char *str)
{
	if (*str == '-')
		str++;

	do
	{
		if (NOT(isdigit(*str++)))
			return false;
	}
	while (*str != '\0');

	return true;
}


/* modal dialog handler for the loop skewing dialog box */
Boolean
skew_handler(Dialog *di, Generic ALLH, Generic item_id)
{
	AllDia       *allh;
	PedInfo         ped;

	allh = (AllDia *) ALLH;
	ped = (PedInfo) (allh->ped);

	switch (item_id)
	{
	case DIALOG_CANCEL_ID:
		return DIALOG_QUIT;

	case SKEW_DEGREE:
		if (skew_is_valid(allh->str))
		{
			item_title_change(allh->text, allh->str2);
			dialog_item_ability(di, SKEW_ONE, DIALOG_ENABLE);
		}
		else
		{
			item_title_change(allh->text, "Skew degree must be an integer");
			dialog_item_ability(di, SKEW_ONE, DIALOG_DISABLE);
		}
		return DIALOG_NOMINAL;

	case SKEW_ONE:
		doSkew(ped, allh->str);
		PED_SELECTION(ped) = PED_SELECTED_LOOP(ped);
		return DIALOG_QUIT;

	default:
		return DIALOG_NOMINAL;
	}
}


/* create the skew dialog box and run it */
void
skew_dialog_run(AllDia *allh)
{
	Dialog         *di;
	char            tempstr[20];
	int             advised_degree;

	/* if we can get an estimate of how much to skew - use it */
	advised_degree = pt_skew_estimate((PedInfo)allh->ped);	

	if (advised_degree)
	{
		sprintf(tempstr, "%d", advised_degree);
		allh->str = ssave(tempstr);
	}
	else
	{
		allh->str = ssave("");
	}

	di = dialog_create(
                "Loop Skewing", 
                skew_handler, 
                (dialog_helper_callback)0, 
                (Generic)allh,
		dialog_desc_group(
                   DIALOG_VERT_CENTER, 
                   3,
                   item_button(
                      SKEW_ONE, 
                      "Do Skew", 
                      DEF_FONT_ID, 
                      false),
                   item_text(
                      SKEW_DEGREE, 
                      "Degree of skew  ", 
                      DEF_FONT_ID, 
                      &(allh->str), 
                      10),
                allh->text = item_title(
                                UNUSED, 
                                "__________________________________________________\n\n",
                                DEF_FONT_ID)));

	if (advised_degree)
	{
		allh->str2 = "Loop skew will expose parallelism";
		dialog_item_ability(di, SKEW_ONE, DIALOG_ENABLE);
	}
	else
	{
		allh->str2 = "Loop skew will not expose parallelism";
		dialog_item_ability(di, SKEW_ONE, DIALOG_DISABLE);
	}

	item_title_change(allh->text, allh->str2);
	dialog_modal_run(di);
	dialog_destroy(di);
}
