/* $Id: Editor.C,v 1.7 1997/03/11 14:32:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/Editor.C						*/
/*									*/
/*	Editor -- Abstract class for all Editors			*/
/*	Last edited: November 6, 1993 at 10:25 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/Editor.h>


#include <libs/graphicInterface/framework/UserFilter.h>
#include <libs/graphicInterface/framework/UserFilterDef.h>
#include <libs/graphicInterface/framework/FilterDefSet.h>
#include <libs/graphicInterface/framework/CClipboard.h>
#include <libs/support/arrays/FlexibleArray.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* Editor object */

typedef struct Editor_Repr_struct
  {
    /* creation parameters */
      DBObject *	contents;

    /* filtering */
      FilterDefSet *	filterDefSet;

    /* change notification */
      int		freeze;
      Boolean		changeDeferred;

  } Editor_Repr;


#define R(ob)		(ob->Editor_repr)

#define INHERITED	Object






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




void Editor::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(DBObject);
    REQUIRE_INIT(View);
    REQUIRE_INIT(Selection);
    REQUIRE_INIT(Scrap);
    REQUIRE_INIT(CClipboard);
}




void Editor::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(Editor)




Editor::Editor(Context context,
               DB_FP * session_fp)
{
  /* allocate instance's private data */
    this->Editor_repr = (Editor_Repr *) get_mem(sizeof(Editor_Repr),
                                                "Editor instance");

  /* save creation parameters */
    /* ... */

  /* initialize change notification */
    R(this)->freeze = 0;
    R(this)->changeDeferred = false;

}




Editor::~Editor()
{
  delete R(this)->filterDefSet;
  free_mem((void*) this->Editor_repr);
}






/*************/
/*  Database */
/*************/




void Editor::Open(Context context,
		  Context mod_in_pgm_context,
		  Context pgm_context,
		  DB_FP * session_fp,
		  DBObject * contents)
{
  if( contents == nil )
    contents = this->openContents(context, mod_in_pgm_context, pgm_context, session_fp);
  this->setContents(contents);

  R(this)->filterDefSet = new FilterDefSet(context, session_fp, this);
  R(this)->filterDefSet->Open(context, mod_in_pgm_context, pgm_context, session_fp);
  if( session_fp == DB_NULLFP )
    this->AddStandardFilterDefs(R(this)->filterDefSet);
}




void Editor::Save(Context context, DB_FP * fp)
{
  R(this)->contents->Save(context, fp);
  R(this)->filterDefSet->Save(context, fp);
}




void Editor::Close(void)
{
  /* nothing */
}




DBObject * Editor::openContents(Context context,
				Context mod_in_pgm_context,
				Context pgm_context,
                                DB_FP * session_fp)
{
  return nil;
}




void Editor::setContents(DBObject * contents)
{
  R(this)->contents = contents;
}




DBObject * Editor::getContents(void)
{
  return R(this)->contents;
}






/*************/
/*  Viewing  */
/*************/




View * Editor::OpenView(Context context, DB_FP * session_fp)
{
  SUBCLASS_RESPONSIBILITY("Editor::OpenView");
  return nil;
}




void Editor::CloseView(View * view)
{
  delete view;
}




void Editor::SetView(View * view)
{
  /* nothing */
}






/***************/
/*  Filtering  */
/***************/




UserFilterDef * Editor::OpenFilterDef(Context context,
				      Context mod_in_pgm_context,
				      Context pgm_context,
				      DB_FP * session_fp)
{
  UserFilterDef * def;

  def = new UserFilterDef(context, session_fp);
  def->Open(context, mod_in_pgm_context, pgm_context, session_fp);

  return def;
}




void Editor::AddStandardFilterDefs(FilterDefSet * defs)
{
  UserFilterDef * def;

  def = new UserFilterDef(CONTEXT_NULL, DB_NULLFP);
  def->Open(CONTEXT_NULL, CONTEXT_NULL, CONTEXT_NULL, DB_NULLFP);
  def->SetName("normal");
  defs->AddFilterDef(def);
}




