/* $Id: SrcEditor.C,v 1.2 1997/03/11 14:30:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/src/SrcEditor.C						*/
/*									*/
/*	SrcEditor -- Ded source code sub-editor				*/
/*	Last edited: November 7, 1993 at 10:36 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/src/SrcEditor.h>

#include <libs/graphicInterface/framework/ScrollView.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>
#include <libs/graphicInterface/cmdProcs/dEditor/DedEditor.h>
#include <libs/graphicInterface/cmdProcs/dEditor/src/SrcView.h>
#include <libs/graphicInterface/cmdProcs/dEditor/src/SrcFilterDef.h>


#define tt_SEL_CHANGED    0






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* SrcEditor object */

typedef struct SrcEditor_Repr_struct
  {
    /* creation parameters */
      DedEditor *	owner;
      DedDocument *	contents;

    /* subparts */
      SrcView *	view;

  } SrcEditor_Repr;


#define R(ob)		(ob->SrcEditor_repr)
#define INHERITED	CFortEditor






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




void SrcEditor::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(CFortEditor);

    REQUIRE_INIT(DedDocument);
    REQUIRE_INIT(DedEditor);
    REQUIRE_INIT(SrcView);
    REQUIRE_INIT(SrcFilterDef);
}




void SrcEditor::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(SrcEditor)




SrcEditor::SrcEditor(Context context,
                           DB_FP * session_fd,
                           DedEditor * owner)
         : CFortEditor(context, session_fd)
{
  /* allocate instance's private data */
    this->SrcEditor_repr =
        (SrcEditor_Repr *) get_mem(sizeof(SrcEditor_Repr), "SrcEditor instance");

  /* save creation parameters */
    R(this)->owner = owner;
}




SrcEditor::~SrcEditor(void)
{
  free_mem((void*) this->SrcEditor_repr);
}




DedEditor * SrcEditor::GetOwner(void)
{
  return R(this)->owner;
}




void SrcEditor::setContents(DBObject * contents)
{
  R(this)->contents = (DedDocument *) contents;
  this->INHERITED::setContents(contents);
}






/*************/
/*  Database */
/*************/




void SrcEditor::Open(Context context,
		     Context mod_in_pgm_context,
		     Context pgm_context,
		     DB_FP * fd, DBObject * contents)
{
  FortTextTree ftt;
  FortTree ft;

  ((DedDocument *) contents)->GetSource(ftt, ft);
  this->INHERITED::Open(context, mod_in_pgm_context, pgm_context, fd, contents, ftt, ft);
}






/***********************/
/* Change notification */
/***********************/




void SrcEditor::SetCurrentLoop(FortTreeNode node)
{
  R(this)->owner->SetCurrentLoop(node);
}






/*************/
/*  Viewing  */
/*************/




View * SrcEditor::OpenView(Context context, DB_FP * session_fd)
{
  R(this)->view = SrcView::Create(context, session_fd, this,
                                  makePoint(0, 0), FortView_Font);
  return new ScrollView(R(this)->view, true, true);
}




void SrcEditor::SetView(View * view)
{
  R(this)->view = (SrcView *) view;
}






/*************/
/* Filtering */
/*************/




UserFilterDef * SrcEditor::OpenFilterDef(Context context,
					 Context mod_in_pgm_context,
					 Context pgm_context,
                                         DB_FP * session_fd)
{
  UserFilterDef * def;

  def = new SrcFilterDef(context,  session_fd);
  def->Open(context, mod_in_pgm_context, pgm_context, session_fd);
  return def;
}




void SrcEditor::AddStandardFilterDefs(FilterDefSet * defs)
{
  SrcFilterDef::AddStandardDefs(defs);
}






/********************/
/*  Input handling  */
/********************/




Boolean SrcEditor::MenuChoice(Generic cmd)
{
  Boolean handled = true;
  
  switch( cmd )
    {
      /* "view" menu commands */

      case CMD_DEF_SRC_FILTER:
        this->DefineFilters();
        break;

      case CMD_SET_SRC_FILTER:
        this->ChooseFilterFor(R(this)->view);
        break;


      /* other menu commands */

      default:
        handled = this->INHERITED::MenuChoice(cmd);
        break;
    }
    
  return handled;
}






/***************/
/*  Selection  */
/***************/




void SrcEditor::SetSelection(int line1, int char1, int line2, int char2)
{
  Selection * sel;

  this->INHERITED::SetSelection(line1, char1, line2, char2);
  
  this->GetSelection(sel);
  R(this)->owner->NoteChange(this, CHANGE_SRC_SELECTION, (void *) sel);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
