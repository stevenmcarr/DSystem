/* $Id: fortsym.i,v 1.11 1997/03/11 14:29:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef fortsym_i 
#define fortsym_i

#ifndef context_h
#include <libs/support/database/context.h>
#endif

#ifndef newdatabase_h
#include <libs/support/database/newdatabase.h>
#endif

#ifndef fortsym_h
#include <libs/frontEnd/fortTree/fortsym.h>
#endif

/*
 * The way all this is supposed to work:
 *
 * Per module (file), we have a TableDescriptor, with a counter for the number
 *  of procedures in the module, an error counter for the module, and
 *  a pointer to the first procedure's symbol table.
 *
 * Per procedure, we have a SymDescriptor, with its own Symbol Table, 
 *  the procedure's name (for identification), and an error counter.
 *
 * The underlying assumptions behind this design are:
 *  (1) the number of procedures is small
 *  (2) editing references display some degree of procedure locality.
 *
 * This means:
 *  (1) batch type checking should be quick, especially if we add a flag that
 *      tells the statement type checker that it will be handed the right 
 *      contexts.
 *  (2) incremental type checking will need to find, for each statement, the
 *      name of the procedure, check that against the current context procedure,
 *      and (possibly) change context SymTableDescriptors.
 *
 *      That will require O(nesting depth) operations to find procedure name, 
 *      a small constant number of operations for the context check, and 
 *      O(# procedures) operations for the set.
 *
 *  (3) linear search is used for setting the context.
 *
 *
 */

struct TableDescriptor_t {
    Generic            FortTreePtr;
    cNameValueTable    SubPgmHt;
    unsigned int       NumberOfModuleEntries;
    unsigned int       Errors; 
    Boolean            ModuleSymsComputed;
};


typedef struct ImplicitList_t {
    AST_INDEX stmt;
    struct ImplicitList_t *next;
} *ImplicitList;

typedef struct {
	struct {
		Boolean seen;
		int type;
	} typedecl[256];
	ImplicitList IList;
} Implicits;

typedef struct {
	fst_index_t leader;
	unsigned int varcount;
} LeaderTable_t;

typedef struct {
	int tablesize;
	LeaderTable_t *LeaderTable;
	int *EqChainNext;
} EquivalenceMap;


struct SymDescriptor_t {
    struct TableDescriptor_t *parent_td;
    char *name;
    SymTable Table;
	Implicits implicits;
	Boolean dataIsStatic;
	EquivalenceMap EQmap;
    unsigned int Errors; 
	unsigned int reference_cnt;
};

EXTERN(void, fst_Init, (void));
EXTERN(void, fst_Fini, (void));
EXTERN(TableDescriptor, fst_Open, (Generic ft));
EXTERN(void, fst_Recompute, (TableDescriptor td));
EXTERN(void, fst_Close, (TableDescriptor td));
EXTERN(void, fst_Save, (TableDescriptor td, Context context,
	DB_FP *fp));
EXTERN(SymDescriptor, fst_GetTable, (TableDescriptor td, char *entry));
EXTERN(unsigned int, fst_NumberOfModuleEntries, (TableDescriptor td));

EXTERN(void, TableDescriptorHashTableReInit, (TableDescriptor td));
EXTERN(Boolean, SymDescriptorAlloc, (TableDescriptor td,
  char *proc, SymDescriptor *d));

#define ST_INITIAL_NUM_SLOTS    1023

#define DONT_KNOW -9090

/* some useful macros */

#define IS_MANIFEST_CONSTANT(t,i) \
	((SymGetFieldByIndex((t), (i), SYMTAB_OBJECT_CLASS) == OC_IS_DATA) && \
	 (SymGetFieldByIndex((t), (i), SYMTAB_STORAGE_CLASS) == SC_CONSTANT))

#define IS_INTRINSIC_OR_GENERIC(t,i) \
	((SymGetFieldByIndex((t), (i), SYMTAB_OBJECT_CLASS) == OC_IS_EXECUTABLE) &&\
	 (SymGetFieldByIndex((t), (i), SYMTAB_STORAGE_CLASS) & \
			(SC_INTRINSIC | SC_GENERIC)))

#define IS_ARRAY(t,i) \
	((SymGetFieldByIndex((t), (i), SYMTAB_OBJECT_CLASS) & OC_IS_DATA) &&\
	 (SymGetFieldByIndex((t), (i), SYMTAB_NUM_DIMS) != 0))

#define IS_DUMMY_PARAM_FOR_ENTRY_OR_PROCEDURE(t,i) \
	((SymGetFieldByIndex((t), (i), SYMTAB_OBJECT_CLASS) & \
		(OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG)))

/*
 *  Fields used in handling of (transitively) overlapping 
 *  equivalenced variables.  Do not reference directly, but through
 *  fst_EquivSuperClassByIndex.
 */
#define SYMTAB_EQ_FIRST              "eq_first: fst_index_t"
#define SYMTAB_EQ_EXTENT             "eq_extent: int"

#endif
