/* $Id: DistEditor.C,v 1.2 1997/03/11 14:30:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/dist/DistEditor.C					*/
/*									*/
/*	DistEditor -- Ded distribution sub-editor			*/
/*	Last edited: November 11, 1993 at 2:30 am			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/dist/DistEditor.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>
#include <libs/graphicInterface/cmdProcs/dEditor/DedEditor.h>
#include <libs/graphicInterface/cmdProcs/dEditor/dist/DistFilterDef.h>

#include <libs/graphicInterface/framework/ColumnView.h>
#include <libs/graphicInterface/framework/ScrollView.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* DistEditor object */

typedef struct DistEditor_Repr_struct
  {
    /* creation parameters */
      DedEditor *	owner;
      DedDocument *	contents;
      
    /* view */
      ColumnView *	view;

  } DistEditor_Repr;




#define R(ob)		(ob->DistEditor_repr)
#define INHERITED	ColumnEditor






/***************/
/* Line layout */
/***************/




/* column numbers */

#define COLUMN_NAME              0
#define COLUMN_ALIGN             1
#define COLUMN_DISTRIB           2
#define COLUMN_DECOMP            3




/* character positions of fields */

#define LINE_WIDTH		  100

#define LINEPOS_NAME		   0
#define LINEPOS_ALIGN		 ( 8 + LINEPOS_NAME)
#define LINEPOS_DISTRIB		 (10 + LINEPOS_ALIGN)
#define LINEPOS_DECOMP		 (25 + LINEPOS_DISTRIB)




/* column layout for view */

#define de_numCols    4

static char * de_headings[] =
  {"NAME", "ALIGNMENT", "DISTRIBUTION", "DECOMPOSITION"};

static int de_colWidths[] =
  { LINEPOS_ALIGN   - LINEPOS_NAME,		/* NAME */
    LINEPOS_DISTRIB - LINEPOS_ALIGN,		/* ALIGNMENT */
    LINEPOS_DECOMP  - LINEPOS_DISTRIB,		/* DISTRIBUTION */
    LINE_WIDTH      - LINEPOS_DECOMP		/* DECOMPOSITION */
  };

#define de_numDivLines    3

static int de_divLineCols[] =
  {COLUMN_ALIGN, COLUMN_DISTRIB, COLUMN_DECOMP};






/*************************/
/*  Forward declarations */
/*************************/




static char * formatAlign(char * fortd_string);

static char * formatDistrib(char * fortd_string);

static char * formatDecomp(char * fortd_string);






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void DistEditor::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(ColumnEditor);
    REQUIRE_INIT(ColumnView);
    REQUIRE_INIT(DedDocument);
    REQUIRE_INIT(DedEditor);
    REQUIRE_INIT(DistFilterDef);
}




void DistEditor::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(DistEditor)




DistEditor::DistEditor(Context context,
                       DB_FP * session_fd,
                       DedEditor * owner)
          : ColumnEditor(context, session_fd)
{
  /* allocate instance's private data */
    this->DistEditor_repr =
        (DistEditor_Repr *) get_mem(sizeof(DistEditor_Repr), "DistEditor instance");

  /* save creation parameters */
    R(this)->owner = owner;
}




DistEditor::~DistEditor()
{
  free_mem((void*) this->DistEditor_repr);
}




void DistEditor::setContents(DBObject * contents)
{
  R(this)->contents = (DedDocument *) contents;
  this->INHERITED::setContents(R(this)->contents);
}






/*************/
/*  Database */
/*************/




void DistEditor::Open(Context context, 
		      Context mod_in_pgm_context,
		      Context pgm_context,
		      DB_FP * fd, DBObject * contents)
{
  this->INHERITED::Open(context, mod_in_pgm_context, pgm_context, fd, contents);
}




void DistEditor::Save(Context context, DB_FP * fd)
{
  this->INHERITED::Save(context, fd);
}






/***********************/
/* Change notification */
/***********************/




void DistEditor::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);

  switch( kind )
    {
      case CHANGE_LOOP:
      case CHANGE_VARIABLE:	/*** SHOULD BE CHANGE_DIST ???? ****/
        this->BeginEdit();
        this->SetSelectionNone();
        this->ChangedEverything();
        this->EndEdit();
        break;

      default:
        this->INHERITED::NoteChange(ob, kind, change);
    }
}






/*************/
/*  Viewing  */
/*************/




View * DistEditor::OpenView(Context context, DB_FP * session_fd)
{
  R(this)->view = new ColumnView(context, session_fd, this, makePoint(0, 0),
                                 UNUSED, UNUSED,
                                 de_numCols, de_colWidths, de_headings,
                                 de_numDivLines, de_divLineCols);
  R(this)->view->SetSelectionOptions(true, true);
  return new ScrollView(R(this)->view, false, true);
}




