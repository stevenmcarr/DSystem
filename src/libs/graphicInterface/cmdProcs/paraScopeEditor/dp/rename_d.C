/* $Id: rename_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*										*/
/*	ped_cp/PEditorCP/dp/rename_d.c		*/
/*										*/
/*	Array Renaming Dialog				*/
/*										*/
/*										*/
/************************************************************************/

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>

Boolean
renaming_handler ();

void
renaming_dialog_run (Generic DP)
{
    	char       *msg;
    	PedInfo     ped;
	DiaDesc	   *text;
	int	    ret;
	Dialog     *di;
    
    	ped = (PedInfo)(DP);

    	ret = pt_test_renaming (ped, PED_SELECTED_LOOP(ped), 
                                PED_SELECTION(ped), (char**)&msg);
  
    	switch (ret)
    	{
    		case CANNOT_CHANGE:
    			message (msg);
			break;

		case CAN_CHANGE:
			di = dialog_create ("Array Renaming", renaming_handler, (dialog_helper_callback) 0, DP,
	  			dialog_desc_group( DIALOG_VERT_CENTER, 2,
	    		 		item_button (RENAMING_DO, "Do Array Renaming", DEF_FONT_ID, false),
					text = item_title(UNUSED, msg, DEF_FONT_ID)
          			));
    			item_title_justify_left (text);
			dialog_modal_run (di);
			dialog_destroy (di);
			break;

		default:
			break;
    }
    sfree (msg);
}

Boolean
renaming_handler (Dialog *di, Generic DP, Generic item_id)
{
    	PedInfo	ped;

    	ped = (PedInfo)(DP);
	
	switch (item_id)
	{
		case DIALOG_CANCEL_ID:
			return DIALOG_QUIT;

		case RENAMING_DO:
			doRenamingExpansion (ped);
			PED_SELECTION(ped) = PED_SELECTED_LOOP(ped);
			return DIALOG_QUIT;
	}

	return DIALOG_NOMINAL;
}


