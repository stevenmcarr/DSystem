/* $Id: DepEditor.C,v 1.2 1997/03/11 14:30:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/dep/DepEditor.C						*/
/*									*/
/*	DepEditor -- Ded Dependences sub-editor				*/
/*	Last edited: November 12, 1993 at 4:55 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/dep/DepEditor.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>
#include <libs/graphicInterface/cmdProcs/dEditor/DedEditor.h>
#include <libs/graphicInterface/cmdProcs/dEditor/dep/DepFilterDef.h>

#include <libs/graphicInterface/framework/CMenu.h>
#include <libs/graphicInterface/framework/ColumnView.h>
#include <libs/graphicInterface/framework/ScrollView.h>
#include <libs/graphicInterface/framework/Text.h>
#include <libs/graphicInterface/framework/CViewFilter.h>
#include <libs/graphicInterface/framework/LineSelection.h>
#include <libs/graphicInterface/framework/Dependence.h>

#include <libs/support/arrays/FlexibleArray.h>

#define tt_NOTIFY_DOC_CHANGED    1






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* DepEditor object */

typedef struct DepEditor_Repr_struct
  {
    /* creation parameters */
      DedEditor *	owner;
      DedDocument *	contents;
      FortTextTree	ftt;

    /* view */
      ColumnView *	view;
      CViewFilter *	mainFilter;

    /* current dependences */
      DedCurDepPolicy	curDepPolicy;
      LineSelection *	sourceSel;
      Flex *		curDeps;

  } DepEditor_Repr;




#define R(ob)		(ob->DepEditor_repr)
#define INHERITED	ColumnEditor






/***************/
/* Line layout */
/***************/



/* column numbers */

#define COLUMN_TYPE              0
#define COLUMN_SRC               1
#define COLUMN_SINK              2
#define COLUMN_VECTOR            3
#define COLUMN_LEVEL             4
#define COLUMN_BLOCK             5




/* character positions of fields */

#define LINE_WIDTH		80

#define LINEPOS_TYPE		 0
#define LINEPOS_SRC		( 8 + LINEPOS_TYPE)
#define LINEPOS_SINK		(22 + LINEPOS_SRC)
#define LINEPOS_VECTOR		(22 + LINEPOS_SINK)
#define LINEPOS_LEVEL		(12 + LINEPOS_VECTOR)
#define LINEPOS_BLOCK		( 3 + LINEPOS_LEVEL)

#define LINEPOS_STMT		 8
#define LINEPOS_STMT_ELLIPSIS	60
#define MAX_STMT_LEN		(LINEPOS_STMT_ELLIPSIS-LINEPOS_STMT)




/* column layout for view */

#define de_numCols    6

static char * de_headings[] =
  {"TYPE", "SOURCE", "SINK", "VECTOR", "LVL", "BLOCK"};

static int de_colWidths[] =
  { LINEPOS_SRC    - LINEPOS_TYPE,	/* TYPE   */
    LINEPOS_SINK   - LINEPOS_SRC,	/* SOURCE */
    LINEPOS_VECTOR - LINEPOS_SINK,	/* SINK   */
    LINEPOS_LEVEL  - LINEPOS_VECTOR,	/* VECTOR */
    LINEPOS_BLOCK  - LINEPOS_LEVEL,	/* LVL    */
    LINE_WIDTH     - LINEPOS_BLOCK     /* BLOCK  */
  };

#define de_numDivLines    0

static int de_divLineCols[] =
  {UNUSED};    /* g++ 2.4.5 can't handle empty initializer */






/*************************/
/*  Miscellaneous	 */
/*************************/




/* check mark glyph, defined by Rn but not declared anywhere */

extern "C" char * CHECK_MARKED;






/*************************/
/*  Forward declarations */
/*************************/




static void adjustCurrentDependences(DepEditor * ded, Boolean notify);

static Boolean depIsCrossProcessor(DepEditor * ded, int depNum);

static Color depCrossProcessorColor(DepEditor * ded, int depNum);

static Boolean depInSourceSelection(DepEditor * ded, int depNum, LineSelection * sel);

