/* $Id: dia.C,v 1.1 1997/06/25 14:42:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/PEditorCP/dia.c						*/
/*									*/
/*	dia.c -- does most of the interfacing between the cp and its 	*/
/*		 dialogs						*/
/*									*/
/************************************************************************/

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pedCP/PedPrivate.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>

void
pedcp_create_dialogs (PEditorCP pedcp)
{
	PedInfo   ped;
	
	ped = (PedInfo)(R(pedcp)->ped);

	show_message2("Building dialogs.");
	beginComputeBound();			/* 910621, mpal */

	R(pedcp)->eh.DG		= PED_DG(ped);
	R(pedcp)->eh.EL		= PED_EL(ped);
	R(pedcp)->eh.ped	= (PedInfo) ped;
	R(pedcp)->eh.update	= (edge_dialog_update_func)edgeUpdate;
	R(pedcp)->eh.pop	= (edge_dialog_pop_func)edgePop;
	R(pedcp)->eh.push	= (edge_dialog_push_func)edgePush;

	R(pedcp)->pm.first_stmt = AST_NIL;
	R(pedcp)->pm.last_stmt  = AST_NIL;
	R(pedcp)->pm.di         = perf_dialog_create(&(R(pedcp)->pm));
	R(pedcp)->pm.dep        = R(pedcp)->ped;

	R(pedcp)->th.ih         = &(R(pedcp)->ih);
	R(pedcp)->th.dh         = &(R(pedcp)->dh);
	R(pedcp)->th.sih        = &(R(pedcp)->sih);
	R(pedcp)->th.smih       = &(R(pedcp)->smih);
	R(pedcp)->th.allh       = &(R(pedcp)->allh);
	R(pedcp)->th.ped        = R(pedcp)->ped;
	R(pedcp)->th.PD		= pedcp;
	R(pedcp)->th.selection  = AST_NIL;

	R(pedcp)->mm.ped        = R(pedcp)->ped;
	R(pedcp)->mm.ft         = NULL;

	R(pedcp)->ih.ped        = R(pedcp)->ped;
    
	R(pedcp)->dh.ped        = R(pedcp)->ped;
	R(pedcp)->dh.ret        = PG_PARALLEL;

	R(pedcp)->sih.ped 	= R(pedcp)->ped;

	R(pedcp)->smih.ped 	= R(pedcp)->ped;
	R(pedcp)->allh.ped 	= R(pedcp)->ped;

	R(pedcp)->sh.ped 	= (PedInfo)(R(pedcp)->ped);
	R(pedcp)->sh.selection  = AST_NIL;
	R(pedcp)->sh.PD         = (Generic) pedcp;
	R(pedcp)->sh.LI         = PED_LI(ped);
	R(pedcp)->sh.EL         = PED_EL(ped);

	transform_menu_create (&(R(pedcp)->th));
#if 0
	memory_menu_create (&(R(pedcp)->mm));
#endif
	
	R(pedcp)->eh.di		= edge_dialog_create 	    (&(R(pedcp)->eh));
	R(pedcp)->sh.di		= shared_dialog_create 	    (&(R(pedcp)->sh));

	endComputeBound();			/* 910621, mpal */
	hide_message2();
}

void 
pedcp_destroy_dialogs (PEditorCP pedcp)
{
	transform_menu_destroy	   (&(R(pedcp)->th));
#if 0
	memory_menu_destroy	   (&(R(pedcp)->mm));
#endif
	shared_dialog_destroy      (&(R(pedcp)->sh));
	edge_dialog_destroy        (&(R(pedcp)->eh));
	perf_dialog_destroy        (&(R(pedcp)->pm));
}

void
dialogUpdate (PEditorCP pedcp)
{
	PedInfo   ped = (PedInfo)(R(pedcp)->ped);

	R(pedcp)->eh.DG		= PED_DG(ped);
	R(pedcp)->eh.EL		= PED_EL(ped);
	R(pedcp)->sh.LI		= PED_LI(ped);
	R(pedcp)->sh.EL		= PED_EL(ped);

	shared_dialog_update       (&(R(pedcp)->sh));
	edge_dialog_update         (&(R(pedcp)->eh));
	R(pedcp)->th.selection   = ((PedInfo)R(pedcp)->ped)->selected_loop;
}
void
dialogHide (PEditorCP pedcp)
{
	shared_dialog_hide         (&(R(pedcp)->sh));
	edge_dialog_hide           (&(R(pedcp)->eh));
	R(pedcp)->th.selection   = ((PedInfo)R(pedcp)->ped)->selected_loop;
}
