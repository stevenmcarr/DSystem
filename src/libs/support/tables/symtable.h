/* $Id: symtable.h,v 1.8 1997/03/11 14:37:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef symtable_h
#define symtable_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

typedef struct SymTable_internal_structure *SymTable;

EXTERN(SymTable, SymInit, (unsigned int size));
EXTERN(void, SymKill, (SymTable ip));

EXTERN(int, SymMaxIndex, (SymTable ip));
EXTERN(int, SymIndex, (SymTable ip, char *name));
EXTERN(int, SymQueryIndex, (SymTable ip, char *name));

EXTERN(int, SymGetFieldByIndex, (SymTable ip, int index, char *field));
EXTERN(int, SymGetField, (SymTable ip, char *name, char *field));

EXTERN(void, SymPutFieldByIndex, (SymTable ip, int index, char *field, int val));
EXTERN(void, SymPutField, (SymTable ip, char *name, char *field, int val));

typedef FUNCTION_POINTER(void, SymCleanupFunc, (int val));
EXTERN(void, SymInitField, (SymTable ip, char *field, int val, SymCleanupFunc cleanup));
EXTERN(void, SymKillField, (SymTable ip, char *field));

EXTERN(int, SymFieldExists, (SymTable ip, char *field));

typedef FUNCTION_POINTER(void, SymIteratorFunc, (SymTable ip, int index, Generic extra_arg));
EXTERN(void, SymForAll, (SymTable ip, SymIteratorFunc func, int extra_arg));

EXTERN(void, SymDumpEntryByIndex,(SymTable ip, int index));
EXTERN(void, SymDump, (SymTable ip));

#define SYM_INVALID_INDEX  -1
#define SYM_NAME_FIELD     "name"

#endif
