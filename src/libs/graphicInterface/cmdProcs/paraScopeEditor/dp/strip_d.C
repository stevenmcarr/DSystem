/* $Id: strip_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*													*/
/*	ped_cp/PEditorCP/dp/strip_d.c					*/
/*													*/
/*	Strip Mine Dialog for PED						*/
/*													*/
/************************************************************************/

#include <stdlib.h>
#include <ctype.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>
#include <libs/graphicInterface/oldMonitor/include/items/text.h>


/*--------------------------------------------------------------------

	strip mine

*/


void
strip_dialog_run(StripDia *smih)
  // StripDia       *smih;
{
	Dialog         *di;

	smih->step = ssave("");
	di = dialog_create("Strip Mine", strip_handler, (dialog_helper_callback) 0, (Generic) smih,
			   dialog_desc_group(DIALOG_VERT_CENTER, 3,
		   item_button(DO_STRIP, "Do Strip Mine", DEF_FONT_ID, false),
	    item_text(STEP_SIZE, "Step size", DEF_FONT_ID, &(smih->step), 12),
					     smih->text = item_title(UNUSED, "---------------------------------------------------\n\n\n",
								  DEF_FONT_ID)
					     ));

	item_title_justify_left(smih->text);
	item_title_change(smih->text, " ");
	dialog_item_ability(di, DO_STRIP, DIALOG_DISABLE);
	dialog_modal_run(di);
	dialog_destroy(di);

}

Boolean
strip_handler(Dialog *di, Generic SMIH, Generic item_id)
  // Dialog         *di;
  // Generic         SMIH;
  // Generic         item_id;
  
{
	StripDia       *smih;
	Boolean error;


	smih = (StripDia *) SMIH;

	switch (item_id)
	{
	case DIALOG_CANCEL_ID:
		return (DIALOG_QUIT);

	case STEP_SIZE:
		/* make sure step is positive, accept symbolic step size */

		error = false;

		/* get rid of blanks */
		while (smih->step[0] == ' ')
			strcpy(smih->step, smih->step+1);

		if ((smih->step[0] == '\0') || (smih->step[0] == '-'))
			error = true;
		if (isdigit(smih->step[0]) && (atoi(smih->step) <= 1))
			error = true;

		if (error)
		{
			dialog_item_ability(di, DO_STRIP, DIALOG_DISABLE);
			item_title_change(smih->text, 
				"Step size must be positive integer. ");
		}
		else
		{
			item_title_change(smih->text, "");
			dialog_item_ability(di, DO_STRIP, DIALOG_ENABLE);
		}
		return (DIALOG_NOMINAL);

	case DO_STRIP:
		doStripMine((PedInfo) (smih->ped), smih->step);
		return (DIALOG_QUIT);
	}

	return (DIALOG_NOMINAL);
}

