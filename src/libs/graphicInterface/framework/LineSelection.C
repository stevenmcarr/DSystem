/* $Id: LineSelection.C,v 1.4 1997/03/11 14:32:45 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/LineSelection.C					*/
/*									*/
/*	LineSelection -- Abstract class for all persistent objects	*/
/*	Last edited: October 13, 1990 at 6:23 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/LineSelection.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* LineSelection object */

typedef struct LineSelection_Repr_struct
  {
    /* not used */

  } LineSelection_Repr;


#define R(ob)		(ob->LineSelection_repr)

#define INHERITED	Selection






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




void LineSelection::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(Selection);

  /* ... */
}




void LineSelection::FiniClass(void)
{
  /* ... */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(LineSelection)




LineSelection::LineSelection(void)
{
  this->line1 = UNUSED;
  this->char1 = UNUSED;
  this->line2 = UNUSED;
  this->char2 = UNUSED;
}




LineSelection::LineSelection(int line1, int char1, int line2, int char2)
{
  /* save creation parameters */
    this->line1 = line1;
    this->char1 = char1;
    this->line2 = line2;
    this->char2 = char2;
}







/***********************/
/* Access to selection */
/***********************/




Boolean LineSelection::Empty(void)
{
  return
    BOOL( (this->line1 == UNUSED) ||
          (this->line1 >  this->line2) ||
          (this->line1 == this->line2 && this->char1 > this->char2 )
        );
}







/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
