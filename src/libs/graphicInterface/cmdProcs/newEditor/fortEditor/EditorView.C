/* $Id: EditorView.C,v 1.1 1997/06/25 13:43:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditor/EditorView.c					*/
/*									*/
/*	EditorView -- screen module showing a view on a FortEditor	*/
/*	Last edited: October 6, 1992 at 5:46 pm				*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/FortEditor.i>

#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/EditorView.h>

#include <libs/graphicInterface/oldMonitor/include/sms/text_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/scroll_sm.h>




/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




typedef struct
  {
    FortEditor	editor;
    FortVFilter	filter;

    TextView	tv;
  } ev_Repr;


#define	SM(ob)		((Pane *) ob)
#define	R(ob)		((ev_Repr *) SM(ob)->pane_information)






/************************/
/* Screen module	*/
/************************/




/* these must be declared early for sm structure below */

STATIC(void, init,(void));
STATIC(void, fini,(void));
STATIC(void, create,(EditorView ev));
STATIC(void, destroy,(EditorView ev));
STATIC(void, resize,(EditorView ev));
STATIC(void, input,(EditorView ev, Rectangle r));


static aScreenModule ev_screenModuleOps =
{
  "EditorView",
  (sm_start_func)init,
  (sm_finish_func)fini,
  (sm_create_func)create,
  (sm_resize_func)resize,
  (sm_propagate_change_func)standardNoSubWindowPropagate,
  (sm_destroy_func)destroy,
  (sm_input_func)input,
  (sm_window_tile_func)standardTileNoWindow,
  (sm_window_destroy_func)standardDestroyWindow
};




/* Customizing methods for TextView */

STATIC(void, getDocSize,(FortEditor ed, Point *size));/* need this declaration early */
STATIC(Boolean, getLine,(FortEditor ed, int line, TextString *ext, TV_Data *data));
   /* need this declaration early */

static TV_Methods ev_Methods =
  { 
    (tv_GetDocSizeFunc)getDocSize,	/* converts from function to result parameter */
    (tv_GetLineFunc)getLine,		/* adds handling of "data" argument & bool result */
    (tv_GetSelectionFunc)ed_GetSelection,
    (tv_SetSelectionFunc)ed_SetSelection
  };



/************************/
/* Forward declarations	*/
/************************/



/* static void		getDocSize(); */
/* static Boolean	getLine(); */






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Screen module	*/
/************************/




short ev_ScreenModuleIndex()
{
  return getScreenModuleIndex(&ev_screenModuleOps);
}




Point ev_ViewSize(Point charSize, short font)
{
  return tv_ViewSize(charSize,font);
}






/************************/
/*  Instance init	*/
/************************/




void ev_PaneInit(EditorView ev, FortEditor ed, FortVFilter filter, 
                 Point scrollPos, short font)
{
  ViewFilter vf = ff_ViewFilter(filter);
  
  R(ev)->editor = ed;
  R(ev)->filter = filter;

  tv_PaneInit(R(ev)->tv,ed,&ev_Methods,vf,scrollPos,font);
}




void ev_ScrollBars(EditorView ev, ScrollBar hscroll, ScrollBar vscroll)
{
  tv_ScrollBars(R(ev)->tv, hscroll, vscroll);
}




void ev_NoteChange(EditorView ev, int kind, Boolean autoScroll, FortTreeNode node, 
                   int first, int last, int delta)
{
  ff_NoteChange(R(ev)->filter, kind, autoScroll, node, first, last, delta);
}






/************************/
/*  Access to View	*/
/************************/




Point ev_GetViewSize(EditorView ev)
{
  return tv_GetViewSize(R(ev)->tv);
}




Point ev_GetScroll(EditorView ev)
{
  return tv_GetScroll(R(ev)->tv);
}




void ev_SetScroll(EditorView ev, Point scrollPos)
{
  tv_SetScroll(R(ev)->tv,scrollPos);
}




void ev_ScrollBy(EditorView ev, Point delta)
{
  tv_ScrollBy(R(ev)->tv,delta);
}




void ev_EnsureVisible(EditorView ev, Point pt, Boolean bounce)	
/* => EnsureContentsVisible */
{
  tv_EnsureContentsVisible(R(ev)->tv,pt,bounce);
}




void ev_GetSelectionBehavior(EditorView ev, int *beh)
{
  tv_GetSelectionBehavior(R(ev)->tv, beh);
}




void ev_SetSelectionBehavior(EditorView ev, int beh)
{
  tv_SetSelectionBehavior(R(ev)->tv, beh);
}






/************************/
/*  View filtering	*/
/************************/




/* ARGSUSED */

void ev_GetFilter(EditorView ev, FortVFilter *filter)
{
  *filter = R(ev)->filter;
}




void ev_SetFilter(EditorView ev, FortVFilter filter, Boolean coords, Rectangle changed)
{
  ViewFilter vf = ff_ViewFilter(filter);

  R(ev)->filter = filter;
  tv_SetFilter(R(ev)->tv,vf,coords,changed);
}




/* ARGSUSED */

void ev_GetConceal(EditorView ev, int line, Boolean *conceal)
{
  *conceal = false;
}




/* ARGSUSED */

void ev_SetConceal(EditorView ev, int line1, int line2, Boolean conceal)
{
  char * what = (conceal ? "concealing" : "revealing");

  notImplemented(what);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void init()
{
  /* nothing */
}




static
void fini()
{
  /* nothing */
}




static
void create(EditorView ev)
{
  short tv_sm = tv_ScreenModuleIndex();

  SM(ev)->border_width = 1;
  SM(ev)->pane_information = (Generic)get_mem(sizeof(ev_Repr),"EditorView");

  R(ev)->tv = (TextView) newSlavePane(SM(ev), tv_sm, SM(ev)->position, SM(ev)->size,
                                      SM(ev)->border_width);

  R(ev)->editor = nil;
}





static
void destroy(EditorView ev)
{
  destroyPane(R(ev)->tv);
  free_mem((void*) SM(ev)->pane_information);
}




static
void resize(EditorView ev)
{
  resizePane(R(ev)->tv, SM(ev)->position, SM(ev)->size);
}




/* ARGSUSED */

static
void input(EditorView ev, Rectangle r)
{
  handlePane(R(ev)->tv);
}




static
void getDocSize(FortEditor ed, Point *size)
{
  *size = ed_DocSize(ed);
}




/* ARGSUSED */

static
Boolean getLine(FortEditor ed, int line, TextString *text, TV_Data *data)
{
  if( line < ed_NumLines(ed) )
    { ed_GetLine(ed,line,text);
      tv_DefaultData(data);
      return true;
    }
  else
    return false;
}
