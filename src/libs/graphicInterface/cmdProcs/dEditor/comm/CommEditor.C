/* $Id: CommEditor.C,v 1.2 1997/03/11 14:30:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/comm/CommEditor.C					*/
/*									*/
/*	CommEditor -- Ded communication sub-editor			*/
/*	Last edited: September 22, 1994 at 1:11 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/comm/CommEditor.h>

#include <libs/graphicInterface/framework/ColumnView.h>
#include <libs/graphicInterface/framework/ScrollView.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>
#include <libs/graphicInterface/cmdProcs/dEditor/DedEditor.h>
#include <libs/graphicInterface/cmdProcs/dEditor/comm/CommFilterDef.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* CommEditor object */

typedef struct CommEditor_Repr_struct
  {
    /* creation parameters */
      DedEditor *	owner;
      DedDocument *	contents;
      
    /* view */
      ColumnView *	view;

  } CommEditor_Repr;




#define R(ob)		(ob->CommEditor_repr)
#define INHERITED	ColumnEditor




/***************/
/* Line layout */
/***************/


/* column numbers */

#define COLUMN_MSG_TYPE          0
#define COLUMN_COMM_REDUC_TYPE   1
#define COLUMN_PROCRANGE         2
#define COLUMN_DIRECTION         3
#define COLUMN_SECTIONS          4




/* character positions of fields */

#define LINE_WIDTH		  80

#define LINEPOS_MSG_TYPE	   0
#define LINEPOS_COMM_REDUC_TYPE	 ( 10 + LINEPOS_MSG_TYPE)
#define LINEPOS_PROCRANGE	 ( 9 + LINEPOS_COMM_REDUC_TYPE)
#define LINEPOS_DIRECTION	 ( 7 + LINEPOS_PROCRANGE)
#define LINEPOS_SECTIONS	 ( 5 + LINEPOS_DIRECTION)




/* column layout for view */

#define ce_numCols    5

static char * ce_headings[] =
{
  "TYPE",
  "COMM",
  "PROCS",
  "DIR",
  "SECTIONS"
};

static int ce_colWidths[] =
{
  LINEPOS_COMM_REDUC_TYPE - LINEPOS_MSG_TYPE,		/* TYPE     */
  LINEPOS_PROCRANGE       - LINEPOS_COMM_REDUC_TYPE,	/* COMM     */
  LINEPOS_DIRECTION       - LINEPOS_PROCRANGE,		/* PROCS    */
  LINEPOS_SECTIONS        - LINEPOS_DIRECTION,		/* DIR      */
  LINE_WIDTH              - LINEPOS_SECTIONS,		/* SECTIONS */
};

#define ce_numDivLines    4

static int ce_divLineCols[] =
{
  COLUMN_COMM_REDUC_TYPE,
  COLUMN_PROCRANGE,
  COLUMN_DIRECTION,
  COLUMN_SECTIONS
};




/*************************/
/*  Forward declarations */
/*************************/


static char * formatMsgType(Mesg_type mt);

static char * formatCommType(Comm_type ct);

static char * formatReducType(Reduc_type rt);

static char * formatProcRange(char * fortd_string, Comm_type commType, Boolean receive);

static char * formatSection(char * fortd_string);






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void CommEditor::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(ColumnEditor);
    REQUIRE_INIT(ColumnView);
    REQUIRE_INIT(DedDocument);
    REQUIRE_INIT(DedEditor);
    REQUIRE_INIT(CommFilterDef);
}




void CommEditor::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(CommEditor)




CommEditor::CommEditor(Context context,
                       DB_FP * session_fd,
                       DedEditor * owner)
          : ColumnEditor(context, session_fd)
{
  /* allocate instance's private data */
    this->CommEditor_repr =
        (CommEditor_Repr *) get_mem(sizeof(CommEditor_Repr), "CommEditor instance");

  /* save creation parameters */
    R(this)->owner = owner;
}




CommEditor::~CommEditor()
{
  free_mem((void*) this->CommEditor_repr);
}




void CommEditor::setContents(DBObject * contents)
{
  R(this)->contents = (DedDocument *) contents;
  this->INHERITED::setContents(R(this)->contents);
}






/*************/
/*  Database */
/*************/




void CommEditor::Open(Context context, 
		      Context mod_in_pgm_context,
		      Context pgm_context,
		      DB_FP * fd, DBObject * contents)
{
  this->INHERITED::Open(context, mod_in_pgm_context, pgm_context, fd, contents);
}




