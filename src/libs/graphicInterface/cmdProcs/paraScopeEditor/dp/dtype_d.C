/* $Id: dtype_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/PEditorCP/dp/dtype_d.c				*/
/*									*/
/*	dtype_d.c -- uses a view filter to adjust which 		*/
/*		dependences are viewed in the dependence pane		*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <ctype.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>
#include <libs/graphicInterface/oldMonitor/include/items/check_box.h>

void
dep_type_dialog_run (PedInfo ped)
{
	Dialog	*di;
   	Boolean lc 	= el_get_view_lc(PED_EL(ped));
   	Boolean control	= el_get_view_control(PED_EL(ped));
   	Boolean li 	= el_get_view_li(PED_EL(ped));
   	Boolean Private = el_get_view_private(PED_EL(ped));

   	di = dialog_create ("View Dependence Type", dep_type_handler, 
		(dialog_helper_callback) 0, (Generic) ped,
   		dialog_desc_group (DIALOG_VERT_LEFT, 4,
			item_check_box(DF_LC, "Loop Carried Dependences", DEF_FONT_ID, &lc),
			item_check_box(DF_LI, "Loop Independent Dependences", DEF_FONT_ID, &li),
			item_check_box(DF_CONTROL, "Control Dependences", DEF_FONT_ID, &control),
			item_check_box(DF_PRIVATE, "Private Variable Dependences", DEF_FONT_ID, &Private)
		));
	dialog_modal_run (di);

	el_set_view_lc(PED_EL(ped), lc);
	el_set_view_control( PED_EL(ped), control);
	el_set_view_li( PED_EL(ped), li);
	el_set_view_private( PED_EL(ped), Private);
	
	dialog_destroy (di);
}

Boolean
dep_type_handler (Dialog *di, Generic ped, Generic item_id)
{	
	switch (item_id)
	{
    		case DIALOG_CANCEL_ID:
      			return(DIALOG_QUIT);
		default:
			return(DIALOG_NOMINAL);
	}

}







