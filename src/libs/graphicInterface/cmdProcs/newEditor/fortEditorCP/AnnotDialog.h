/* $Id: AnnotDialog.h,v 1.6 1997/06/25 13:47:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditorCP/AnnotDialog.h				*/
/*									*/
/*	AnnotDialog -- browse and navigate annotations			*/
/*	Last edited: August 25, 1993 at 12:47 pm			*/
/*									*/
/************************************************************************/




#ifndef AnnotDialog_h
#define AnnotDialog_h


typedef Generic AnnotDialog;




/************************/
/*  Initialization	*/
/************************/

EXTERN(void, adlg_Init, (void));
EXTERN(void, adlg_Fini, (void));

EXTERN(AnnotDialog, adlg_Open,
      (Context context, DB_FP* fp, FortEditorCP edcp, FortEditor editor));
EXTERN(void, adlg_Close, (AnnotDialog adl));
EXTERN(void, adlg_Save,
      (AnnotDialog adlg, Context context, DB_FP* fp));




/************************/
/*  User interaction	*/
/************************/

EXTERN(void, adlg_Dialog, (AnnotDialog adlg, Generic annot));




/************************/
/*  Notification	*/
/************************/

EXTERN(void, adlg_NoteChange,
      (AnnotDialog adlg, int kind, Boolean autoScroll, FortTreeNode node,
       int first, int last, int delta));




#endif
