#ifndef ArrayTable_h
#define ArrayTable_h

#include <misc/Arena.h>
#include <fort/FortTextTree.h>

typedef struct ArrayTableStruct {
  AST_INDEX node;
  int       LineNum;
  char      *Text;
 } ArrayTableType;

typedef struct TableInfoTypeStruct {
  ArrayTableType *ArrayTable;
  int             count;
  int             LineNum;
  FortTextTree    ftt;
  int             MaxLength;
 }  TableInfoType;

EXTERN(void, memory_BuildArrayTableInfo, (AST_INDEX root,
					  int       level,
					  TableInfoType *TableInfo,
					  arena_type *ar));
#endif
