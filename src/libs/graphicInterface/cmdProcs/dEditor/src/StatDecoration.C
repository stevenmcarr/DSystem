/* $Id: StatDecoration.C,v 1.2 1997/03/11 14:30:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/src/StatDecoration.C					*/
/*									*/
/*	StatDecoration -- Statement-cost coloring for Ded src pane	*/
/*	Last edited: November 6, 1993 at 1:40 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/src/StatDecoration.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>

#include <libs/frontEnd/ast/groups.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* StatDecoration object */

typedef struct StatDecoration_Repr_struct
  {
    /* creation parameters */
      DedDocument *	doc;

  } StatDecoration_Repr;




#define R(ob)		(ob->StatDecoration_repr)
#define INHERITED	NodeDecoration






/*************************/
/*  Forward declarations */
/*************************/




/* none */






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void StatDecoration::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(NodeDecoration);
    REQUIRE_INIT(DedDocument);
}




void StatDecoration::FiniClass(void)
{
  /* nothing */
}






/**********************/
/*  Instance creation */
/**********************/




StatDecoration * StatDecoration::Create(DedDocument * doc)
{
  StatDecoration * d;
  
  d = new StatDecoration;
  d->StatDecoration::Init(doc);
  d->StatDecoration::PostInit();
  return d;
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(StatDecoration)




StatDecoration::StatDecoration(void)
               : NodeDecoration()
{
  /* allocate instance's private data */
    this->StatDecoration_repr =
        (StatDecoration_Repr *) get_mem(sizeof(StatDecoration_Repr),
                                         "StatDecoration instance");
}




void StatDecoration::Init(DedDocument * doc)
{
  FortTextTree ftt;
  FortTree dummy;
  
  doc->GetSource(ftt, dummy);
  this->INHERITED::Init(ftt, true);  

  /* save creation parameters */
    R(this)->doc = doc;
}




void StatDecoration::Destroy(void)
{
  this->INHERITED::Destroy();
}




StatDecoration::~StatDecoration()
{
  free_mem((void*) this->StatDecoration_repr);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/*******************/
/* Node processing */
/*******************/




Boolean StatDecoration::isDecoratedNode(FortTreeNode node)
{
  return BOOL( is_executable_stmt(node) );
}




void StatDecoration::getNodeDecoration(FortTreeNode node, ColorPair &colors)
{
  colors.foreground = ( is_loop_stmt(node) ? R(this)->doc->GetLoopColor(node)
                                           : R(this)->doc->GetStatementColor(node) );
}
