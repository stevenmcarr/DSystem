/* $Id: PedExtern.h,v 1.3 1997/06/25 13:52:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef PedExtern_h
#define PedExtern_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif


#include <libs/moduleAnalysis/dependence/dependenceTest/dep_dt.h>
#include <libs/graphicInterface/oldMonitor/include/items/item_list.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/perf.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pedCP/PedPrivate.h>

/* defined in depdisp.c */
EXTERN(void, depDispInit, (PedInfo ped));
EXTERN(void, loopUpdate, (PedInfo ped, AST_INDEX newloop));
EXTERN(void, edgeClear, (PedInfo ped));
EXTERN(void, subs_attach, (AST_INDEX root, FortTextTree ftt, SideInfo *infoPtr));
EXTERN(void, subs_free, (SideInfo *infoPtr));
EXTERN(char*, ped_DepToText, (PedInfo ped, DG_Edge *ea, int edge, int id));

/* defined in dia.c */
EXTERN(void, dialogUpdate, (PEditorCP pedcp));
EXTERN(void, dialogHide, (PEditorCP pedcp));

/* defined in PEditorCP.c */
EXTERN(FortEditor, ped_cp_get_FortEditor, (PedInfo ped));
EXTERN(void, pedRoot, (PedInfo ped));
EXTERN(int, pedLine, (PedInfo ped));
EXTERN(Boolean, save_as_pfc, (PEditorCP pedcp, Context module_context, Context
                              mod_in_prog_context, Context prog_context));
EXTERN(int, pedcp_Edit, (int argc, char **argv));
   
/* defined in ped_dg.c */
EXTERN(void, pedReinitialize, (PedInfo ped));

/* defined in ped_util_dg.c */
EXTERN(void, ped_print_deps, (PedInfo ped));

/* defined in ali_d.c */
EXTERN(void, ali_dialog_run, (InterDia *ih));

/* defined in align_d.c */
EXTERN(Boolean, align_handler, (Dialog *di, Generic ALLH, Generic item_id));
EXTERN(void, align_dialog_run, (AllDia *allh));

/* defined in dtype_d.c */
EXTERN(void, dep_type_dialog_run, (PedInfo ped));
EXTERN(Boolean, dep_type_handler, (Dialog *di, Generic ped, Generic item_id));

/* defined in edge_d.c */
EXTERN(void, convert, (char *str));

/* defined in fusion_d.c */
EXTERN(Boolean, fusion_handler, (Dialog *di, Generic ALLH, Generic item_id));
EXTERN(void, fusion_dialog_run, (AllDia *allh));

/* defined in loop_d.c */
EXTERN(Boolean, reverse_handler, (Dialog *di, Generic ALLH, Generic item_id));
EXTERN(void, reverse_dialog_run, (AllDia *allh));
EXTERN(Boolean, adjust_handler, (Dialog *di, Generic ALLH, Generic item_id));
EXTERN(void, adjust_dialog_run, (AllDia *allh));
EXTERN(Boolean, unswitch_handler, (Dialog *di, Generic ALLH, Generic item_id));
EXTERN(void, unswitch_dialog_run, (AllDia *allh));


/* defined in rename_d.c */
EXTERN(void, renaming_dialog_run, (Generic DP));
EXTERN(Boolean, renaming_handler, (Dialog *di, Generic DP, Generic item_id));

/* defined in shared_d.c */
EXTERN(ListItemEntry, shared_dialog_shared_handler, (SharDia *sh, DiaDesc *dd, 
                                                     Boolean first, Generic curr));
EXTERN(ListItemEntry, shared_dialog_private_handler, (SharDia *sh, DiaDesc *dd,
                                                      Boolean first, Generic curr));
EXTERN(void, shared_dialog_destroy, (SharDia *sh));

/* defined in skew_d.c */
EXTERN(Boolean, skew_is_valid, (char *str));
EXTERN(Boolean, skew_handler, (Dialog *di, Generic ALLH, Generic item_id));
EXTERN(void, skew_dialog_run, (AllDia *allh));

/* defined in split_d.c */
EXTERN(Boolean, split_handler, (Dialog *di, Generic SMIH, Generic item_id));
EXTERN(void, split_dialog_run, (StripDia *smih));

/* defined in sreplace_d.c */
EXTERN(Boolean, replace_s_handler, (Dialog *di, Generic ALLH, Generic item_id));
EXTERN(void, replace_s_dialog_run, (AllDia *allh));

