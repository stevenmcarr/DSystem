#ifndef annotate_h
#define annotate_h

#include <fort/ast.h>
#include <fort/FortTextTree.h>

typedef struct IncInfoTypeStruct {
  AST_INDEX Stmt;
  Boolean MainProgram;
  Boolean IsLoad;
 }  IncInfoType;

#endif
