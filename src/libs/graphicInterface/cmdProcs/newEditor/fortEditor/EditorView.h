/* $Id: EditorView.h,v 1.4 1997/03/11 14:30:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditor/EditorView.h					*/
/*									*/
/*	EditorView -- screen module showing a view on a FortEditor	*/
/*	Last edited: September 21, 1989 at 5:32 pm			*/
/*									*/
/************************************************************************/




#ifndef EditorView_h
#define EditorView_h


typedef Generic EditorView;




/************************/
/*  Initialization	*/
/************************/

EXTERN(void,	        ev_Init,(void));
EXTERN(void,		ev_Fini,(void));




/************************/
/*  Screen module	*/
/************************/

EXTERN(short,		ev_ScreenModuleIndex,(void));
EXTERN(Point,		ev_ViewSize,(Point charSize, short font));




/************************/
/*  Instance init	*/
/************************/

EXTERN(void,		ev_PaneInit,(EditorView ev, FortEditor ed, 
                                     FortVFilter filter, Point scrollPos, 
                                     short font));
EXTERN(void,		ev_ScrollBars,(EditorView ev, ScrollBar hscroll,
                                       ScrollBar vscroll));




/************************/
/*  Change notification	*/
/************************/

EXTERN(void,		ev_NoteChange,(EditorView ev, int kind, Boolean autoScroll,
                                       FortTreeNode node, int first, int last,
                                       int delta));




/************************/
/*  Access to view	*/
/************************/

EXTERN(Point,		ev_GetViewSize,(EditorView ev));
EXTERN(Point,		ev_GetScroll,(EditorView ev));
EXTERN(void,		ev_SetScroll,(EditorView ev, Point scrollPos));
EXTERN(void,		ev_ScrollBy,(EditorView ev, Point delta));
EXTERN(void,		ev_EnsureVisible,(EditorView ev, Point pt, 
                                          Boolean bounce));
EXTERN(void,		ev_GetSelectionBehavior,(EditorView ev, int *beh));
EXTERN(void,		ev_SetSelectionBehavior,(EditorView ev, int beh));




/************************/
/*  View filtering	*/
/************************/

EXTERN(void,		ev_GetFilter,(EditorView ev, FortVFilter *filter));
EXTERN(void,		ev_SetFilter,(EditorView ev, FortVFilter filter, 
                                      Boolean coords, Rectangle changed));
EXTERN(void,		ev_GetConceal,(EditorView ev, int line, 
                                       Boolean *conceal));
EXTERN(void,		ev_SetConceal,(EditorView ev, int line1, int line2, 
                                       Boolean conceal));




#endif
