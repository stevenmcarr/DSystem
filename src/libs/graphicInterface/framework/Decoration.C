/* $Id: Decoration.C,v 1.2 1997/03/11 14:32:36 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/Decoration.C						*/
/*									*/
/*	Decoration -- Addition to a CTextView display			*/
/*	Last edited: October 13, 1993 at 5:43 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/Decoration.h>

#include <libs/graphicInterface/framework/CTextView.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* Decoration object */

typedef struct Decoration_Repr_struct
  {
    /* viewing state */
    CTextView *		view;

  } Decoration_Repr;


#define R(ob)		(ob->Decoration_repr)

#define INHERITED	Object






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




void Decoration::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(Object);
    REQUIRE_INIT(CTextView);
}




void Decoration::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(Decoration)




Decoration::Decoration(void)
          : Object()
{
  /* allocate instance's private data */
    this->Decoration_repr = (Decoration_Repr *) get_mem(sizeof(Decoration_Repr), "Decoration instance");
}




void Decoration::Init(void)
{
  this->INHERITED::Init();
  
  /* initialize viewing state */
    R(this)->view = nil;
}




void Decoration::Destroy(void)
{
  this->INHERITED::Destroy();
}




Decoration::~Decoration()
{
  free_mem((void*) this->Decoration_repr);
}






/********************/
/*  View attachment */
/********************/




void Decoration::AttachView(CTextView * view)
{
  R(this)->view = view;
}




CTextView * Decoration::GetView(void)
{
  return R(this)->view;
}




LineEditor * Decoration::GetEditor(void)
{
  if( R(this)->view != nil )
    return (LineEditor *) R(this)->view->GetEditor();
  else
    return nil;
}






/************/
/*  Drawing */
/************/




void Decoration::ColorizeLine(int c_linenum,
                              int marginWidth,
                              TextString &text,
                              TextData &data)
{
  /* nothing */
}




void Decoration::Draw(void)
{
  /* nothing */
}




Rectangle Decoration::BBox(void)
{
  return EmptyRect;
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
