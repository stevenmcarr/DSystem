/* $Id: DedEditor.C,v 1.2 1997/03/11 14:30:10 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ded_cp/DedEditor.C						*/
/*                                                                      */
/*      DedEditor -- Fortran D Editor					*/
/*	Last edited: November 12, 1993 at 4:24 pm			*/
/*                                                                      */
/************************************************************************/


#define ANNOTATION_BROWSER_WORKING 0


#include <libs/graphicInterface/cmdProcs/dEditor/DedEditor.h>

#include <libs/graphicInterface/framework/EditorCP.h>
#include <libs/graphicInterface/framework/SplitView.h>
#include <libs/graphicInterface/framework/LineSelection.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>
#include <libs/graphicInterface/cmdProcs/dEditor/nav/NavEditor.h>
#include <libs/graphicInterface/cmdProcs/dEditor/src/SrcEditor.h>
#include <libs/graphicInterface/cmdProcs/dEditor/dep/DepEditor.h>
#include <libs/graphicInterface/cmdProcs/dEditor/comm/CommEditor.h>
#include <libs/graphicInterface/cmdProcs/dEditor/dist/DistEditor.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotPI.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* DedEditor object */

typedef struct DedEditor_Repr_struct
  {
    DedEditor_Repr_struct();
    ~DedEditor_Repr_struct();
    /* creation parameters */
      DedDocument *	contents;

    /* sub-editors */
      NavEditor *	navEditor;
      SrcEditor *	srcEditor;
      DepEditor *	depEditor;
      CommEditor *	commEditor;
      DistEditor *	distEditor;

  } DedEditor_Repr;

DedEditor_Repr_struct::DedEditor_Repr_struct() : contents(0), navEditor(0), 
srcEditor(0), depEditor(0), commEditor(0), distEditor(0)
{
}

DedEditor_Repr_struct::~DedEditor_Repr_struct()

{ 
  delete contents; 
#if 0
  delete navEditor;
  delete srcEditor; 
  delete depEditor; 
  delete commEditor; 
  delete distEditor;
#endif
}


#define R(ob)		(ob->DedEditor_repr)


#define INHERITED	SplitEditor






/*************************/
/*  Forward declarations */
/*************************/




static void arrowCommand(DedEditor * ped);






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void DedEditor::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(SplitEditor);
    REQUIRE_INIT(NavEditor);
    REQUIRE_INIT(SrcEditor);
    REQUIRE_INIT(DepEditor);
    REQUIRE_INIT(CommEditor);
    REQUIRE_INIT(DepEditor);
    REQUIRE_INIT(SplitView);
    REQUIRE_INIT(DedDocument);

  /* initialize ned annotations */
#if ANNOTATION_BROWSER_WORKING 
    FortAnnotMgr_Init();
#endif
    ContentsSrc_Init();
    DeclSrc_Init();
    LoopsSrc_Init();
    InterProcSrc_Init();
}




void DedEditor::FiniClass(void)
{
  /* finalize ned annotations */
    InterProcSrc_Fini();
    LoopsSrc_Fini();
    DeclSrc_Fini();
    ContentsSrc_Fini();
#if ANNOTATION_BROWSER_WORKING 
    FortAnnotMgr_Fini();
#endif
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(DedEditor)




DedEditor::DedEditor(Context context, DB_FP * session_fd)
          : SplitEditor (context, session_fd)
{
  /* allocate instance's private data */
#if 0
    this->DedEditor_repr = (DedEditor_Repr * ) get_mem(sizeof(DedEditor_Repr),
                                                       "DedEditor instance");
#endif
    this->DedEditor_repr = new DedEditor_Repr;
}




DedEditor::~DedEditor()
{
#if 0
  free_mem((void*) this->DedEditor_repr);
#endif
  delete this->DedEditor_repr;
}






/*************/
/*  Database */
/*************/




void DedEditor::GetContentsAttribute(char * &contents_attr)
{
  contents_attr = DedDocument_Attribute;
}




void DedEditor::Open(Context context,
                Context mod_in_pgm_context,
                Context pgm_context,
                DB_FP * fd,
                DBObject * contents)
{
  this->INHERITED::Open(context, mod_in_pgm_context, pgm_context, fd, contents);
}



DBObject * DedEditor::openContents(Context context,
				   Context mod_in_pgm_context,
				   Context pgm_context,
				   DB_FP * session_fd)
{
  R(this)->contents = new DedDocument(context, session_fd);
  R(this)->contents->Open(context, mod_in_pgm_context, pgm_context, session_fd);

  return R(this)->contents;
}




void DedEditor::setContents(DBObject * contents)
{
  R(this)->contents = (DedDocument *) contents;
  this->INHERITED::setContents(contents);
}






/*************/
/*  Viewing  */
/*************/




View * DedEditor::OpenView(Context context, DB_FP * session_fd)
{
  /* this must happen after superclass is ready to record visibilities */
    this->SetSubviewVisibility(1, SPLIT_IN_WINDOW);

  return this->INHERITED::OpenView(context, session_fd);
}






/********************/
/*  Input handling  */
/********************/




Boolean DedEditor::MenuChoice(Generic cmd)
{
  Boolean handled = true;
  View * subview;
  Editor * subeditor;
  char * dummy;
  char caption[100];

  switch( cmd )
    {
      /* "search" menu commands */

      case CMD_GENERAL_INFO:
      case CMD_SELECTED_INFO:
        handled = R(this)->srcEditor->MenuChoice(cmd);
        break;


      /* other menu commands */

      case CMD_ANALYZE:
        R(this)->contents->Analyze();
        break;

      /*** TEMPORARY -- EDITING IS DISABLED ***/
      case CMD_CUT:
      case CMD_PASTE:
      case CMD_CLEAR:
        /* nothing */
        break;
      default:
        handled = this->INHERITED::MenuChoice(cmd);
        break;
    }

  return handled;
}




void DedEditor::Keystroke(KbChar kb)
{
  /*** TEMPORARY -- EDITING IS DISABLED ***/
  /* nothing */
}






/**************************/
/*  Global editing state  */
/**************************/




void DedEditor::SetCurrentLoop(FortTreeNode loop)
{
  R(this)->contents->SetCurrentLoop(loop);
}




void DedEditor::GetCurrentLoop(FortTreeNode &loop)
{
  loop = R(this)->contents->GetCurrentLoop();
}




int DedEditor::NumCurrentDependences(void)
{
  return R(this)->depEditor->NumCurrentDependences();
}




void DedEditor::GetCurrentDependence(int k, int &depNum)
{
  R(this)->depEditor->GetCurrentDependence(k, depNum);
}




void DedEditor::SetCurrentDependencePolicy(DedCurDepPolicy policy)
{
  R(this)->depEditor->SetCurrentDependencePolicy(policy);
}




void DedEditor::GetCurrentDependencePolicy(DedCurDepPolicy &policy)
{
  R(this)->depEditor->GetCurrentDependencePolicy(policy);
}






/*************/
/* Selection */
/*************/




void DedEditor::SetSourceSelection(int line1,
                                   int char1,
                                   int line2,
                                   int char2)
{
  R(this)->srcEditor->SetSelection(line1, char1, line2, char2);
}






/****************/
/*  Navigation  */
/****************/




void DedEditor::NavigateTo(FortTreeNode target)
{
  R(this)->srcEditor->NavigateTo(target);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




void DedEditor::addSubEditors(Flex * subEditors,
                              Flex * subEditorCaptions,
                              Editor * &mainSubEditor,
                              Context context,
                              DB_FP * session_fd)
{
  char * caption;

  R(this)->navEditor = new NavEditor(context, session_fd, this);
  flex_insert_one(subEditors, flex_length(subEditors), (char *) &R(this)->navEditor);
  caption = "overview";
  flex_insert_one(subEditorCaptions, flex_length(subEditorCaptions), (char *) &caption);

  R(this)->srcEditor = new SrcEditor(context, session_fd, this);
  flex_insert_one(subEditors, flex_length(subEditors), (char *) &R(this)->srcEditor);
  caption = "source code";
  flex_insert_one(subEditorCaptions, flex_length(subEditorCaptions), (char *) &caption);

  R(this)->depEditor = new DepEditor(context, session_fd, this);
  flex_insert_one(subEditors, flex_length(subEditors), (char *) &R(this)->depEditor);
  caption = "dependences";
  flex_insert_one(subEditorCaptions, flex_length(subEditorCaptions), (char *) &caption);

  R(this)->commEditor = new CommEditor(context, session_fd, this);
  flex_insert_one(subEditors, flex_length(subEditors), (char *) &R(this)->commEditor);
  caption = "communication";
  flex_insert_one(subEditorCaptions, flex_length(subEditorCaptions), (char *) &caption);

  R(this)->distEditor = new DistEditor(context, session_fd, this);
  flex_insert_one(subEditors, flex_length(subEditors), (char *) &R(this)->distEditor);
  caption = "data layout";
  flex_insert_one(subEditorCaptions, flex_length(subEditorCaptions), (char *) &caption);

  mainSubEditor = R(this)->srcEditor;
  R(this)->contents->SetEditor(R(this)->srcEditor);
}
