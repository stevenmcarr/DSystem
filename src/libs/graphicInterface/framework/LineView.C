/* $Id: LineView.C,v 1.6 1997/03/11 14:32:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/LineView.C						*/
/*									*/
/*	LineView -- View of a LineEditor				*/
/*	Last edited: October 13, 1993, at 6:26 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/LineView.h>


#include <libs/graphicInterface/framework/LineEditor.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* LineView object */

typedef struct LineView_Repr_struct
  {
    /* creation parameters */
      LineEditor *	editor;

  } LineView_Repr;


#define R(ob)		(ob->LineView_repr)

#define INHERITED	CTextView






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




void LineView::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(CTextView);
    REQUIRE_INIT(LineEditor);
}




void LineView::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(LineView)




LineView::LineView(Context context,
                     DB_FP * session_fp,
                     LineEditor * editor,
                     Point initScrollPos,
                     int font)
   : CTextView (context, session_fp, editor, initScrollPos, font)
{
  /* allocate instance's private data */
    this->LineView_repr = (LineView_Repr *) get_mem(sizeof(LineView_Repr),
                                                    "LineView instance");

  /* save creation arguments */
    R(this)->editor = editor;
}




LineView::~LineView(void)
{
  free_mem((void*) this->LineView_repr);
}






/**********************/
/* Access to contents */
/**********************/




int LineView::numLines(void)
{
  return R(this)->editor->NumLines();
}




int LineView::maxLineWidth(void)
{
  return R(this)->editor->MaxLineWidth();
}




void LineView::getLine(int k, TextString &ts, TextData &td)
{
  R(this)->editor->GetLine(k, ts, td);
}







/***********************/
/* Access to selection */
/***********************/




void LineView::getSelection(int &line1, int &char1, int &line2, int &char2)
{
  R(this)->editor->GetSelection(line1, char1, line2, char2);
}




void LineView::setSelection(int line1, int char1, int line2, int char2)
{
  R(this)->editor->SetSelection(line1, char1, line2, char2);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