void CommEditor::Save(Context context, DB_FP * fd)
{
  this->INHERITED::Save(context, fd);
}






/***********************/
/* Change notification */
/***********************/




void CommEditor::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);

  switch( kind )
    {
      case CHANGE_LOOP:
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




View * CommEditor::OpenView(Context context, DB_FP * session_fd)
{
  R(this)->view = new ColumnView(context, session_fd, this, makePoint(0, 0),
                                 UNUSED, UNUSED,
                                 ce_numCols, ce_colWidths, ce_headings,
                                 ce_numDivLines, ce_divLineCols);
  R(this)->view->SetSelectionOptions(true, true);
  return new ScrollView(R(this)->view, false, true);
}




void CommEditor::SetView(View * view)
{
  R(this)->view = (ColumnView *) view;
}






/*************/
/* Filtering */
/*************/




UserFilterDef * CommEditor::OpenFilterDef(Context context,
					  Context mod_in_pgm_context,
					  Context pgm_context,
                                          DB_FP * session_fd)
{
  CommFilterDef * def;

  def = new CommFilterDef(context, session_fd, this);
  def->Open(context, mod_in_pgm_context, pgm_context, session_fd);
  return def;
}




void CommEditor::AddStandardFilterDefs(FilterDefSet * defs)
{
  CommFilterDef_AddStandardDefs(defs);
}






/********************/
/*  Input handling  */
/********************/




Boolean CommEditor::MenuChoice(Generic cmd)
{
  Boolean handled = true;
  
  switch( cmd )
    {
      /* "view" menu commands */

      case CMD_DEF_COMM_FILTER:
        this->DefineFilters();
        break;

      case CMD_SET_COMM_FILTER:
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




void CommEditor::SetSelection(int line1, int char1, int line2, int char2)
{
  /* force selection to be whole messages (even/odd line pairs) */

  if( this->IsSelection(line1, char1, line2, char2) )
    { line1 = 2 * (line1 / 2);
      line2 = 2 * (line2 / 2) + 1;
      this->INHERITED::SetSelection(line1, 0, line2, INFINITY);
    }
  else
    this->INHERITED::SetSelection(line1, char1, line2, char2);
}




void CommEditor::GetNavigationTarget(int line, int col, FortTreeNode &target)
{

  /* TEMPORARY */
  target = nil;
}






/************/
/* Contents */
/************/




int CommEditor::NumLines(void)
{
  return 2 * R(this)->contents->NumMessages();
}




int CommEditor::MaxLineWidth(void)
{
  return LINE_WIDTH;
}




void CommEditor::GetLine(int linenum, TextString &ts, TextData &td)
{
  int messageNum, sublineNum;
  Comm_type commType;
  FortDMesg * message;
  char * mtype;
  char * crtype;
  char * direction;
  char * range;
  char * section;
  char line[LINE_WIDTH+1];
  int k;

# define PUT(FIELD, STRING)			\
           strncpy(&line[LINEPOS_##FIELD],	\
                   STRING,			\
                   min(strlen(STRING), ce_colWidths[COLUMN_##FIELD]))


  /* determine what information should be shown on the requested line */
    messageNum = linenum / 2;
    sublineNum = linenum % 2;
    R(this)->contents->GetMessage(messageNum, message);
    commType = message->CommType();
    
  /* compute the appropriate subline of the message's display */
    switch( sublineNum )
      {
        case 0:
          direction = "from";
          mtype     = formatMsgType(message->MesgType());
          crtype    = ( commType == FD_COMM_REDUCE
                        ? formatReducType(message->ReducType())
                        : formatCommType(commType) );

          switch( commType )
            {
              case FD_COMM_REDUCE:
                direction  = "";
                range      = ssave("all");
                section    = formatSection(message->SendSectionString()->string);
                break;

              case FD_COMM_SEND_RECV:
              case FD_COMM_SHIFT:
              case FD_COMM_BCAST:
              default:
                range      = formatProcRange(message->SendProcessorRangeString()->string,
                                             commType, false);
                section    = formatSection(message->SendSectionString()->string);
                break;
            }

          break;

        case 1:
          direction = "to  ";
          mtype     = ssave("");
          crtype    = ssave("");

          switch( commType )
            {
              case FD_COMM_REDUCE:
                direction  = "";
                range      = ssave("");
                section    = nssave(2, "msg size ", message->ReducMsgSizeStr()->string);
                break;

              case FD_COMM_SEND_RECV:
              case FD_COMM_SHIFT:
              case FD_COMM_BCAST:
              default:
                range   = formatProcRange(message->RecvProcessorRangeString()->string,
                                          commType, true);
                section = formatSection(message->RecvSectionString()->string);
                break;
            }
            
          break;
          
      }

  /* compose the fields into a line */
    for( k = 0;  k < LINE_WIDTH;  k++ )  line[k] = ' ';
    line[LINE_WIDTH] = '\0';

    PUT(MSG_TYPE,        mtype    );
    PUT(COMM_REDUC_TYPE, crtype   );
    PUT(PROCRANGE,       range    );
    PUT(DIRECTION,       direction);
    PUT(SECTIONS,        section  );

  /* convert line buffer to textstring + textdata */
    ts = makeTextString(line, STYLE_NORMAL, "CommEditor::GetLine");
    ts.ephemeral = false;

  /* color the line appropriately */
    Text_DefaultData(td);

  /* discard temporary storage */
    sfree(range);
    sfree(section);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
char * formatMsgType(Mesg_type mt)
{
  /* length of returned string is 9 */
  
  switch( mt )
    {
      case FD_MESG_INDEP:        return "outside  ";
      case FD_MESG_CARRIED_ALL:  return "inside   ";
      case FD_MESG_CARRIED_PART: return "pipelined";
      case FD_MESG_REDUC:        return "reduction";
      default:                   return "?????????";
    }
}




static
char * formatCommType(Comm_type ct)
{
  /* length of returned string is 9 */
  
  switch( ct )
    {
      case FD_COMM_SEND_RECV: return "send-recv";
      case FD_COMM_SHIFT:     return "shift    ";
      case FD_COMM_BCAST:     return "broadcast";
      case FD_COMM_REDUCE:    return "???reduce";  /* not used */
      case FD_COMM_GATHER:    return "gather   ";
      case FD_COMM_TRANSPOSE: return "transpose";
      case FD_COMM_INSPECT:   return "inspector";
      case FD_COMM_RUNTIME:   return "runtime  ";
      default:                return "?????????";
    }
}




static
char * formatReducType(Reduc_type rt)
{
  /* length of returned string is 7 */
  
  switch( rt )
    {
      case FD_REDUC_PLUS:    return "plus   ";
      case FD_REDUC_MINUS:   return "minus  ";
      case FD_REDUC_TIMES:   return "times  ";
      case FD_REDUC_DIV:     return "div    ";
      case FD_REDUC_MIN:     return "min    ";
      case FD_REDUC_MAX:     return "max    ";
      case FD_REDUC_OR:      return "or     ";
      case FD_REDUC_AND:     return "and    ";
      case FD_REDUC_XOR:     return "xor    ";
      case FD_REDUC_MIN_LOC: return "min loc";
      case FD_REDUC_MAX_LOC: return "max loc";
      default:               return "???????";
    }
}





/* caller must free the returned string */

static
char * formatProcRange(char * fortd_string, Comm_type commType, Boolean receive)
{
  int len;
  char buffer[1000];
  int from_front, from_back;

  if( (commType == FD_COMM_BCAST) && receive )
    strcpy(buffer, "other");

  else
    { /* remove "--<<...( " from front and " )  >>-- " from back */

        if( commType == FD_COMM_SHIFT )
          { from_front = 30;
            from_back  =  9;
          }
        else
          { from_front = 21;
            from_back  =  5;
          }

        len = strlen(fortd_string) - from_front - from_back;

        strncpy(buffer, &fortd_string[from_front], len);
        buffer[len] = '\0';
    }

  return ssave(buffer);
}





/* caller must free the returned string */

static
char * formatSection(char * fortd_string)
{
  int start, len;
  char * deleted1;
  char * deleted2;
  char buffer[200];

  /* remove "--<< XXXX " from front and " >>--" from back */
    if( find(fortd_string, "Global Accumulation") != UNUSED )
      deleted1 = "--<< Global Accumulation of ";
    else
      deleted1 = "--<< Send ";    /* or "... Recv " */
    deleted2 = " >>--";

    start = strlen(deleted1);
    len   = strlen(fortd_string) - strlen(deleted1) - strlen(deleted2);

    strncpy(buffer, &fortd_string[start], len);
    buffer[len] = '\0';

  return ssave(buffer);
}
