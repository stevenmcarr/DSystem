/* $Id: Selection.C,v 1.4 1997/03/11 14:32:52 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/Selection.C						*/
/*									*/
/*	Selection -- Abstract class for all selection specifiers	*/
/*	Last edited: October 13, 1993 at 10:40 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/Selection.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* Selection object */

typedef struct Selection_Repr_struct
  {
    /* not used */

  } Selection_Repr;


#define R(ob)		(ob->Selection_repr)

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




void Selection::InitClass(void)
{
  /* initialize needed submodules */
    /* ... */
}




void Selection::FiniClass(void)
{
  /* ... */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(Selection)




/* none */






/*****************************/
/*  Access to selection      */
/*****************************/




Boolean Selection::Empty(void)
{
  SUBCLASS_RESPONSIBILITY("Selection::Empty");
  return true;
}








/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
