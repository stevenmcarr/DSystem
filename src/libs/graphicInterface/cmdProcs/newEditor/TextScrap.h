/* $Id: TextScrap.h,v 1.3 1997/03/11 14:30:35 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/TextScrap.h						*/
/*									*/
/*	TextScrap -- text fragment for the Scrap			*/
/*	Last edited: July 17, 1990 at 11:27 am				*/
/*									*/
/************************************************************************/




#ifndef TextScrap_h
#define TextScrap_h


typedef Generic TextScrap;




/************************/
/*  Initialization	*/
/************************/

EXTERN(void, txsc_Init, (void));
EXTERN(void, txsc_Fini, (void));

EXTERN(TextScrap, txsc_Create, (void));
EXTERN(void, txsc_Destroy, (void));



/************************/
/*  Rendering		*/
/************************/

/* ... */




#endif
