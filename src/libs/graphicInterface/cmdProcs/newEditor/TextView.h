/* $Id: TextView.h,v 1.9 1997/03/11 14:30:35 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/TextView.h						*/
/*									*/
/*	TextView -- screen module showing a view on a text		*/
/*	Last edited:  August 27, 1993 at 6:45 pm	    		*/
/*									*/
/************************************************************************/




#ifndef TextView_h
#define TextView_h


#ifdef sm_h
typedef Pane * TextView;
#else
typedef Generic TextView;
#endif

typedef FUNCTION_POINTER(void, tv_GetDocSizeFunc,
 (Generic contents, Point *size));
typedef FUNCTION_POINTER(Boolean, tv_GetLineFunc,
 (Generic contents, int lineNum, TextString *text,
  /* VF_MaxData * */ Generic info));
typedef FUNCTION_POINTER(void, tv_GetSelectionFunc,
 (Generic contents, int *line, int *sel1, int *sel2));
typedef FUNCTION_POINTER(void, tv_SetSelectionFunc,
 (Generic contents, int line, int sel1, int sel2));

typedef struct
  {
    tv_GetDocSizeFunc	getDocSize;
    tv_GetLineFunc	getLine;
    tv_GetSelectionFunc	getSelection;
    tv_SetSelectionFunc	setSelection;
  } TV_Methods;






/***************************/
/* Color and customization */
/***************************/


#define tv_MAX_CHARS  500

typedef struct TV_ColorPair_struct
  {
    Color		foreground;
    Color		background;

  } TV_ColorPair;


typedef struct TV_Data_struct
  {
    Boolean		multiColored;
    TV_ColorPair	all;
    TV_ColorPair	chars[tv_MAX_CHARS];

  } TV_Data;




typedef FUNCTION_POINTER(void, tv_CustomRepaintFunc,
 (Generic ob, Pane * textPane, /* ViewFilter */ Generic filter,
  Rectangle * viewRect, int pane_line1, int pane_line2));





/************************/
/*  Screen module	*/
/************************/


EXTERN(short, tv_ScreenModuleIndex, (void));

EXTERN(Point, tv_ViewSize, (Point charSize, short font));


/************************/
/*  Instance init	*/
/************************/


EXTERN(void, tv_DefaultData, (TV_Data *data));

EXTERN(void, tv_MultiColoredData, (TV_Data *data));

EXTERN(void, tv_PaneInit, (TextView tv, Generic contents,
 TV_Methods *methods, /* ViewFilter */ Generic filter, Point scrollPos,
 int font));

EXTERN(void, tv_ScrollBars, (TextView tv,
 /* ScrollBar */ Pane *hscroll, /* ScrollBar */ Pane *vscroll));

EXTERN(Generic, tv_getTextPane, (TextView tv));

EXTERN(void, tv_CustomizeRepainting,
 (TextView tv, Generic repaintOb, tv_CustomRepaintFunc repaintProc));


/************************/
/*  Change notification	*/
/************************/


#define tt_NOTIFY_SEL_CHANGED        0
#define tt_NOTIFY_DOC_WILL_CHANGE   -1
#define tt_NOTIFY_DOC_CHANGED        1

EXTERN(void, tv_NoteChange, (TextView tv, int kind,
 Boolean autoScroll, int first, int last, int delta));




/************************/
/*  Access to view	*/
/************************/


EXTERN(Point, tv_GetViewSize, (TextView tv));

EXTERN(Point, tv_GetScroll, (TextView tv));

EXTERN(void, tv_SetScroll, (TextView tv, Point scrollPos));

EXTERN(void, tv_ScrollBy, (TextView tv, Point delta));

EXTERN(void, tv_EnsureContentsVisible, (TextView tv, Point pt,
 Boolean bounce));

EXTERN(void, tv_EnsureVisible, (TextView tv, Point pt,
 Boolean bounce));

EXTERN(/* ViewFilter */ Generic, tv_GetFilter, (TextView tv));

EXTERN(void, tv_SetFilter, (TextView tv,
 /* ViewFilter */ Generic filter, Boolean coords, Rectangle changed));


/* selection behavior kinds  -- must concur with FortEditor.h	*/

#define tv_SB_NORMAL		0
#define tv_SB_LINES_ONLY	1

EXTERN(void, tv_GetSelectionBehavior, (TextView tv, int *beh));
EXTERN(void, tv_SetSelectionBehavior, (TextView tv, int beh));




#endif
