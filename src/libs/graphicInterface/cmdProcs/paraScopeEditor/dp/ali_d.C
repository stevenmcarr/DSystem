/* $Id: ali_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/PEditorCP/dp/ali_d.c					*/
/*									*/
/*	Advanced Loop Interchange Dialog for PED			*/
/*									*/
/*									*/
/************************************************************************/

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>

static Boolean 
ali_handler(Dialog *di, Generic IH, Generic item_id)
{
    InterDia *ih;
    
    ih = (InterDia *) IH;
    switch (item_id)
    {
    case DIALOG_CANCEL_ID:
	return (DIALOG_QUIT);
	
    case INTERCHANGE_DO:
	if (ih->loop_type > OK_LOOP)
	    doInterchange (ih, ih->ped);
	return (DIALOG_QUIT);
    }
    return DIALOG_NOMINAL;
}


void
ali_dialog_run (InterDia *ih)
{  
    Dialog *di;
    PedInfo ped;
    char *msg,*msg2;
    char buf[256];
    
    di  = dialog_create ("Advanced Loop Interchange", ali_handler, 
			 (dialog_helper_callback) 0, (Generic) ih,
			 dialog_desc_group( DIALOG_VERT_CENTER, 2,
					   ih->but  = item_button (INTERCHANGE_DO, "Do Interchange", DEF_FONT_ID, false),
					   ih->text = item_title(UNUSED, "__________________________________________________\n\n",
								 DEF_FONT_ID))
			 );
    item_title_justify_left (ih->text);
    dialog_item_ability (di, INTERCHANGE_DO, DIALOG_DISABLE);
    item_title_change (ih->text, "");
    
    ped = (PedInfo)(ih->ped);
    
    if (PED_SELECTED_LOOP(ped) == AST_NIL)
    {
	item_title_change (ih->text," ");
     	dialog_item_ability (di, INTERCHANGE_DO, DIALOG_DISABLE);
	return;
    }
    
    pt_set_loop_type(ped,PED_SELECTED_LOOP(ped),
       &ih->loop_type,&ih->par_status,&ih->tri_type);
    
    switch (ih->loop_type) {
    case RECT_LOOP:
	msg="Rectangular interchange is valid"; break;
    case SKEW_LOOP:
	msg="Skewed interchange is valid"; break;
    case TRI_LOOP:
	msg="Triangular interchange is valid"; break;
    case TRAP_LOOP:
	msg="Trapezoidal interchange is valid"; break;
    case PENT_LOOP:
	msg="Pentagonal interchange is valid"; break;
    case HEX_LOOP:
	msg="Hexagonal interchange is valid"; break;
    default:
	msg="Loop interchange not possible"; break;
    }
    
    if (ih->loop_type > OK_LOOP) {
	switch(ih->par_status) {
	case OUTER_ONLY:
	    msg2 = "\nAfter interchange outer loop is parallel."; break;
	case INNER_ONLY:
	    msg2 = "\nAfter interchange inner loop is parallel."; break;
	case BOTH:
	    msg2 = "\nAfter interchange both loops are parallel."; break;
	case NONE:
	    msg2 = "\nAfter interchange neither loop is parallel."; break;
	default:
	    msg2 = ""; break;
	}
    }
    else {
	switch (ih->loop_type) {
	case BAD_INNER_LOOP:
	    msg2 = "\nNeed perfectly nested inner loop"; break;
	case BAD_OUTER_LOOP:
	    msg2 = "\nCouldn't detect outer loop"; break;
	case BAD_DEP_LOOP:
	    msg2 = "\nDependence prevents interchange"; break;
	default:
	    msg2 = "\nLoop bounds too complex"; break;
	}
    }
    
    strcpy(buf, msg);
    strcat (buf,msg2);
    
    item_title_change (ih->text,buf);
    if (ih->loop_type < OK_LOOP)
	dialog_item_ability (di, INTERCHANGE_DO, DIALOG_DISABLE);
    else
	dialog_item_ability (di, INTERCHANGE_DO, DIALOG_ENABLE);
    
    dialog_modal_run (di);
    dialog_destroy (di);
}
