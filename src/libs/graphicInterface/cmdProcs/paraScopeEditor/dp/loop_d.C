/* $Id: loop_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*													*/
/*	ped_cp/PEditorCP/dp/loop_d.c					*/
/*													*/
/*	Simple Loop Dialogs for PED						*/
/*													*/
/*	Reverse											*/
/*	Adjust											*/
/*	Unswitch										*/
/*													*/
/*													*/
/************************************************************************/

#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/yes_no.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>
#include <libs/graphicInterface/oldMonitor/include/items/text.h>


/*--------------------------------------------------------------------

	reverse loop

*/

Boolean
reverse_handler(Dialog *di, Generic ALLH, Generic item_id)
{
	AllDia			*allh;

	allh = (AllDia *) ALLH;

	switch (item_id)
	{
	case DIALOG_CANCEL_ID:
		return (DIALOG_QUIT);

	case REVERSE_DO:
		doReverse((PedInfo)allh->ped);
		return (DIALOG_QUIT);
	}
	return (DIALOG_NOMINAL);
}

void
reverse_dialog_run(AllDia *allh)
{
	Dialog         *di;
	int type;

	allh->str = ssave("");
	di = dialog_create("Reverse Loop", reverse_handler, (dialog_helper_callback) 0, (Generic) allh,
			dialog_desc_group(DIALOG_VERT_CENTER, 2,
				item_button(REVERSE_DO, "Do Reverse", DEF_FONT_ID, false),
				allh->text = item_title(UNUSED, 
				"---------------------------------------------------\n\n\n",
				DEF_FONT_ID)));

	type = pt_reverse_estimate((PedInfo)allh->ped);
	if (type == REV_OKAY)
	{
		item_title_change(allh->text, "Loop may be reversed.");
		dialog_item_ability(di, REVERSE_DO, DIALOG_ENABLE);
	}
	else if (type == REV_UNABLE)
	{
		item_title_change(allh->text, 
			"Symbolic step and/or bounds \ninhibit loop reverse.");
		dialog_item_ability(di, REVERSE_DO, DIALOG_DISABLE);
	}
	else if (type == REV_ILLEGAL)
	{
		item_title_change(allh->text, "Dependences inhibit loop reverse.");
		dialog_item_ability(di, REVERSE_DO, DIALOG_DISABLE);
	}

	dialog_modal_run(di);
	dialog_destroy(di);
}



/*--------------------------------------------------------------------

	adjust loop bounds

*/

Boolean
adjust_handler(Dialog *di, Generic ALLH, Generic item_id)
{
	AllDia			*allh;
	Boolean			okay;

	allh = (AllDia *) ALLH;

	switch (item_id)
	{
	case DIALOG_CANCEL_ID:
		return (DIALOG_QUIT);

	case ADJUST_DO:
		doAdjust((PedInfo)allh->ped, allh->str);
		return (DIALOG_QUIT);

	case ADJUST_ARG:
		okay = true;
		if (allh->str[0] == '-')
		{
			if (!isdigit(allh->str[1]) || (atoi(allh->str+1) < 1))
				okay = false;
		}
		else if (!isdigit(allh->str[0]) || (atoi(allh->str) < 1))
		{
			okay = false;
		}

		if (okay)
		{
			item_title_change(allh->text, "");
			dialog_item_ability(di, ADJUST_DO, DIALOG_ENABLE);
		}
		else
		{
			item_title_change(allh->text, 
			"Loop adjust value must be a nonzero integer.");
			dialog_item_ability(di, ADJUST_DO, DIALOG_DISABLE);
		}

		break;
	}
	return (DIALOG_NOMINAL);
}

void
adjust_dialog_run(AllDia *allh)
{
	Dialog         *di;

	allh->str = ssave("");
	di = dialog_create("Adjust Loop Bounds", adjust_handler, 
			(dialog_helper_callback) 0, (Generic) allh,
			dialog_desc_group(DIALOG_VERT_CENTER, 3,
				item_button(ADJUST_DO, "Do Adjust", DEF_FONT_ID, false),
				item_text(ADJUST_ARG, "Adjust bounds by ", 
				DEF_FONT_ID, &(allh->str), 12),
				allh->text = item_title(UNUSED, 
				"---------------------------------------------------\n\n\n",
				DEF_FONT_ID)));

	item_title_change(allh->text, " ");
	dialog_item_ability(di, ADJUST_DO, DIALOG_DISABLE);
	dialog_modal_run(di);
	dialog_destroy(di);
}



/*--------------------------------------------------------------------

	loop unswitching

*/

Boolean
unswitch_handler(Dialog *di, Generic ALLH, Generic item_id)
{
	AllDia			*allh;

	allh = (AllDia *) ALLH;

	switch (item_id)
	{
	case DIALOG_CANCEL_ID:
		return (DIALOG_QUIT);

	case UNSWITCH_DO:
		doUnswitch((PedInfo)allh->ped);
		return (DIALOG_QUIT);
	}
	return (DIALOG_NOMINAL);
}

void
unswitch_dialog_run(AllDia *allh)
{
	Dialog         *di;

	allh->str = ssave("");
	di = dialog_create("Unswitch Loop", unswitch_handler, (dialog_helper_callback) 0, 
				(Generic) allh,
				dialog_desc_group(DIALOG_VERT_CENTER, 2,
				item_button(UNSWITCH_DO, "Do Unswitch", DEF_FONT_ID, false),
				allh->text = item_title(UNUSED, 
				"---------------------------------------------------\n\n\n",
				DEF_FONT_ID)));

	switch (pt_unswitch_test((PedInfo)allh->ped))
	{
		case UNSWITCH_OKAY:
/*
			item_title_change(allh->text, 
				"Loop unswitching allowed,\nIF statement found.");
			dialog_item_ability(di, UNSWITCH_DO, DIALOG_ENABLE);
*/
			item_title_change(allh->text, 
				"Loop unswitching not yet implemented.");
			dialog_item_ability(di, UNSWITCH_DO, DIALOG_DISABLE);
			break;

		case UNSWITCH_NOIF:
			item_title_change(allh->text, 
				"Loop unswitching inapplicable,\nno IF statement found.");
			dialog_item_ability(di, UNSWITCH_DO, DIALOG_DISABLE);
			break;

		case UNSWITCH_IFPLUS:
			item_title_change(allh->text, 
				"Loop unswitching illegal,\nmultiple statements in loop.");
			dialog_item_ability(di, UNSWITCH_DO, DIALOG_DISABLE);
			break;

	}

	dialog_modal_run(di);
	dialog_destroy(di);
}

