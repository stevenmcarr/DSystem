/* $Id: shared_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*										*/
/*	ped_cp/PEditorCP/dp/shared_d.c		*/
/*										*/
/*	Shared Variable Dialog				*/
/*										*/
/*										*/
/************************************************************************/

#include <stdio.h>
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#include <libs/moduleAnalysis/dependence/loopInfo/private_li.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/oldMonitor/include/items/item_list.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>

void shared_dialog_hide (SharDia *sh);
ListItemEntry shared_dialog_shared_handler (SharDia *sh, DiaDesc *dd, Boolean first, 
                                            Generic curr);
ListItemEntry shared_dialog_private_handler(SharDia *sh, DiaDesc *dd, Boolean first, 
                                            Generic curr);


Dialog*
shared_dialog_create (SharDia *sh)
{
    Dialog	*di;
    Point	size;

    	size.x = 15;
    	size.y = DEF_VARS;
    	sh->ssel = NO_SELECTION;
    	sh->psel = NO_SELECTION;

	sh->snum = 0;
	sh->pnum = 0;

	if (sh->ped == 0)
	{
		sh->selection = AST_NIL;
	}
	else if ((sh->selection = ((PedInfo)(sh->ped))->selected_loop) != AST_NIL) 
	{
		sh->snum = el_get_num_shared_vars (sh->LI);
		sh->pnum = el_get_num_private_vars (sh->LI);
	}
	sh->stype = ALL_SHARED;
	sh->menu  = make_menu ("Show", NO_SELECTION, 1, 8,
		ALL_SHARED, toKbChar(0), 1, "all the shared variables", (char *) 0,
		ALL_PRIV, toKbChar(0), 1, "all the private variables", (char *) 0,	       
		NO_MOD_DEF, toKbChar(0), 1, "only used in loop", 
			"The shared variables that are used but not modified in the loop,\n and that must be defined before it.",
		MAY_MOD, toKbChar(0), 1,  "defined before, may modify",
			"The shared variables that may be modified in the loop, \nand are defined before the loop.\n This means they may have an upwards exposed definition.",
		MAY_USE, toKbChar(0), 1,  "defined before, used after, may modify",
			"The shared variables that may be modified in the loop,\nand that may be defined before and are used after\nthe loop. This means they may have an upwards \nexposed definition, and they do have a downwards \n exposed use.",
		COMMON, toKbChar(0), 1, "in common and used",
			"The shared variables that are in common, and used in the loop.",
		USER_SPEC_SHAR, toKbChar(0), 1, "user specified shared vars",
			"A user has specified that this variable be shared.",
		USER_SPEC_PRIV, toKbChar(0), 1, "user specified private vars",
			"A user has specified that this variable be private."
	);
  di = dialog_create(
          "Variable Classification", 
          shared_handler, 
          (dialog_helper_callback)0, 
          (Generic)sh,
	  dialog_desc_group(
             DIALOG_VERT_LEFT, 
             2,
	     dialog_desc_group(
                DIALOG_HORIZ_TOP, 
                2,
	        dialog_desc_group(
                   DIALOG_VERT_LEFT, 
                   4,
		   sh->stitle = item_title(
                                   UNUSED, 
                                   "All.__________.........................", 
                                   DEF_FONT_ID),
	   	   item_title(UNUSED, "Shared Variables", DEF_FONT_ID),
		   dialog_desc_group( 
                      DIALOG_HORIZ_CENTER, 
                      3,
                      sh->sdia = item_list(
                                    SLIST, 
                                    "", 
                                    (Generic)sh, 
                                    (item_list_elem_proc)shared_dialog_shared_handler,
				    &(sh->ssel), 
                                    (Generic)NO_SELECTION, 
                                    false, 
                                    DEF_FONT_ID,
                                    size),
                      item_title (UNUSED, "         ", DEF_FONT_ID),
                      dialog_desc_group(
                         DIALOG_VERT_CENTER, 
                         2,
                         item_button(RIGHT_ARROW, "->", DEF_FONT_ID, false),
                         item_button(LEFT_ARROW, "<-", DEF_FONT_ID, false))),
                   item_button(CLASSIFY, "classify vars",DEF_FONT_ID, false)),	
	        dialog_desc_group(
                   DIALOG_VERT_LEFT, 
                   3,
                   sh->ptitle = item_title(
                                   UNUSED, 
                                   "All.__________............", 
                                   DEF_FONT_ID),
                   item_title(UNUSED, "Private Variables", DEF_FONT_ID),
                   sh->pdia = item_list(
                                 PLIST, 
                                 "", 
				 (Generic) sh, 
                                 (item_list_elem_proc)shared_dialog_private_handler,
				 &(sh->psel), 
                                 (Generic) NO_SELECTION, 
                                 false, 
                                 DEF_FONT_ID,
				 size))),
             sh->text = item_title(
                           UNUSED, 
                           ".............................................\n.\n,\n\n,\n,\n, ",
                           DEF_FONT_ID)));
    
    item_title_justify_left (sh->stitle);
    (void) item_title_change (sh->stitle, "All");
    item_title_justify_left (sh->ptitle);
    (void) item_title_change (sh->ptitle, "All");
    
    item_title_justify_left (sh->text);
    (void) item_title_change (sh->text, "");
    return (di);
}

