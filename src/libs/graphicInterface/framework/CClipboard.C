/* $Id: CClipboard.C,v 1.5 1997/03/11 14:32:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/CClipboard.C						*/
/*									*/
/*	CClipboard -- Class of the unique global clipboard		*/
/*	Last edited: October 13, 1993 at 12:03 am			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/CClipboard.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* CClipboard object */

typedef struct CClipboard_Repr_struct
  {
    /* creation parameters */
      /* ... */

  } CClipboard_Repr;


#define R(ob)		(ob->CClipboard_repr)

#define INHERITED	Object






/*************************/
/*  Miscellaneous	 */
/*************************/




/* global clipboard */

CClipboard * theClipboard;






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




void CClipboard::InitClass(void)
{
  /* initialize needed submodules */
    /* none */

  /* create the global clipboard */
    theClipboard = new CClipboard();
}




void CClipboard::FiniClass(void)
{
  /* destroy the global clipboard */
    delete theClipboard;
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(CClipboard)




CClipboard::CClipboard(void)
{
  /* allocate instance's private data */
    this->CClipboard_repr = (CClipboard_Repr *) get_mem(sizeof(CClipboard_Repr),
                                                        "CClipboard instance");

  /* initialize to empty scrap */
    /* ... */
}




CClipboard::~CClipboard()
{
  free_mem((void*) this->CClipboard_repr);
}






/********************/
/*  Access to scrap */
/********************/




Scrap * CClipboard::GetScrap(void)
{
  /* ... */
  return nil;
}




void CClipboard::SetScrap(Scrap * scrap)
{
  /* ... */
}







/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
