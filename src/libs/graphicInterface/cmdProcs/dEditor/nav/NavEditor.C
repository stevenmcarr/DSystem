/* $Id: NavEditor.C,v 1.2 1997/03/11 14:30:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/nav/NavEditor.C						*/
/*									*/
/*	NavEditor -- Ded navigation sub-editor				*/
/*	Last edited: November 7, 1993 at 10:36 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/nav/NavEditor.h>

#include <libs/graphicInterface/framework/ScrollView.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>
#include <libs/graphicInterface/cmdProcs/dEditor/DedEditor.h>
#include <libs/graphicInterface/cmdProcs/dEditor/nav/NavView.h>
#include <libs/graphicInterface/cmdProcs/dEditor/nav/NavFilterDef.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* NavEditor object */

typedef struct NavEditor_Repr_struct
  {
    /* creation parameters */
      DedEditor *	owner;
      DedDocument *	contents;
      
    /* subparts */
      NavView *	view;

  } NavEditor_Repr;




#define R(ob)		(ob->NavEditor_repr)
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




void NavEditor::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(CFortEditor);

    REQUIRE_INIT(DedDocument);
    REQUIRE_INIT(DedEditor);
    REQUIRE_INIT(NavView);
    REQUIRE_INIT(NavFilterDef);
}




void NavEditor::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(NavEditor)




NavEditor::NavEditor(Context context,
                     DB_FP * session_fd,
                     DedEditor * owner)
         : CFortEditor(context, session_fd)
{
  /* allocate instance's private data */
    this->NavEditor_repr =
        (NavEditor_Repr *) get_mem(sizeof(NavEditor_Repr), "NavEditor instance");

  /* save creation parameters */
    R(this)->owner = owner;
}




NavEditor::~NavEditor(void)
{
  free_mem((void*) this->NavEditor_repr);
}




void NavEditor::setContents(DBObject * contents)
{
  R(this)->contents = (DedDocument *) contents;
  this->INHERITED::setContents(contents);
}






/*************/
/*  Database */
/*************/




void NavEditor::Open(Context context,
		     Context mod_in_pgm_context,
		     Context pgm_context,
		     DB_FP * fd,
		     DBObject * contents)
{
  FortTextTree ftt;
  FortTree ft;

  ((DedDocument *) contents)->GetSource(ftt, ft);
  this->INHERITED::Open(context, mod_in_pgm_context, pgm_context, fd, contents, ftt, ft);
  this->INHERITED::SetSelectionNone();
}






/***********************/
/* Change notification */
/***********************/




void NavEditor::SetCurrentLoop(FortTreeNode node)
{
  R(this)->owner->SetCurrentLoop(node);
}






/*************/
/*  Viewing  */
/*************/




View * NavEditor::OpenView(Context context, DB_FP * session_fd)
{
  R(this)->view = NavView::Create(context, session_fd, this, makePoint(0, 0));
  return new ScrollView(R(this)->view, false, true);
}




void NavEditor::SetView(View * view)
{
  R(this)->view = (NavView *) view;
}






/*************/
/* Filtering */
/*************/




UserFilterDef * NavEditor::OpenFilterDef(Context context,
					 Context mod_in_pgm_context,
					 Context pgm_context,
                                         DB_FP * session_fd)
{
  UserFilterDef * def;

  def = new NavFilterDef(context,  session_fd);
  def->Open(context, mod_in_pgm_context, pgm_context, session_fd);
  return def;
}




void NavEditor::AddStandardFilterDefs(FilterDefSet * defs)
{
  NavFilterDef::AddStandardDefs(defs);
}






/********************/
/*  Input handling  */
/********************/




Boolean NavEditor::MenuChoice(Generic cmd)
{
  Boolean handled = true;
  
  switch( cmd )
    {
      /* "view" menu commands */

      case CMD_DEF_NAV_FILTER:
        this->DefineFilters();
        break;

      case CMD_SET_NAV_FILTER:
        this->ChooseFilterFor(R(this)->view);
        break;


      /* other menu commands */

      default:
        handled = this->INHERITED::MenuChoice(cmd);
        break;
    }
    
  return handled;
}




void NavEditor::Keystroke(KbChar kb)
{
  /* nothing -- NavEditor is read only */
}






/***************/
/*  Selection  */
/***************/




void NavEditor::SetSelection(int line1, int char1, int line2, int char2)
{
  FortTreeNode node;
  int dummy;

  if( this->IsSelection(line1, char1, line2, char2) )
    { /* force selection to be whole line */
        char1 = 0;
        char2 = INFINITY;

      /* set selection in source pane rather than here */
        R(this)->owner->SetSourceSelection(line1, char1, line2, char2);

      /* navigate to selected line */
        this->GetLineInfo(line1, node, dummy);
        R(this)->owner->NavigateTo(node);
    }
}






/*************/
/*  Editing  */
/*************/




void NavEditor::Cut(void)
{
  /* nothing -- NavEditor is read only */
}




void NavEditor::Paste(void)
{
  /* nothing -- NavEditor is read only */
}




void NavEditor::Clear(void)
{
  /* nothing -- NavEditor is read only */
}




void NavEditor::Replace(Selection * sel, Scrap * scrap)
{
  /* nothing -- NavEditor is read only */
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
