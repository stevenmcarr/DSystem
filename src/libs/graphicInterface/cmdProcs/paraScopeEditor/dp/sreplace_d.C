/* $Id: sreplace_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*													*/
/*	ped_cp/PEditorCP/dp/sreplace_d.c				*/
/*													*/
/*	Scalar Replacement Dialog for PED				*/
/*													*/
/*													*/
/************************************************************************/

#include <ctype.h>
#include <stdarg.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/yes_no.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>


/*--------------------------------------------------------------------

	Scalar Replacement

*/

Boolean
replace_s_handler(Dialog *di, Generic ALLH, Generic item_id)
{
	AllDia			*allh;

	allh = (AllDia *) ALLH;

	switch (item_id)
	{
	case DIALOG_CANCEL_ID:
		return (DIALOG_QUIT);

	case REPLACE_S_DO:
		doReplaceS((PedInfo)allh->ped);
		return (DIALOG_QUIT);
	}
	return (DIALOG_NOMINAL);
}

void
replace_s_dialog_run(AllDia *allh)
{
	Dialog         *di;
	char buffer[200];

	allh->str = ssave("");
	di = dialog_create("Scalar Replacement", replace_s_handler, (dialog_helper_callback) 0, (Generic) allh,
			dialog_desc_group(DIALOG_VERT_CENTER, 2,
				item_button(REPLACE_S_DO, "Do Replacement", DEF_FONT_ID, false),
				allh->text = item_title(UNUSED, 
				"---------------------------------------------------\n\n\n",
				DEF_FONT_ID)));

	switch (pt_rep_s_estimate((PedInfo)allh->ped, buffer))
	{
		case REP_S_BAD:
			item_title_change(allh->text, 
				ssave("Scalar replacement is not allowed."));
			dialog_item_ability(di, REPLACE_S_DO, DIALOG_DISABLE);
			break;

		case REP_S_VAR:
			item_title_change(allh->text, 
				ssave("Selection must be array reference."));
			dialog_item_ability(di, REPLACE_S_DO, DIALOG_DISABLE);
			break;

		case REP_S_INLOOP:
			item_title_change(allh->text, 
				ssave("Selection must be in current loop."));
			dialog_item_ability(di, REPLACE_S_DO, DIALOG_DISABLE);
			break;

		case REP_S_INNER:
			item_title_change(allh->text, 
				ssave("Scalar replacement is only supported\nfor innermost loop."));
			dialog_item_ability(di, REPLACE_S_DO, DIALOG_DISABLE);
			break;

		case REP_S_NONE:
			item_title_change(allh->text, 
				ssave("Scalar replacement is not profitable,\ninsufficient reuse opportunities exist."));
			dialog_item_ability(di, REPLACE_S_DO, DIALOG_DISABLE);
			break;

		case REP_S_GOOD:
			item_title_change(allh->text, ssave("Scalar replacement is profitable."));
			dialog_item_ability(di, REPLACE_S_DO, DIALOG_ENABLE);
			break;

	}

	dialog_modal_run(di);
	dialog_destroy(di);
}


