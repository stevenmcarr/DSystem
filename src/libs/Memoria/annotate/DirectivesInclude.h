/* $Id: DirectivesInclude.h,v 1.3 1997/03/27 20:22:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef DirectivesInclude_h
#define DirectivesInclude_h

#include <stdio.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/support/lists/list.h>

typedef enum {PrefetchInstruction,FlushInstruction} Instruction;

typedef struct DirectiveStruct {
  Instruction Instr;
  AST_INDEX   Subscript;
  UtilList    *DependenceList;
  int         DirectiveNumber;
 }  Directive;

EXTERN(Boolean, a2i_string_parse, (char *str,Directive *Dir,SymDescriptor symtab));

#define DirectiveInfoPtr(n) \
   ((Directive *)ast_get_scratch(n))

#define PutDirectiveInfoPtr(n,d) \
   ast_put_scratch(n,(SCRATCH)d)

#endif
