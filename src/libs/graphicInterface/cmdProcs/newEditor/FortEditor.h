/* $Id: FortEditor.h,v 1.5 1997/03/11 14:30:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditor.h						*/
/*									*/
/*	FortEditor -- abstract text/structure editor for Fortran	*/
/*	Last edited: September 25, 1992 at 4:25 pm			*/
/*									*/
/************************************************************************/




#ifndef FortEditor_h
#define FortEditor_h

#ifndef FortVFilter_h
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/FortVFilter.h>
#endif

#ifndef scroll_sm_h
#include <libs/graphicInterface/oldMonitor/include/sms/scroll_sm.h>
#endif


typedef Generic FortEditor;
typedef Generic FortEditorView;
typedef	Generic	FortScrap;


extern char	*ed_SourceAttribute;




/************************/
/*  Initialization	*/
/************************/

EXTERN(void, ed_Init, (void));
EXTERN(void, ed_Fini, (void));

EXTERN(FortEditor, ed_Open, (Context context, DB_FP *fp, FortTextTree ftt,
			     FortTree ft));
EXTERN(void, ed_Close, (FortEditor ed));
EXTERN(void, ed_Save, (FortEditor ed, Context context,
		       DB_FP *fp, Boolean saveEditState));

typedef FUNCTION_POINTER(void, ed_NotifyFunc,
			 (Generic ob, int kind, Boolean autoscroll, FortTreeNode node,
			  int first, int last, int delta));
EXTERN(void, ed_Notify,
       (FortEditor ed, Generic ob, ed_NotifyFunc method));




/************************/
/*  Contents		*/
/************************/

EXTERN(Point, ed_DocSize, (FortEditor ed));
EXTERN(void, ed_GetTextTree, (FortEditor ed, FortTextTree *ftt));
EXTERN(void, ed_GetTree, (FortEditor ed, FortTree *ft));
EXTERN(void, ed_GetRoot, (FortEditor ed, FortTreeNode *node));
EXTERN(int, ed_NumLines, (FortEditor ed));
EXTERN(void, ed_GetLine, (FortEditor ed, int lineNum,
			  TextString *text));
EXTERN(int, ed_GetLineLength, (FortEditor ed, int lineNum));
EXTERN(int, ed_GetLineIndent, (FortEditor ed, int lineNum));
EXTERN(char *, ed_GetTextLine, (FortEditor ed, int lineNum));
EXTERN(Boolean, ed_NodeToText, (FortEditor ed, FortTreeNode node,
				int *line1, int *char1, int *line2, int *char2));
EXTERN(Boolean, ed_TextToNode, (FortEditor ed, int line1, int char1,
				int line2, int char2, FortTreeNode *node));
EXTERN(void, ed_TreeWillChange, (FortEditor ed, FortTreeNode node));
EXTERN(void, ed_TreeChanged, (FortEditor ed, FortTreeNode node));




/************************/
/*  Viewing		*/
/************************/

EXTERN(short, ed_ViewScreenModuleIndex, (void));
EXTERN(Point, ed_ViewSize, (Point charSize, short font));

EXTERN(void, ed_ViewInit, (FortEditor ed, FortEditorView pane,
			   FortVFilter filter, Point scrollPos, short font));

#ifdef scroll_sm_h
EXTERN(void, ed_ViewScrollBars, (FortEditor ed, FortEditorView pane,
				 ScrollBar hscroll, ScrollBar vscroll));
#endif

EXTERN(Point, ed_ViewGetSize, (FortEditor ed, FortEditorView pane));

EXTERN(void, ed_ViewGetFilter, (FortEditor ed, FortEditorView pane,
				FortVFilter *filter));
EXTERN(void, ed_ViewSetFilter, (FortEditor ed, FortEditorView pane,
				FortVFilter filter));
EXTERN(void, ed_ViewSetFilterFast, (FortEditor ed,
				    FortEditorView pane, FortVFilter filter, Rectangle changed));
