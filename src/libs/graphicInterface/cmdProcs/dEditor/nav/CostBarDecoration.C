/* $Id: CostBarDecoration.C,v 1.2 1997/03/11 14:30:18 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/nav/CostBarDecoration.C				*/
/*									*/
/*	CostBarDecoration -- Barchart of comm/comp costs in left margin	*/
/*	Last edited: October 14, 1993 at 1:20 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/nav/CostBarDecoration.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* CostBarDecoration object */

typedef struct CostBarDecoration_Repr_struct
  {
    /* creation parameters */
      DedDocument *	doc;

  } CostBarDecoration_Repr;




#define R(ob)		(ob->CostBarDecoration_repr)
#define INHERITED	MarginDecoration






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




void CostBarDecoration::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(MarginDecoration);
    REQUIRE_INIT(DedDocument);
}




void CostBarDecoration::FiniClass(void)
{
  /* nothing */
}






/**********************/
/*  Instance creation */
/**********************/




CostBarDecoration * CostBarDecoration::Create(DedDocument * doc)
{
  CostBarDecoration * d;
  
  d = new CostBarDecoration;
  d->CostBarDecoration::Init(doc);
  d->CostBarDecoration::PostInit();
  return d;
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(CostBarDecoration)




CostBarDecoration::CostBarDecoration(void)
          : MarginDecoration()
{
  /* allocate instance's private data */
    this->CostBarDecoration_repr =
        (CostBarDecoration_Repr *) get_mem(sizeof(CostBarDecoration_Repr), "CostBarDecoration instance");
}




void CostBarDecoration::Init(DedDocument * doc)
{
  this->INHERITED::Init();  

  /* save creation parameters */
    R(this)->doc = doc;
}




void CostBarDecoration::Destroy(void)
{
  this->INHERITED::Destroy();
}




CostBarDecoration::~CostBarDecoration()
{
  free_mem((void*) this->CostBarDecoration_repr);
}






/********/
/* Text */
/********/



      
int CostBarDecoration::Width(void)
{
  /* ... */
  return 0;
}



      
void CostBarDecoration::GetMarginText(int c_linenum,
                                      TextChar * textstring_tc_ptr,
                                      ColorPair * textdata_chars)
{
  /* ... */
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
