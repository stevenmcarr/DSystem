/* $Id: NedScrap.C,v 1.1 1997/06/25 13:51:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/Scrap.c							*/
/*									*/
/*	Scrap -- global data fragment for inter-cp cut & paste		*/
/*	Last edited: June 12, 1990 at 11:35 am				*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>
#include <libs/graphicInterface/cmdProcs/newEditor/NedScrap.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Static data		*/
/************************/



/* registered scrap types */

int		sc_numRegistered;
char *		sc_name[20];
SC_Methods	sc_methods[20];





/* current scrap value */

SC_Type sc_type;
Generic sc_owner;
Generic sc_scrap;






/************************/
/* Forward declarations	*/
/************************/




STATIC(void,		destroy,(void));






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




void sc_Init()
{
  sc_numRegistered = 0;
  sc_type = 0;
}




void sc_Fini()
{
  /* nothing */
}




SC_Type sc_Register(char *name, SC_Methods *methods)
{
  sc_numRegistered += 1;
  sc_name[sc_numRegistered]    = name;
  sc_methods[sc_numRegistered] = *methods;

  return sc_numRegistered;
}




/*ARGSUSED*/
void sc_Unregister(Generic type)
{
  /* nothing for now */
}






/************************/
/*  Scrap contents	*/
/************************/




void sc_Set(SC_Type type, Generic owner, Generic scrap)
{
  destroy();

  sc_type  = type;
  sc_owner = owner;
  sc_scrap = scrap;
}




int sc_NumTypes()
{
  /*** DUMMY ***/
  return (sc_type = 0  ?  0  :  1);
}




/*ARGSUSED*/
SC_Type	 sc_GetType(Generic *scrap)
{
  /*** DUMMY ***/
  return 1;
}




/*ARGSUSED*/
void sc_Get(SC_Type type, Generic *owner, Generic *scrap)
{
  if( sc_type != 0 )
    { *owner = sc_owner;
      *scrap = sc_scrap;
    }
  else *scrap = nil;
}






/************************/
/*  Rendering		*/
/************************/




/* ... */






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void destroy()
{
  if( sc_type != 0 )
    (sc_methods[sc_type].destroy) (sc_type, sc_owner, sc_scrap);
}



