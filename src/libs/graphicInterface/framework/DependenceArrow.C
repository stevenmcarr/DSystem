/* $Id: DependenceArrow.C,v 1.2 1997/03/11 14:32:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/DependenceArrow.C					*/
/*									*/
/*	DependenceArrow -- Arrow in a FortView depicting a dependence	*/
/*	Last edited: October 14, 1993 at 6:47 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/DependenceArrow.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* DependenceArrow object */

typedef struct DependenceArrow_Repr_struct
  {
    /* requested appearance */
      Dependence	dependence;

  } DependenceArrow_Repr;




#define R(ob)		(ob->DependenceArrow_repr)
#define INHERITED	FortArrow






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




void DependenceArrow::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(FortArrow);
}




void DependenceArrow::FiniClass(void)
{
  /* nothing */
}






/**********************/
/*  Instance creation */
/**********************/




DependenceArrow * DependenceArrow::Create(Dependence dep, ColorPair colors)
{
  DependenceArrow * d;
  
  d = new DependenceArrow;
  d->DependenceArrow::Init(dep, colors);
  d->DependenceArrow::PostInit();
  return d;
}




/****************************/
/*  Instance initialization */
/****************************/




META_IMP(DependenceArrow)




DependenceArrow::DependenceArrow(void)
               : FortArrow()
{
  /* allocate instance's private data */
    this->DependenceArrow_repr =
        (DependenceArrow_Repr *) get_mem(sizeof(DependenceArrow_Repr), "DependenceArrow instance");
}




void DependenceArrow::Init(Dependence dep, ColorPair colors)
{
  this->INHERITED::Init(dep.src, dep.sink, colors);

  /* save creation arguments */
    R(this)->dependence = dep;
}




void DependenceArrow::Destroy(void)
{
  this->INHERITED::Destroy();
}




DependenceArrow::~DependenceArrow()
{
  free_mem((void*) this->DependenceArrow_repr);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
