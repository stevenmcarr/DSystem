/* $Id: SplitEditor.C,v 1.2 1997/03/11 14:32:53 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      framework/SplitEditor.C						*/
/*                                                                      */
/*	SplitEditor -- Editor made of sub-editors			*/
/*	Last edited: November 10, 1993 at 10:25 pm			*/
/*                                                                      */
/************************************************************************/




#include <libs/graphicInterface/framework/SplitEditor.h>

#include <libs/graphicInterface/framework/EditorCP.h>
#include <libs/graphicInterface/framework/SplitView.h>
#include <libs/graphicInterface/framework/LineSelection.h>

#include <libs/support/arrays/FlexibleArray.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




struct AuxInfo
  {
    View *		view;
    Window *		window;
    
  };
  



/* SplitEditor object */

typedef struct SplitEditor_Repr_struct
  {
    /* creation parameters */
      DBObject *	contents;

    /* sub-editors */
      Flex *		subEditors;
      Flex *		subEditorCaptions;
      Flex *		subEditorVisibility;
      Flex *		subEditorAuxInfo;
      Editor *		mainSubEditor;

    /* view and sub-views */
      SplitView *	splitView;
      Flex *		subViews;

    /* status */
      Boolean		initialized;

  } SplitEditor_Repr;


#define R(ob)		(ob->SplitEditor_repr)


#define INHERITED	Editor






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




void SplitEditor::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(Editor);
    REQUIRE_INIT(DBObject);
    REQUIRE_INIT(SplitView);
}




void SplitEditor::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(SplitEditor)




SplitEditor::SplitEditor(Context context, DB_FP * session_fd)
           : Editor(context, session_fd)
{
  /* allocate instance's private data */
    this->SplitEditor_repr = (SplitEditor_Repr * ) get_mem(sizeof(SplitEditor_Repr),
                                                          "SplitEditor instance");
  R(this)->initialized = false;
}



SplitEditor::~SplitEditor()
{
  int num = flex_length(R(this)->subEditors);
  int k;
  Editor * e;

  /* destroy sub-editors */
    for( k = 0;  k < num;  k++ )
      { flex_get_buffer(R(this)->subEditors, k, 1, (char *) &e);
        delete e;
      }

  flex_destroy(R(this)->subEditors);
  flex_destroy(R(this)->subEditorCaptions);
  flex_destroy(R(this)->subEditorVisibility);
  flex_destroy(R(this)->subEditorAuxInfo);

  free_mem((void*) this->SplitEditor_repr);
}






/***********************/
/* Change notification */
/***********************/




void SplitEditor::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);

  this->Changed(kind, change);
}






/*************/
/*  Database */
/*************/




void SplitEditor::Open(Context context,
		       Context mod_in_pgm_context,
		       Context pgm_context,
		       DB_FP * fd,
                       DBObject * contents)
{
  int num, k;
  Editor * e;
  SplitEditor_Visibility vis;
  
  this->INHERITED::Open(context, mod_in_pgm_context, pgm_context, fd, contents);

  /* create sub-editors */
    R(this)->subEditors = flex_create(sizeof(Editor *));
    R(this)->subEditorCaptions = flex_create(sizeof(char *));
    this->addSubEditors(R(this)->subEditors, R(this)->subEditorCaptions,
                        R(this)->mainSubEditor,
                        context, fd);

  /* open sub-editors */
    num = flex_length(R(this)->subEditors);
    for( k = 0;  k < num;  k++ )
      { flex_get_buffer(R(this)->subEditors, k, 1, (char *) &e);
        e->Open(context, mod_in_pgm_context, pgm_context, fd, R(this)->contents);
      }

  /* initialize visibility of subeditors */                        
    R(this)->subEditorVisibility = flex_create(sizeof(SplitEditor_Visibility));
    R(this)->subEditorAuxInfo    = flex_create(sizeof(AuxInfo));

    vis = SPLIT_IN_PANE;
    num = flex_length(R(this)->subEditors);
    for( k = 0;  k < num;  k++ )
      { flex_insert_one(R(this)->subEditorVisibility, k, (char *) &vis);
        flex_insert(R(this)->subEditorAuxInfo, k, 1);
      }
  
  /* set up change notification */
    for( k = 0;  k < num;  k++ )
      { flex_get_buffer(R(this)->subEditors, k, 1, (char *) &e);
        this->Notify(e, true);
      }
    R(this)->mainSubEditor->Notify(R(this)->contents,  true);
    R(this)->contents->Notify(this, true);
}




void SplitEditor::Close(void)
{
  Flex * subEditors = R(this)->subEditors;
  int num = flex_length(R(this)->subEditors);
  int k;
  Editor * e;

  /* close editors */
    for( k = 0;  k < num;  k++ )
      { flex_get_buffer(subEditors, k, 1, (char *) &e);
        e->Close();
      }

  this->INHERITED::Close();
}




void SplitEditor::Save(Context context, DB_FP * fd)
{
  Flex * subEditors = R(this)->subEditors;
  int num = flex_length(R(this)->subEditors);
  int k;
  Editor * e;

  this->INHERITED::Save(context, fd);

  /* ... */

  /* save sub-editors */
    for( k = 0;  k < num;  k++ )
      { flex_get_buffer(subEditors, k, 1, (char *) &e);
        e->Save(context, fd);
      }
}




