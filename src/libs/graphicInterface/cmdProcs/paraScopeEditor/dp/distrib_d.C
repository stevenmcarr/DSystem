/* $Id: distrib_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/PEditorCP/dp/distrib_d.c					*/
/*									*/
/*	Loop Distribution Dialog for PED				*/
/*									*/
/*									*/
/************************************************************************/

#include <ctype.h>
#include <stdarg.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pedCP/PedPrivate.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/yes_no.h>


/*--------------------------------------------------------------------

	loop distribution

*/

STATIC(char, *count_newlines,(char *msg));


void
distribute_dialog_run (DistrDia *dh)
{
    Dialog  *di;
    Generic list;
    PedInfo ped;
    char	*msg, *tmsg;
    int     i;
    int	doit;
    
    di = dialog_create ("Loop Distribution", select_distribute_handler, 
			(dialog_helper_callback) 0, (Generic) dh,
			dialog_desc_group( DIALOG_VERT_CENTER, 3,
					  item_button (PG_PARALLEL,    "Maximal Parallelism", DEF_FONT_ID, false),
					  item_button (PG_VECTOR, "   Vectorization   ", DEF_FONT_ID, false),
					  item_title (UNUSED,"                           ", DEF_FONT_ID)
					  ));
    
    
    dh->ret = CAN_CHANGE;
    dialog_modal_run (di);
    dialog_destroy (di);
    if (dh->ret == CAN_CHANGE)
	return;
    
    ped = (PedInfo)(dh->ped);
    list = (Generic) pt_make_adj_list (ped, PED_SELECTED_LOOP(ped));
    doit = pt_find_distribution (ped, PED_SELECTED_LOOP(ped), list, dh->ret);
    msg  = pt_print_distribution (list, (Generic) ped);
    tmsg = count_newlines (msg);
    
    di  = dialog_create ("Loop Distribution", distribute_handler, 
			 (dialog_helper_callback) 0, (Generic) dh,
			 dialog_desc_group (DIALOG_VERT_CENTER, 2,
					    dh->but  = item_button (DISTRIBUTE_DO, "Do Distribution", DEF_FONT_ID, false),
					    dh->text = item_title(UNUSED, tmsg, DEF_FONT_ID)
					    ));
    item_title_justify_left (dh->text);
    item_title_change(dh->text, msg);
    sfree(tmsg);
    if (doit == CAN_CHANGE)
	dialog_item_ability (di, DISTRIBUTE_DO, DIALOG_ENABLE);
    else
	dialog_item_ability (di, DISTRIBUTE_DO, DIALOG_DISABLE);
    dialog_modal_run (di);
    dialog_destroy (di);
}

/*static*/ Boolean
select_distribute_handler(Dialog *di, Generic DH, Generic item_id)
{
    DistrDia *dh;
    
    dh = (DistrDia *) DH;
    switch (item_id)
    {
    case PG_PARALLEL:
	dh->ret = PG_PARALLEL;
	break;
    case PG_VECTOR:
	dh->ret = PG_VECTOR;
	break;
    default:
	return (DIALOG_QUIT);			
    }
    return (DIALOG_QUIT);
}


/*static*/ Boolean
distribute_handler(Dialog *di, Generic DH, Generic item_id)
{
    DistrDia *dh;
    
    dh = (DistrDia *) DH;
    switch (item_id)
    {
    case DIALOG_CANCEL_ID:
	return(DIALOG_QUIT);
	
    case DISTRIBUTE_DO:
	doDistribution (dh, dh->ped, dh->ret);
	if (dh->ret == CANNOT_CHANGE)
	    return(DIALOG_QUIT);
	return (DIALOG_QUIT);
    }
    return (DIALOG_NOMINAL);
}

static char *
count_newlines (char *msg)
{	
    int i;
    int l;
    int len;
    char *tmsg;
    static char *buf = 
	"___________________________________________________________";
    
    len = strlen(msg);
    
    for (l = 0, i = 0; i < len; i++)
    {
	if (msg[i] == '\n')
	    l++;
    }
    
    tmsg = (char *) get_mem((strlen(buf) + l + 1) * sizeof(char), "count_newlines");
    
    strcpy(tmsg, buf);
    
    for (i = 0; i < l; i++)
    {
	strcat(tmsg, "\n");
    }
    
    return tmsg;
}


void
distribute_dialog_destroy (DistrDia *dh)
{
    dialog_destroy (dh->di);
}


