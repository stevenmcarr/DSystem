/* $Id: NavView.C,v 1.2 1997/03/11 14:30:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ded_cp/nav/NavView.C						*/
/*                                                                      */
/*	NavView -- Overview of DedDocument's source code		*/
/*	Last edited: October 18, 1993 at 10:35 am			*/
/*                                                                      */
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/nav/NavView.h>

#include <libs/graphicInterface/cmdProcs/dEditor/nav/CostBarDecoration.h>
#include <libs/graphicInterface/cmdProcs/dEditor/nav/SubprogDecoration.h>
#include <libs/graphicInterface/cmdProcs/dEditor/nav/LoopDecoration.h>
#include <libs/graphicInterface/cmdProcs/dEditor/nav/NavEditor.h>
#include <libs/graphicInterface/cmdProcs/dEditor/DedEditor.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* NavView object */

typedef struct NavView_Repr_struct
  {
    /* creation parameters */
      NavEditor *		editor;
      DedDocument *		doc;

    /* decorations */
      CostBarDecoration *	costDec;
      SubprogDecoration *	subprogDec;
      LoopDecoration *		loopDec;

  } NavView_Repr;


#define R(ob)			(ob->NavView_repr)
#define INHERITED		FortView






/*************************/
/*  Miscellaneous	 */
/*************************/




/* appearance */

static short nv_font;		/* set at init time */






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




void NavView::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(FortView);
    REQUIRE_INIT(CostBarDecoration);
    REQUIRE_INIT(SubprogDecoration);
    REQUIRE_INIT(LoopDecoration);
    REQUIRE_INIT(DedEditor);
    REQUIRE_INIT(NavEditor);
        
  /* initialize appearance details */
    nv_font = fontOpen("screen.12.rnf");
}




void NavView::FiniClass(void)
{
  /* finalize appearance details */
    fontClose(nv_font);
}






/**********************/
/*  Instance creation */
/**********************/




NavView * NavView::Create(Context context,
                          DB_FP * session_fd,
                          NavEditor * editor,
                          Point initScrollPos)
{
  NavView * nv;

  nv = new NavView(context, session_fd, editor, initScrollPos);
  nv->NavView::Init();
  nv->NavView::PostInit();
  return nv;
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(NavView)




NavView::NavView(Context context,
                 DB_FP * session_fd,
                 NavEditor * editor,
                 Point initScrollPos)
       : FortView(context, session_fd, editor, initScrollPos, nv_font)
{
  FortTree dummy;

  /* allocate instance's private data */
    this->NavView_repr =
        (NavView_Repr *) get_mem(sizeof(NavView_Repr), "NavView instance");

  /* save creation arguments */
    R(this)->editor = editor;
    R(this)->doc    = (DedDocument *) editor->getContents();
    
  /* create subparts */
    R(this)->costDec    = CostBarDecoration::Create(R(this)->doc);
    R(this)->subprogDec = SubprogDecoration::Create(R(this)->doc);
    R(this)->loopDec    = LoopDecoration   ::Create(R(this)->doc);
}




void NavView::Init(void)
{
  this->INHERITED::Init();
}




void NavView::Destroy(void)
{
  R(this)->costDec->Destroy();
  R(this)->subprogDec->Destroy();
  R(this)->loopDec->Destroy();
  
  this->INHERITED::Destroy();
}




NavView::~NavView(void)
{
  free_mem((void*) this->NavView_repr);
}






/******************/
/*  Window layout */
/******************/




void NavView::GetSizePrefs(Point &minSize, Point &defSize)
{
  this->INHERITED::GetSizePrefs(minSize, defSize);
  defSize.y = max(defSize.y/3, minSize.y);
}




void NavView::InitPanes(void)
{
  this->INHERITED::InitPanes();
  
  this->AddDecoration(R(this)->costDec);
  this->AddDecoration(R(this)->subprogDec);
  this->AddDecoration(R(this)->loopDec);
}






/***********************/
/* Change notification */
/***********************/




void NavView::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);
  LineEditorChange lch;

  switch( kind )
    {
      case CHANGE_DOCUMENT:
        /* very sad -- use of 'delta' optimization may not work due to multiple notifications */
          lch = * ((LineEditorChange *) change);
          if( lch.delta != 0 )
            { lch.last  = INFINITY;
              lch.delta = 0;
            }
        this->INHERITED::NoteChange(ob, kind, (void *) &lch);
        break;

      default:
        this->INHERITED::NoteChange(ob, kind, change);
        break;
    }
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
