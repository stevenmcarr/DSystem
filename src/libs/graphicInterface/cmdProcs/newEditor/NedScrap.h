/* $Id: NedScrap.h,v 1.4 1997/03/11 14:30:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/Scrap.h							*/
/*									*/
/*	Scrap -- global data fragment for inter-cp cut & paste		*/
/*	Last edited: October 17, 1991 at 3:04 pm			*/
/*									*/
/************************************************************************/




#ifndef Scrap_h
#define Scrap_h


/* no instances of this module */




typedef Generic SC_Type;

typedef FUNCTION_POINTER(void, sc_DestroyFunc,
  (SC_Type type, Generic owner, Generic scrap));

typedef struct
  {
    sc_DestroyFunc	destroy;
  } SC_Methods;






/************************/
/*  Initialization	*/
/************************/

EXTERN(void, sc_Init, (void));
EXTERN(void, sc_Fini, (void));

EXTERN(SC_Type, sc_Register, (char *name, SC_Methods *methods));
EXTERN(void, sc_Unregister, (Generic type));




/************************/
/*  Scrap contents	*/
/************************/

EXTERN(void, sc_Set, (SC_Type type, Generic owner, Generic scrap));
EXTERN(int, sc_NumTypes, (void));
EXTERN(SC_Type, sc_GetType, (Generic *scrap));
EXTERN(void, sc_Get, (SC_Type type, Generic *owner, Generic *scrap));




/************************/
/*  Rendering		*/
/************************/

/* ... */




#endif
