/* $Id: FortTree.h,v 1.8 1997/03/11 14:29:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*   ned_cp/FortTree.h                                                  */
/*                                                                      */
/*   FortTree -- a Fortran AST with context-sensitive semantics         */
/*                                                                      */
/*  Modification History                                                */
/*   January 7, 1992                          John Mellor-Crummey       */
/*     added interface for symbol table and unique node number mapping  */
/*                                                                      */
/************************************************************************/




#ifndef FortTree_h
#define FortTree_h


#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/astlist.h>
#include <libs/frontEnd/ast/asttree.h>
#include <include/frontEnd/astsel.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/ast/aphelper.h>
#include <include/maxdefs.h>
#include <libs/frontEnd/fortTree/fortsym.h>

#ifdef __cplusplus
typedef class NeedProvSet *NeedProvSetPtr;
#else
typedef struct NeedProvSet *NeedProvSetPtr;
#endif /* __cplusplus */

#include <libs/support/database/context.h>


typedef struct FortTree_internal_structure *FortTree;
typedef unsigned int	FortTreeNode;		/* really AST_INDEX */
typedef Generic		FortTreeSideArray;
extern	char		*ft_SourceAttribute;


#define ft_SIMPLE	0
#define ft_OPEN		1
#define ft_CLOSE	2


typedef enum {
  ft_UNINITIALIZED, ft_INITIALIZED, 
  ft_CORRECT, ft_WARNINGS_ONLY,
  ft_ERRONEOUS
  } ft_States;


/************************/
/*  Initialization	*/
/************************/

EXTERN(void, ft_Init, (void));
EXTERN(void, ft_Fini, (void));

EXTERN(FortTree, ft_Open, (Context context, DB_FP *fp));
EXTERN(void, ft_Close, (FortTree ft));
EXTERN(void, ft_Save, (FortTree ft, Context context, DB_FP *fp));

EXTERN(FortTree, ft_Create, (void));
EXTERN(int, ft_Read, (FortTree ft, DB_FP *file));
EXTERN(int, ft_Write, (FortTree ft, DB_FP *file));


/************************/
/*  AST access		*/
/************************/

EXTERN(void, ft_AstSelect, (FortTree ft));
EXTERN(FortTreeNode, ft_Root, (FortTree ft));
EXTERN(void, ft_SetRoot, (FortTree ft, FortTreeNode node));




/************************/
/* Semantics access	*/
/************************/

/* ... */




/************************/
/* Updating semantics	*/
/************************/

/* ... */


/***********************/
/* Contents            */
/***********************/

EXTERN(ft_States, ft_GetState, (FortTree ft));
EXTERN(void, ft_ResetStateToInitialized, (FortTree ft));

/************************/
/*  Side Array		*/
/************************/

EXTERN(FortTreeSideArray, ft_AttachSideArray, (FortTree ft, int size,
 Generic *initials));
EXTERN(void, ft_DetachSideArray, (FortTree ft,
 FortTreeSideArray sideArray));
EXTERN(Generic, ft_GetFromSideArray, (FortTreeSideArray sideArray,
 FortTreeNode node, int k));
EXTERN(void, ft_PutToSideArray, (FortTreeSideArray sideArray,
 FortTreeNode node, int k, Generic value));
EXTERN(void, ft_ReadSideArray, (FortTree ft,
 FortTreeSideArray sideArray, Context context, DB_FP *fp));
EXTERN(void, ft_WriteSideArray, (FortTree ft,
 FortTreeSideArray sideArray, Context context, DB_FP *fp));


EXTERN(void, ft_ReadSideArrayFromFile, 
       (FortTree ft, FortTreeSideArray sideArray, DB_FP *fp));
EXTERN(void, ft_WriteSideArrayToFile, 
       (FortTree ft, FortTreeSideArray sideArray, DB_FP *fp));





/************************/
/* Display parameters	*/
/************************/

EXTERN(void, ft_SetComma, (FortTreeNode node, Boolean value));
EXTERN(Boolean, ft_GetComma, (FortTreeNode node));
EXTERN(void, ft_SetShow, (FortTreeNode node, Boolean value));
EXTERN(Boolean, ft_GetShow, (AST_INDEX node));
EXTERN(void, ft_SetEmphasis, (FortTreeNode node, Boolean value));
EXTERN(Boolean, ft_GetEmphasis, (AST_INDEX node));
EXTERN(void, ft_SetConceal, (FortTree ft, FortTreeNode node,
 int bracket, int value));
EXTERN(int, ft_GetConceal, (FortTree ft, AST_INDEX node,
 int bracket));




/************************/
/* Error access		*/
/************************/

EXTERN(void, ft_SetSemanticErrorCode, (FortTree ft, FortTreeNode node,
 short value));
EXTERN(short, ft_GetSemanticErrorCode, (FortTreeNode node));
EXTERN(void, ft_SetParseErrorCode, (FortTreeNode node, short value));
EXTERN(short, ft_GetParseErrorCode, (FortTreeNode node));
EXTERN(Boolean, ft_IsErroneous, (FortTree ft, FortTreeNode node,
 int bracket));
EXTERN(void, ft_GetErrorMessage, (FortTree ft, AST_INDEX node,
 int bracket, char *Message));




/************************/
/* Type Checking	*/
/************************/

EXTERN(ft_States, ft_Check, (FortTree ft));


/************************/
/* Symbol Table Access  */
/************************/

EXTERN(ft_States, ft_SymRecompute, (FortTree ft));
EXTERN(SymDescriptor, ft_SymGetTable, (FortTree ft, char *entry_point));
EXTERN(unsigned int, ft_SymNumberOfModuleEntries, (FortTree ft));

EXTERN(NeedProvSetPtr, ft_GetNeeds, (FortTree ft));
EXTERN(NeedProvSetPtr, ft_GetProvs, (FortTree ft));

/*************************************************************************/
/*  Node Numbering                                                       */
/*  N.B. Node numbers are currently defined only for callsites and loops */
/*************************************************************************/

EXTERN(int, ft_NodeToNumber, (FortTree ft, FortTreeNode node));
EXTERN(FortTreeNode, ft_NumberToNode, (FortTree ft, int number));
EXTERN(void, ft_RecomputeNodeNumbers, (FortTree ft));


#endif
