/* $Id: DepFilterDef.C,v 1.2 1997/03/11 14:30:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/dep/DepFilterDef.C					*/
/*									*/
/*	DepFilterDef -- View filter defs for Ded dependence pane	*/
/*	Last edited: November 10, 1993 at 11:42 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/dep/DepFilterDef.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>
#include <libs/graphicInterface/cmdProcs/dEditor/dep/DepEditor.h>

#include <libs/graphicInterface/framework/FilterDefSet.h>
#include <libs/graphicInterface/framework/UserFilterDef.h>
#include <libs/graphicInterface/framework/UserFilter.h>
#include <libs/graphicInterface/framework/Dependence.h>

#include <stdlib.h>







/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* DepFilterDef object */

typedef struct DepFilterDef_Repr_struct
  {
    /* creation arguments */
      DepEditor *	editor;

  } DepFilterDef_Repr;




#define R(ob)		(ob->DepFilterDef_repr)
#define INHERITED	UserFilterDef






/*************************/
/*  Miscellaneous	 */
/*************************/




/* Ded dependence-pane predicate opcodes */

#define OPBASE           71

#define OP_TYPE          (OPBASE+ 0)
#define OP_SOURCE        (OPBASE+ 1)
#define OP_SOURCE_NAME   (OPBASE+ 2)
#define OP_SOURCE_LINE   (OPBASE+ 3)
#define OP_SINK          (OPBASE+ 4)
#define OP_SINK_NAME     (OPBASE+ 5)
#define OP_SINK_LINE     (OPBASE+ 6)
#define OP_VECTOR        (OPBASE+ 7)
#define OP_LEVEL         (OPBASE+ 8)
#define OP_BLOCK         (OPBASE+ 9)
#define OP_ACCEPTED      (OPBASE+10)
#define OP_REJECTED      (OPBASE+11)
#define OP_PENDING       (OPBASE+12)
#define OP_REASON        (OPBASE+13)
#define OP_LINE          (OPBASE+14)
#define OP_CURLOOP       (OPBASE+15)
#define OP_LOOP_CARRIED  (OPBASE+16)
#define OP_LOOP_INDEP    (OPBASE+17)
#define OP_CONTROL       (OPBASE+18)
#define OP_PRIVATE       (OPBASE+19)






/*************************/
/*  Forward declarations */
/*************************/




static void add(FilterDefSet * defs, char * name, char * text);

static Boolean lineRange(FortTextTree ftt, FortTreeNode node, char * range);

static int findNB(char * s, char * target);





/************************************************************************/
/*	Interface Operations						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void DepFilterDef::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(UserFilterDef);
    REQUIRE_INIT(DedDocument);
    REQUIRE_INIT(DepEditor);
}




void DepFilterDef::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(DepFilterDef)




DepFilterDef::DepFilterDef(Context context,
                           DB_FP * session_fd,
                           DepEditor * editor)
            : UserFilterDef(context, session_fd)
{
  /* allocate instance's private data */
    this->DepFilterDef_repr =
        (DepFilterDef_Repr *) get_mem(sizeof(DepFilterDef_Repr),
                                      "DepFilterDef instance");

  /* save creation arguments */
    R(this)->editor = editor;
}




DepFilterDef::~DepFilterDef()
{
  free_mem((void*) this->DepFilterDef_repr);
}






/********************/
/* Filter instances */
/********************/




UserFilter * DepFilterDef::MakeFilter(void * environment)
{
  UserFilter * uf;

  uf = this->INHERITED::MakeFilter(environment);
  uf->SetElision(ViewFilter_ELISION_NONE);

  return uf;
}






/**************************/
/* Filter def compilation */
/**************************/




