/* $Id: FortView.C,v 1.2 1997/03/11 14:32:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      framework/FortView.C						*/
/*                                                                      */
/*      FortView -- View of Fortran source code				*/
/*	Last edited: October 15, 1993 at 1:11 pm			*/
/*                                                                      */
/************************************************************************/




#include <libs/graphicInterface/framework/FortView.h>

#include <libs/graphicInterface/framework/CFortEditor.h>
#include <libs/graphicInterface/framework/LineNumDecoration.h>
#include <libs/graphicInterface/framework/LoopIconDecoration.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* FortView object */

typedef struct FortView_Repr_struct
  {
    /* creation parameters */
      CFortEditor *		editor;

    /* subparts */
      LineNumDecoration *	linenumDecoration;
      LoopIconDecoration *	loopDecoration;

  } FortView_Repr;


#define R(ob)			(ob->FortView_repr)

#define INHERITED		CTextView






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




void FortView::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(CTextView);
    REQUIRE_INIT(CFortEditor);
    REQUIRE_INIT(LineNumDecoration);
    REQUIRE_INIT(LoopIconDecoration);
}




void FortView::FiniClass(void)
{
  /* nothing */
}






/**********************/
/*  Instance creation */
/**********************/




FortView * FortView::Create(Context context,
                            DB_FP * session_fd,
                            CFortEditor * editor,
                            Point initScrollPos,
                            int font)
{
  FortView * fv;
  
  fv= new FortView(context, session_fd, editor, initScrollPos, font);
  fv->FortView::Init();
  
  return fv;
}




/****************************/
/*  Instance initialization */
/****************************/




META_IMP(FortView)




FortView::FortView(Context context,
                   DB_FP * session_fd,
                   CFortEditor * editor,
                   Point initScrollPos,
                   int font)
        : CTextView(context, session_fd, editor, initScrollPos, font)
{
  FortTree dummy;

  /* allocate instance's private data */
    this->FortView_repr = (FortView_Repr *) get_mem(sizeof(FortView_Repr),
                                                  "FortView instance");

  /* save creation arguments */
    R(this)->editor = editor;
}




void FortView::Init(void)
{
  /* nothing */
}




void FortView::Destroy(void)
{
  R(this)->linenumDecoration->Destroy();
  R(this)->loopDecoration->Destroy();
  
  this->INHERITED::Destroy();
}




FortView::~FortView(void)
{
  free_mem((void*) this->FortView_repr);
  
  /* decorations are destroyed by superclass */
}






/************************/
/*  Change notification */
/************************/




void FortView::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);

  switch( kind )
    {
      case CHANGE_LOOP:
        if( ob == R(this)->loopDecoration )
          { R(this)->editor->SetCurrentLoop((FortTreeNode) change);
          }
        else
          R(this)->loopDecoration->NoteChange(ob, kind, change);
        break;
        
      default:
        this->INHERITED::NoteChange(ob, kind, change);
        break;
    }
}






/******************/
/*  Window layout */
/******************/




void FortView::GetSizePrefs(Point &minSize, Point &defSize)
{
  minSize = makePoint(160,  24);
  defSize = makePoint(600, 250);
}




void FortView::InitPanes(void)
{
  this->INHERITED::InitPanes();

  /* add appropriate decorations to view */
    R(this)->linenumDecoration = LineNumDecoration::Create();
    this->AddMarginDecoration(R(this)->linenumDecoration);

    R(this)->loopDecoration = LoopIconDecoration::Create(R(this)->linenumDecoration);
    this->AddMarginDecoration(R(this)->loopDecoration);
}




void FortView::GetLineNumPosition(int &start, int &width)
{
  start = R(this)->linenumDecoration->Start();
  width = R(this)->linenumDecoration->Width();
}






/****************/
/* Current loop */
/****************/




void FortView::GetCurrentLoop(FortTreeNode &node, Rectangle &bbox)
{
  R(this)->loopDecoration->GetCurrentLoop(node, bbox);
}







/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
