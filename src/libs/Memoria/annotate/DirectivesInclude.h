#ifndef DirectivesInclude_h
#define DirectivesInclude_h

#include <stdio.h>
#include <fort/ast.h>

typedef enum {PrefetchInstruction,FlushInstruction} Instruction;

typedef struct DirectiveStruct {
  Instruction Instr;
  AST_INDEX   Subscript;
 }  Directive;

EXTERN(Boolean, a2i_string_parse, (char *str,Directive *Dir,SymDescriptor symtab));

#endif