void
shared_dialog_run (SharDia *sh)
{      
   	dialog_modeless_show (sh->di);
}

void 
shared_dialog_hide (SharDia *sh)
{
  	dialog_modeless_hide (sh->di);
}

/*ARGSUSED*/
/*static*/ Boolean
shared_handler(Dialog *di, Generic SH, Generic item_id)
    /* di: the dialog instance */
    /* SH: the selection variable */
    /* item_id: the id of the item */
{	
    Slist 	*node;
    SharDia 	*sh;
    Generic	selection;
    PedInfo	ped;
    
    sh  = (SharDia *)SH;
    ped = (PedInfo)(sh->ped);
    
    switch (item_id)
    {
    	case CLASSIFY:
		selection = select_from_menu (sh->menu, false);
		switch (selection)
		{
			case ALL_SHARED:
		                sh->ssel = NO_SELECTION;
				sh->stype = ALL_SHARED;
				(void) item_title_change (sh->stitle, "All");
				item_list_modified (sh->sdia);
				break;
			case ALL_PRIV:
				sh->ptype = ALL_PRIV;
		                sh->psel = NO_SELECTION;
				(void) item_title_change (sh->ptitle, "All");
				item_list_modified (sh->pdia);
				break;
			case NO_MOD_DEF:
				sh->stype = NO_MOD_DEF;
		                sh->ssel = NO_SELECTION;
				(void) item_title_change (sh->stitle, "Only Used in Loop");
				item_list_modified (sh->sdia);
				break;
			case MAY_MOD:
				sh->stype = MAY_MOD;
		                sh->ssel = NO_SELECTION;
				(void) item_title_change (sh->stitle, "Defined Before, May Modify");
				item_list_modified (sh->sdia);
				break;
			case MAY_USE:
				sh->stype = MAY_USE;
		                sh->ssel = NO_SELECTION;
				(void) item_title_change (sh->stitle, "Defined Before, Used After, May Modify");
				item_list_modified (sh->sdia);
				break;
			case COMMON:
				sh->stype = COMMON;
		                sh->ssel = NO_SELECTION;
				(void) item_title_change (sh->stitle, "In Common, and Used");
				item_list_modified (sh->sdia);
				break;
			case USER_SPEC_SHAR:
		                sh->ssel = NO_SELECTION;
				sh->stype = USER_SPEC_SHAR;
				(void) item_title_change (sh->stitle, "User Specified Shared");
				item_list_modified (sh->sdia);
				break;
			case USER_SPEC_PRIV:
				sh->ptype = USER_SPEC_PRIV;
		                sh->ssel = NO_SELECTION;
				(void) item_title_change (sh->ptitle, "User Specified Private");
				item_list_modified (sh->pdia);
				break;                   /* added - kef */
			default:
				break;		
		}
		(void) item_title_change (sh->text, "");
		break;
		
    	case DIALOG_CANCEL_ID:
    		shared_dialog_hide (sh);
		break;
	
	case SLIST:
		sh->psel = NO_SELECTION;
                dialog_item_modified(sh->di,PLIST);		
		node = (Slist *) sh->ssel;
		dialog_item_modified(sh->di, SLIST);
		item_title_change(sh->text, 
                                  el_get_shared_info(sh->LI, 
                                                     node->name, 
                                                     (Generic)sh->ped, 
                                                     (GetTextCallback)pt_get_stmt_text));
		break;	       

	case PLIST:
		sh->ssel = NO_SELECTION;
                dialog_item_modified(sh->di,SLIST);		
		node = (Slist *) sh->psel;
		dialog_item_modified(sh->di, PLIST);
		item_title_change(sh->text, 
                                  el_get_private_info(sh->LI, 
                                                      node->name, 
                                                      (Generic)sh->ped, 
                                                      (GetTextCallback)pt_get_stmt_text));
		break;	       

	
	case RIGHT_ARROW:
		/* take the selection in the shared variable list
		 * and put it in the private list
		 */
		if (sh->ssel != NO_SELECTION)
		{
			node = (Slist *)(sh->ssel);
			if (node->user == true)            /* kef - toggle this value */
			  node->user = false;
			else
			  node->user = true;
			ped->TreeWillChange(PED_ED_HANDLE(ped),PED_SELECTED_LOOP(ped));
			el_remove_shared_var( PED_LI(ped), PED_SELECTED_LOOP(ped), node->name);
			el_add_private_var( PED_LI(ped), PED_SELECTED_LOOP(ped), node);
			sh->ssel = NO_SELECTION;
			sh->psel = NO_SELECTION;
			forcePedUpdate(sh->ped,PED_SELECTED_LOOP(ped),PED_SELECTION(ped));
			ped->TreeChanged(PED_ED_HANDLE(ped),PED_SELECTED_LOOP(ped));
			(sh->snum)--;
			(sh->pnum)++;
			item_list_modified (sh->sdia);
			item_list_modified (sh->pdia);
			(void) item_title_change (sh->text, "");	
		}
		break;

	case LEFT_ARROW:
		/* take the selection in the private variable list
		 * and put it in the shared list
		 */
		if (sh->psel != NO_SELECTION)
		{
			node = (Slist *)(sh->psel);
			if (node->user == true)
			  node->user = false;
			else
			  node->user = true;
			ped->TreeWillChange(PED_ED_HANDLE(ped),PED_SELECTED_LOOP(ped));
			el_remove_private_var( PED_LI(ped), PED_SELECTED_LOOP(ped), node->name);
			el_add_shared_var( PED_LI(ped), PED_SELECTED_LOOP(ped), node);
			forcePedUpdate(sh->ped,PED_SELECTED_LOOP(ped),PED_SELECTION(ped));
			ped->TreeChanged(PED_ED_HANDLE(ped),PED_SELECTED_LOOP(ped));
			sh->psel = NO_SELECTION;
			sh->ssel = NO_SELECTION;
			(sh->pnum)--;
			(sh->snum)++;
			item_list_modified (sh->pdia);
			item_list_modified (sh->sdia);
			(void) item_title_change (sh->text, "");
		}
		break;

	default:
		break;
    }
    return DIALOG_NOMINAL;
}


