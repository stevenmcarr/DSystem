/* $Id: DedDependenceArrow.C,v 1.2 1997/03/11 14:30:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/src/DedDependenceArrow.C					*/
/*									*/
/*	DedDependenceArrow -- Ded custom-colored dependence arrow	*/
/*	Last edited: November 10, 1993 at 9:10 am			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/src/DedDependenceArrow.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* DedDependenceArrow object */

typedef struct DedDependenceArrow_Repr_struct
  {
    /* creation parameters */
      DedDocument *	doc;
      
    /* requested appearance */
      Dependence	dependence;

  } DedDependenceArrow_Repr;




#define R(ob)		(ob->DedDependenceArrow_repr)
#define INHERITED	DependenceArrow






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




void DedDependenceArrow::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(DependenceArrow);
    REQUIRE_INIT(DedDocument);
}




void DedDependenceArrow::FiniClass(void)
{
  /* nothing */
}






/**********************/
/*  Instance creation */
/**********************/




DedDependenceArrow * DedDependenceArrow::Create(Dependence dep, DedDocument * doc)
{
  DedDependenceArrow * d;
  
  d = new DedDependenceArrow;
  d->DedDependenceArrow::Init(dep, doc);
  d->DedDependenceArrow::PostInit();
  return d;
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(DedDependenceArrow)




DedDependenceArrow::DedDependenceArrow(void)
                  : DependenceArrow()
{
  /* allocate instance's private data */
    this->DedDependenceArrow_repr =
        (DedDependenceArrow_Repr *) get_mem(sizeof(DedDependenceArrow_Repr),
                                            "DedDependenceArrow instance");
}




void DedDependenceArrow::Init(Dependence dep, DedDocument * doc)
{
  ColorPair colors;
  Color c;

  /* save creation arguments */
    R(this)->dependence = dep;
    R(this)->doc        = doc;

  /* calculate appropriate colors */
    c = doc->GetDependenceColor(dep.edge);
    colors.foreground = white_color;
    colors.background = (c != NULL_COLOR ? c : Ded_PurpleColor);

  this->INHERITED::Init(dep, colors);
}




void DedDependenceArrow::Destroy(void)
{
  this->INHERITED::Destroy();
}




DedDependenceArrow::~DedDependenceArrow()
{
  free_mem((void*) this->DedDependenceArrow_repr);
}






/**************/
/* Appearance */
/**************/




void DedDependenceArrow::getShaft(int &width, LineStyle * &style, Color &color)
{
  this->INHERITED::getShaft(width, style, color);

#if 0
  if( ! R(this)->doc->IsDependenceCrossProcessor(R(this)->dependence.edge) )
    style = line_style_dashed;
#endif
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
