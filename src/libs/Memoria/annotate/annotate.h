/* $Id: annotate.h,v 1.4 1997/03/27 20:22:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef annotate_h
#define annotate_h

#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/Memoria/annotate/DirectivesInclude.h>

typedef struct CallInfoTypeStruct {
  AST_INDEX Stmt;
  char      *routine;
  FortTextTree ftt;
  char         CacheRoutine[15];
  SymDescriptor symtab;
 }  CallInfoType;

#define PutLineNumber(n,v) \
  ast_put_scratch(n,v)

#define GetLineNumber(n) \
  ((int)ast_get_scratch(n))


#endif
