/* $Id: SearchDialog.h,v 1.5 1997/03/11 14:30:58 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditorCP/SearchDialog.h				*/
/*									*/
/*	SearchDialog -- specify and control global searches etc.	*/
/*	Last edited: September 22, 1989 at 11:45 am			*/
/*									*/
/************************************************************************/




#ifndef SearchDialog_h
#define SearchDialog_h


typedef Generic SearchDialog;




/************************/
/*  Initialization	*/
/************************/

EXTERN(void,		sdlg_Init,(void));
EXTERN(void,		sdlg_Fini,(void));

EXTERN(SearchDialog,	sdlg_Open,(Context context, DB_FP *fd, FortEditorCP edcp,
                                   FortEditor editor));
EXTERN(void,		sdlg_Close,(SearchDialog sdlg));
EXTERN(void,		sdlg_Save,(SearchDialog sdlg, Context context, DB_FP *fd));
EXTERN(void,		sdlg_SetEditor,(SearchDialog sdlg, FortEditor editor));




/************************/
/*  User interaction	*/
/************************/

EXTERN(void,		sdlg_Find,(SearchDialog sdlg));
EXTERN(void,		sdlg_FindNext,(SearchDialog sdlg));
EXTERN(void,		sdlg_FindPrevious,(SearchDialog sdlg));
EXTERN(void,		sdlg_Replace,(SearchDialog sdlg));




/************************/
/*  Notification	*/
/************************/

EXTERN(void,		sdlg_NoteChange,(SearchDialog sdlg, int kind, Boolean 
                                         autoScroll, FortTreeNode node, int first,
                                         int last, int delta));




#endif
