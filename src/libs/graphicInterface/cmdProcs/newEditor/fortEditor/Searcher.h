/* $Id: Searcher.h,v 1.5 1997/03/11 14:30:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditor/Searcher.h					*/
/*									*/
/*	Searcher -- text and tree pattern matcher for a FortEditor	*/
/*	Last edited: April 28, 1989 at 2:10 pm				*/
/*									*/
/************************************************************************/




#ifndef Searcher_h
#define Searcher_h


typedef Generic Searcher;




/************************/
/*  Initialization	*/
/************************/

EXTERN(void,		srch_Init,(void));
EXTERN(void,		srch_Fini,(void));

EXTERN(Searcher,       	srch_Open,(Context context, DB_FP *fd, FortEditor ed));
EXTERN(void,		srch_Close,(Searcher srch));
EXTERN(void,		srch_Save,(Searcher srch, Context context, DB_FP *fd));




/************************/
/*  Patterns		*/
/************************/

EXTERN(void,		srch_SetPattern,(Searcher srch, FortVFilter filter));
EXTERN(void,		srch_SetPatternText,(Searcher srch, char *str, 
                                             Boolean fold));




/************************/
/*  Search control	*/
/************************/

EXTERN(Boolean,		srch_Find,(Searcher srch, Boolean dir, Boolean wrap));
EXTERN(Boolean,		srch_FindPlaceholder,(Searcher srch, Boolean dir, 
                                              Boolean wrap));
EXTERN(int,		srch_ReplaceText,(Searcher srch, Boolean dir, Boolean 
                                          global, Boolean all, char *newStr));
EXTERN(int,		srch_ReplaceTree,(Searcher srch, Boolean dir, Boolean 
                                          global, Boolean all, FortTreeNode 
                                          newNode));




#endif