static Boolean intersect(int node_line, int node_char1, int node_char2,
                         int sel_line1, int sel_char1, int sel_line2, int sel_char2);

static void clearCurDeps(DepEditor * ded);

static void addCurDep(DepEditor * ded, int depNum);






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void DepEditor::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(ColumnEditor);
    REQUIRE_INIT(ColumnView);
    
    REQUIRE_INIT(DepFilterDef);
    REQUIRE_INIT(DedDocument);
    REQUIRE_INIT(DedEditor);
}




void DepEditor::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(DepEditor)




DepEditor::DepEditor(Context context,
                     DB_FP * session_fd,
                     DedEditor * owner)
         : ColumnEditor(context, session_fd)
{
  /* allocate instance's private data */
    this->DepEditor_repr = (DepEditor_Repr *) get_mem(sizeof(DepEditor_Repr),
                            "DepEditor instance");

  /* save creation parameters */
    R(this)->owner = owner;

  /* initialize current dependences */
    R(this)->curDepPolicy = DedDoc_CURDEP_CROSS;
    R(this)->sourceSel    = new LineSelection;
    R(this)->curDeps      = flex_create(sizeof(int));
}




DepEditor::~DepEditor()
{
  flex_destroy(R(this)->curDeps);
  delete R(this)->sourceSel;
  free_mem((void*) this->DepEditor_repr);
}




void DepEditor::setContents(DBObject * contents)
{
  FortTree dummy;

  R(this)->contents = (DedDocument *) contents;
  this->INHERITED::setContents(R(this)->contents);

  R(this)->contents->GetSource(R(this)->ftt, dummy);
}






/*************/
/*  Database */
/*************/




void DepEditor::Open(Context context,
		     Context mod_in_pgm_context,
		     Context pgm_context,
		     DB_FP * fd, DBObject * contents)
{
  this->INHERITED::Open(context, mod_in_pgm_context, pgm_context, fd, contents);
  this->SetSortColumn(COLUMN_SRC);
}




void DepEditor::Save(Context context, DB_FP * fd)
{
  this->INHERITED::Save(context, fd);
}






/***********************/
/* Change notification */
/***********************/




void DepEditor::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);

  switch( kind )
    {
      case CHANGE_LOOP:
      case CHANGE_DEPENDENCE:
        this->BeginEdit();
        this->INHERITED::SetSelectionNone();
        this->ChangedEverything();
        this->EndEdit();
        adjustCurrentDependences(this, BOOL( kind == CHANGE_DEPENDENCE ));
        break;

      case CHANGE_FILTER:
        if( ob == R(this)->view )
          { R(this)->mainFilter = (CViewFilter *) change;
            if( R(this)->curDepPolicy == DedDoc_CURDEP_ALL )
              adjustCurrentDependences(this, true);
          }
        else
          this->INHERITED::NoteChange(ob, kind, change);
        break;

      case CHANGE_SRC_SELECTION:
        delete R(this)->sourceSel;
        R(this)->sourceSel = (LineSelection *) change;
        if( R(this)->curDepPolicy == DedDoc_CURDEP_SEL )
          adjustCurrentDependences(this, true);
        break;

      default:
        this->INHERITED::NoteChange(ob, kind, change);
    }
}






/*************/
/*  Viewing  */
/*************/




View * DepEditor::OpenView(Context context, DB_FP * session_fd)
{
  R(this)->view = new ColumnView(context, session_fd, this, makePoint(0, 0),
                                 UNUSED, UNUSED,
                                 de_numCols, de_colWidths, de_headings,
                                 de_numDivLines, de_divLineCols);
  R(this)->view->SetSelectionOptions(true, true);
  R(this)->view->Notify(this, true);
  return new ScrollView(R(this)->view, false, true);
}




void DepEditor::CloseView(View * view)
{
  R(this)->view->Notify(this, false);
  this->INHERITED::CloseView(view);
}




void DepEditor::SetView(View * view)
{
  R(this)->view = (ColumnView *) view;
}






/*************/
/* Filtering */
/*************/




