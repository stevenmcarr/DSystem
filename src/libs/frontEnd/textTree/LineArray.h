/* $Id: LineArray.h,v 1.5 1997/03/11 14:30:03 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/TextTree/LineArray.h					*/
/*									*/
/*	LineArray -- sequence of "line tokens" for a FortranTree	*/
/*	Last edited: November 8, 1988 at 4:53 pm			*/
/*									*/
/************************************************************************/




#ifndef LineArray_h
#define LineArray_h


typedef Generic LineArray;




/************************/
/*  Initialization	*/
/************************/

EXTERN(void,		la_Init,(void));
EXTERN(void,		la_Fini,(void));

EXTERN(LineArray,	la_Open,(void));
EXTERN(void,		la_Close,(void));
EXTERN(void,		la_Save,(void));




/************************/
/*  Access to lines	*/
/************************/

/* ... */




/************************/
/*  Editing of lines	*/
/************************/

/* ... */




/************************/
/*  Editing of tree	*/
/************************/

/* ... */




#endif