Boolean DepFilterDef::function(char * name, int &numArgs, int &opcode)
{
  Boolean recognized = true;

  if( strcmp("type", name) == 0 )
    { numArgs = 1;
      opcode  = OP_TYPE;
    }
  else if( strcmp("source", name) == 0 )
    { numArgs = 1;
      opcode  = OP_SOURCE;
    }
  else if( strcmp("source_name", name) == 0 )
    { numArgs = 1;
      opcode  = OP_SOURCE_NAME;
    }
  else if( strcmp("source_line", name) == 0 )
    { numArgs = 1;
      opcode  = OP_SOURCE_LINE;
    }
  else if( strcmp("sink", name) == 0 )
    { numArgs = 1;
      opcode  = OP_SINK;
    }
  else if( strcmp("sink_name", name) == 0 )
    { numArgs = 1;
      opcode  = OP_SINK_NAME;
    }
  else if( strcmp("sink_line", name) == 0 )
    { numArgs = 1;
      opcode  = OP_SINK_LINE;
    }
  else if( strcmp("vector", name) == 0 )
    { numArgs = 1;
      opcode  = OP_VECTOR;
    }
  else if( strcmp("level", name) == 0 )
    { numArgs = 1;
      opcode  = OP_LEVEL;
    }
  else if( strcmp("block", name) == 0 )
    { numArgs = 1;
      opcode  = OP_BLOCK;
    }
  else if( strcmp("accepted", name) == 0 )
    { numArgs = 0;
      opcode  = OP_ACCEPTED;
    }
  else if( strcmp("rejected", name) == 0 )
    { numArgs = 0;
      opcode  = OP_REJECTED;
    }
  else if( strcmp("pending", name) == 0 )
    { numArgs = 0;
      opcode  = OP_PENDING;
    }
  else if( strcmp("reason", name) == 0 )
    { numArgs = 1;
      opcode  = OP_REASON;
    }
  else if( strcmp("line", name) == 0 )
    { numArgs = 1;
      opcode  = OP_LINE;
    }
  else if( strcmp("current_loop", name) == 0 )
    { numArgs = 0;
      opcode  = OP_CURLOOP;
    }
  else if( strcmp("loop_carried", name) == 0 )
    { numArgs = 0;
      opcode  = OP_LOOP_CARRIED;
    }
  else if( strcmp("loop_independent", name) == 0 )
    { numArgs = 0;
      opcode  = OP_LOOP_INDEP;
    }
  else if( strcmp("control", name) == 0 )
    { numArgs = 0;
      opcode  = OP_CONTROL;
    }
  else if( strcmp("private", name) == 0 )
    { numArgs = 0;
      opcode  = OP_PRIVATE;
    }
  else
    recognized = this->INHERITED::function(name, numArgs, opcode);

  return recognized;
}






/*****************************/
/* Filter def interpretation */
/*****************************/




