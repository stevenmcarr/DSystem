/* $Id: DistFilterDef.C,v 1.2 1997/03/11 14:30:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/dist/DistFilterDef.C					*/
/*									*/
/*	DistFilterDef -- View filter defs for Ded distribution pane	*/
/*	Last edited: October 17, 1993 at 10:55 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/dist/DistFilterDef.h>

#include <libs/graphicInterface/framework/CViewFilter.h>
#include <libs/graphicInterface/framework/UserFilter.h>
#include <libs/graphicInterface/framework/FilterDefSet.h>
#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>
#include <libs/graphicInterface/cmdProcs/dEditor/dist/DistEditor.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* DistFilterDef object */

typedef struct DistFilterDef_Repr_struct
  {
    /* not used */

  } DistFilterDef_Repr;


#define R(ob)		(ob->DistFilterDef_repr)

#define INHERITED	UserFilterDef






/*************************/
/*  Miscellaneous	 */
/*************************/




/* Ded variable-pane predicate opcodes */

#define OP_SHARED        81
#define OP_PRIVATE       82
#define OP_NAME          83
#define OP_DIM           84
#define OP_BLOCK         85
#define OP_INCOMMON      86
#define OP_DEFBEFORE     87
#define OP_USEAFTER      88
#define OP_USER          89
#define OP_REASON        90






/*************************/
/*  Forward declarations */
/*************************/




static void add(FilterDefSet * defs, char * name, char * text);






/************************************************************************/
/*	Interface Operations						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void DistFilterDef::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(UserFilterDef);
}




void DistFilterDef::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(DistFilterDef)




DistFilterDef::DistFilterDef(Context context,
                                 DB_FP * session_fd,
                                 DistEditor * editor)
               : UserFilterDef (context, session_fd)
{
  /* allocate instance's private data */
    this->DistFilterDef_repr =
        (DistFilterDef_Repr *) get_mem(sizeof(DistFilterDef_Repr),
                                         "DistFilterDef instance");
}




DistFilterDef::~DistFilterDef()
{
  free_mem((void*) this->DistFilterDef_repr);
}




void DistFilterDef_AddStandardDefs(FilterDefSet * defs)
{
  add(defs, "normal",                             "normal");
  add(defs, "shared",                             "show if shared");
  add(defs, "private",                            "show if private");
  add(defs, "only used in loop",                  "show if def_before and not used_after");
  add(defs, "def before, may modify",             "show if def_before and not used_after");
  add(defs, "def before, used after, may modify", "show if def_before and used_after");
  add(defs, "in common and used",                 "show if in_common");
  add(defs, "user classified shared",             "show if user_classified and shared");
  add(defs, "user classified private",            "show if user_classified and private");
}






/**************************/
/* Filter def compilation */
/**************************/




Boolean DistFilterDef::function(char * name, int &numArgs, int &opcode)
{
  Boolean recognized = true;

  if( strcmp("shared", name) == 0 )
    { numArgs = 0;
      opcode  = OP_SHARED;
    }
  else if( strcmp("private", name) == 0 )
    { numArgs = 0;
      opcode  = OP_PRIVATE;
    }
  else if( strcmp("name", name) == 0 )
    { numArgs = 1;
      opcode  = OP_NAME;
    }
  else if( strcmp("dim", name) == 0 )
    { numArgs = 1;
      opcode  = OP_DIM;
    }
  else if( strcmp("block", name) == 0 )
    { numArgs = 1;
      opcode  = OP_BLOCK;
    }
  else if( strcmp("in_common", name) == 0 )
    { numArgs = 0;
      opcode  = OP_INCOMMON;
    }
  else if( strcmp("def_before", name) == 0 )
    { numArgs = 0;
      opcode  = OP_DEFBEFORE;
    }
  else if( strcmp("used_after", name) == 0 )
    { numArgs = 0;
      opcode  = OP_USEAFTER;
    }
  else if( strcmp("user_classified", name) == 0 )
    { numArgs = 0;
      opcode  = OP_USER;
    }
  else if( strcmp("reason", name) == 0 )
    { numArgs = 1;
      opcode  = OP_REASON;
    }
  else
    recognized = this->INHERITED::function(name, numArgs, opcode);

  return recognized;
}






/*****************************/
/* Filter def interpretation */
/*****************************/




Boolean DistFilterDef::execute(int opcode, int linenum, char * line,
                              void * environment, int &result)
{
  DedDocument * pdoc = (DedDocument *) environment;
  PedVariable var;
  char * name;
  char * dim;
  char * block;
  char * defBefore;
  char * useAfter;
  char * kind;
  char * user;
  char * reason;
  char * arg;
  int len, dummy;
  Boolean executed = true;

  /* get the variable */
    /*** pdoc->GetVariable(linenum, var); ***/

  switch( opcode )
    {
      case OP_SHARED:
        result = (var.kind == pedVarShared);
        break;

      case OP_PRIVATE:
        result = (var.kind == pedVarPrivate);
        break;

      case OP_NAME:
        arg = (char *) this->operand();
        len = strlen(arg);
        if( strncmp(name, arg, len) == 0 )
          result = (name[len] == '\0');
        else
          result = false;
        break;

      case OP_DIM:
        arg = (char *) this->operand();
        len = strlen(arg);
        if( strncmp(dim, arg, len) == 0 )
          result = (dim[len] == '\0');
        else
          result = false;
        break;

      case OP_BLOCK:
        arg = (char *) this->operand();
        len = strlen(arg);
        if( strncmp(block, arg, len) == 0 )
          result = (block[len] == '\0');
        else
          result = false;
        break;

      case OP_INCOMMON:
        result = (strlen(block) != 0);
        break;

      case OP_DEFBEFORE:
        result = (var.defBefore != nil);
        break;

      case OP_USEAFTER:
        result = (var.useAfter != nil);
        break;

      case OP_USER:
        result = var.user;
        break;

      case OP_REASON:
        arg = (char *) this->operand();
        result = BOOL( find(reason, arg) != UNUSED );
        break;

      default:
        executed = this->INHERITED::execute(opcode, linenum, line, nil, result);
        break;
    }

  return executed;
}






/********************/
/* Filter instances */
/********************/




UserFilter * DistFilterDef::MakeFilter(void * environment)
{
  UserFilter * uf;

  uf = this->INHERITED::MakeFilter(environment);
  uf->SetElision(ViewFilter_ELISION_NONE);

  return uf;
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void add(FilterDefSet * defs, char * name, char * text)
{
  DistEditor * editor = (DistEditor *) defs->GetEditor();
  DistFilterDef * def;
  char * dummy;

  def = new DistFilterDef(CONTEXT_NULL, DB_NULLFP, editor);
  def->Open(CONTEXT_NULL, CONTEXT_NULL, CONTEXT_NULL, DB_NULLFP);

  def->SetName(name);
  (void) def->SetDefinition(text, true, true, dummy);

  defs->AddFilterDef(def);
}