EXTERN(void, ed_ViewGetConceal, (FortEditor ed, FortEditorView pane,
				 int line, Boolean *conceal));
EXTERN(void, ed_ViewSetConceal, (FortEditor ed, FortEditorView pane,
				 int line1, int line2, Boolean conceal));
EXTERN(void, ed_ViewSetConcealNone, (FortEditor ed, 
				     FortEditorView pane, int line1, int line2));

EXTERN(Point, ed_ViewGetScroll, (FortEditor ed, FortEditorView pane));
EXTERN(void, ed_ViewSetScroll, (FortEditor ed, FortEditorView pane,
				Point scrollPos));
EXTERN(void, ed_ViewScrollBy, (FortEditor ed, FortEditorView pane,
			       Point delta));
EXTERN(void, ed_ViewEnsureVisible, (FortEditor ed,
				    FortEditorView pane, Point pt));
EXTERN(void, ed_ViewEnsureSelVisible, (FortEditor ed, 
				       FortEditorView pane));

EXTERN(void, ed_ViewMouse, (FortEditor ed, FortEditorView pane,
			    Point mousePt));


/* selection behavior kinds  -- must concur with TextView.h	*/

#define ed_SB_NORMAL		0
#define ed_SB_LINES_ONLY	1

EXTERN(void, ed_ViewGetSelectionBehavior, (FortEditor ed,
					   FortEditorView pane, int *beh));
EXTERN(void, ed_ViewSetSelectionBehavior, (FortEditor ed,
					   FortEditorView pane, int beh));






/************************/
/*  Selecting		*/
/************************/

EXTERN(void, ed_GetSelection, (FortEditor ed, int *line, int *sel1,
			       int *sel2));
EXTERN(void, ed_SetSelection, (FortEditor ed, int line, int sel1,
			       int sel2));
EXTERN(void, ed_MoreSelection, (FortEditor ed));

EXTERN(void, ed_GetSelectedNode, (FortEditor ed, FortTreeNode *node));
EXTERN(void, ed_SetSelectedNode, (FortEditor ed, FortTreeNode node));

EXTERN(void, ed_ToggleTypingField, (FortEditor ed));




/************************/
/*  Editing		*/
/************************/

EXTERN(void, ed_BeginEdit, (FortEditor ed));
EXTERN(void, ed_EndEdit, (FortEditor ed));

EXTERN(void, ed_Key, (FortEditor ed, KbChar kb));

EXTERN(void, ed_Expand, (FortEditor ed, int choice));

EXTERN(void, ed_Copy, (FortEditor ed, FortScrap *scrap));
EXTERN(void, ed_Cut, (FortEditor ed, FortScrap *scrap));
EXTERN(void, ed_Paste, (FortEditor ed, FortScrap scrap));
EXTERN(void, ed_PasteString, (FortEditor ed, char *str));
EXTERN(void, ed_Clear, (FortEditor ed));
EXTERN(void, ed_ClearToEndOfLine, (FortEditor ed));
EXTERN(void, ed_CheckLine, (FortEditor ed));
EXTERN(Boolean, ed_CheckModule, (FortEditor ed));




/************************/
/*  Searching		*/
/************************/

EXTERN(void, ed_SetPattern, (FortEditor ed, FortVFilter filter));
EXTERN(void, ed_SetPatternText, (FortEditor ed, char *str,
				 Boolean fold));
EXTERN(Boolean, ed_Find, (FortEditor ed, Boolean dir));
EXTERN(Boolean, ed_FindPlaceholder, (FortEditor ed, Boolean dir));
EXTERN(int, ed_ReplaceText, (FortEditor ed, Boolean dir,
			     Boolean global, Boolean all, char *newStr));
EXTERN(int, ed_ReplaceTree, (FortEditor ed, Boolean dir,
			     Boolean global, Boolean all, FortTreeNode newNode));




#endif