void
shared_dialog_update (SharDia *sh)
{
	if (!sh->ped)
	{
		sh->selection = AST_NIL;
		sh->snum = 0;
		sh->pnum = 0;
	}
	else
	{
		sh->selection = ((PedInfo)(sh->ped))->selected_loop;
		sh->snum = el_get_num_shared_vars (sh->LI);
		sh->pnum = el_get_num_private_vars (sh->LI);
	}
	sh->ssel = NO_SELECTION;
	sh->psel = NO_SELECTION;	
	item_list_modified (sh->sdia);
	item_list_modified (sh->pdia);
	item_title_change (sh->text, "");

}


ListItemEntry
shared_dialog_shared_handler (SharDia *sh, DiaDesc *dd, Boolean first, 
                              Generic curr)
{
	ListItemEntry  line;
	Slist 	      *node;
	int	       len;

	if (first)
	{
		if (sh->snum == 0)
			return (NULL_LIST_ITEM_ENTRY);
		node = el_get_first_shared_node (sh->LI, &len, sh->stype);
		if (node == NULL)
			return (NULL_LIST_ITEM_ENTRY);
		line = item_list_create_entry(dd, (Generic) node, node->name, true);
		return (line);	
	}
	node = el_get_next_shared_node (sh->LI, (Slist *)curr, &len, sh->stype);
	if (node == NULL)
		return (NULL_LIST_ITEM_ENTRY);
	line = item_list_create_entry(dd, (Generic) node, node->name, true);
	return (line);
}


ListItemEntry
shared_dialog_private_handler(SharDia *sh, DiaDesc *dd, Boolean first, 
                              Generic curr)
{
	ListItemEntry  line;
	Slist 	      *node;
	int            len;

	if (first)
	{
		if (sh->pnum == 0)
			return (NULL_LIST_ITEM_ENTRY);

		node = el_get_first_private_node (sh->LI, &len, sh->ptype);
		if (node == NULL)
			return (NULL_LIST_ITEM_ENTRY);

		line = item_list_create_entry(dd, (Generic) node, node->name, true);
		return (line);	
	}
	node = el_get_next_private_node (sh->LI, (Slist *)curr, &len, sh->ptype);
	if (node == NULL)
		return (NULL_LIST_ITEM_ENTRY);

	line = item_list_create_entry(dd, (Generic) node, node->name, true);
	return (line);
}

void 
shared_dialog_destroy (SharDia *sh)
{
    dialog_destroy(sh->di);
}
