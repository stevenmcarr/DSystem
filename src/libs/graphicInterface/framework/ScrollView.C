/* $Id: ScrollView.C,v 1.7 1997/03/11 14:32:51 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      framework/ScrollView.C                                          */
/*                                                                      */
/*      ScrollView -- Scroll bars surrounding another view              */
/*	Last edited: November 15, 1993 at 11:11 am			*/
/*                                                                      */
/************************************************************************/




#include <libs/graphicInterface/framework/ScrollView.h>


#include <libs/graphicInterface/framework/CTextView.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/scroll_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/graphicInterface/oldMonitor/include/mon/cp.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* ScrollView object */

typedef struct ScrollView_Repr_struct
  {
    /* creation parameters */
      CTextView *	contents;
      Boolean		hscrollWanted;
      Boolean		vscrollWanted;
      Boolean		legendWanted;

    /* subparts */
      Generic		hscroll;
      Generic		vscroll;
      Generic		legend;
      Generic		wasted;

    /* status */
      Point		scrollPos;

  } ScrollView_Repr;


#define R(ob)		(ob->ScrollView_repr)

#define INHERITED	View






/*************************/
/*  Miscellaneous	 */
/*************************/




/* appearance of legend */

#define LEGEND_WIDTH		150




/* size preferences */

#define SCROLLBAR_MIN		 (SB_MIN_LENGTH+20)






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




void ScrollView::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(View);
}




void ScrollView::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(ScrollView)




ScrollView::ScrollView(CTextView * contents,
                       Boolean hscrollWanted,
                       Boolean vscrollWanted)
          : View(CONTEXT_NULL, DB_NULLFP, nil)
{
  /* allocate instance's private data */
    this->ScrollView_repr = (ScrollView_Repr *) get_mem(sizeof(ScrollView_Repr),
                                                        "ScrollView instance");

  /* save creation parameters */
    R(this)->contents      = contents;
    R(this)->hscrollWanted = hscrollWanted;
    R(this)->vscrollWanted = vscrollWanted;
    R(this)->legendWanted  = false;    /* TEMPORARY */
    
  /* set up change notification */
    contents->Notify(this, true);
}




ScrollView::~ScrollView(void)
{
  free_mem((void*) this->ScrollView_repr);
}






/************************/
/*  Change notification */
/************************/




void ScrollView::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);

  if( ob == R(this)->contents )
    this->Changed(kind, change);
  else
    R(this)->contents->NoteChange(ob, kind, change);
}






/******************/
/*  Window layout */
/******************/




void ScrollView::GetSizePrefs(Point &minSize, Point &defSize)
{
  int hscrollWidth = (R(this)->hscrollWanted ? SB_WIDTH : 0);
  int vscrollWidth = (R(this)->vscrollWanted ? SB_WIDTH : 0);
  
  R(this)->contents->GetSizePrefs(minSize, defSize);

  minSize.x  = max(minSize.x + vscrollWidth, SCROLLBAR_MIN);
  minSize.y  = max(minSize.y + hscrollWidth, SCROLLBAR_MIN);

  defSize.x += vscrollWidth;
  defSize.y += hscrollWidth;
}




Generic ScrollView::GetTiling(Boolean init, Point size)
{
  int hscrollWidth = (R(this)->hscrollWanted ? SB_WIDTH : 0);
  int vscrollWidth = (R(this)->vscrollWanted ? SB_WIDTH : 0);
  int legendWidth  = (R(this)->legendWanted  ? LEGEND_WIDTH : 0);
  Point contentsSize, legendSize, hscrollSize, vscrollSize, wastedSize;
  Generic td;

  /* initialize if necessary */
    if( init )
      { R(this)->hscroll = sm_scroll_get_index();
        R(this)->vscroll = sm_scroll_get_index();
        R(this)->legend  = sm_vanilla_get_index();
        R(this)->wasted  = sm_vanilla_get_index();
      }

  /* calculate sizes of all the panes */
    /* first see whether there will be room for scroll bars */
      if( size.x < vscrollWidth )  vscrollWidth  = 0;
      if( size.y < hscrollWidth )  hscrollWidth  = 0;

    /* the contents pane */
      contentsSize.x = size.x - vscrollWidth;
      contentsSize.y = size.y - hscrollWidth;

    /* the view legend */
      legendSize.x = legendWidth;
      legendSize.y = hscrollWidth;

    /* the horizontal scrollbar */
      hscrollSize.x = contentsSize.x - legendSize.x;
      hscrollSize.y = hscrollWidth;

    /* the vertical scrollbar */
      vscrollSize.x = vscrollWidth;
      vscrollSize.y = contentsSize.y;

    /* the wasted bottom right corner */
      wastedSize.x = vscrollWidth;
      wastedSize.y = hscrollWidth;

  /* compute the tiling descriptor */
    td = R(this)->contents->GetTiling(init, contentsSize);
    
    if( vscrollWidth != 0 )
      td = cp_td_join(TILE_LEFT,
                      (aTilingDesc*)td,
                      (aTilingDesc*)cp_td_pane((Pane**)&R(this)->vscroll, vscrollSize));
    else
      R(this)->vscroll = nil;
       
    if( hscrollWidth != 0 )
      td = cp_td_join(
              TILE_UP,
              (aTilingDesc*)td,
              (aTilingDesc*)cp_td_join(
                               TILE_LEFT,
                               (aTilingDesc*)cp_td_pane(
                                                (Pane**)&R(this)->legend, 
                                                legendSize),
                               (aTilingDesc*)cp_td_join(
                                                TILE_LEFT,
                                                (aTilingDesc*)cp_td_pane(
                                                                 (Pane**)&R(this)->hscroll, 
                                                                 hscrollSize),
                                                (aTilingDesc*)cp_td_pane(
                                                                 (Pane**)&R(this)->wasted, 
                                                                 wastedSize))));
    else
      R(this)->hscroll = nil;

  return td;
}




void ScrollView::InitPanes(void)
{
  /* TEMPORARY */

  R(this)->contents->InitPanes();
  R(this)->contents->SetScrollBars(R(this)->hscroll, R(this)->vscroll);
}






/**************/
/*  Scrolling */
/**************/




Point ScrollView::GetScroll(void)
{
  return R(this)->contents->GetScroll();
}




void ScrollView::SetScroll(Point scrollPos)
{
  R(this)->contents->SetScroll(scrollPos);
}




void ScrollView::ScrollBy(Point delta)
{
  R(this)->contents->ScrollBy(delta);
}




void ScrollView::EnsureVisible(Point pt)
{
  R(this)->contents->EnsureVisible(pt);
}




void ScrollView::EnsureSelVisible(void)
{
  R(this)->contents->EnsureSelVisible();
}






/*************/
/* Filtering */
/*************/




CViewFilter * ScrollView::GetFilter(void)
{
  return R(this)->contents->GetFilter();
}




void ScrollView::SetFilter(CViewFilter * filter)
{
  R(this)->contents->SetFilter(filter);
}






/******************/
/* Input handling */
/******************/




Boolean ScrollView::SelectionEvent(Generic generator, Point info)
{
  return R(this)->contents->SelectionEvent(generator, info);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