/* defined in statement_d.c */
EXTERN(void, add_stmt_dialog_run, (AllDia *allh));
EXTERN(void, delete_stmt_dialog_run, (Generic DP));
EXTERN(void, sinter_dialog_run, (SinterDia *sih));
EXTERN(Boolean, sinter_handler, (Dialog *di, Generic SIH, Generic item_id));

/* defined in unroll_d.c */
EXTERN(Boolean, unroll_handler, (Dialog *di, Generic ALLH, Generic item_id));
EXTERN(void, unroll_dialog_run, (AllDia *allh));
EXTERN(Boolean, unroll_jam_handler, (Dialog *di, Generic ALLH, Generic item_id));
EXTERN(void, unroll_jam_dialog_run, (AllDia *allh));

/* defined in ped_dt.c */
EXTERN(void, ped_dt_update, (PedInfo ped, AST_INDEX root));

/* defined in ped_rsd.c */
EXTERN(void, ped_rsd_vector_init, (PedInfo ped, AST_INDEX root));

/* defined in symtab.c */
EXTERN(void, perf_sym_init, (void));
EXTERN(int, sym_search, (char *text, Boolean *newp));
EXTERN(int, hash, (char *string));
EXTERN(int, sym_alloc_rent, (void));
EXTERN(void, sym_init_rent, (int field, int val));
EXTERN(void, sym_free_rent, (int i));

/* defined in perf_init.c */
EXTERN(void, fast_init, (FILE *fp));
EXTERN(void, output_fastinit, (char *fname));

#if 0
/* defined in fit.c */
EXTERN(void, compute_pkt_costs, (float a, float b, CommData *oldcptr, 
                                 float *stup, float *pkt));
#endif


/* defined in PEDInterface.c */
EXTERN(AST_INDEX, get_invocation_stmt_list, (AST_INDEX stmt, char *name));

/* defined in fusion.c */
EXTERN(int, pt_fuse_test, (PedInfo ped, AST_INDEX loop));
EXTERN(int, pt_fuse_prev, (DT_info *dt, Subs_list *src, Subs_list *sink,
                           Loop_list *loops1, Loop_list *loops2, int level));
EXTERN(int, pt_fuse_outer, (FortTree ft, SideInfo *infoPtr, DT_info *dt_info,
                            AST_INDEX loop1, AST_INDEX loop2));
EXTERN(void, pt_fuse, (PedInfo ped, AST_INDEX loop));
EXTERN(void, pt_fuse_all, (PedInfo ped, AST_INDEX body, Boolean carry));

/* defined in loop.c */
EXTERN(void, pt_adjust, (PedInfo ped, char *adjust));
EXTERN(void, pt_reverse, (PedInfo ped));
EXTERN(void, pt_unswitch, (PedInfo ped));
EXTERN(int,pt_reverse_estimate,(PedInfo ped));
EXTERN(int,pt_unswitch_test,(PedInfo ped));

/* defined in trans.c */
EXTERN(void,pedDelete,(Generic PD, PedInfo ped));
EXTERN(void,doInterchange,(InterDia *ih, Generic handle));
EXTERN(void,doDistribution,(DistrDia *dh, Generic handle, int type));
EXTERN(void,doRenamingExpansion,(PedInfo ped));
EXTERN(void,doAddStmt,(PedInfo ped, char *arg));
EXTERN(void,doReverse,(PedInfo ped));
EXTERN(void,doAdjust,(PedInfo ped, char *adjust));
EXTERN(void,doAlign,(PedInfo ped, char *arg));
EXTERN(void,doUnroll,(PedInfo ped, char *arg));
EXTERN(void,doUnrollJam,(PedInfo ped, char *arg));
EXTERN(void,doFusion,(PedInfo ped));
EXTERN(void,doUnswitch,(PedInfo ped));
EXTERN(void,makeParallel,(PedInfo ped));
EXTERN(void,makeSequential,(PedInfo ped));

EXTERN(void,doPeelIterations,(PedInfo ped, Boolean iteration, char *iter));
EXTERN(void,doScalarExpansion,(PedInfo ped));
EXTERN(void,doSkew,(PedInfo ped, char *skew_degree));
EXTERN(void,doSplit,(PedInfo ped, char *step));
EXTERN(void,doReplaceS,(PedInfo ped));
EXTERN(void,doDeleteStmt,(PedInfo ped));
EXTERN(void,doStmtInter,(PedInfo ped));
EXTERN(void,doStripMine,(PedInfo ped, char *step));

#endif