UserFilterDef * DepEditor::OpenFilterDef(Context context,
					 Context mod_in_pgm_context,
					 Context pgm_context,
                                         DB_FP * session_fd)
{
  DepFilterDef * def;

  def = new DepFilterDef(context, session_fd, this);
  def->Open(context, mod_in_pgm_context, pgm_context, session_fd);
  return def;
}




void DepEditor::AddStandardFilterDefs(FilterDefSet * defs)
{
  DepFilterDef::AddStandardDefs(defs);
}






/********************/
/*  Input handling  */
/********************/




Boolean DepEditor::MenuChoice(Generic cmd)
{
  Boolean handled = true;
  
  switch( cmd )
    {
      /* "view" menu commands */

      case CMD_DEF_DEP_FILTER:
        this->DefineFilters();
        break;

      case CMD_SET_DEP_FILTER:
        this->ChooseFilterFor(R(this)->view);
        break;


      /* "dependence" menu commands */

      case CMD_ARROWS:
        this->arrowCommand();
        break;


    /* other menu commands */
    
      default:
        handled = this->INHERITED::MenuChoice(cmd);
        break;
    }
    
  return handled;
}






/*************/
/* Selection */
/*************/




void DepEditor::SetSelection(int line1, int char1, int line2, int char2)
{
  this->INHERITED::SetSelection(line1, char1, line2, char2);

  if( R(this)->curDepPolicy == DedDoc_CURDEP_1 )
    adjustCurrentDependences(this, true);
}




void DepEditor::GetNavigationTarget(int line, int col, FortTreeNode &target)
{
  Dependence dep;

  R(this)->contents->GetDependence(line, dep);

  if( LINEPOS_SRC <= col  &&  col < LINEPOS_SINK )
    target = dep.src;
  else if( LINEPOS_SINK <= col  &&  col < LINEPOS_VECTOR )
    target = dep.sink;
  else
    target = nil;
}






/************/
/* Contents */
/************/




int DepEditor::NumLines(void)
{
  FortTreeNode loop;

  R(this)->owner->GetCurrentLoop(loop);

  return ( loop == AST_NIL ? 0 : R(this)->contents->NumDependences() );
}




int DepEditor::MaxLineWidth(void)
{
  return LINE_WIDTH;
}




void DepEditor::GetLine(int linenum, TextString &ts, TextData &td)
{
  char line[LINE_WIDTH+1];
  int k;
  Dependence dep;
  char * type;
  char * src;
  char * sink;
  char * vector;
  char * level;
  char * block;
  char * stmt;
  char * reason = "";    /*** TEMPORARY ***/
  int curDep;
  Color color;

  /* get the dependence's fields as strings */
    R(this)->contents->GetDependence(linenum, dep);
    R(this)->contents->GetDependenceTexts(linenum, type, src, sink, vector, level, block, stmt);

  /* compose the fields into a line */
    for( k = 0;  k < LINE_WIDTH;  k++ )  line[k] = ' ';
    line[LINE_WIDTH] = '\0';

    strncpy(&line[LINEPOS_TYPE],   type,   strlen(type)  );

    if( strlen(stmt) == 0 )
      { strncpy(&line[LINEPOS_SRC],    src,    strlen(src)   );
        strncpy(&line[LINEPOS_SINK],   sink,   strlen(sink)  );
        strncpy(&line[LINEPOS_VECTOR], vector, strlen(vector));
        strncpy(&line[LINEPOS_LEVEL],  level,  strlen(level) );
        strncpy(&line[LINEPOS_BLOCK],  block,  strlen(block) );
      }
    else
      { strncpy(&line[LINEPOS_STMT], stmt, min(strlen(stmt), MAX_STMT_LEN));
        if( strlen(stmt) > MAX_STMT_LEN )
          strncpy(&line[LINEPOS_STMT_ELLIPSIS], "...", 3);
      }

  /* convert line buffer to textstring + textdata */
    ts = makeTextString(line, STYLE_NORMAL, "DepEditor::GetLine");
    ts.ephemeral = false;

  /* color the line appropriately */
    Text_DefaultData(td);
    color = R(this)->contents->GetDependenceColor(dep.edge);
    if( color != NULL_COLOR )
      { td.all.foreground = color;
        for( k = 0;  k < ts.num_tc;  k++ )
          ts.tc_ptr[k].style |= STYLE_BOLD;
      }
}




