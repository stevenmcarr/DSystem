/* $Id: SearchDialog.C,v 1.1 1997/06/25 13:47:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditorCP/SearchDialog.c				*/
/*									*/
/*	SearchDialog -- specify and control global searches etc.	*/
/*	Last edited: September 22, 1989 at 11:48 am			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditorCP/FortEditorCP.i>

#include <libs/graphicInterface/cmdProcs/newEditor/fortEditorCP/SearchDialog.h>

#include <libs/graphicInterface/oldMonitor/include/dialogs/find.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




typedef struct
  {
    /* creation parameters */
      FortEditorCP         edcp;
      FortEditor           editor;

    /* contents */
      aFRDia *             dialog;

    /* status */
      /* ... */

  } sdlg_Repr;

#define	R(ob)		((sdlg_Repr *) ob)




/*************************/
/*  Forward declarations */
/*************************/


static Boolean findProc(SearchDialog sdlg, aFRDia *dialog, char *findString, Boolean
                        dir, Boolean fold);
static void    replaceProc(SearchDialog sdlg, aFRDia *dialog, char *findString,
                           Boolean fold, char *replaceString);
static int     globalReplaceProc(SearchDialog sdlg, aFRDia *dialog, char *findString,
                                 Boolean all, Boolean dir, Boolean fold, 
                                 char *replaceString);






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




void sdlg_Init()
{
  /* nothing */
}




void sdlg_Fini()
{
  /* nothing */
}




/*ARGSUSED*/

SearchDialog sdlg_Open(Context context, DB_FP *fp, FortEditorCP edcp, FortEditor editor)
{
  SearchDialog sdlg;

  /* allocate a new instance */
    sdlg = (SearchDialog) get_mem(sizeof(sdlg_Repr),"FortEditorCP:SearchDialog");
    if( (Generic) sdlg == 0 ) return UNUSED;

  /* initialize the parts */
    /* set creation parameters */
      R(sdlg)->edcp = edcp;
      R(sdlg)->editor = editor;

    /* make a standard find/replace dialog */
      R(sdlg)->dialog = find_dialog_create("Find", "Replace",
                                           "", "", FRD_FORWARD, true,
                                           findProc,
                                           replaceProc,
                                           globalReplaceProc,
                                           sdlg
                                          );

  return sdlg;
}




/*ARGSUSED*/

void sdlg_Close(SearchDialog sdlg)
{
  find_dialog_destroy(R(sdlg)->dialog);

  free_mem((void*) sdlg);
}




/*ARGSUSED*/

void sdlg_Save(SearchDialog sdlg, Context context, DB_FP *fp)
{
  /* ... */
}




void sdlg_SetEditor(SearchDialog sdlg, FortEditor editor)
{
  R(sdlg)->editor = editor;
  sdlg_NoteChange(sdlg,
                  NOTIFY_DOC_CHANGED, false, UNUSED, UNUSED, UNUSED, UNUSED);
}






/************************/
/*  User interaction	*/
/************************/




void sdlg_Find(SearchDialog sdlg)
{
  find_dialog_run_find(R(sdlg)->dialog);
}




void sdlg_FindNext(SearchDialog sdlg)
{
  char * findString;
  char * sdummy;
  Boolean fold,bdummy;

  find_dialog_get_values(R(sdlg)->dialog,&findString,&sdummy,&bdummy,&fold);
  (void) findProc(sdlg,R(sdlg)->dialog,findString,FRD_FORWARD,fold);
}




void sdlg_FindPrevious(SearchDialog sdlg)
{
  char * findString;
  char * sdummy;
  Boolean fold,bdummy;

  find_dialog_get_values(R(sdlg)->dialog,&findString,&sdummy,&bdummy,&fold);
  (void) findProc(sdlg,R(sdlg)->dialog,findString,FRD_BACKWARD,fold);
}




void sdlg_Replace(SearchDialog sdlg)
{
  find_dialog_run_replace(R(sdlg)->dialog);
}






/************************/
/*  Notification	*/
/************************/




/*ARGSUSED*/

void sdlg_NoteChange(SearchDialog sdlg, int kind, Boolean autoScroll, 
                     FortTreeNode node, int first, int last, int delta)
{
  find_dialog_dirty(R(sdlg)->dialog);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/*ARGSUSED*/

static
Boolean findProc(SearchDialog sdlg, aFRDia *dialog, char *findString, 
                 Boolean dir, Boolean fold)
{
  ed_SetPatternText(R(sdlg)->editor,findString,fold);
  return ed_Find(R(sdlg)->editor,dir);
}




/*ARGSUSED*/

static
void replaceProc(SearchDialog sdlg, aFRDia *dialog, char *findString, 
                 Boolean fold, char *replaceString)
{
  ed_SetPatternText(R(sdlg)->editor,findString,fold);
  (void) ed_ReplaceText(R(sdlg)->editor,FRD_FORWARD,false,false,replaceString);
}




/*ARGSUSED*/

static
int globalReplaceProc(SearchDialog sdlg, aFRDia *dialog, char *findString, 
                      Boolean all, Boolean dir, Boolean fold, char *replaceString)
{
  ed_SetPatternText(R(sdlg)->editor,findString,fold);
  return ed_ReplaceText(R(sdlg)->editor,dir,true,all,replaceString);
}









