/* $Id: memos.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <stdio.h>
#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/f2i/ai.h>
#include <libs/f2i/classes.h>
#include <libs/f2i/sym.h>

static int StaticLabel = -1;




/* function to return the static label if it is initialized */
int  aiStaticLabel()
{
  if (StaticLabel == -1)
     ERROR("aiStaticLabel", "uninitialized", FATAL);
  return StaticLabel;
} /* aiStaticLabel */




/* function to record the static labels */
void aiRecordStaticLabel(char *s)
  //   char *s;
{
  StaticLabel = SymInsertSymbol(s, TYPE_INTEGER, OC_IS_SPECIAL, 0, 
				SC_STMT_LABEL, NO_ALIAS);
  fst_PutFieldByIndex(ft_SymTable, StaticLabel, SYMTAB_REG, 1);
} /* aiRecordStaticLabel */




/* function to return the stack base */
int aiStackBase()
{
  return fst_QueryIndex(ft_SymTable, "_stack");
} /* aiStackBase */




static int FunctionValueIndex = -1;




/* returns the index of the return value of a FORTRAN function */
int aiFunctionValueIndex()
{
  return FunctionValueIndex;
} /*aiFunctionValueIndex */




/* records the index of the return value of a FORTRAN function */
void aiRecordFunctionValueIndex(int i)
  // int i;
{
  FunctionValueIndex = i;
  fst_PutFieldByIndex(ft_SymTable, i, SYMTAB_REG, 2); 
} /* aiRecordFunctionValueIndex */
