/* $Id: split_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/****************************************************************/
/*															*/
/*	ped_cp/PEditorCP/dp/split_d.c							*/
/*															*/
/*	Loop Split (Index Set Splitting) Dialog for PED			*/
/*															*/
/****************************************************************/
#include <ctype.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>
#include <libs/graphicInterface/oldMonitor/include/items/text.h>

/*--------------------------------------------------------------------

	loop splitting

*/

Boolean
split_handler(Dialog *di, Generic SMIH, Generic item_id)
{
	StripDia       *smih;

	smih = (StripDia *) SMIH;

	switch (item_id)
	{
	case DIALOG_CANCEL_ID:
		return (DIALOG_QUIT);

	case STEP_SIZE:
		/* make sure it's positive */
		if (isalpha(smih->step[0]))
		{		/* FUTURE: check if step is a declared variable */
			/* if so - dialog enable */
			/* else return an error */
			item_title_change(smih->text, "Invalid iteration index- must be a constant integer. ");
			dialog_item_ability(di, DO_STRIP, DIALOG_DISABLE);
		}
		else
		{
			item_title_change(smih->text, "");
			dialog_item_ability(di, DO_STRIP, DIALOG_ENABLE);
		}
		return (DIALOG_NOMINAL);

	case DO_STRIP:
		doSplit((PedInfo) (smih->ped), smih->step);
		return (DIALOG_QUIT);
	}
	return (DIALOG_NOMINAL);
}

void
split_dialog_run(StripDia *smih)
{
	Dialog         *di;

	smih->step = ssave("");
	di = dialog_create("Split Loop", split_handler, (dialog_helper_callback) 0, (Generic) smih,
			   dialog_desc_group(DIALOG_VERT_CENTER, 3,
		      item_button(DO_STRIP, "Do Split", DEF_FONT_ID, false),
					     item_text(STEP_SIZE, "Iteration Index to Split at", DEF_FONT_ID, &(smih->step), 12),
					     smih->text = item_title(UNUSED, "---------------------------------------------------\n\n\n",
								  DEF_FONT_ID)
					     ));

	item_title_justify_left(smih->text);
	item_title_change(smih->text, " ");
	dialog_item_ability(di, DO_STRIP, DIALOG_DISABLE);
	dialog_modal_run(di);
	dialog_destroy(di);

}