void DepEditor::SetSortColumn(int colNum)
{
  PedDependenceOrder sort;

  switch( colNum )
    {
      case COLUMN_TYPE:  sort = pedDepSortType;               break;
      case COLUMN_SRC:   sort = pedDepSortSource;             break;
      case COLUMN_SINK:  sort = pedDepSortSink;               break;
 /*** case COLUMN_LEVEL: sort = ???;                          break; ***/
      case COLUMN_BLOCK: sort = pedDepSortBlock;              break;
      default:           sort = (PedDependenceOrder) UNUSED;  break;
    }

  if( sort != UNUSED )
    { this->INHERITED::SetSortColumn(colNum);
      R(this)->contents->SortDependences(sort);
    }
}






/***********************/
/* Current dependences */
/***********************/




int DepEditor::NumCurrentDependences(void)
{
  return flex_length(R(this)->curDeps);
}




void DepEditor::GetCurrentDependence(int k, int &depNum)
{
  flex_get_buffer(R(this)->curDeps, k, 1, (char *) &depNum);
}




void DepEditor::SetCurrentDependencePolicy(DedCurDepPolicy policy)
{
  R(this)->curDepPolicy = policy;
  adjustCurrentDependences(this, true);
}




void DepEditor::GetCurrentDependencePolicy(DedCurDepPolicy &policy)
{
  policy = R(this)->curDepPolicy;
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void adjustCurrentDependences(DepEditor * ded, Boolean notify)
{
  Boolean changed = true;
  int line1, char1, line2, char2, old;
  Point filteredSize;
  int k, depNum;
  Color theColor;

  switch( R(ded)->curDepPolicy )
    {
      case DedDoc_CURDEP_1:
        if( ded->HasSelection() )
          { ded->GetSelection(line1, char1, line2, char2);

            /* special case of no-change is important for dep-reason typing speed */
              changed = NOT( ded->NumCurrentDependences() == 1  &&
                             (ded->GetCurrentDependence(0, old), old == line1) );

            if( changed )
              { clearCurDeps(ded);
                addCurDep(ded, line1);
              }
          }
        else
          clearCurDeps(ded);

        break;

        // added check for the dependence color so that non-communication causing
        // arrows won't be displayed (i.e. the communication they cause if carried
        // by something other than what is currently selected.  -- curetonk 5/94
      case DedDoc_CURDEP_CROSS:
        clearCurDeps(ded);
        R(ded)->mainFilter->GetDocSize(filteredSize);
        for( k = 0;  k < filteredSize.y;  k++ )
          { depNum = R(ded)->mainFilter->ViewToContentsLinenum(k);
            if( depIsCrossProcessor(ded, depNum) )
              {
                theColor = depCrossProcessorColor(ded, depNum);
                if( theColor != NULL_COLOR )
                  {
                    addCurDep(ded, depNum);
                  }
              }
          }
        break;

      case DedDoc_CURDEP_ALL:
        clearCurDeps(ded);
        /* kludge: ensure view filter knows filtered ht accurately */
          (void) R(ded)->mainFilter->ViewToContentsLinenum(INFINITY-1);  /* worse, 'INFINITY' won't do! */
        R(ded)->mainFilter->GetDocSize(filteredSize);
        for( k = 0;  k < filteredSize.y;  k++ )
          { depNum = R(ded)->mainFilter->ViewToContentsLinenum(k);
            addCurDep(ded, depNum);
          }
        break;

      case DedDoc_CURDEP_SEL:
        clearCurDeps(ded);
        R(ded)->mainFilter->GetDocSize(filteredSize);
        for( k = 0;  k < filteredSize.y;  k++ )
          { depNum = R(ded)->mainFilter->ViewToContentsLinenum(k);
            if( depInSourceSelection(ded, depNum, R(ded)->sourceSel) )
              addCurDep(ded, depNum);
          }
        break;

    }

  if( changed && notify )
    { /*** ded->Changed(CHANGE_CURRENT_DEPENDENCE, nil); ***/
      R(ded)->owner->NoteChange(ded, CHANGE_CURRENT_DEPENDENCE, nil);
    }
}




static
Boolean depIsCrossProcessor(DepEditor * ded, int depNum)
{
  void * edge;
  
  edge = R(ded)->contents->GetDependenceEdge(depNum);
  return R(ded)->contents->IsDependenceCrossProcessor(edge);
}



static
Color depCrossProcessorColor(DepEditor * ded, int depNum)
{
  void * edge;
  
  edge = R(ded)->contents->GetDependenceEdge(depNum);

  if( R(ded)->contents->IsDependenceCrossProcessor(edge) )
    {
      return R(ded)->contents->GetDependenceColor(edge);
    }
  else
    {
      return Ded_PurpleColor;
    }
}




static
Boolean depInSourceSelection(DepEditor * ded, int depNum, LineSelection * sel)
{
  Dependence dep;
  int src_l1,  src_c1,  src_l2,  src_c2;
  int sink_l1, sink_c1, sink_l2, sink_c2;
  Boolean srcIntersects, sinkIntersects;

  R(ded)->contents->GetDependence(depNum, dep);

  /* ASSERT: source and sink nodes each lie on a single line */

  if( is_subscript(tree_out(dep.src)) )  dep.src = tree_out(dep.src);
  ftt_NodeToText(R(ded)->ftt, dep.src, &src_l1, &src_c1, &src_l2, &src_c2);
  srcIntersects =  intersect(src_l1, src_c1, src_c2,
                             sel->line1, sel->char1, sel->line2, sel->char2);

  if( is_subscript(tree_out(dep.sink)) )  dep.sink = tree_out(dep.sink);
  ftt_NodeToText(R(ded)->ftt, dep.sink, &sink_l1, &sink_c1, &sink_l2, &sink_c2);
  sinkIntersects = intersect(sink_l1, sink_c1, sink_c2,
                             sel->line1, sel->char1, sel->line2, sel->char2);

  return BOOL( srcIntersects || sinkIntersects );
}




static
Boolean intersect(int node_line, int node_char1, int node_char2,
                  int sel_line1, int sel_char1, int sel_line2, int sel_char2)
{
  return BOOL( (sel_line1 <= node_line && node_line <= sel_line2)
               &&
               (sel_line1 < sel_line2 ||
                   ! (node_char2 < sel_char1 || node_char1 > sel_char2)
               )
             );
}




static
void clearCurDeps(DepEditor * ded)
{
  flex_delete(R(ded)->curDeps, 0, flex_length(R(ded)->curDeps));
}




static
void addCurDep(DepEditor * ded, int depNum)
{
  flex_insert_one(R(ded)->curDeps, flex_length(R(ded)->curDeps), (char *) &depNum);
}




void DepEditor::arrowCommand(void)
{
  CMenu * m;
  DedCurDepPolicy policy;
  Generic choice;
  Boolean chosen;

  m = new CMenu("Show Arrows");
  m->AddItem(DedDoc_CURDEP_1,     KB_Nul, "for a single dependence",                 "", UNUSED, 0);
  m->AddItem(DedDoc_CURDEP_CROSS, KB_Nul, "for cross-processor dependences",         "", UNUSED, 0);
  m->AddItem(DedDoc_CURDEP_ALL,   KB_Nul, "for all listed dependences",              "", UNUSED, 0);
  m->AddItem(DedDoc_CURDEP_SEL,   KB_Nul, "for dependences selected in source pane", "", UNUSED, 0);

  this->GetCurrentDependencePolicy(policy);
  m->CheckItem(policy, true);

  chosen = m->Select(choice);
  if( chosen )
    this->SetCurrentDependencePolicy((DedCurDepPolicy) choice);

  delete m;
}
