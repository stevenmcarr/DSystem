/* $Id: SrcView.C,v 1.2 1997/03/11 14:30:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ded_cp/src/SrcView.C						*/
/*                                                                      */
/*      SrcView -- View of DedDocument's source code			*/
/*	Last edited: October 14, 1993 at 3:43 pm			*/
/*                                                                      */
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/src/SrcView.h>

#include <libs/graphicInterface/cmdProcs/dEditor/src/StatDecoration.h>
#include <libs/graphicInterface/cmdProcs/dEditor/src/RefDecoration.h>
#include <libs/graphicInterface/cmdProcs/dEditor/src/DedDependenceArrow.h>
#include <libs/graphicInterface/framework/DecorationSet.h>
#include <libs/graphicInterface/cmdProcs/dEditor/src/SrcEditor.h>
#include <libs/graphicInterface/cmdProcs/dEditor/DedEditor.h>

#include <libs/graphicInterface/framework/Dependence.h>

/* Ned stuff */
#define tt_SEL_CHANGED    0    /* sigh */






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* SrcView object */

typedef struct SrcView_Repr_struct
  {
    /* creation parameters */
      SrcEditor *		editor;
      DedDocument *		doc;

    /* current dependence details */
      StatDecoration *		statDec;
      RefDecoration *		refDec;
      DecorationSet *		arrowSet;

  } SrcView_Repr;




#define R(ob)			(ob->SrcView_repr)
#define INHERITED		FortView






/*************************/
/*  Forward declarations */
/*************************/




STATIC(void, autoScroll, (SrcView * sv));

STATIC(int, scrollNeeded, (int target, int top, int bottom));

STATIC(int, maxAbs, (int a, int b));

STATIC(int, minAbs, (int a, int b));

STATIC(int, local_abs, (int a));






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void SrcView::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(FortView);

    REQUIRE_INIT(StatDecoration);
    REQUIRE_INIT(RefDecoration);
    REQUIRE_INIT(DedDependenceArrow);
    REQUIRE_INIT(DecorationSet);
    REQUIRE_INIT(DedEditor);
    REQUIRE_INIT(SrcEditor);
}




void SrcView::FiniClass(void)
{
  /* nothing */
}






/**********************/
/*  Instance creation */
/**********************/




