/* $Id: ViewDialog.h,v 1.4 1997/03/11 14:30:58 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditorCP/ViewDialog.h				*/
/*									*/
/*	ViewDialog -- see and change view filters			*/
/*	Last edited: November 8, 1988 at 2:38 pm			*/
/*									*/
/************************************************************************/




#ifndef ViewDialog_h
#define ViewDialog_h


typedef Generic ViewDialog;




/************************/
/*  Initialization	*/
/************************/

EXTERN(void,		vdlg_Init,(void));
EXTERN(void,		vdlg_Fini,(void));

EXTERN(ViewDialog,	vdlg_Open,(Context context, DB_FP *fd, FortEditorCP edcp, 
                                   FortEditor editor));
EXTERN(void,		vdlg_Close,(ViewDialog vdlg));
EXTERN(void,		vdlg_Save,(ViewDialog vdlg, Context context, DB_FP *fd));
EXTERN(int,		vdlg_Duplcate,(void/* oldContext, newContext */));




/************************/
/*  User interaction	*/
/************************/

EXTERN(Boolean,		vdlg_Dialog,(ViewDialog vdlg));
EXTERN(Boolean,		vdlg_Menu,(ViewDialog vdlg, FortVFilter *filter));




/************************/
/*  Access to filters	*/
/************************/

EXTERN(int,		vdlg_NumFilterSpecs,(ViewDialog vdlg));
EXTERN(void,		vdlg_GetFilterSpec,(ViewDialog vdlg, int num, char **name, 
                                            char **definition, Boolean *concealed, 
                                            Boolean *errors));
EXTERN(void,		vdlg_GetFilterByName,(ViewDialog vdlg, char *name, 
                                              FortVFilter *filter));




#endif