void Editor::GetFilterDefByName(char * name, UserFilterDef * &def)
{
  R(this)->filterDefSet->GetFilterDefByName(name, def);
}




void Editor::DefineFilters(void)
{
  R(this)->filterDefSet->DoDialog();
}




void Editor::ChooseFilterFor(View * view)
{
  UserFilter * oldFilter;
  UserFilter * newFilter;
 
  oldFilter = (UserFilter *) (view->GetFilter());

  newFilter = oldFilter;
  R(this)->filterDefSet->DoMenu(newFilter, (void *) R(this)->contents);

  if( newFilter != oldFilter )
    view->SetFilter(newFilter);
}







/********************/
/*  Input handling  */
/********************/




Boolean Editor::MenuChoice(Generic cmd)
{
  Boolean handled = true;

  switch( cmd )
    {
      /* "edit" menu commands */

      case CMD_UNDO:
        notImplemented("edit/undo");
        break;

      case CMD_COPY:
        this->Copy();
        break;

      case CMD_CUT:
        this->Cut();
        break;

      case CMD_PASTE:
        this->Paste();
        break;

      case CMD_CLEAR:
        this->Clear();
        break;

      case CMD_SELECT_ALL:
        notImplemented("edit/select all");
        break;


      /* other menu commands */

      default:
        handled = false;
        break;
    }

  return handled;
}




void Editor::Keystroke(KbChar kb)
{
  /* nothing */
}






/*************************/
/*  Change notification  */
/*************************/




void Editor::BeginEdit(void)
{
  R(this)->freeze += 1;
}




void Editor::EndEdit(void)
{
  int kind;
  void * change;

  R(this)->freeze -= 1;

  if( R(this)->freeze == 0  &&  R(this)->changeDeferred )
    { this->getDeferredChanges(kind, change);
      this->clearDeferredChanges();
      R(this)->changeDeferred = false;

      this->Changed(kind, change);
    }
}




Boolean Editor::Frozen(void)
{
  return BOOL(R(this)->freeze > 0);
}




void Editor::Changed(int kind, void * change)
{
  if( R(this)->freeze == 0 )
    this->INHERITED::Changed(kind, change);
  else
    { this->addDeferredChange(kind, change);
      R(this)->changeDeferred = true;
    }
}




void Editor::clearDeferredChanges(void)
{
  /* nothing */
}




void Editor::addDeferredChange(int kind, void * change)
{
  /* nothing */
}




void Editor::getDeferredChanges(int &kind, void * &change)
{
  kind   = CHANGE_UNKNOWN;
  change = (void *) nil;
}






/***************/
/*  Selection  */
/***************/




void Editor::GetSelection(Selection * &sel)
{
  SUBCLASS_RESPONSIBILITY("Editor::GetSelection");
}




void Editor::SetSelection(Selection * sel)
{
  SUBCLASS_RESPONSIBILITY("Editor::SetSelection");
}






/*************/
/*  Editing  */
/*************/




void Editor::Copy(void)
{
  Selection * sel;

  this->GetSelection(sel);
  theClipboard->SetScrap(this->Extract(sel));
  delete sel;
}




void Editor::Cut(void)
{
  this->Copy();
  this->Clear();
}




void Editor::Paste(void)
{
  Selection * sel;

  this->GetSelection(sel);
  this->Replace(sel, theClipboard->GetScrap());
  delete sel;
}




void Editor::Clear(void)
{
  Selection * sel;

  this->GetSelection(sel);
  this->Replace(sel, nil);
  delete sel;
}




Scrap * Editor::Extract(Selection * sel)
{
  SUBCLASS_RESPONSIBILITY("Editor::Extract");
  return nil;
}




void Editor::Replace(Selection * sel, Scrap * scrap)
{
  SUBCLASS_RESPONSIBILITY("Editor::Replace");
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */

