/* $Id: CFortEditor.C,v 1.2 1997/03/11 14:32:28 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/CFortEditor.C						*/
/*									*/
/*	CFortEditor -- Fortran source code editor			*/
/*									*/
/************************************************************************/

#define ANNOTATION_BROWSER_WORKING 0



#include <libs/graphicInterface/framework/CFortEditor.h>

#include <libs/graphicInterface/framework/EditorCP.h>
#include <libs/graphicInterface/framework/ScrollView.h>
#include <libs/graphicInterface/framework/LineSelection.h>
#include <libs/graphicInterface/framework/FortView.h>
#include <libs/graphicInterface/framework/FortFilterDef.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#include <libs/graphicInterface/cmdProcs/newEditor/TextView.h>




/* Ned stuff needed here */
EXTERN(Boolean,  edcp_Start, (Generic cpm));    /* ped2 needs to init ned */
EXTERN(void,  edcp_Finish, (void));    /* ped2 needs to finish ned */

#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/textTree/TextTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/graphicInterface/cmdProcs/newEditor/TextView.h>
#include <libs/graphicInterface/cmdProcs/newEditor/ViewFilter.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/FortVFilter.h>
#include <libs/graphicInterface/cmdProcs/newEditor/FortEditor.h>
#include <libs/graphicInterface/include/FortEditorCP.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotMgr.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditorCP/AnnotDialog.h>
#define tt_SEL_CHANGED    0





/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* CFortEditor object */

typedef struct CFortEditor_Repr_struct
  {
    /* creation parameters */
      DBObject *	contents;
      FortTextTree	ftt;
      FortTree		ft;

    /* Ned components */
      FortEditor	fortEditor;
#if ANNOTATION_BROWSER_WORKING 
      FortAnnotMgr *    annotator;
      AnnotDialog	browser;
#endif

  } CFortEditor_Repr;


#define R(ob)		(ob->CFortEditor_repr)


#define INHERITED	LineEditor






/*************************/
/*  Miscellaneous	 */
/*************************/




/* none */






/*************************/
/*  Forward declarations */
/*************************/




static void noteChange(CFortEditor * fed,
                       int kind,
                       Boolean autoScroll,
                       FortTreeNode node,
                       int first,
                       int last,
                       int delta);

static void annotGotoFunc(Generic fed);






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void CFortEditor::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(LineEditor);
    REQUIRE_INIT(FortFilterDef);
    REQUIRE_INIT(FortView);
    REQUIRE_INIT(ScrollView);
    REQUIRE_INIT(LineSelection);
    
  /* initialize ned */
    (void) edcp_Start(0);
}




void CFortEditor::FiniClass(void)
{
  /* finalize ned */
    (void) edcp_Finish();
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(CFortEditor)




CFortEditor::CFortEditor(Context context, DB_FP * session_fd)
          : LineEditor(context, session_fd)
{
  /* allocate instance's private data */
    this->CFortEditor_repr = (CFortEditor_Repr *) get_mem(sizeof(CFortEditor_Repr),
                                                      "CFortEditor instance");

  /* save creation parameters */
    /* none */
}




CFortEditor::~CFortEditor(void)
{
  /* destroy Ned components */
    /* ... */

  free_mem((void*) this->CFortEditor_repr);
}






/*************/
/*  Database */
/*************/




void CFortEditor::Open(Context context,
		      Context mod_in_pgm_context,
		      Context pgm_context,
		      DB_FP * fd,
                      DBObject * contents,
		      FortTextTree ftt, FortTree ft)
{
  R(this)->contents = contents;  /* should be '::setContents' ??? */
  this->INHERITED::Open(context, mod_in_pgm_context, pgm_context, fd, contents);

  R(this)->ftt = ftt;
  R(this)->ft  = ft;
  R(this)->fortEditor = ed_Open(context, fd, ftt, ft);
  ed_Notify(R(this)->fortEditor, (Generic) this, (ed_NotifyFunc) noteChange);

#if ANNOTATION_BROWSER_WORKING 
  R(this)->annotator = new FortAnnotMgr(context, fd, context);
  R(this)->annotator->Open(context, fd, context, (Generic) this, annotGotoFunc,
                           R(this)->fortEditor);

  R(this)->browser   = adlg_Open(context, fd, (Generic) this, R(this)->fortEditor);
#endif
}




void CFortEditor::Close(void)
{
#if ANNOTATION_BROWSER_WORKING 
  /* close annotation manager */
    R(this)->annotator->Close();
#endif
}




void CFortEditor::Save(Context context, DB_FP * fd)
{
  this->INHERITED::Save(context, fd);
  /*** ed_Save(R(this)->fortEditor, context, fd, true); ***/	/*???*/
}




void CFortEditor::GetContentsAttribute(char * &contents_attr)
{
  NOT_CALLED("CFortEditor::GetContentsAttribute");
}






/**********************/
/* Trap door for CPed */
/**********************/




Generic CFortEditor::getFortEditor(void)
{
  return R(this)->fortEditor;
}






/***********************/
/* Change notification */
/***********************/




void CFortEditor::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);

  /* it's not clear that this is right, but without it, */
  /* variable classification doesn't work in Ped2       */
    if( ob != R(this)->contents )  return;

  switch( kind )
    {
      case CHANGE_TREE_WILL_CHANGE:
        ed_TreeWillChange(R(this)->fortEditor, (FortTreeNode) change);
        break;

      case CHANGE_TREE_CHANGED:
        ed_TreeChanged(R(this)->fortEditor, (FortTreeNode) change);
        break;

      default:
        this->INHERITED::NoteChange(ob, kind, change);
        break;
    }
}






/*************/
/*  Viewing  */
/*************/




View * CFortEditor::OpenView(Context context, DB_FP * session_fd)
{
  return new ScrollView(new FortView(context, session_fd, this,
                                     makePoint(0, 0), FortView_Font),
                        true,
                        true);
}






/*************/
/* Filtering */
/*************/




UserFilterDef * CFortEditor::OpenFilterDef(Context context,
					  Context mod_in_pgm_context,
					  Context pgm_context,
                                          DB_FP * session_fd)
{
  UserFilterDef * def;

  def = new FortFilterDef(context,  session_fd);
  def->Open(context, mod_in_pgm_context, pgm_context, session_fd);

  return def;
}




void CFortEditor::AddStandardFilterDefs(FilterDefSet * defs)
{
  FortFilterDef::AddStandardDefs(defs);
}






/********************/
/*  Input handling  */
/********************/




Boolean CFortEditor::MenuChoice(Generic cmd)
{
  Boolean handled = true;
  int line, sel1, sel2;
  int l1, c1, l2, c2;

  /* TEMPORARY -- DOES NOT DEAL WITH C++ SCRAP */

  switch( cmd )
    {
      /* "edit" menu commands */

      case CMD_COPY:
        ed_Copy(R(this)->fortEditor, (FortScrap *) nil);
        break;

      case CMD_CUT:
        ed_Cut(R(this)->fortEditor, (FortScrap *) nil);
        break;

      case CMD_PASTE:
        ed_Paste(R(this)->fortEditor, nil);
        break;

      case CMD_CLEAR:
        ed_Clear(R(this)->fortEditor);
        break;

      case CMD_SELECT_ALL:
        this->SetSelection(0, 0, 99999, 99999);
        break;


      /* "search" menu commands */

#if ANNOTATION_BROWSER_WORKING 
      case CMD_GENERAL_INFO:
        adlg_Dialog(R(this)->browser,(Generic) R(this)->annotator->GetGlobal());
        break;

      case CMD_SELECTED_INFO:
        ed_GetSelection(R(this)->fortEditor, &line, &sel1, &sel2);
        if( line == UNUSED )
          { l1 = sel1;  c1 = UNUSED;
            l2 = sel2;  c2 = UNUSED;
          }
        else
          { l1 = line;  c1 = sel1;
            l2 = line;  c2 = sel2;
          }
        adlg_Dialog(R(this)->browser,
                    (Generic) R(this)->annotator->GetSelection(l1, c1, l2, c2));
        break;
#endif


      /* other menu commands */

      default:
        handled = this->INHERITED::MenuChoice(cmd);
        break;
    }

  return handled;
}




void CFortEditor::Keystroke(KbChar kb)
{
  ed_Key(R(this)->fortEditor, kb);
}






/************************/
/*  Access to contents  */
/************************/




int CFortEditor::NumLines(void)
{
  return ed_NumLines(R(this)->fortEditor);
}




int CFortEditor::MaxLineWidth(void)
{
  Point size = ed_DocSize(R(this)->fortEditor);

  return size.x;
}




void CFortEditor::GetLine(int k, TextString &ts, TextData &td)
{
  ed_GetLine(R(this)->fortEditor, k, &ts);
  Text_DefaultData(td);
}




void CFortEditor::GetLineInfo(int k, FortTreeNode &node, int &bracket)
{
  ftt_GetLineInfo(R(this)->ftt, k, &node, &bracket);
}




void CFortEditor::NodeToText(FortTreeNode node, int &line1, int &char1, int &line2, int &char2)
{
  (void) ftt_NodeToText(R(this)->ftt, node, &line1, &char1, &line2, &char2);
}




void CFortEditor::TextToNode(int line1, int char1, int line2, int char2, FortTreeNode &node)
{
  (void) ftt_TextToNode(R(this)->ftt, line1, char1, line2, char2, &node);
}






/***************/
/*  Selection  */
/***************/




void CFortEditor::GetSelection(Selection * &sel)
{
  int line1, char1, line2, char2;

  this->GetSelection(line1, char1, line2, char2);
  sel = new LineSelection(line1, char1, line2, char2);
}




void CFortEditor::GetSelection(int &line1, int &char1, int &line2, int &char2)
{
  int linenum, sel1, sel2;

  ed_GetSelection(R(this)->fortEditor, &linenum, &sel1, &sel2);

  if( linenum != UNUSED )
    { line1 = linenum;
      char1 = sel1;
      line2 = linenum;
      char2 = sel2;
    }
  else
    { line1 = sel1;
      char1 = 0;
      line2 = sel2;
      char2 = 9999;
    }
}




void CFortEditor::SetSelection(Selection * sel)
{
  LineSelection * ls = (LineSelection *) sel;

  this->SetSelection(ls->line1, ls->char1, ls->line2, ls->char2);
}




void CFortEditor::SetSelection(int line1, int char1, int line2, int char2)
{
  int linenum, sel1, sel2;
  Selection * sel;

  /* convert selection to Ned form */
    if( line1 == line2  &&  char2 != INFINITY )
      { linenum = line1;
        sel1    = char1;
        sel2    = char2;
      }
    else
      { linenum = UNUSED;
        sel1    = line1;
        sel2    = line2;
      }

  /* set selection here & in superclass with only one change notice */
    this->BeginEdit();
    ed_SetSelection(R(this)->fortEditor, linenum, sel1, sel2);
    this->INHERITED::SetSelection(line1, char1, line2, char2);
    this->EndEdit();
}




void CFortEditor::GetSelectedData(Generic &data)
{
  FortTreeNode node;

  ed_GetSelectedNode(R(this)->fortEditor, &node);
  data = (Generic) node;
}




void CFortEditor::SetSelectedData(Generic data)
{
  ed_SetSelectedNode(R(this)->fortEditor, (FortTreeNode) data);
}






/*************/
/*  Editing  */
/*************/




Scrap * CFortEditor::Extract(Selection * sel)
{
  /* ... */
  return nil;
}




void CFortEditor::Replace(Selection * sel, Scrap * scrap)
{
  /* ... */
}






/***********/
/* Errors  */
/***********/




Boolean CFortEditor::CheckLineData(int k)
{
  /* TEMPORARY */
  return true;
}




Boolean CFortEditor::CheckData(void)
{
  return ed_CheckModule(R(this)->fortEditor);
}






/***************/
/* Navigation  */
/***************/




void CFortEditor::NavigateTo(FortTreeNode node)
{
  int l1, c1, l2, c2;
  Point pt;

  ftt_NodeToText(R(this)->ftt, node, &l1, &c1, &l2, &c2);
  pt = makePoint(c1, l1);
  this->Changed(CHANGE_NAVIGATE_TO, (void *) &pt);
}






/*******************************/
/* Access to Fortran contents  */
/*******************************/




FortTree CFortEditor::GetFortTree(void)
{
  return R(this)->ft;
}




FortTextTree CFortEditor::GetFortTextTree(void)
{
  return R(this)->ftt;
}




void CFortEditor::SetCurrentLoop(FortTreeNode loop)
{
  /* nothing */
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static void noteChange(CFortEditor * fed,
                       int kind,
                       Boolean autoScroll,
                       FortTreeNode node,
                       int first,
                       int last,
                       int delta)
{
  LineEditorChange change;

  change.kind       = kind;
  change.autoScroll = autoScroll;
  change.data       = (Generic) node;
  change.first      = first;
  change.last       = last;
  change.delta      = delta;

  fed->Changed((kind == tt_SEL_CHANGED ? CHANGE_SELECTION : CHANGE_DOCUMENT),
               (void *) &change);
}




static
void annotGotoFunc(Generic fed)
{
  CFortEditor * This = (CFortEditor *) fed;
  int l1, c1, l2, c2;
  Point pt;

  /* ensure that the selection is visible */
    This->GetSelection(l1, c1, l2, c2);
    pt = makePoint(c1, l1);
    This->Changed(CHANGE_NAVIGATE_TO, (void *) &pt);

  CP::CurrentWindowToTop();
}
