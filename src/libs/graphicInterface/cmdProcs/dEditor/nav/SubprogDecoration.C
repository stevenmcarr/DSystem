/* $Id: SubprogDecoration.C,v 1.2 1997/03/11 14:30:22 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/nav/SubprogDecoration.C					*/
/*									*/
/*	SubprogDecoration -- Containing subprog name in left margin	*/
/*	Last edited: October 14, 1993 at 3:43 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/nav/SubprogDecoration.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* SubprogDecoration object */

typedef struct SubprogDecoration_Repr_struct
  {
    /* creation parameters */
      DedDocument *	doc;

  } SubprogDecoration_Repr;




#define R(ob)		(ob->SubprogDecoration_repr)
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




void SubprogDecoration::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(MarginDecoration);
    REQUIRE_INIT(DedDocument);
}




void SubprogDecoration::FiniClass(void)
{
  /* nothing */
}






/**********************/
/*  Instance creation */
/**********************/




SubprogDecoration * SubprogDecoration::Create(DedDocument * doc)
{
  SubprogDecoration * d;
  
  d = new SubprogDecoration;
  d->SubprogDecoration::Init(doc);
  d->SubprogDecoration::PostInit();
  return d;
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(SubprogDecoration)




SubprogDecoration::SubprogDecoration(void)
          : MarginDecoration()
{
  /* allocate instance's private data */
    this->SubprogDecoration_repr =
        (SubprogDecoration_Repr *) get_mem(sizeof(SubprogDecoration_Repr),
                                           "SubprogDecoration instance");
}




void SubprogDecoration::Init(DedDocument * doc)
{
  this->INHERITED::Init();  

  /* save creation parameters */
    R(this)->doc = doc;
}




void SubprogDecoration::Destroy(void)
{
  this->INHERITED::Destroy();
}




SubprogDecoration::~SubprogDecoration()
{
  free_mem((void*) this->SubprogDecoration_repr);
}






/********/
/* Text */
/********/



      
int SubprogDecoration::Width(void)
{
  /* ... */
  return 0;
}



      
void SubprogDecoration::GetMarginText(int c_linenum,
                                     TextChar * textstring_tc_ptr,
                                     ColorPair * textdata_chars)
{
  /* ... */
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