void SplitEditor::setContents(DBObject * contents)
{
  R(this)->contents = (DBObject *) contents;
  this->INHERITED::setContents(contents);
}






/*************/
/*  Viewing  */
/*************/




View * SplitEditor::OpenView(Context context, DB_FP * session_fd)
{
  SplitView * split;
  Flex * subEditors = R(this)->subEditors;
  Flex * subEditorCaptions = R(this)->subEditorCaptions;
  Flex * subEditorVisibility = R(this)->subEditorVisibility;
  int num = flex_length(R(this)->subEditors);
  int k;
  Editor * e;
  char * c;
  View * v;
  SplitEditor_Visibility vis;
  AuxInfo aux;

  /* make a split view */
    split = new SplitView(context, session_fd, this, true);
    R(this)->splitView = split;

  /* add the subviews as panes and aux windows */
    for( k = 0;  k < num;  k++ )
      { flex_get_buffer(subEditorVisibility, k, 1, (char *) &vis);
        flex_get_buffer(subEditors, k, 1, (char *) &e);
        flex_get_buffer(subEditorCaptions, k, 1, (char *) &c);

        v = e->OpenView(context, session_fd);
        split->AddView(v, e, c);
        split->SetViewVisible(k, BOOL(vis == SPLIT_IN_PANE));

        if( vis == SPLIT_IN_WINDOW )
          { aux.view   = e->OpenView(context, session_fd);
            aux.window = CP::CurrentCP->OpenAuxWindow(aux.view, c, true);
            flex_set_one(R(this)->subEditorAuxInfo, k, (char *) &aux);

            /* splitview won't notify this subview so we have to */
              this->Notify(aux.view, true);
          }
      }

  R(this)->initialized = true;

  return split;
}




void SplitEditor::SetSubviewVisibility(int k, SplitEditor_Visibility vis)
{
  Editor * subEditor;
  SplitEditor_Visibility old_vis;
  AuxInfo aux;
  char * caption;
  View * view;
  Editor * dummyEditor;
  char * dummyCaption;
  
  flex_get_buffer(R(this)->subEditors, k, 1, (char *) &subEditor);

  /* store the new visibility */
    flex_get_buffer(R(this)->subEditorVisibility, k, 1, (char *) &old_vis);
    if( vis == old_vis )  return;
    flex_set_one(R(this)->subEditorVisibility, k, (char *) &vis);

  if( R(this)->initialized )
    { /* adjust the splitview's panes */
        R(this)->splitView->SetViewVisible(k, BOOL(vis == SPLIT_IN_PANE));

      /* remove the old display if any */
        switch( old_vis )
          {
            case SPLIT_IN_WINDOW:
              flex_get_buffer(R(this)->subEditorAuxInfo, k, 1, (char *) &aux);
              CP::CurrentCP->CloseAuxWindow(aux.window);
              subEditor->CloseView(aux.view);
              break;
          }
      
      /* add the new display if any */
        switch( vis )
          {
            case SPLIT_IN_PANE:
              R(this)->splitView->GetView(k, view, dummyEditor, dummyCaption);
              subEditor->SetView(view);
              break;
              
            case SPLIT_IN_WINDOW:
              flex_get_buffer(R(this)->subEditorCaptions, k, 1, (char *) &caption);
              aux.view   = subEditor->OpenView((Context) nil, (DB_FP *) nil);  /* ??? */
              aux.window = CP::CurrentCP->OpenAuxWindow(aux.view, caption, true);
              flex_set_one(R(this)->subEditorAuxInfo, k, (char *) &aux);
              break;
          }
    }
}






/********************/
/*  Input handling  */
/********************/




Boolean SplitEditor::MenuChoice(Generic cmd)
{
  View * dummy1;
  Editor * currentEditor;
  char * dummy2;
  Flex * subEditors = R(this)->subEditors;
  int num = flex_length(R(this)->subEditors);
  int k;
  Editor * e;
  Boolean handled;

  /* let the current editor have first chance at it */
    R(this)->splitView->GetCurrentView(dummy1, currentEditor, dummy2);
    handled = currentEditor->MenuChoice(cmd);

  /* otherwise, submit the menu choice to each editor in turn */
    if( ! handled )
      for( k = 0;  ! handled && k < num;  k++ )
        { flex_get_buffer(subEditors, k, 1, (char *) &e);
          if( e != currentEditor )
            handled = e->MenuChoice(cmd);
        }

  return handled;
}




void SplitEditor::Keystroke(KbChar kb)
{
  View * dummy1;
  Editor * currentEditor;
  char * dummy2;

#if 0
  R(this)->splitView->GetCurrentView(dummy1, currentEditor, dummy2);
  currentEditor->Keystroke(kb);
#else
  R(this)->mainSubEditor->Keystroke(kb);
#endif
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




void SplitEditor::addSubEditors(Flex * subEditors,
                                Flex * subEditorCaptions,
                                Editor * &mainSubEditor,
                                Context context,
                                DB_FP * session_fd)
{
  SUBCLASS_RESPONSIBILITY("SplitEditor::addSubEditors");
}
