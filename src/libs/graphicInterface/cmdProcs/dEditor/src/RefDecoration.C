/* $Id: RefDecoration.C,v 1.2 1997/03/11 14:30:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/src/RefDecoration.C					*/
/*									*/
/*	RefDecoration -- Reference-cost coloring for Ded src pane	*/
/*	Last edited: November 6, 1993 at 1:40 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/src/RefDecoration.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>

#include <libs/frontEnd/ast/groups.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* RefDecoration object */

typedef struct RefDecoration_Repr_struct
  {
    /* creation parameters */
      DedDocument *	doc;

  } RefDecoration_Repr;




#define R(ob)		(ob->RefDecoration_repr)
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




void RefDecoration::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(NodeDecoration);
    REQUIRE_INIT(DedDocument);
}




void RefDecoration::FiniClass(void)
{
  /* nothing */
}






/**********************/
/*  Instance creation */
/**********************/




RefDecoration * RefDecoration::Create(DedDocument * doc)
{
  RefDecoration * d;
  
  d = new RefDecoration;
  d->RefDecoration::Init(doc);
  d->RefDecoration::PostInit();
  return d;
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(RefDecoration)




RefDecoration::RefDecoration(void)
              : NodeDecoration()
{
  /* allocate instance's private data */
    this->RefDecoration_repr =
        (RefDecoration_Repr *) get_mem(sizeof(RefDecoration_Repr),
                                       "RefDecoration instance");
}




void RefDecoration::Init(DedDocument * doc)
{
  FortTextTree ftt;
  FortTree dummy;
  
  doc->GetSource(ftt, dummy);
  this->INHERITED::Init(ftt, true);  

  /* save creation parameters */
    R(this)->doc = doc;
}




void RefDecoration::Destroy(void)
{
  this->INHERITED::Destroy();
}




RefDecoration::~RefDecoration()
{
  free_mem((void*) this->RefDecoration_repr);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/*******************/
/* Node processing */
/*******************/




Boolean RefDecoration::isDecoratedNode(FortTreeNode node)
{
  return BOOL( is_subscript(node) );
}




void RefDecoration::getNodeDecoration(FortTreeNode node, ColorPair &colors)
{
  colors.foreground = R(this)->doc->GetRefColor(node);
}
