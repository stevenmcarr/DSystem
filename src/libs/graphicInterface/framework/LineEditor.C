/* $Id: LineEditor.C,v 1.6 1997/03/11 14:32:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/LineEditor.C						*/
/*									*/
/*	LineEditor -- Editor for things like line-structured text	*/
/*	Last edited: October 13, 1993 at 6:17 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/LineEditor.h>


#include <libs/graphicInterface/framework/EditorCP.h>
#include <libs/graphicInterface/framework/Text.h>
#include <libs/graphicInterface/framework/LineSelection.h>

#define tt_NOTIFY_SEL_CHANGED  0
#define tt_NOTIFY_DOC_CHANGED  1






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* LineEditor object */

typedef struct LineEditor_Repr_struct
  {
    /* creation parameters */
      Text *		contents;

    /* selection */
      LineSelection *	sel;

    /* deferred changes */
      Boolean		changesDeferred;
      LineEditorChange	change;

  } LineEditor_Repr;


#define R(ob)		(ob->LineEditor_repr)

#define INHERITED	Editor






/*************************/
/*  Forward declarations */
/*************************/




/* none */






/************************************************************************/
/*	Public Interface Operations 					*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void LineEditor::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(Text);
    REQUIRE_INIT(Editor);
}




void LineEditor::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(LineEditor)




LineEditor::LineEditor(Context context, DB_FP * session_fp)
   : Editor (context, session_fp)
{
  /* allocate instance's private data */
    this->LineEditor_repr = (LineEditor_Repr *) get_mem(sizeof(LineEditor_Repr),
                                                        "LineEditor instance");

  /* save creation parameters */
    /* ... */

  /* selection */
    R(this)->sel = new LineSelection;

  /* deferred changes */
    R(this)->changesDeferred = false;
}




LineEditor::~LineEditor()
{
  delete R(this)->sel;
  free_mem((void*) this->LineEditor_repr);
}




void LineEditor::setContents(DBObject * contents)
{
  R(this)->contents = (Text *) contents;
  this->INHERITED::setContents(contents);
}






/************************/
/*  Access to contents  */
/************************/




int LineEditor::NumLines(void)
{
  return R(this)->contents->NumLines();
}




int LineEditor::MaxLineWidth(void)
{
  return R(this)->contents->MaxLineWidth();
}




void LineEditor::GetLine(int k, TextString &ts, TextData &td)
{
  R(this)->contents->GetLine(k, ts, td);
}




void LineEditor::SetLine(int k, TextString line)
{
  R(this)->contents->SetLine(k, line);
  /* TEMPORARY */
  this->Changed(0, nil);
}




void LineEditor::InsertLine(int k, TextString line)
{
  R(this)->contents->InsertLine(k, line);
  /* TEMPORARY */
  this->Changed(0, nil);
}




void LineEditor::DeleteLine(int k)
{
  R(this)->contents->DeleteLine(k);
  /* TEMPORARY */
  this->Changed(0, nil);
}






/***************/
/*  Selection  */
/***************/




void LineEditor::GetSelection(Selection * &sel)
{
  sel  = new LineSelection;
  *sel = * R(this)->sel;
}




void LineEditor::GetSelection(int &line1, int &char1, int &line2, int &char2)
{
  line1 = R(this)->sel->line1;
  char1 = R(this)->sel->char1;
  line2 = R(this)->sel->line2;
  char2 = R(this)->sel->char2;
}




void LineEditor::SetSelection(Selection * sel)
{
  LineSelection * lsel = (LineSelection *) sel;

  this->SetSelection(lsel->line1, lsel->char1, lsel->line2, lsel->char2);
}




void LineEditor::SetSelection(int line1, int char1, int line2, int char2)
{
  int oldLine1 = R(this)->sel->line1;
  int oldLine2 = R(this)->sel->line2;
  int numLines = this->NumLines();
  LineEditorChange change;

  /* "no selection" is represented by line1 == line2 == INFINITY */
    if( numLines == 0 )
      line1 = line2 = INFINITY;
    if( line1 != INFINITY )
      line1 = max(0, min(numLines-1, line1));
    if( line2 != INFINITY )
      line2 = max(0, min(numLines-1, line2));

  * R(this)->sel = LineSelection(line1, char1, line2, char2);

  /* send appropriate notification */
    change.kind       = tt_NOTIFY_SEL_CHANGED;
    change.autoScroll = false;
    change.data       = (Generic) nil;
    change.first      = min(oldLine1, line1);
    change.last       = max(oldLine2, line2);
    change.delta      = 0;
    this->Changed(CHANGE_SELECTION, (void *) &change);
}




void LineEditor::GetSelectedData(Generic &data)
{
  data = UNUSED;
}




void LineEditor::SetSelectedData(Generic data)
{
  /* nothing */
}




void LineEditor::SetSelectionNone(void)
{
  /* "no selection" is represented by line1 == line2 == INFINITY */

  this->SetSelection(INFINITY, 1, INFINITY, 0);
}




Boolean LineEditor::HasSelection(void)
{
  int line1, char1, line2, char2;

  this->GetSelection(line1, char1, line2, char2);
  return this->IsSelection(line1, char1, line2, char2);
}




Boolean LineEditor::IsSelection(int line1, int char1, int line2, int char2)
{
  /* "no selection" is represented by line1 == line2 == INFINITY */

  return BOOL( line1 != INFINITY );
}






/*************/
/*  Editing  */
/*************/




Scrap * LineEditor::Extract(Selection * sel)
{
  /* ... */
  return nil;
}




void LineEditor::Replace(Selection * sel, Scrap * scrap)
{
  /* ... */
}






/***********/
/* Errors  */
/***********/




Boolean LineEditor::CheckLineData(int k)
{
  return true;
}




Boolean LineEditor::CheckData(void)
{
  return true;
}




void LineEditor::SetShowErrors(Boolean show)
{
  /* ??? */
}






/***********************/
/* Change notification */
/***********************/




void LineEditor::ChangedEverything(void)
{
  LineEditorChange lch;

  lch.kind       = tt_NOTIFY_DOC_CHANGED;
  lch.autoScroll = false;
  lch.first      = 0;
  lch.last       = INFINITY;
  lch.delta      = INFINITY;
  this->Changed(CHANGE_DOCUMENT, (void *) &lch);
}




void LineEditor::clearDeferredChanges(void)
{
  R(this)->changesDeferred = false;
}




void LineEditor::addDeferredChange(int kind, void * change)
{
  LineEditorChange * lch = (LineEditorChange *) change;

  if( R(this)->changesDeferred )
    { if( lch->kind == tt_NOTIFY_DOC_CHANGED )
        R(this)->change.kind = lch->kind;

      if( lch->autoScroll == true )
        R(this)->change.autoScroll = lch->autoScroll;

      if( lch->first < R(this)->change.first ||
          R(this)->change.first == UNUSED )
        R(this)->change.first = lch->first;

      if( lch->last > R(this)->change.last ||
          R(this)->change.last == UNUSED )
        R(this)->change.last = lch->last;

      /* combining deltas is a problem */
      if( lch->delta != 0 || R(this)->change.delta != 0 )
        R(this)->change.delta = INFINITY;
    }
  else
    { R(this)->changesDeferred = true;
      R(this)->change = *lch;
    }
}




void LineEditor::getDeferredChanges(int &kind, void * &change)
{
  static LineEditorChange lch;

  lch = R(this)->change;

  kind   = (lch.kind == tt_NOTIFY_SEL_CHANGED ? CHANGE_SELECTION : CHANGE_DOCUMENT);
  change = (void *) &lch;
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
