#ifndef DirectivesInclude_h
#define DirectivesInclude_h

#include <stdio.h>
#include <fort/ast.h>
#include <misc/list.h>

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
