/* $Id: DirectivesInclude.h,v 1.10 2002/02/20 16:17:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef DirectivesInclude_h
#define DirectivesInclude_h

#include <stdio.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/fortTree/fortsym.h>

typedef enum {PrefetchInstruction,
	      FlushInstruction,
	      Dependence,
	      Cluster} Instruction;

typedef struct DirectiveStruct {
  Instruction Instr;
  AST_INDEX   Subscript;
  UtilList    *DependenceList;
  int         DirectiveNumber;
  int         StmtNumber;
  int         SpecialLoadStride;
  AST_INDEX   AddressLeader; /* used in AddressOptimization */
  AST_INDEX   FirstInLoop; /* used in AddressOptimization */
  int         Offset; /* used in AddressOptimization */
  int         Cluster;
 }  Directive;

EXTERN(Boolean, a2i_string_parse, (char *,
				   Directive *,
				   SymDescriptor));

#define DirectiveInfoPtr(n) \
   ((Directive *)ast_get_scratch(n))

#define PutDirectiveInfoPtr(n,d) \
   ast_put_scratch(n,(SCRATCH)d)

#endif








