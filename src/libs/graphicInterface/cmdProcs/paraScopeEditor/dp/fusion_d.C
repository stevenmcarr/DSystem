/* $Id: fusion_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*													*/
/*	ped_cp/PEditorCP/dp/fusion_d.c					*/
/*													*/
/*	Loop Fusion Dialog for PED						*/
/*													*/
/*													*/
/*													*/
/************************************************************************/

#include <ctype.h>
#include <stdarg.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/yes_no.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>


/*--------------------------------------------------------------------

	loop fusion

*/


Boolean
fusion_handler(Dialog *di, Generic ALLH, Generic item_id)
{
	AllDia			*allh;
	PedInfo ped;

	allh = (AllDia *) ALLH;
	ped = (PedInfo) allh->ped;

	switch (item_id)
	{
	case DIALOG_CANCEL_ID:
		return (DIALOG_QUIT);

	case FUSE_DO:
		doFusion(ped); /*,PED_SELECTED_LOOP(ped));*/  /*function takes 1 arg only!*/
		return (DIALOG_QUIT);
	}
	return (DIALOG_NOMINAL);
}

void
fusion_dialog_run(AllDia *allh)
{
	Dialog         *di;
	int ret;
	PedInfo ped;

	ped = (PedInfo) allh->ped;

	allh->str = ssave("");
	di = dialog_create("Loop Fusion", fusion_handler, 
			(dialog_helper_callback) 0, (Generic) allh,
			dialog_desc_group(DIALOG_VERT_CENTER, 2,
				item_button(FUSE_DO, "Do Fusion", DEF_FONT_ID, false),
				allh->text = item_title(UNUSED, 
				"---------------------------------------------------\n\n\n\n\n",
				DEF_FONT_ID)));


	switch (pt_fuse_test(ped,PED_SELECTED_LOOP(ped)))
	{
		case FUSE_LOOP:
			item_title_change(allh->text, ssave("Loop fusion inapplicable, \nnext statement is not a loop."));
			dialog_item_ability(di, FUSE_DO, DIALOG_DISABLE);
			break;

		case FUSE_BOUND:
			item_title_change(allh->text, ssave("Loop fusion disabled, \nloop bounds differ."));
			dialog_item_ability(di, FUSE_DO, DIALOG_DISABLE);
			break;

		case FUSE_STEP:
			item_title_change(allh->text, ssave("Loop fusion disabled, \nloop steps differ."));
			dialog_item_ability(di, FUSE_DO, DIALOG_DISABLE);
			break;

		case FUSE_DEP_ILL:
			item_title_change(allh->text, ssave("Dependences prevent loop fusion."));
			dialog_item_ability(di, FUSE_DO, DIALOG_DISABLE);
			break;

		case FUSE_OK_CARRY:
			item_title_change(allh->text, ssave("Loop fusion is legal. \nLoop-carried dependences will be introduced."));
			dialog_item_ability(di, FUSE_DO, DIALOG_ENABLE);
			break;

		case FUSE_OK:
			item_title_change(allh->text, ssave("Loop fusion is legal."));
			dialog_item_ability(di, FUSE_DO, DIALOG_ENABLE);
			break;
	}

	dialog_modal_run(di);
	dialog_destroy(di);
}