SrcView * SrcView::Create(Context context,
                          DB_FP * session_fd,
                          SrcEditor * editor,
                          Point initScrollPos,
                          int font)
{
  SrcView * sv;

  sv = new SrcView(context, session_fd, editor, initScrollPos, font);
  sv->SrcView::Init();
  sv->SrcView::PostInit();
  return sv;
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(SrcView)




SrcView::SrcView(Context context,
                 DB_FP * session_fd,
                 SrcEditor * editor,
                 Point initScrollPos,
                 int font)
       : FortView(context, session_fd, editor, initScrollPos, font)
{
  FortTree dummy;

  /* allocate instance's private data */
    this->SrcView_repr =
        (SrcView_Repr *) get_mem(sizeof(SrcView_Repr), "SrcView instance");

  /* save creation arguments */
    R(this)->editor = editor;
    R(this)->doc    = (DedDocument *) editor->getContents();
    
  /* create subparts */
    R(this)->statDec = StatDecoration::Create(R(this)->doc);
    R(this)->refDec  = RefDecoration ::Create(R(this)->doc);
    R(this)->arrowSet = DecorationSet ::Create();
}




void SrcView::Init(void)
{
  this->INHERITED::Init();
}




void SrcView::Destroy(void)
{
  R(this)->statDec->Destroy();
  R(this)->refDec ->Destroy();
  R(this)->arrowSet->Destroy();
  
  this->INHERITED::Destroy();
}




SrcView::~SrcView(void)
{
  free_mem((void*) this->SrcView_repr);
}






/******************/
/*  Window layout */
/******************/




void SrcView::InitPanes(void)
{
  this->INHERITED::InitPanes();
  this->AddDecoration(R(this)->statDec);
  this->AddDecoration(R(this)->refDec);
  this->AddDecoration(R(this)->arrowSet);
}






/***********************/
/* Change notification */
/***********************/




void SrcView::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);
  LineEditorChange lch;

  switch( kind )
    {
      case CHANGE_LOOP:
        this->calcDepDecorations();
        this->INHERITED::NoteChange(ob, kind, change);
        this->autoScroll();
        break;

      case CHANGE_CURRENT_DEPENDENCE:
        this->calcDepDecorations();
        lch.kind       = tt_SEL_CHANGED;
        lch.autoScroll = false;
        lch.first      = 0;
        lch.last       = INFINITY;
        lch.delta      = 0;
        this->INHERITED::NoteChange(ob, CHANGE_SELECTION, (void *) &lch);
        if( NOT( R(this)->arrowSet->IsEmpty() ) )
          this->autoScroll();
        /* superclass doesn't know about this kind of change */
        break;

      case CHANGE_DOCUMENT:
        this->calcDepDecorations();
        /* very sad -- use of 'delta' optimization doesn't work due to multiple notifications */
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




/**********************/
/* Dependence display */
/**********************/




void SrcView::calcDepDecorations(void)
{
  DedEditor * owner = R(this)->editor->GetOwner();
  int numCurrDeps, depNum, k;
  Dependence dep;
  DependenceArrow * arrow;

  R(this)->arrowSet->SetEmpty();

  numCurrDeps = owner->NumCurrentDependences();
  for( k = 0;  k < numCurrDeps;  k++ )
    { owner->GetCurrentDependence(k, depNum);
      R(this)->doc->GetDependence(depNum, dep);
      arrow = DedDependenceArrow::Create(dep, R(this)->doc);
      R(this)->arrowSet->Add(arrow);
    }
}




void SrcView::autoScroll(void)
{
  int viewFirst, viewLast, viewHeight;
  int loopFirst, loopLast, loopHeight;
  int depFirst,  depLast,  depHeight;
  Boolean loop, deps;
  FortTreeNode node;
  Point pt;
  int scroll, d1, d2;
  FortTreeNode dummy;
  Rectangle loopBBox, arrowsBBox;

# define Needed(x)    scrollNeeded(x, viewFirst, viewLast)

  /* compute locations and sizes of things */
    /* what is currently visible in the pane */
      pt = this->TextSize();
      viewHeight = pt.y - 1;   /* fudge factor */
      pt = this->GetScroll();
      viewFirst = pt.y;
      viewLast  = viewFirst + viewHeight - 1;

    /* the current loop */
      this->GetCurrentLoop(node, loopBBox);
      loop = BOOL( node != AST_NIL );
      if( loop )
        { loopFirst  = this->ContentsToViewLinenum(loopBBox.ul.y);
          loopLast   = this->ContentsToViewLinenum(loopBBox.lr.y);
          loopHeight = loopLast - loopFirst + 1;
        }

    /* the current dependences */
      deps = NOT( R(this)->arrowSet->IsEmpty() );
      if( deps )
        { arrowsBBox = R(this)->arrowSet->BBox();
          depFirst   = this->ContentsToViewLinenum(arrowsBBox.ul.y);
          depLast    = this->ContentsToViewLinenum(arrowsBBox.lr.y);
          depHeight  = depLast - depFirst + 1;
        }

  /* determine what will fit on screen */
    if( loop && loopHeight <= viewHeight )
      scroll = maxAbs(Needed(loopFirst), Needed(loopLast));
    else if( deps && depHeight <= viewHeight )
      scroll = maxAbs(Needed(depFirst),  Needed(depLast));
    else if( deps )
      scroll = minAbs(Needed(depFirst),  Needed(depLast));
    else if( loop )
      scroll = minAbs(Needed(loopFirst), Needed(loopLast));
    else
      scroll = 0;

  /* scroll to show whatever was chosen */
    this->ScrollBy(makePoint(0, scroll));
}




static
int scrollNeeded(int target, int top, int bottom)
{
  if( target < top )
    return target - top;
  else if( target > bottom )
    return target - bottom;
  else
    return 0;
}




static
int maxAbs(int a, int b)
{
  return (local_abs(a) > local_abs(b) ? a : b);
}




static
int minAbs(int a, int b)
{
  return (local_abs(a) < local_abs(b) ? a : b);
}




static
int local_abs(int a)
{
  return (a > 0 ? a : -a);
}
