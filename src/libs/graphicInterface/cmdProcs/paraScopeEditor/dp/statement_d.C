/* $Id: statement_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/PEditorCP/dp/statement_d.c				*/
/*									*/
/*	Statement Dialogs for PED					*/
/*									*/
/*	Add								*/
/*	Delete								*/
/*	Interchange							*/
/*									*/
/************************************************************************/

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/yes_no.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>
#include <libs/graphicInterface/oldMonitor/include/items/text.h>

/*--------------------------------------------------------------------

	add statement

*/

static Boolean
add_stmt_handler(Dialog *di, Generic ALLH, Generic item_id)
{
    AllDia			*allh;
    
    allh = (AllDia *) ALLH;
    
    switch (item_id)
    {
    case DIALOG_CANCEL_ID:
	return (DIALOG_QUIT);
	
    case ADD_STMT_DO:
	doAddStmt((PedInfo)allh->ped, allh->str);
	return (DIALOG_QUIT);
	
    case ADD_STMT_ARG:
	dialog_item_ability(di, ADD_STMT_DO, DIALOG_ENABLE);
	return (DIALOG_NOMINAL);
    }
    return (DIALOG_NOMINAL);
}


void
add_stmt_dialog_run(AllDia *allh)
{
    Dialog         *di;
    int type;
    
    allh->str = ssave("       ");
    di = dialog_create("Add Statement", add_stmt_handler, 
		       (dialog_helper_callback) 0, (Generic) allh,
		       dialog_desc_group(DIALOG_VERT_CENTER, 3,
					 item_button(ADD_STMT_DO, "Do add", DEF_FONT_ID, false),
					 item_text(ADD_STMT_ARG, "Statement ", 
						   DEF_FONT_ID, &(allh->str), 70),
					 allh->text = item_title(UNUSED, 
								 "----------------------------------------------------------\n\n\n",
								 DEF_FONT_ID)));
    
    dialog_item_ability(di, ADD_STMT_DO, DIALOG_DISABLE);
    item_title_change(allh->text, 
		      "Statement inserted before selection.");
    dialog_modal_run(di);
    dialog_destroy(di);
}


/*--------------------------------------------------------------------

	delete statement

*/

void
delete_stmt_dialog_run(Generic DP)
{
    char           *msg;
    int             ret;
    Boolean         answer;
    PedInfo         ped;
    
    ped = (PedInfo) DP;
    
    ret = pt_test_delete_stmt(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped), &msg);
    
    switch (ret)
    {
    case CANNOT_CHANGE:
	message(msg);
	break;
	
    case CAN_CHANGE:
	(void) yes_no("Delete the selected statement?", &answer, false);
	if (answer)
	    doDeleteStmt(ped);
	break;
	
    default:
	break;
    }
}


/*--------------------------------------------------------------------

	statement interchange

*/

void
sinter_dialog_run(SinterDia *sih)
{
    Dialog         *di;
    PedInfo         ped;
    
    di = dialog_create("Statement Interchange", sinter_handler, (dialog_helper_callback) 0, (Generic) sih,
            dialog_desc_group(DIALOG_VERT_CENTER, 2,
	      item_button(SINTER_DO, "Do Statement Interchange", DEF_FONT_ID, false),
	      sih->text = item_title(UNUSED, "---------------------------------------------------\n\n\n", DEF_FONT_ID)
			      )
		       );
    item_title_justify_left(sih->text);
    sih->ret = CANNOT_CHANGE;
    dialog_item_ability(di, SINTER_DO, DIALOG_DISABLE);
    
    
    ped = (PedInfo) (sih->ped);
    
    if (PED_SELECTED_LOOP(ped) == AST_NIL)
    {
	dialog_item_ability(di, SINTER_DO, DIALOG_DISABLE);
	item_title_change(sih->text, "");
	return;
    }
    sih->ret = pt_can_reorder_stmts(ped, PED_SELECTION(ped),
				    loop_level(PED_SELECTED_LOOP(ped)));
    switch (sih->ret)
    {
    case CANNOT_REORDER:
	dialog_item_ability(di, SINTER_DO, DIALOG_DISABLE);
	item_title_change(sih->text, "Statement interchange violates the\ndependences between the statements.");
	break;
	
    case CAN_REORDER:
	dialog_item_ability(di, SINTER_DO, DIALOG_ENABLE);
	item_title_change(sih->text, "Statements can be interchanged.");
	break;
	
    case ILLEGAL_STMTS:
	dialog_item_ability(di, SINTER_DO, DIALOG_DISABLE);
	item_title_change(sih->text, "The statements selected are illegal\nfor interchange");
	break;
	
    default:
	dialog_item_ability(di, SINTER_DO, DIALOG_DISABLE);
	item_title_change(sih->text, "");
	break;
    }
    
    dialog_modal_run(di);
    dialog_destroy(di);
}

Boolean
sinter_handler(Dialog *di, Generic SIH, Generic item_id)
{
    SinterDia      *sih;
    
    sih = (SinterDia *) SIH;
    
    switch (item_id)
    {
    case DIALOG_CANCEL_ID:
	return (DIALOG_QUIT);
	
    case SINTER_DO:
	if (sih->ret == CAN_REORDER)
	{
	    doStmtInter((PedInfo) (sih->ped));
	    return (DIALOG_QUIT);
	}
	/* Because doStmtInter results in an update call sih->ret can
	 * change.  If selection is done properly this would be
	 * redundant, but currently it's not. */
	if (sih->ret == CANNOT_REORDER)
	    return (DIALOG_QUIT);
	break;
    }
    return DIALOG_NOMINAL;
    
}





