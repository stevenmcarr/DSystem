/* $Id: SrcFilterDef.C,v 1.2 1997/03/11 14:30:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/src/SrcFilterDef.C					*/
/*									*/
/*	SrcFilterDef -- View filter defs for Ded source pane		*/
/*	Last edited: October 14, 1993 at 4:40 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/src/SrcFilterDef.h>

#include <libs/graphicInterface/framework/FilterDefSet.h>
#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>

#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* SrcFilterDef object */

typedef struct SrcFilterDef_Repr_struct
  {
    /* not used */

  } SrcFilterDef_Repr;




#define R(ob)		(ob->SrcFilterDef_repr)
#define INHERITED	FortFilterDef






/*************************/
/*  Miscellaneous	 */
/*************************/




/* Ded source-pane predicate opcodes */

#define OP_SOURCE        61
#define OP_SINK          62
#define OP_PARALLEL      63
#define OP_CURLOOP       64






/*************************/
/*  Forward declarations */
/*************************/




static void add(FilterDefSet * defs,
                char * name,
                char * text,
                Boolean concealed,
                Boolean errors);






/************************************************************************/
/*	Interface Operations						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void SrcFilterDef::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(FortFilterDef);
    REQUIRE_INIT(DedDocument);
}




void SrcFilterDef::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(SrcFilterDef)




SrcFilterDef::SrcFilterDef(Context context, DB_FP * session_fd)
             : FortFilterDef (context, session_fd)
{
  /* allocate instance's private data */
    this->SrcFilterDef_repr =
        (SrcFilterDef_Repr *) get_mem(sizeof(SrcFilterDef_Repr),
                                      "SrcFilterDef instance");
}




SrcFilterDef::~SrcFilterDef()
{
  free_mem((void*) this->SrcFilterDef_repr);
}






/**************************/
/* Filter def compilation */
/**************************/




Boolean SrcFilterDef::function(char * name, int &numArgs, int &opcode)
{
  Boolean recognized = true;

  if( strcmp("source", name) == 0 )
    { numArgs = 1;
      opcode  = OP_SOURCE;
    }
  else if( strcmp("sink", name) == 0 )
    { numArgs = 1;
      opcode  = OP_SINK;
    }
  else if( strcmp("parallel", name) == 0 )
    { numArgs = 0;
      opcode  = OP_PARALLEL;
    }
  else if( strcmp("current_loop", name) == 0 )
    { numArgs = 0;
      opcode  = OP_CURLOOP;
    }
  else
    recognized = this->INHERITED::function(name, numArgs, opcode);

  return recognized;
}






/*****************************/
/* Filter def interpretation */
/*****************************/




Boolean SrcFilterDef::execute(int opcode, int linenum, char * line,
                              void * environment, int &result)
{
  DedDocument * doc = (DedDocument *) environment;
  FortTextTree ftt;
  FortTree ft;
  int dummy;
  FortTreeNode loop, node;
  int bracket;
  Boolean executed = true;

  doc->GetSource(ftt, ft);
  ft_AstSelect(ft);

  switch( opcode )
    {
      case OP_SOURCE:
        dummy = this->operand();
        result = false;
        break;

      case OP_SINK:
        dummy = this->operand();
        result = false;
        break;

      case OP_PARALLEL:
        result = false;
        break;

      case OP_CURLOOP:
        loop = doc->GetCurrentLoop();
        if( loop == AST_NIL )
          result = false;
        else
          { ftt_GetLineInfo(ftt, linenum, &node, &bracket);
            while( node != loop && node != nil )
              node = tree_out(node);
            result = BOOL( node == loop );
          }
        break;

      default:
        executed = this->INHERITED::execute(opcode, linenum, line,
                                            (void *) ftt, result);
        break;
    }

  return executed;
}




Boolean SrcFilterDef::getConcealed(int linenum, void * environment)
{
  DedDocument * doc = (DedDocument *) environment;
  FortTextTree ftt;
  FortTree ft;

  doc->GetSource(ftt, ft);
  return this->INHERITED::getConcealed(linenum, (void *) ftt);
}




Boolean SrcFilterDef::getErroneous(int linenum, void * environment)
{
  DedDocument * doc = (DedDocument *) environment;
  FortTextTree ftt;
  FortTree ft;

  doc->GetSource(ftt, ft);
  return this->INHERITED::getErroneous(linenum, (void *) ftt);
}




char * SrcFilterDef::getErrorMessage(int linenum, void * environment)
{
  DedDocument * doc = (DedDocument *) environment;
  FortTextTree ftt;
  FortTree ft;

  doc->GetSource(ftt, ft);
  return this->INHERITED::getErrorMessage(linenum, (void *) ftt);
}






/*************************/
/*  Standard definitions */
/*************************/




void SrcFilterDef::AddStandardDefs(FilterDefSet * defs)
{
  /* would like to just call 'FortFilterDef::AddStandardDefs',     */
  /* but we need SrcFilterDef rather than FortFilterDef objects    */
  /* because the 'environment' argument is interpreted differently */
    add(defs, "normal",        "bold if erroneous",       true,  false);
    add(defs, "all",           "dim if concealed",        false, true);
    add(defs, "errors only",   "hide if not erroneous",   true,  true);
    add(defs, "headings only", "hide if not heading",     true,  false);
    add(defs, "no comments",   "hide if comment",         true,  false);

  /* source-specific filter defs */
    add(defs, "sequential",    "dim if parallel",         true,  false);
    add(defs, "current loop",  "dim if not current_loop", true,  false);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void add(FilterDefSet * defs,
         char * name,
         char * text,
         Boolean concealed,
         Boolean errors)
{
  SrcFilterDef * def;
  char * dummy;

  def = new SrcFilterDef(CONTEXT_NULL, DB_NULLFP);
  def->Open(CONTEXT_NULL, CONTEXT_NULL, CONTEXT_NULL, DB_NULLFP);

  def->SetName(name);
  (void) def->SetDefinition(text, concealed, errors, dummy);

  defs->AddFilterDef(def);
}
