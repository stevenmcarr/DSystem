/* $Id: DirectivesInclude.h,v 1.6 1999/03/31 21:56:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef DirectivesInclude_h
#define DirectivesInclude_h

#include <stdio.h>
#include <libs/support/lists/list.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/frontEnd/fortTree/fortsym.h>

typedef enum {PrefetchInstruction,FlushInstruction,Dependence} Instruction;

typedef struct DirectiveStruct {
  Instruction Instr;
  AST_INDEX   Subscript;
  UtilList    *DependenceList;
  int         DirectiveNumber;
  int         StmtNumber;
  AST_INDEX   AddressLeader; /* used in AddressOptimization */
  int         Offset; /* used in AddressOptimization */
 }  Directive;

EXTERN(Boolean, a2i_string_parse, (char *str,Directive *Dir,
				   SymDescriptor symtab));

#define DirectiveInfoPtr(n) \
   ((Directive *)ast_get_scratch(n))

#define PutDirectiveInfoPtr(n,d) \
   ast_put_scratch(n,(SCRATCH)d)

#endif
