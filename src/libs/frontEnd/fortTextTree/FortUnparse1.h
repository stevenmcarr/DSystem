/* $Id: FortUnparse1.h,v 1.8 1997/10/30 15:27:36 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	FortTextTree/FortUnparse1.h				*/
/*									*/
/*	FortUnparse1 -- low-level unparser for FortTrees		*/
/*									*/
/************************************************************************/




#ifndef FortUnparse1_h
#define FortUnparse1_h

#include <libs/frontEnd/fortTextTree/FortTextTree.i>

/* no instances of this module */


#define UNARY_MINUS_PRECEDENCE       10
#define BINARY_EXPONENT_PRECEDENCE   UNARY_MINUS_PRECEDENCE - 1
#define BINARY_TIMES_PRECEDENCE      BINARY_EXPONENT_PRECEDENCE - 1
#define BINARY_DIVIDE_PRECEDENCE     BINARY_TIMES_PRECEDENCE
#define BINARY_PLUS_PRECEDENCE       BINARY_DIVIDE_PRECEDENCE - 1
#define BINARY_MINUS_PRECEDENCE      BINARY_PLUS_PRECEDENCE
#define BINARY_CONCAT_PRECEDENCE     BINARY_MINUS_PRECEDENCE - 1
#define BINARY_EQ_PRECEDENCE         BINARY_CONCAT_PRECEDENCE - 1
#define BINARY_NE_PRECEDENCE         BINARY_EQ_PRECEDENCE
#define BINARY_GE_PRECEDENCE         BINARY_EQ_PRECEDENCE
#define BINARY_GT_PRECEDENCE         BINARY_EQ_PRECEDENCE
#define BINARY_LE_PRECEDENCE         BINARY_EQ_PRECEDENCE
#define BINARY_LT_PRECEDENCE         BINARY_EQ_PRECEDENCE
#define UNARY_NOT_PRECEDENCE         BINARY_LT_PRECEDENCE - 1        
#define BINARY_AND_PRECEDENCE        UNARY_NOT_PRECEDENCE - 1
#define BINARY_OR_PRECEDENCE         BINARY_AND_PRECEDENCE - 1
#define BINARY_EQV_PRECEDENCE        BINARY_OR_PRECEDENCE - 1
#define BINARY_NEQV_PRECEDENCE       BINARY_EQV_PRECEDENCE

typedef enum {LEFT,RIGHT} DirectionValue;

/************************/
/*  Initialization	*/
/************************/

EXTERN(void,		unp1_Init, (void));
EXTERN(void,		unp1_Fini, (void));




/************************/
/*  Unparsing		*/
/************************/

EXTERN(void,		unp1_Unparse, (FortTextTree ftt, TT_Line line,
                                       TextString *text));
EXTERN(void,		unp1_TextToNode, (FortTextTree ftt, TT_Line line,
                                          int firstChar, FortTreeNode *node));
EXTERN(void,		unp1_NodeToText, (FortTextTree ftt, TT_Line line, 
                                          FortTreeNode node, int *firstChar,
                                          int *lastChar));

EXTERN(Boolean,		unp1_pred_parens, (FortTreeNode node));



/************************/
/*  Expanding		*/
/************************/

EXTERN(void,		unp1_Expandee, (FortTextTree ftt, TT_Line line, int 
                                        expandChar, Flex *expansions));
EXTERN(void,		unp1_Expand, (FortTextTree ftt, fx_Expansion ex,
                                      FortTreeNode *newNode, FortTreeNode *focus));




#endif
