/* $Id: View.C,v 1.9 1997/03/11 14:32:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      framework/View.C                                                */
/*                                                                      */
/*      View -- Abstract class for tilable views                        */
/*	Last edited: October 13, 1993 at 11:00 pm			*/
/*                                                                      */
/************************************************************************/




#include <libs/graphicInterface/framework/View.h>

#include <libs/graphicInterface/framework/CViewFilter.h>
#include <libs/graphicInterface/framework/LineEditor.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/graphicInterface/oldMonitor/include/mon/cp.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* View object */

typedef struct View_Repr_struct
  {
    /* creation parameters */
      Editor *		editor;

    /* default tiling */
      Generic		defaultPane;	/* not used by subclasses */

    /* viewing state */
      CViewFilter *	filter;

  } View_Repr;


#define R(ob)		(ob->View_repr)

#define INHERITED	Object






/*************************/
/*  Forward declarations */
/*************************/




/* none */







/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void View::InitClass(void)
{
  /* initialize needed submodules */
    /* ... */
}




void View::FiniClass(void)
{
  /* ... */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(View)




View::View(Context context,
           DB_FP * session_fp,
           Editor * editor)
   : DBObject (context, session_fp)
{
  /* allocate instance's private data */
    this->View_repr = (View_Repr *) get_mem(sizeof(View_Repr), "View instance");

  /* save creation parameters */
    R(this)->editor = editor;

  /* get persistent info */
    if( false /*** session_fp != DB_NULLFP ***/ )
      { /* ... */
      }
    else
      { /* ... */
      }

  /* request change notification */
    if( editor != nil )
      editor->Notify(this, true);
  
  /* initialize viewing state */
    R(this)->filter = nil;
}




View::~View(void)
{
  free_mem((void*) this->View_repr);
}




Editor * View::GetEditor(void)
{
  return R(this)->editor;
}






/******************/
/*  Window layout */
/******************/




void View::GetSizePrefs(Point &minSize, Point &defSize)
{
  minSize = makePoint(100,  50);
  defSize = makePoint(600, 200);
}




Generic View::GetTiling(Boolean init, Point size)
{
  if( init )  R(this)->defaultPane = sm_vanilla_get_index();
  return cp_td_pane((Pane**)&R(this)->defaultPane, size);
}




void View::InitPanes(void)
{
  /* nothing */
}






/*************/
/* Filtering */
/*************/




CViewFilter * View::GetFilter(void)
{
  return R(this)->filter;
}




void View::SetFilter(CViewFilter * filter)
{
  LineEditorChange lch;

  R(this)->filter = filter;
  this->Changed(CHANGE_FILTER, (void *) filter);

  lch.kind = 1;
  lch.autoScroll = false;
  lch.first = 0;
  lch.last  = INFINITY;
  lch.delta = INFINITY;
  this->NoteChange(nil, CHANGE_DOCUMENT, &lch);
}






/******************/
/* Input handling */
/******************/




Boolean View::SelectionEvent(Generic generator, Point info)
{
  return false;
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