void DistEditor::SetView(View * view)
{
  R(this)->view = (ColumnView *) view;
}






/*************/
/* Filtering */
/*************/




UserFilterDef * DistEditor::OpenFilterDef(Context context,
					  Context mod_in_pgm_context,
					  Context pgm_context,
                                          DB_FP * session_fd)
{
  DistFilterDef * def;

  def = new DistFilterDef(context, session_fd, this);
  def->Open(context, mod_in_pgm_context, pgm_context, session_fd);
  return def;
}




void DistEditor::AddStandardFilterDefs(FilterDefSet * defs)
{
  DistFilterDef_AddStandardDefs(defs);
}






/********************/
/*  Input handling  */
/********************/




Boolean DistEditor::MenuChoice(Generic cmd)
{
  Boolean handled = true;
  
  switch( cmd )
    {
      /* "view" menu commands */

      case CMD_DEF_DIST_FILTER:
        this->DefineFilters();
        break;

      case CMD_SET_DIST_FILTER:
        this->ChooseFilterFor(R(this)->view);
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




void DistEditor::SetSelection(int line1, int char1, int line2, int char2)
{
  int newVar;

  this->INHERITED::SetSelection(line1, char1, line2, char2);

  if( this->HasSelection() )
    { this->GetSelection(line1, char1, line2, char2);
      newVar = line1;
    }
  else
    newVar = UNUSED;

  /***** R(this)->owner->SetCurrentVariable(newVar); *****/
}




void DistEditor::GetNavigationTarget(int line, int col, FortTreeNode &target)
{
  /*** TEMPORARY ***/
  target = nil;
}






/************/
/* Contents */
/************/




int DistEditor::NumLines(void)
{
  return R(this)->contents->NumVariables();
}




int DistEditor::MaxLineWidth(void)
{
  return LINE_WIDTH;
}




void DistEditor::GetLine(int linenum, TextString &ts, TextData &td)
{
  PedVariable var;
  FortDRef * ref;
  char * name;
  char * align;
  char * distrib;
  char * decomp;
  char line[LINE_WIDTH+1];
  int k;

# define PUT(FIELD, STRING)			\
           strncpy(&line[LINEPOS_##FIELD],	\
                   STRING,			\
                   min(strlen(STRING), de_colWidths[COLUMN_##FIELD]))


  /* determine what information should be shown on the requested line */
    R(this)->contents->GetVariable(linenum, var, ref);

  /* compute the appropriate text fields */
    name = var.name;
    if( ref != nil )
      { align   = formatAlign  ( (*ref->AlignStr()  )[0] );
        distrib = formatDistrib( (*ref->DistribStr())[0] );
        decomp  = formatDecomp ( (*ref->DecompStr() )[0] );
      }
    else
      { align   = ssave("");
        distrib = ssave("");
        decomp  = ssave("");
      }

  /* compose the fields into a line */
    for( k = 0;  k < LINE_WIDTH;  k++ )  line[k] = ' ';
    line[LINE_WIDTH] = '\0';

    PUT(NAME,    name   );
    PUT(ALIGN,   align  );
    PUT(DISTRIB, distrib);
    PUT(DECOMP,  decomp );

  /* convert line buffer to textstring + textdata */
    ts = makeTextString(line, STYLE_NORMAL, "DistEditor::GetLine");
    ts.ephemeral = false;

  /* color the line appropriately */
    Text_DefaultData(td);

  /* discard temporary storage */
    delete ref;
    sfree(align);
    sfree(distrib);
    sfree(decomp);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* caller must free returned string */

static
char * formatAlign(char * fortd_string)
{
  int start, len;
  char buffer[200];

  /* remove "ALIGN ... with " from front */
    start = find(fortd_string, " with ") + strlen(" with ");
    len = strlen(fortd_string) - start;
    strncpy(buffer, &fortd_string[start], len);
    buffer[len] = '\0';

  return ssave(buffer);
}




static
char * formatDistrib(char * fortd_string)
{
 int start, len;
  char buffer[200];

  /* remove "DISTRIBUTE " from front and " \n" from back */
    start = strlen("DISTRIBUTE ");
    len = strlen(fortd_string) - start - 2;
    strncpy(buffer, &fortd_string[start], len);
    buffer[len] = '\0';

  return ssave(buffer);
}



static
char * formatDecomp(char * fortd_string)
{
 int start, len;
  char buffer[200];

  /* remove "DECOMPOSITION " from front and " \n" from back */
    start = strlen("DECOMPOSITION ");
    len = strlen(fortd_string) - start - 2;
    strncpy(buffer, &fortd_string[start], len);
    buffer[len] = '\0';

  return ssave(buffer);
}
