/* $Id: MarginDecoration.C,v 1.2 1997/03/11 14:32:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/MarginDecoration.C					*/
/*									*/
/*	MarginDecoration -- Addition to a CTextView's left margin	*/
/*	Last edited: October 13, 1993 at 10:13 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/MarginDecoration.h>

#include <libs/graphicInterface/framework/CTextView.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* MarginDecoration object */

typedef struct MarginDecoration_Repr_struct
  {
    /* layout */
      int		start;

  } MarginDecoration_Repr;


#define R(ob)		(ob->MarginDecoration_repr)

#define INHERITED	Decoration






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




void MarginDecoration::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(Decoration);
    REQUIRE_INIT(CTextView);
}




void MarginDecoration::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(MarginDecoration)




MarginDecoration::MarginDecoration(void)
          : Decoration()
{
  /* allocate instance's private data */
    this->MarginDecoration_repr = (MarginDecoration_Repr *) get_mem(sizeof(MarginDecoration_Repr), "MarginDecoration instance");
}




MarginDecoration::~MarginDecoration()
{
  free_mem((void*) this->MarginDecoration_repr);
}






/********/
/* Text */
/********/



      
int MarginDecoration::Width(void)
{
  return 0;
}



      
void MarginDecoration::SetStart(int start)
{
  R(this)->start = start;
}



      
int MarginDecoration::Start(void)
{
  return R(this)->start;
}



      
void MarginDecoration::GetMarginText(int c_linenum,
                                     TextChar * textstring_tc_ptr,
                                     ColorPair * textdata_chars)
{
  /* nothing */
}






/*********/
/* Input */
/********/



      
void MarginDecoration::Clicked(int c_linenum, int char1)
{
  /* nothing */
}







/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
