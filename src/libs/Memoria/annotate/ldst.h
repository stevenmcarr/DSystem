/* $Id: ldst.h,v 1.2 1997/03/27 20:22:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef annotate_h
#define annotate_h

#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>

typedef struct IncInfoTypeStruct {
  AST_INDEX Stmt;
  Boolean MainProgram;
  Boolean IsLoad;
 }  IncInfoType;

#endif
