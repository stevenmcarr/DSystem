/* $Id: FortFilterDef.C,v 1.8 1997/03/11 14:32:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/FortFilterDef.C					*/
/*									*/
/*	FortFilterDef -- View filter defs for Fortran source code	*/
/*	Last edited: October 13, 1993 at 6:08 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/FortFilterDef.h>

#include <libs/graphicInterface/framework/FilterDefSet.h>

#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/textTree/TextTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* FortFilterDef object */

typedef struct FortFilterDef_Repr_struct
  {
    /* not used */

  } FortFilterDef_Repr;


#define R(ob)		(ob->FortFilterDef_repr)

#define INHERITED	UserFilterDef






/*************************/
/*  Miscellaneous	 */
/*************************/




/* Fortran predicate opcodes */

#define OP_IS_DECL        51
#define OP_IS_EXEC        52
#define OP_IS_HEADING     53
#define OP_IS_INSUBPROG	  54
#define OP_IS_COMMENT     55






/*************************/
/*  Forward declarations */
/*************************/




static Boolean isDeclaration(FortTextTree ftt, FortTreeNode node, int bracket);

static Boolean isExecutable(FortTextTree ftt, FortTreeNode node, int bracket);

static Boolean isHeading(FortTextTree ftt, FortTreeNode node, int bracket);

static Boolean isInSubprogram(FortTextTree ftt,
                              char * subprogName,
                              FortTreeNode node,
                              int bracket);

static Boolean isComment(FortTextTree ftt, FortTreeNode node, int bracket);

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




void FortFilterDef::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(UserFilterDef);
}




void FortFilterDef::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/



META_IMP(FortFilterDef)




FortFilterDef::FortFilterDef(Context context, DB_FP * session_fp)
   : UserFilterDef (context, session_fp)
{
  /* allocate instance's private data */
    this->FortFilterDef_repr = (FortFilterDef_Repr *)
                               get_mem(sizeof(FortFilterDef_Repr),
                                       "FortFilterDef instance");
}




FortFilterDef::~FortFilterDef()
{
  free_mem((void*) this->FortFilterDef_repr);
}






/**************************/
/* Filter def compilation */
/**************************/




Boolean FortFilterDef::function(char * name, int &numArgs, int &opcode)
{
  Boolean recognized = true;

  if( strcmp("decl", name) == 0 )
    { numArgs = 0;
      opcode  = OP_IS_DECL;
    }
  else if( strcmp("exec", name) == 0 )
    { numArgs = 0;
      opcode  = OP_IS_EXEC;
    }
  else if( strcmp("heading", name) == 0 )
    { numArgs = 0;
      opcode  = OP_IS_HEADING;
    }
  else if( strcmp("subprog", name) == 0 )
    { numArgs = 1;
      opcode  = OP_IS_INSUBPROG;
    }
  else if( strcmp("comment", name) == 0 )
    { numArgs = 0;
      opcode  = OP_IS_COMMENT;
    }
  else
    recognized = false;

  return recognized;
}






/*****************************/
/* Filter def interpretation */
/*****************************/




Boolean FortFilterDef::execute(int opcode, int linenum, char * line,
                               void * environment, int &result)
{
  FortTextTree ftt = (FortTextTree) environment;
  FortTreeNode node;
  int bracket;
  char * s;
  Boolean executed = true;

  ftt_GetLineInfo(ftt, linenum, &node, &bracket);

  switch( opcode )
    {
      case OP_IS_DECL:
        result = isDeclaration(ftt, node, bracket);
        break;

      case OP_IS_EXEC:
        result = isExecutable(ftt, node, bracket);
        break;

      case OP_IS_HEADING:
        result = isHeading(ftt, node, bracket);
        break;

      case OP_IS_INSUBPROG:
        s = (char *) this->operand();
        result = isInSubprogram(ftt, s, node, bracket);
        sfree(s);
        break;

      case OP_IS_COMMENT:
        result = isComment(ftt, node, bracket);
        break;

      default:
        executed = this->INHERITED::execute(opcode, linenum, line,
                                            environment, result);
        break;
    }

  return executed;
}




Boolean FortFilterDef::getConcealed(int linenum, void * environment)
{
  FortTextTree ftt = (FortTextTree) environment;
  Boolean concealed;

  ftt_GetConceal(ftt, linenum, &concealed);
  return concealed;
}




Boolean FortFilterDef::getErroneous(int linenum, void * environment)
{
  FortTextTree ftt = (FortTextTree) environment;

  return ftt_IsErroneous(ftt, linenum);
}




char * FortFilterDef::getErrorMessage(int linenum, void * environment)
{
  FortTextTree ftt = (FortTextTree) environment;
  static char message[256];

  ftt_GetErrorMessage(ftt, linenum, message);

  return message;
}






/*************************/
/*  Standard definitions */
/*************************/




void FortFilterDef::AddStandardDefs(FilterDefSet * defs)
{
  add(defs, "normal",        "bold if erroneous",       true,  false);
  add(defs, "all",           "dim if concealed",        false, true);
  add(defs, "errors only",   "hide if not erroneous",   true,  true);
  add(defs, "headings only", "hide if not heading",     true,  false);
  add(defs, "no comments",   "hide if comment",         true,  false);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
Boolean isDeclaration(FortTextTree ftt, FortTreeNode node, int bracket)
{
  Boolean is;

  ft_AstSelect(ftt_Tree(ftt));

  if( bracket == ftt_SIMPLE || bracket == ftt_OPEN )
    is = BOOL( is_scope(node)
            || is_common(node)
            || is_dimension(node)
            || is_equivalence(node)
            || is_type_statement(node)
            || is_external(node)
            || is_implicit(node)
            || is_intrinsic(node)
            || is_parameter(node)
            || is_save(node)
            || is_entry(node)
            || is_data(node)
            || is_stmt_function(node) );
  else
    is = false;

  return is;
}




static
Boolean isExecutable(FortTextTree ftt, FortTreeNode node, int bracket)
{
  return NOT( isDeclaration(ftt, node, bracket) );
}




static
Boolean isHeading(FortTextTree ftt, FortTreeNode node, int bracket)
{
  Boolean is;

  ft_AstSelect(ftt_Tree(ftt));

  if( bracket == ftt_OPEN )
    is = BOOL( is_scope(node) );
  else
    is = false;

  return is;
}




/*ARGSUSED*/

static
Boolean isInSubprogram(FortTextTree ftt,
                       char * subprogName,
                       FortTreeNode node,
                       int bracket)
{
  FortTreeNode subprogNode, nameNode;
  char * name;

  ft_AstSelect(ftt_Tree(ftt));

  subprogNode = find_scope(node);
  nameNode    = get_name_in_entry(subprogNode);
  name        = gen_get_text(nameNode);

  return BOOL( strcmp(subprogName, name) == 0 );
}




/*ARGSUSED*/

static
Boolean isComment(FortTextTree ftt, FortTreeNode node, int bracket)
{
  ft_AstSelect(ftt_Tree(ftt));

  return BOOL( is_comment(node) );
}




static
void add(FilterDefSet * defs,
         char * name,
         char * text,
         Boolean concealed,
         Boolean errors)
{
  FortFilterDef * def;
  char * dummy;

  def = new FortFilterDef(CONTEXT_NULL, DB_NULLFP);
  def->Open(CONTEXT_NULL, CONTEXT_NULL, CONTEXT_NULL, DB_NULLFP);

  def->SetName(name);
  (void) def->SetDefinition(text, concealed, errors, dummy);

  defs->AddFilterDef(def);
}
