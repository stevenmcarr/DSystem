#ifndef annotate_h
#define annotate_h

#include <fort/ast.h>
#include <fort/FortTextTree.h>
#include <DirectivesInclude.h>

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
