/* $Id: LoopDecoration.C,v 1.2 1997/03/11 14:30:19 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/src/LoopDecoration.C					*/
/*									*/
/*	LoopDecoration -- Loop-cost coloring for Ded nav pane		*/
/*	Last edited: November 6, 1993 at 1:40 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/nav/LoopDecoration.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>

#include <libs/frontEnd/ast/groups.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* LoopDecoration object */

typedef struct LoopDecoration_Repr_struct
  {
    /* creation parameters */
      DedDocument *	doc;

  } LoopDecoration_Repr;




#define R(ob)		(ob->LoopDecoration_repr)
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




void LoopDecoration::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(NodeDecoration);
    REQUIRE_INIT(DedDocument);
}




void LoopDecoration::FiniClass(void)
{
  /* nothing */
}






/**********************/
/*  Instance creation */
/**********************/




LoopDecoration * LoopDecoration::Create(DedDocument * doc)
{
  LoopDecoration * d;
  
  d = new LoopDecoration;
  d->LoopDecoration::Init(doc);
  d->LoopDecoration::PostInit();
  return d;
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(LoopDecoration)




LoopDecoration::LoopDecoration(void)
              : NodeDecoration()
{
  /* allocate instance's private data */
    this->LoopDecoration_repr =
        (LoopDecoration_Repr *) get_mem(sizeof(LoopDecoration_Repr),
                                        "LoopDecoration instance");
}




void LoopDecoration::Init(DedDocument * doc)
{
  FortTextTree ftt;
  FortTree dummy;
  
  doc->GetSource(ftt, dummy);
  this->INHERITED::Init(ftt, false);  

  /* save creation parameters */
    R(this)->doc = doc;
}




void LoopDecoration::Destroy(void)
{
  this->INHERITED::Destroy();
}




LoopDecoration::~LoopDecoration()
{
  free_mem((void*) this->LoopDecoration_repr);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/*******************/
/* Node processing */
/*******************/




Boolean LoopDecoration::isDecoratedNode(FortTreeNode node)
{
  return is_loop_stmt(node);
}




void LoopDecoration::getNodeDecoration(FortTreeNode node, ColorPair &colors)
{
  colors.foreground = R(this)->doc->GetLoopColor(node);
}
