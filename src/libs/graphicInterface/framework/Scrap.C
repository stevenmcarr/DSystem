/* $Id: Scrap.C,v 1.5 1997/03/11 14:32:51 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/Scrap.C						*/
/*									*/
/*	Scrap -- Abstract class for all edit scraps			*/
/*	Last edited: October 13, 1993 at 10:13 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/Scrap.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* Scrap object */

typedef struct Scrap_Repr_struct
  {
    /* creation parameters */
      /* ... */

  } Scrap_Repr;


#define R(ob)		(ob->Scrap_repr)

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




void Scrap::InitClass(void)
{
  /* initialize needed submodules */
    /* ... */
}




void Scrap::FiniClass(void)
{
  /* ... */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(Scrap)




Scrap::Scrap(void)
{
  /* allocate instance's private data */
    this->Scrap_repr = (Scrap_Repr *) get_mem(sizeof(Scrap_Repr),
                                              "Scrap instance");

  /* save creation parameters */
    /* ... */
}




Scrap::~Scrap()
{
  free_mem((void*) this->Scrap_repr);
}






/*************/
/*  ...      */
/*************/




/* ... */







/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
