/* $Id: NavFilterDef.C,v 1.2 1997/03/11 14:30:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/nav/NavFilterDef.C					*/
/*									*/
/*	NavFilterDef -- View filter defs for Ded nav pane		*/
/*	Last edited: November 10, 1993 at 4:11 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/nav/NavFilterDef.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>
#include <libs/graphicInterface/cmdProcs/dEditor/src/SrcFilterDef.h>

#include <libs/graphicInterface/framework/FilterDefSet.h>
#include <libs/graphicInterface/framework/UserFilter.h>

#include <libs/frontEnd/ast/groups.h>
#include <libs/frontEnd/fortTree/FortTree.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* NavFilterDef object */

typedef struct NavFilterDef_Repr_struct
  {
    /* not used */

  } NavFilterDef_Repr;




#define R(ob)		(ob->NavFilterDef_repr)
#define INHERITED	SrcFilterDef






/*************************/
/*  Miscellaneous	 */
/*************************/




/* Ded nav-pane predicate opcodes */

#define OP_DUMMY         91






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




void NavFilterDef::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(SrcFilterDef);
}




void NavFilterDef::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(NavFilterDef)




NavFilterDef::NavFilterDef(Context context, DB_FP * session_fd)
            : SrcFilterDef(context, session_fd)
{
  /* allocate instance's private data */
    this->NavFilterDef_repr =
        (NavFilterDef_Repr *) get_mem(sizeof(NavFilterDef_Repr), "NavFilterDef instance");
}




NavFilterDef::~NavFilterDef()
{
  free_mem((void*) this->NavFilterDef_repr);
}






/**************************/
/* Filter def compilation */
/**************************/




Boolean NavFilterDef::function(char * name, int &numArgs, int &opcode)
{
  Boolean recognized = true;

  /* TEMPORARY */
  
  recognized = this->INHERITED::function(name, numArgs, opcode);

  return recognized;
}






/*****************************/
/* Filter def interpretation */
/*****************************/




Boolean NavFilterDef::execute(int opcode, int linenum, char * line,
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
      default:
        executed = this->INHERITED::execute(opcode, linenum, line,
                                            (void *) ftt, result);
        break;
    }

  return executed;
}






/*************/
/* Filtering */
/*************/




Boolean NavFilterDef::filterLine(UserFilter * uf,
                                 Boolean countOnly,
                                 int line,
                                 int &subline,
                                 TextString &text,
                                 TextData &data,
                                 void * environment)
{
  DedDocument * doc = (DedDocument *) environment;
  FortTextTree ftt;
  FortTree ft;
  FortTreeNode node;
  int bracket, k;
  Boolean subprog_header, loop_header, header, marked;

  /* determine whether requested line is a proc/loop header */
    doc->GetSource(ftt, ft);
    ftt_GetLineInfo(ftt, line, &node, &bracket);

    if( bracket == ftt_OPEN )
      { ft_AstSelect(ft);
        subprog_header = BOOL( is_subprogram_stmt(node) );
        loop_header    = BOOL( is_loop_stmt(node) );
      }
    else
      { subprog_header = false;
        loop_header    = false;
      }
      
    header = BOOL( subprog_header || loop_header );
    
  /* override filtering behavior to always conceal non-header lines */
    if( countOnly && ! header )
      { subline = 0;
        marked = false;
      }
    else
      marked = this->INHERITED::filterLine(uf, countOnly, line, subline, text, data, environment);

  /* override filtering behavior to always make subprogram header lines bold */
#if 0
    if( ! countOnly && subprog_header )
      for( k = 0;  k < text.num_tc;  k++ )
        text.tc_ptr[k].style |= STYLE_BOLD;
#endif

  return marked;
}






/*************************/
/*  Standard definitions */
/*************************/




void NavFilterDef::AddStandardDefs(FilterDefSet * defs)
{
  /* would like to just call 'FortFilterDef::AddStandardDefs',     */
  /* but we need NavFilterDef rather than FortFilterDef objects    */
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







/********************/
/* Filter instances */
/********************/




UserFilter * NavFilterDef::MakeFilter(void * environment)
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
void add(FilterDefSet * defs,
         char * name,
         char * text,
         Boolean concealed,
         Boolean errors)
{
  NavFilterDef * def;
  char * dummy;

  def = new NavFilterDef(CONTEXT_NULL, DB_NULLFP);
  def->Open(CONTEXT_NULL, CONTEXT_NULL, CONTEXT_NULL, DB_NULLFP);

  def->SetName(name);
  (void) def->SetDefinition(text, concealed, errors, dummy);

  defs->AddFilterDef(def);
}
