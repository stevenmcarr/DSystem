/* $Id: align_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*													*/
/*	ped_cp/PEditorCP/dp/align_d.c					*/
/*													*/
/*	Alignment Dialog for PED						*/
/*													*/
/*													*/
/************************************************************************/

#include <stdlib.h>
#include <ctype.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/oldMonitor/include/items/text.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/yes_no.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/align.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>

/*--------------------------------------------------------------------

	align loop

*/

Boolean
align_handler(Dialog *di, Generic ALLH, Generic item_id)
{
	AllDia			*allh;

	allh = (AllDia *) ALLH;

	switch (item_id)
	{
	case DIALOG_CANCEL_ID:
		return (DIALOG_QUIT);

	case ALIGN_DO:
		doAlign((PedInfo)allh->ped, allh->str);
		return (DIALOG_QUIT);

	case ALIGN_ARG:
		if (!isdigit(allh->str[0]) || (atoi(allh->str) < 1))
		{
			item_title_change(allh->text, 
			"Align value must be\na positive integer.");
			dialog_item_ability(di, ALIGN_DO, DIALOG_DISABLE);
		}
		else
		{
			item_title_change(allh->text, "");
			dialog_item_ability(di, ALIGN_DO, DIALOG_ENABLE);
		}
		return (DIALOG_NOMINAL);
	}
	return (DIALOG_NOMINAL);
}

void
align_dialog_run(AllDia *allh)
{
	Dialog         *di;
	align_info	al_info;
	char		tempstr[40];

	/* get the proper alignment degree from the dependence */
	al_info = pt_align_degree((PedInfo)allh->ped);

	if ((al_info.degree)&&!(al_info.danger)&&!(al_info.other_dep)
		&&!al_info.loop_depth)
	{
		sprintf(tempstr, "%d", al_info.degree);
		allh->str = ssave(tempstr);
	}
	else
	{
	allh->str = ssave("");
	}

	di = dialog_create("Align Dependence", align_handler, 
			   (dialog_helper_callback) 0, (Generic) allh,
			   dialog_desc_group(DIALOG_VERT_CENTER, 3,
				item_button(ALIGN_DO, "Do Align", DEF_FONT_ID, false),
				item_text(ALIGN_ARG, "Align Degree ",
				DEF_FONT_ID, &(allh->str), 12),
				allh->text = item_title(UNUSED,
"----------------------------------------------------------\n\n\n",
				DEF_FONT_ID)));


	if (al_info.loop_depth)
		item_title_change(allh->text, 
			"Alignment only supported for innermost loop.");
	else if ((al_info.danger) && (al_info.other_dep))
		item_title_change(allh->text, "Alignment will reverse order of source and sink;\n and alignment conflict may exist.");
	else if (al_info.danger)
		item_title_change(allh->text, "Alignment will reverse order of source and sink.");
	else if (al_info.other_dep)
		item_title_change(allh->text, "Alignment conflict may exist.");
	else
		item_title_change(allh->text, " ");

	if ((al_info.degree)&&!(al_info.danger)&&!(al_info.other_dep)
		&&!al_info.loop_depth)
		dialog_item_ability(di, ALIGN_DO, DIALOG_ENABLE);
	else
		dialog_item_ability(di, ALIGN_DO, DIALOG_DISABLE);
	dialog_modal_run(di);
	dialog_destroy(di);
}




