/* $Id: unroll_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*													*/
/*	ped_cp/PEditorCP/dp/unroll_d.c					*/
/*													*/
/*	Unroll Dialogs for PED							*/
/*													*/
/*	Unroll											*/
/*	Unroll and Jam									*/
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

	unroll loop

*/

Boolean
unroll_handler(Dialog *di, Generic ALLH, Generic item_id)
{
	AllDia			*allh;

	allh = (AllDia *) ALLH;

	switch (item_id)
	{
	case DIALOG_CANCEL_ID:
		return DIALOG_QUIT;

	case UNROLL_DO:
		doUnroll((PedInfo)allh->ped, allh->str);
		return DIALOG_QUIT;

	case UNROLL_ARG:
		if (!isdigit(allh->str[0]) || (atoi(allh->str) < 1))
		{
			item_title_change(allh->text, 
			"Unroll value must be\na positive integer.");
			dialog_item_ability(di, UNROLL_DO, DIALOG_DISABLE);
		}
		else
		{
			item_title_change(allh->text, "");
			dialog_item_ability(di, UNROLL_DO, DIALOG_ENABLE);
		}
		break;
	}
	return DIALOG_NOMINAL;
}

void
unroll_dialog_run(AllDia *allh)
{
	Dialog         *di;
	int type;

	allh->str = ssave("");
	di = dialog_create("Unroll Loop", unroll_handler, (dialog_helper_callback) 0, (Generic) allh,
			dialog_desc_group(DIALOG_VERT_CENTER, 3,
				item_button(UNROLL_DO, "Do Unroll", DEF_FONT_ID, false),
				item_text(UNROLL_ARG, "Unroll Degree ", 
				DEF_FONT_ID, &(allh->str), 12),
				allh->text = item_title(UNUSED, 
				"---------------------------------------------------\n\n\n",
				DEF_FONT_ID)));

	if ((type = pt_unroll_estimate((PedInfo)allh->ped, false)) < UNROLL_OK)
	{
		if (type == UNROLL_SYM_LOBOUND)
		{
			item_title_change(allh->text, 
			"Unroll of loops with complex\nlower bounds not yet supported.");
		}
		else if (type == UNROLL_SYM_STEP)
		{
			item_title_change(allh->text, 
			"Unroll of loops with complex\n step not yet supported.");
		}
		else if (type == UNROLL_IRREG_BOUND)
		{
			item_title_change(allh->text, "Irregular loop bounds\nprevent unroll.");
		}
		else if (type == UNROLL_NEG_STEP)
		{
			item_title_change(allh->text, 
			"Unroll of loops with negative\nstep not yet supported.");
		}

		dialog_item_ability(di, UNROLL_ARG, DIALOG_DISABLE);
	}
	else
	{
		item_title_change(allh->text, " ");
	}

	dialog_item_ability(di, UNROLL_DO, DIALOG_DISABLE);
	dialog_modal_run(di);
	dialog_destroy(di);
}


/*--------------------------------------------------------------------

	unroll and jam loop

*/

Boolean
unroll_jam_handler(Dialog *di, Generic ALLH, Generic item_id)
{
	AllDia			*allh;
	int unroll_deg;

	allh = (AllDia *) ALLH;

	switch (item_id)
	{
	case DIALOG_CANCEL_ID:
		return (DIALOG_QUIT);

	case UNROLL_JAM_DO:
		doUnrollJam((PedInfo)allh->ped, allh->str);
		return (DIALOG_QUIT);

	case UNROLL_JAM_ARG:
		unroll_deg = atoi(allh->str);

		if (isdigit(allh->str[0]) && (unroll_deg > 0) &&
			((allh->val == UNROLL_OK) || (unroll_deg <= allh->val)))
		{
			item_title_change(allh->text, " ");
			dialog_item_ability(di, UNROLL_JAM_DO, DIALOG_ENABLE);
		}
		else
		{
			item_title_change(allh->text, allh->str2);
			dialog_item_ability(di, UNROLL_JAM_DO, DIALOG_DISABLE);
		}

		break;
	}
	return DIALOG_NOMINAL;
}

void
unroll_jam_dialog_run(AllDia *allh)
{
	Dialog         *di;
	char buf[100];
	int type;

	allh->str = ssave("");
	di = dialog_create("Unroll and Jam", unroll_jam_handler, 
			(dialog_helper_callback) 0, (Generic) allh,
			dialog_desc_group(DIALOG_VERT_CENTER, 3,
				item_button(UNROLL_JAM_DO, "Do Unroll and Jam", 
					DEF_FONT_ID, false),
				item_text(UNROLL_JAM_ARG, "Unroll Degree ", 
				DEF_FONT_ID, &(allh->str), 12),
				allh->text = item_title(UNUSED, 
				"---------------------------------------------------\n\n\n",
				DEF_FONT_ID)));

	type = allh->val = pt_unroll_estimate((PedInfo)allh->ped, true);

	if (type < UNROLL_OK)
	{
		switch (type)
		{
		case UNROLL_SYM_LOBOUND:
			item_title_change(allh->text, 
	"Unroll and jam of loops with complex\nlower bounds not yet supported.");
			break;

		case UNROLL_SYM_STEP:
			item_title_change(allh->text, 
			"Unroll of and jam loops with complex\n step not yet supported.");
			break;

		case UNROLL_IRREG_BOUND:
			item_title_change(allh->text, 
			"Irregular loop bounds prevent\nunroll and jam.");
			break;

		case UNROLL_NEG_STEP:
			item_title_change(allh->text, 
			"Unroll and jam of loops with\nnegative step not yet supported.");
			break;

		case UNROLL_DEPS:
			item_title_change(allh->text, 
			"Dependences prevent unroll and jam.");
			break;

		case UNROLL_ONELOOP:
			item_title_change(allh->text, 
			"Only one loop found,\nuse unroll instead.");
			break;
		}

		dialog_item_ability(di, UNROLL_JAM_ARG, DIALOG_DISABLE);
	}

	else if (type > UNROLL_OK)
	{
		if (type == 1)
			sprintf(buf,
				"Dependences allow unroll and\njam value of 1 only.", 
				type);

		else
			sprintf(buf,
				"Dependences allow unroll and jam\nin range [1 to %d] only.", 
				type);

		allh->str2 = ssave(buf);
		item_title_change(allh->text, ssave(buf));
	}
	else		/* type == UNROLL_OK	*/
	{
		allh->str2 = ssave("Unroll value must be\na positive integer.");
		item_title_change(allh->text, " ");
	}

	dialog_item_ability(di, UNROLL_JAM_DO, DIALOG_DISABLE);
	dialog_modal_run(di);
	dialog_destroy(di);
}