Boolean DepFilterDef::execute(int opcode,
                              int linenum,
                              char * line,
                              void * environment,
                              int &result)
{
  DedDocument * doc = (DedDocument *) environment;
  FortTextTree ftt;
  FortTree ft;
  Boolean executed = true;
  Dependence dep;
  char * type;
  char * src;
  char * sink;
  char * vector;
  char * level;
  char * block;
  char * stmt;
  char * reason;
  Boolean stmt_dependence;
  char * arg;
  int line1, len, dummy;

  doc->GetSource(ftt,ft);

  /* get the dependence */
    doc->GetDependence(linenum, dep);
    doc->GetDependenceTexts(linenum, type, src, sink, vector, level, block, stmt);
    stmt_dependence = BOOL(strlen(stmt) != 0);

  switch( opcode )
    {
      case OP_TYPE:
        arg = (char *) this->operand();
        result = BOOL( findNB(type, arg) != UNUSED );
        break;

      case OP_SOURCE:
        arg = (char *) this->operand();
        result = BOOL( findNB(src, arg) != UNUSED );
        break;

      case OP_SOURCE_NAME:
        arg = (char *) this->operand();
        len = strlen(arg);
        if( strncmp(src, arg, len) == 0 )
          result = (src[len] == '(' || src[len] == '\0');
        else
          result = false;
        break;

      case OP_SOURCE_LINE:
        arg = (char *) this->operand();
        result = lineRange(ftt, dep.src, arg);
        break;

      case OP_SINK:
        arg = (char *) this->operand();
        result = BOOL( findNB(sink, arg) != UNUSED );
        break;

      case OP_SINK_NAME:
        arg = (char *) this->operand();
        len = strlen(arg);
        if( strncmp(src, arg, len) == 0 )
          result = (src[len] == '(' || src[len] == '\0');
        else
          result = false;
        break;

      case OP_SINK_LINE:
        arg = (char *) this->operand();
        result = lineRange(ftt, dep.sink, arg);
        break;

      case OP_VECTOR:
        arg = (char *) this->operand();
        result = BOOL( strcmp(vector, arg) == 0 );
        break;

      case OP_LEVEL:
        arg = (char *) this->operand();
        result = BOOL( strcmp(level, arg) == 0 );
        break;

      case OP_BLOCK:
        arg = (char *) this->operand();
        result = BOOL( strcmp(block, arg) == 0 );
        break;

      case OP_REASON:
        arg = (char *) this->operand();
        result = BOOL( find(reason, arg) != UNUSED );
        break;

      case OP_LINE:
        arg = (char *) this->operand();
        result = lineRange(ftt, dep.src, arg) || lineRange(ftt, dep.sink, arg);
        break;

      case OP_CURLOOP:
        /* TEMPORARY */
        result = true;
        break;

      case OP_LOOP_CARRIED:
        result = dep.loopCarried;
        break;

      case OP_LOOP_INDEP:
        result = dep.loopIndependent;
        break;

      case OP_CONTROL:
        result = dep.control;
        break;

      case OP_PRIVATE:
        result = dep.xprivate;
        break;

      default:
        executed = this->INHERITED::execute(opcode, linenum, line, nil, result);
        break;
    }

  return executed;
}






/*************************/
/*  Standard definitions */
/*************************/




void DepFilterDef::AddStandardDefs(FilterDefSet * defs)
{
  add(defs, "normal",       "hide if not (loop_carried and not control)");
  add(defs, "pending",      "hide if not (pending and loop_carried and not control)");
  add(defs, "accepted",     "hide if not (accepted and loop_carried and not control)");
  add(defs, "rejected",     "hide if not (rejected and loop_carried and not control)");

  add(defs, "all",          "normal if true;");
  add(defs, "all pending",  "hide if not pending");
  add(defs, "all accepted", "hide if not accepted");
  add(defs, "all rejected", "hide if not rejected");
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void add(FilterDefSet * defs, char * name, char * text)
{
  DepEditor * editor = (DepEditor *) defs->GetEditor();
  DepFilterDef * def;
  char * dummy;

  def = new DepFilterDef(CONTEXT_NULL, DB_NULLFP, editor);
  def->Open(CONTEXT_NULL, CONTEXT_NULL, CONTEXT_NULL, DB_NULLFP);

  def->SetName(name);
  (void) def->SetDefinition(text, true, true, dummy);

  defs->AddFilterDef(def);
}




static
Boolean lineRange(FortTextTree ftt, FortTreeNode node, char * range)
{
  Boolean inRange;
  int line, line1, line2, dummy;
  char * rest;

  if( node == nil )
    inRange = false;
  else
    { ftt_NodeToText(ftt, node, &line, &dummy, &dummy, &dummy);
      line1 = (int) strtol(range, &rest, 10);
      if( rest[0] == '-' )
        line2 = (int) strtol(rest+1, nil, 10);
      else
        line2 = line1;

      inRange = BOOL( line1 <= line+1 && line+1 <= line2 );
    }

  return inRange;
}




static
int findNB(char * s, char * target)
{
  char s_nb[200];
  char target_nb[200];
  int k, m;

  k = m = 0;
  while( s[k] != '\0' )
    { if( s[k] != ' ' )  s_nb[m++] = s[k];
      k++;
    }
  s_nb[m] = '\0';

  k = m = 0;
  while( target[k] != '\0' )
    { if( target[k] != ' ' )  target_nb[m++] = target[k];
      k++;
    }
  target_nb[m] = '\0';

  return find(s_nb, target_nb);
}
