/* $Id: ArrayTable.h,v 1.4 1997/03/20 15:49:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef ArrayTable_h
#define ArrayTable_h

#include <libs/support/memMgmt/Arena.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>

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
