/* $Id: ped_structs.h,v 1.15 1997/03/11 14:31:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/ped_include/ped_structs.h				*/
/*									*/
/*	ped_structs.h -- include file for all ped dialogs		*/
/*									*/
/*									*/
/************************************************************************/

#ifndef ped_structs_h
#define ped_structs_h

#ifndef	dt_info_h
#include <libs/moduleAnalysis/dependence/dependenceTest/dt_info.h>
#endif
#ifndef	 side_info_h
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#endif
#ifndef	 dg_instance_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#endif
#ifndef	 dg_header_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#endif

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/el.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt.h>
#include <libs/graphicInterface/oldMonitor/include/mon/menu.h>

typedef FUNCTION_POINTER(void, edge_dialog_update_func, (Generic ped));
typedef FUNCTION_POINTER(int, edge_dialog_pop_func, (Generic ped));
typedef FUNCTION_POINTER(int, edge_dialog_push_func, (Generic ped));

typedef struct  {
    Generic	 ped;
    Dialog	*di;
    DiaDesc	*text;
    DiaDesc	*text2;
    int		 val;
    int		 val2;
    char 	*str;
    char 	*str2;
} AllDia;


typedef struct Edged
{
    Dialog         *di;
    Generic         PD;
    DG_Instance	   *DG;
    EL_Instance	   *EL;
    PedInfo	    ped;
    edge_dialog_update_func
		    update;
    edge_dialog_pop_func
		    pop;
    edge_dialog_push_func
		    push;
    AST_INDEX       src_ast;
    AST_INDEX       sink_ast;
    char           *text_type;
    char           *text_name;
    DiaDesc        *text_src;
    DiaDesc        *text_sink;
    char           *text_dims;
    char           *text_block;
    AST_INDEX      selection;
} EdgeDia;

typedef struct shar {
    Dialog        *di;
    Generic        PD;
    PedInfo	   ped;
    LI_Instance	  *LI;
    EL_Instance	  *EL;
    DiaDesc       *text;
    DiaDesc       *sdia;
    DiaDesc       *pdia;
    DiaDesc       *stitle;
    DiaDesc       *ptitle;
    aMenu         *menu;
    int            stype;
    int            ptype;
    int            snum;
    int            pnum;
    Generic        ssel;
    Generic        psel;
    AST_INDEX      selection;
    Generic        ret;
} SharDia;

typedef struct inter {
    Generic	   ped;
    Dialog        *di;
    DiaDesc       *text;
    DiaDesc       *but;
    
    int            loop_type;
    int            par_status;
    int            tri_type;
} InterDia;

typedef struct sinter {
    Generic        ped;
    Dialog        *di;
    DiaDesc       *text;
    DiaDesc       *but;
    int	           ret;
} SinterDia;

typedef struct stripm {
    Generic	 ped;
    Dialog	*di;
    DiaDesc	*text;
    int		 ret;

    char 	*step;
} StripDia;

typedef struct Distr {
    Generic	 ped;
    Dialog	*di;
    DiaDesc	*text;
    DiaDesc 	*but;
    int		 ret;
    
    Generic	 dgraph;   /* distribution graph, based on ControlDep */
} DistrDia;


typedef struct Trans {
    aMenu	**menu;
    InterDia	*ih;
    DistrDia	*dh;
    SinterDia   *sih;
    StripDia	*smih;
    AllDia	*allh;
    Generic      choice;
    Generic	 ped;
    Generic	 PD;
    AST_INDEX    selection;
} TransMenu;

typedef struct {
  aMenu        *menu;
  Generic      ped;
  Generic      ft;
  Context      mod_context;
 } MemoryMenu; 


typedef struct Perfm {
    Dialog    *di;
    DiaDesc   *text;
    DiaDesc   *title;
    AST_INDEX first_stmt;
    DiaDesc   *ftext;
    DiaDesc   *ltext;
    AST_INDEX last_stmt;
    Generic   dep;
} PerfDia;


/* This is a sorry attempt at making this interface C++ compatible.  Read */
/* general.h for more information.  --Don Baker 10/17/91		  */

EXTERN (void, transform_menu, (TransMenu *th));
EXTERN (void, shared_info_dialog_run, ());
EXTERN (void, edge_dialog_run, (EdgeDia *eh));
EXTERN (void, shared_dialog_run, (SharDia *sh));
EXTERN (void, perf_dialog_run, (PerfDia *pf));
EXTERN (void, interchange_dialog_run, ());
EXTERN (void, distribute_dialog_run, (DistrDia *dh));
EXTERN (void, scalar_dialog_run, (Generic DP));
EXTERN (void, peel_dialog_run, (StripDia *smih));
EXTERN (void, strip_dialog_run, (StripDia *smih));

EXTERN (void, shared_dialog_hide, (SharDia *sh));
EXTERN (void, edge_dialog_hide, (EdgeDia *eh));

EXTERN (void, shared_dialog_update, (SharDia *sh));
EXTERN (void, edge_dialog_update, (EdgeDia *eh));

EXTERN (void,    transform_menu_create, (TransMenu *th));
EXTERN (Dialog *, edge_dialog_create, (EdgeDia *eh));
EXTERN (Dialog *, shared_dialog_create, (SharDia *sh));
EXTERN (Dialog *, perf_dialog_create, (PerfDia *pf));

EXTERN (Boolean, edge_handler, (Dialog *di, Generic EH, Generic item_id));
EXTERN (Boolean, shared_handler, (Dialog *di, Generic SH, Generic item_id));
EXTERN (Boolean, interchange_handler, ());
EXTERN (Boolean, select_distribute_handler, (Dialog *di, Generic DH,
                                             Generic item_id));
EXTERN (Boolean, distribute_handler, (Dialog *di, Generic DH, Generic item_id));
EXTERN (Boolean, scalar_handler, (Dialog *di, Generic DP, Generic item_id));
EXTERN (Boolean, peel_handler, (Dialog *di, Generic SMIH, Generic item_id));
EXTERN (Boolean, strip_handler, (Dialog *di, Generic SMIH, Generic item_id));

EXTERN (void, transform_menu_destroy, (TransMenu *th));
EXTERN (void, distribute_dialog_destroy, (DistrDia *dh));
EXTERN (void, edge_dialog_edge_destroy, ());
EXTERN (void, interchange_dialog_destroy, ());
EXTERN (void, edge_dialog_destroy, (EdgeDia *eh)); 
EXTERN (void, perf_dialog_destroy, (PerfDia *pf));
EXTERN (void, fusion_dialog_destroy, (SharDia *sh));

EXTERN (void, edge_dialog_edge_clear, (EdgeDia *eh));
EXTERN (void, edge_dialog_edge_set, (EdgeDia *eh, char *text, char *name, AST_INDEX
                                     src, AST_INDEX sink, char *dims, char *block));

#endif
