/* $Id: FortUnparse2.h,v 1.7 1997/06/24 17:51:10 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	FortTextTree/FortUnparse2.h				*/
/*									*/
/*	FortUnparse2 -- high-level unparser for FortTrees		*/
/*									*/
/************************************************************************/




#ifndef FortUnparse2_h
#define FortUnparse2_h


/* no instances of this module */




/************************/
/*  Initialization	*/
/************************/

EXTERN(void,		unp2_Init, (void));
EXTERN(void,		unp2_Fini, (void));




/************************/
/*  Unparsing		*/
/************************/

typedef FUNCTION_POINTER (void, cacheProcFunc, (Generic,AST_INDEX,int,int,int));

EXTERN(void,		unp2_Unparse, (FortTextTree ftt, FortTreeNode node,
                                       int indent, Flex **lines));
EXTERN(void,		unp2_TextToNode, (FortTextTree ftt, int firstLine,
                                          FortTreeNode *node));
EXTERN(void,		unp2_NodeToText, (FortTextTree ftt, FortTreeNode node,
                                          int *firstLine, int *lastLine, 
                                          int *indent, Generic cacheOb, 
                                          cacheProcFunc cacheProc));




/************************/
/*  Expanding		*/
/************************/

EXTERN(void,		unp2_Expandee, (FortTextTree ftt, FortTreeNode node,
                                        Flex *expansions));
EXTERN(void,		unp2_Expand, (FortTextTree ftt, fx_Expansion ex,
                                      FortTreeNode *newNode, FortTreeNode *focus));



#endif
