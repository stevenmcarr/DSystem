/* $Id: sexpand_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*										*/
/*	ped_cp/PEditorCP/dp/sexpand_d.c		*/
/*										*/
/*	Scalar Expansion Dialog				*/
/*										*/
/************************************************************************/

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/message.h>


void
scalar_dialog_run (Generic DP)
  //Generic DP;
{
    	char       *msg;
    	PedInfo     ped;
	DiaDesc	   *text;
	int	    ret;
	Dialog     *di;
    
    	ped = (PedInfo)(DP);

    	ret = se_test_scalar_expand (ped, PED_SELECTED_LOOP(ped), 
						PED_SELECTION(ped), &msg);
  
    	switch (ret)
    	{
    		case CANNOT_CHANGE:
    			message (msg);
			break;

		case CAN_CHANGE:
			di = dialog_create ("Scalar Expansion", scalar_handler, (dialog_helper_callback) 0, DP,
	  			dialog_desc_group( DIALOG_VERT_CENTER, 2,
	    		 		item_button (SCALAR_DO, "Do Expansion", DEF_FONT_ID, false),
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
scalar_handler (Dialog *di,Generic DP, Generic item_id)
//    	Dialog	*di;
//    	Generic	 DP;
//    	Generic	 item_id;
{
	PedInfo	ped;
	
	switch (item_id)
	{
		case DIALOG_CANCEL_ID:
			return DIALOG_QUIT;

		case SCALAR_DO:
			ped = (PedInfo)(DP);
			doScalarExpansion (ped);
			PED_SELECTION(ped) = PED_SELECTED_LOOP(ped);
			return DIALOG_QUIT;
	}

	return DIALOG_NOMINAL;

}
