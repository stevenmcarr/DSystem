/* $Id: FortParse1.h,v 1.4 1997/03/11 14:29:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortTexTree/FortParse1.h					*/
/*									*/
/*	FortParse1 -- low-level parser for Fortran source		*/
/*	Last edited: August 11, 1987 at 2:33 pm				*/
/*									*/
/************************************************************************/




#ifndef FortParse1_h
#define FortParse1_h

#include <libs/frontEnd/fortTextTree/FortTextTree.i>

/* no instances of this module */




/************************/
/*  Initialization	*/
/************************/

EXTERN(void,		fp1_Init, (void));
EXTERN(void,		fp1_Fini, (void));




/************************/
/*  Parsing		*/
/************************/

EXTERN(Boolean,		fp1_Parse, (FortTextTree ftt, TextString text, 
                                    fx_StatToken *st));
EXTERN(void,		fp1_Copy, (FortTextTree ftt, fx_StatToken *oldToken,
                                   fx_StatToken *newToken));
EXTERN(void,		fp1_Destroy, (FortTextTree ftt, fx_StatToken *st));




/************************/
/*  Error reporting	*/
/************************/

EXTERN(void,		fp1_GetError, (FortTreeNode errorNode, char *msg, int
                                       maxMessageLen));




#endif
