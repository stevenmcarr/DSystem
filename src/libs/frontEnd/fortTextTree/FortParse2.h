/* $Id: FortParse2.h,v 1.4 1997/03/11 14:29:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortTextTree/FortParse2.h				*/
/*									*/
/*	FortParse2 -- high-level parser for Fortran source		*/
/*	Last edited: September 21, 1988 at 7:37 pm			*/
/*									*/
/************************************************************************/




#ifndef FortParse2_h
#define FortParse2_h



/* no instances of this module */




/************************/
/*  Initialization	*/
/************************/

EXTERN(void,		fp2_Init,(void));
EXTERN(void,		fp2_Fini,(void));




/************************/
/*  Parsing		*/
/************************/

EXTERN(Boolean,		fp2_Parse,(FortTextTree ftt, int goal, Flex *lines, 
                                  int first, int count, FortTreeNode *node));
EXTERN(Boolean,		fp2_Synch,(FortTextTree ftt, FortTreeNode node, int *goal));
EXTERN(void,		fp2_Copy,(FortTextTree ftt, FortTreeNode oldNode,
                                  FortTreeNode *newNode));
EXTERN(void,		fp2_Destroy,(FortTextTree ftt, FortTreeNode root));
EXTERN(void,		fp2_SetRoot,(FortTextTree ftt, FortTreeNode node));




/************************/
/*  Error reporting	*/
/************************/

EXTERN(void,		fp2_GetError,(FortTreeNode errorNode, char *msg,
                                      int maxMessageLen));




#endif

