/* $Id: peel_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*													*/
/*	ped_cp/PEditorCP/dp/peel_d.c					*/
/*													*/
/*	Peel Dialog for PED								*/
/*													*/
/************************************************************************/

#include <stdlib.h>
#include <ctype.h>



#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>
#include <libs/graphicInterface/oldMonitor/include/items/text.h>

/*--------------------------------------------------------------------

	peel

*/


void
peel_dialog_run(StripDia *smih)
  // StripDia       *smih;
{
	Dialog         *di;
	char            tempstr[5];

	sprintf(tempstr, "%d", 1);
	smih->step = tempstr;
	di = dialog_create("Peel", peel_handler, (dialog_helper_callback) 0, (Generic) smih,
		dialog_desc_group(DIALOG_VERT_CENTER, 4,
		item_button(PEEL_FIRST, "Peel First Iterations", DEF_FONT_ID, false),
		item_button(PEEL_LAST, "Peel Last Iterations", DEF_FONT_ID, false),
		item_text(ITERS, "Number of Iterations ", DEF_FONT_ID, 
			&(smih->step), 12),
			smih->text = item_title(UNUSED, 
			"---------------------------------------------------\n\n\n",
			DEF_FONT_ID)));

	item_title_justify_left(smih->text);
	item_title_change(smih->text, " ");
	dialog_item_ability(di, PEEL_FIRST, DIALOG_ENABLE);
	dialog_item_ability(di, PEEL_LAST, DIALOG_ENABLE);
	dialog_modal_run(di);
	dialog_destroy(di);

}

Boolean
peel_handler(Dialog *di, Generic SMIH, Generic item_id)
  // Dialog         *di;
  // Generic         SMIH;
  // Generic         item_id;

{
	PedInfo         ped;

	StripDia       *smih;

	smih = (StripDia *) SMIH;

	switch (item_id)
	{
	case DIALOG_CANCEL_ID:
		return (DIALOG_QUIT);

	case ITERS:
		/* make sure it's non-negative  */
		if (isalpha(smih->step[0]))
		{		/* FUTURE: check if step is a declared variable */
			/* if so - dialog enable */
			/* else return an error */
			item_title_change(smih->text, "Invalid number of iterations \n - number of iterations must be non-negative. ");
			dialog_item_ability(di, PEEL_FIRST, DIALOG_DISABLE);
			dialog_item_ability(di, PEEL_LAST, DIALOG_DISABLE);
		}
		else if (atoi(smih->step) <= 0)
		{
			dialog_item_ability(di, PEEL_FIRST, DIALOG_DISABLE);
			dialog_item_ability(di, PEEL_LAST, DIALOG_DISABLE);
			if (smih->step[0] != '\0')
				item_title_change(smih->text, "Invalid number of iterations \n - number of iterations must be non-negative. ");
		}
		else
		{
			item_title_change(smih->text, "");
			dialog_item_ability(di, PEEL_FIRST, DIALOG_ENABLE);
			dialog_item_ability(di, PEEL_LAST, DIALOG_ENABLE);
		}
		return (DIALOG_NOMINAL);

	case PEEL_FIRST:
		ped = (PedInfo) (smih->ped);
		doPeelIterations(ped, true, smih->step);
		PED_SELECTION(ped) = PED_SELECTED_LOOP(ped);
		return (DIALOG_QUIT);

	case PEEL_LAST:
		ped = (PedInfo) (smih->ped);
		doPeelIterations(ped, false, smih->step);
		PED_SELECTION(ped) = PED_SELECTED_LOOP(ped);
		return (DIALOG_QUIT);
	}

	return (DIALOG_NOMINAL);
}

