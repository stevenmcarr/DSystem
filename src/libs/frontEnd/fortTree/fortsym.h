/* $Id: fortsym.h,v 1.23 1997/06/24 17:53:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef fortsym_h
#define fortsym_h


#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif 

#ifndef cNameValueTable_h
#include <libs/support/tables/cNameValueTable.h>
#endif

#ifndef rn_varargs_h
#include <include/rn_varargs.h>
#endif

#ifndef symtable_h
#include <libs/support/tables/symtable.h>
#endif

/************************************************************************
 *                                                                      *
 * The Rn/ParaScope Fortran Symbol Table                    June 1991   *
 * Author: John Mellor-Crummey                                          *
 *                                                                      *
 * This module documents the interface to the Fortran symbol table      *
 * abstraction. A symbol table is constructed as a side effect of       *
 * semantic checking of a Fortran Abstract Syntax tree.                 *
 *                                                                      *
 * Copyright 1991, Rice University, as part of the Rn/ParaScope         *
 * Programming Environment Project.                                     *
 *                                                                      *
 ************************************************************************/

typedef struct TableDescriptor_t *TableDescriptor;
typedef struct SymDescriptor_t *SymDescriptor;

typedef int fst_index_t;

/* a template structure for equivalence classes */
typedef struct _EquivalenceClass { 
  unsigned int members;   /* count of symbols in the same equivalence class */ 
  fst_index_t member[1];  /* array of symbols in the class */
} EquivalenceClass;

typedef enum _ArrayBoundEnum { 
  dim_error, star, constant, symbolic_expn_ast_index
  } ArrayBoundEnum;

typedef union _ArrayBoundVal {
  int const_val;
  AST_INDEX ast;
} ArrayBoundVal;

typedef struct _ArrayBound {
  struct _ArrayBoundS {
    ArrayBoundEnum type;
    ArrayBoundVal value;
  } lb, ub;
} ArrayBound;



EXTERN(void,fst_Init,(void));
EXTERN(void, fst_DumpModuleTables, (TableDescriptor td));
EXTERN(unsigned int, fst_NumEntryPoints, (TableDescriptor td));
EXTERN(Boolean, SymDescriptorHashTableAddEntryPoint, 
       (cNameValueTable SubPgmHt, char* proc, SymDescriptor d));
EXTERN(void, SymDescriptorFree, (SymDescriptor p));
EXTERN(void, SymDescriptorInitializeImplicits, (SymDescriptor p));
EXTERN(void, TableDescriptorHashTableFree, (TableDescriptor td));
EXTERN(void, TableDescriptorHashTableClear, (TableDescriptor td));

/****************************/
/* user interface functions */
/****************************/

EXTERN(fst_index_t, fst_Index, (SymDescriptor d, char *name));
EXTERN(fst_index_t, fst_QueryIndex, (SymDescriptor d, char *name));
EXTERN(fst_index_t, fst_MaxIndex, (SymDescriptor d));

EXTERN(Generic, fst_GetFieldByIndex, 
	(SymDescriptor d, fst_index_t index, char *field));
EXTERN(Generic, fst_GetField, 
	(SymDescriptor d, char *name, char *field));

EXTERN(void, fst_PutFieldByIndex, 
	(SymDescriptor d, fst_index_t index, char *field, Generic value));
EXTERN(void, fst_PutField, 
        (SymDescriptor d, char *name, char* field, Generic value));

EXTERN(void, fst_InitField, 
	(SymDescriptor d, char *field, Generic init_value, SymCleanupFunc cleanup_fn));
EXTERN(void, fst_KillField, (SymDescriptor d, char *field));

/* --- symbol table iteration functions --- */

typedef FUNCTION_POINTER(void, fst_ForAllCallbackV, 
			 (SymDescriptor d, fst_index_t index, 
			  va_list arg_list));

typedef FUNCTION_POINTER(void, fst_ForAllCallback, 
			 (SymDescriptor d, fst_index_t index,  
			  Generic extra_arg));

EXTERN(void, fst_ForAllV, (SymDescriptor d, 
				   fst_ForAllCallbackV func, ...));

EXTERN(void, fst_ForAll, (SymDescriptor d, fst_ForAllCallback func, 
				  Generic extra_arg)); 

EXTERN(Boolean, fst_bound_is_const_ub, (ArrayBound a));
EXTERN(Boolean, fst_bound_is_const_lb, (ArrayBound a));



/* --- the interface to equivalence and common information --- */

/* NOTE: size < 0 is used to signal special conditions;  users of
 *       this interface should be prepared to handle the conditions 
 *       that cause size < 0 to be returned.  -- JMC 3/93
 */
EXTERN(EquivalenceClass *, fst_Symbol_Offset_Size_To_VarSymbols,
	(SymDescriptor d, fst_index_t leader, int offset, 
	int size));
EXTERN(void, fst_Symbol_To_EquivLeader_Offset_Size,
       (SymDescriptor d, fst_index_t index, fst_index_t* leader, int* offset, 
        unsigned int* size));

EXTERN(Boolean, entry_name_symdesc_name_To_leader_offset_size_vtype,
       (char *entry_name, SymDescriptor d, char *name, char **leader, int *offset,
        int *size, unsigned int *vtype));

EXTERN(Boolean, fst_EquivSuperClassByIndex, (SymDescriptor d, fst_index_t var,
					     fst_index_t *first, int *length));

/* dump to stderr a fortran symbol table */
EXTERN(void, fst_Dump, (SymDescriptor d));

/* dump to stderr a single entry in a fortran symbol table */
EXTERN(void, fst_Dump_Symbol, (SymDescriptor d, fst_index_t index));

/* evaluate a constant int expression -- please read this function's header */
/* comment for a description of how to use it. */
EXTERN(int, evalConstantIntExpr,
		(SymDescriptor d, AST_INDEX expr, int *value));

EXTERN(int, fastEvalConstantIntExpr, (SymDescriptor d, AST_INDEX expr, int* value));

/* ***************************************************************************/
/*                       symbol table field names                            */
/* ***************************************************************************/

#define SYMTAB_NAME				SYM_NAME_FIELD

/* value type information */
#define SYMTAB_TYPE_STMT		"typeStmt: AST_INDEX"
#		define TYPE_STMT_IMPLICIT	-1
#		define TYPE_STMT_UNKNOWN 	-2
#define SYMTAB_TYPE				"type: int"

/* array dimension information */
#define SYMTAB_DIM_STMT			"dimsStmt: AST_INDEX"
#define SYMTAB_DIM_LIST			"list: AST_INDEX"
#define SYMTAB_NUM_DIMS			"dimensions: unsigned int"
#define SYMTAB_DIM_BOUNDS		"dim_bound_pairs: ArrayBound[]"

#define SYMTAB_SIZE 		"size: int"   
#              define FST_ADJUSTABLE_SIZE      -2
       /* size info:
        * for variables: based on type and array dimensions
        * for common blocks: based on declarations and equivalences into common
        */

/* procedure info */
#define SYMTAB_NARGS 			"number: unsigned int" 

#define SYMTAB_FORMALS_HT		"formal params: HashTable"
    /* defined for procedures and entry points only, (HashTable) 0 otherwise */
    /* contains (parameter name, parameter position) pairs */

#define SYMTAB_FORMALS_LIST		"list: AST_INDEX" 
    /* list of the formal parameters for the procedure, entry, or stmt func */

/* field names used for common block names */
#define SYMTAB_FIRST_NAME		"next_common: fst_index_t"
    /* head of a list of variables declared in the common block */
#define SYMTAB_LAST_NAME		"last_name: fst_index_t"
    /* tail of a list of variables declared in the common block */
#define SYMTAB_NUM_COMMON_VARS	"number: int" 
    /* number of vars declared in the common block */

/* field names used for variables in a common block */
#define SYMTAB_COMMON_STMT 		"commonStmt: AST_INDEX"
/* the AST_INDEX of the first mention of a common block in a procedure */   
#define SYMTAB_COMMON_NAME_FIRST_USE    "commonNameFirstUse: AST_INDEX"
/* offset in common block, or equiv offset from leader */
#define SYMTAB_NEXT_COMMON		"next_common: fst_index_t"

#define SYMTAB_PRIVATE_SCOPES	"var private in scopes: HashTable"
    /* defined for data values only, (HashTable) 0 otherwise */
    /* contains (private scope AST_INDEX, 0) pairs */

/* equivalence handling info */
#define SYMTAB_PARENT			"parent: fst_index_t"
    /* index of leader of equivalence class 
     * for common variables -- parent is the common block
     */
#define SYMTAB_EQ_OFFSET		"eq_offset: int" 
    /* offset from leader of equivalence class 
     * for common variables, SYMTAB_EQ_OFFSET is offset in common block
     */

#define SYMTAB_EXTERNAL_STMT	"externalStmt: AST_INDEX"
#define SYMTAB_INTRINSIC_STMT	"intrinsicStmt: AST_INDEX"
#define SYMTAB_SAVE_STMT 		"saveStmt: AST_INDEX"

/* character string information */
#define SYMTAB_CHAR_LENGTH		"char_length: unsigned int"
#		define CHAR_LEN_STAR 0

/* used for parameter expression, and statement function expression */
#define SYMTAB_EXPR				"expr: AST_INDEX"

/* used to store expanded statement function definition, i.e. if
   "a(e) = e * e" and "b(r) = a(r) * r", then SYMTAB_BODY_EXPAND for
   statement function b(r) will be the expression "r * r * r". */
#define SYMTAB_SF_EXPR			"expanded stmt func def: AST_INDEX"

/* the AST_INDEX of the statement that contains the reference to a label */
#define SYMTAB_REF_STMT		        "refStmt: AST_INDEX"

/* information about manifest constants defined with a PARAMETER stmt */
#define SYMTAB_PARAM_VALUE     "param_value: int" /* cache for expr eval */
#define SYMTAB_PARAM_STATUS    "param_status: int" /* used by expr evaluator */
#		define PARAM_UNDEFINED		0
#		define PARAM_VALUE_DEFINED	1
#		define PARAM_IN_EVALUATION	2

/* object class defines general characteristics of a symbol table entity */
#define SYMTAB_OBJECT_CLASS		"object_class"  
#		define	OC_UNDEFINED		0x0000
#		define	OC_IS_EXECUTABLE	0x0001
#		define	OC_IS_DATA 		0x0002
#		define	OC_IS_COMMON_NAME	0x0004
      /* OC_IS_EXECUTABLE and OC_IS_DATA can also be overlaid with the value */
#		define	OC_IS_FORMAL_PAR 	0x1000
#		define	OC_IS_STFUNC_ARG 	0x2000
#		define	OC_IS_ENTRY_ARG 	0x4000


/* storage class defines fine characteristics of a symbol table entity */
#define SYMTAB_STORAGE_CLASS	"storage_class: int" 
	/* for data: storage_class field has one of the following values */
#		define	SC_CONSTANT		0x0001
#		define	SC_NO_MEMORY		0x0002
#		define	SC_STACK	       	0x0004
#		define	SC_GLOBAL	       	0x0008
#		define	SC_STATIC	       	0x0010
        /* for executable, storage_class has one of the following values: */
#		define	SC_SUBROUTINE		0x0040
#		define	SC_PROGRAM	       	0x0080
#		define	SC_GENERIC	       	0x0100
#		define	SC_INTRINSIC		0x0200
#		define	SC_ENTRY 	       	0x0400
#		define	SC_STMT_FUNC		0x0800
#		define  SC_STMT_LABEL		0x1000
#		define	SC_BLOCK_DATA		0x2000
#		define	SC_FUNCTION	       	0x4000
       /* an executable's storage class can also be overlaid with the values */
#		define	SC_EXTERNAL	       	0x8000
#		define	SC_CURRENT_PROC         0x10000 
	         /* SC_CURRENT_PROC is set for the function or subroutine name 
	          * in the symbol table for its own scope
	          */

#define BLANK_COMMON 		"//"   

/**************************** some useful macros ***************************/

/* predicate to determine if a symbol table index is meaningful */
#define fst_index_is_valid(index) ((index) != SYM_INVALID_INDEX)

/* return an equivalence class of all variables overlapping a given symbol */
#define fst_OverlappingSymbols(SymDesc, symindex) \
	fst_Symbol_Offset_Size_To_VarSymbols((SymDesc), (symindex), 0, \
		fst_GetFieldByIndex((SymDesc), (symindex), SYMTAB_SIZE))

/* return an equivalence class of all variables that overlap a given region
 * in a common block 
 */
#define fst_CommonName_Offset_Size_To_VarSymbols(SymDesc, cname, off, size) \
	fst_Symbol_Offset_Size_To_VarSymbols((SymDesc), \
		fst_QueryIndex((SymDesc), (cname)), (off), (size))

#define FS_IS_MANIFEST_CONSTANT(d, i) \
	((fst_GetFieldByIndex((d), (i), SYMTAB_OBJECT_CLASS) == OC_IS_DATA) && \
	 (fst_GetFieldByIndex((d), (i), SYMTAB_STORAGE_CLASS) == SC_CONSTANT))

#define FS_IS_INTRINSIC_OR_GENERIC(d, i) \
	((fst_GetFieldByIndex((d), (i), SYMTAB_OBJECT_CLASS) \
			== OC_IS_EXECUTABLE) &&\
	 (fst_GetFieldByIndex((d), (i), SYMTAB_STORAGE_CLASS) & \
			(SC_INTRINSIC | SC_GENERIC)))

#define FS_IS_ARRAY(d, i) \
	((fst_GetFieldByIndex((d), (i), SYMTAB_OBJECT_CLASS) & OC_IS_DATA) &&\
	 (fst_GetFieldByIndex((d), (i), SYMTAB_NUM_DIMS) != 0))

#define FS_IS_DUMMY_PARAM_FOR_ENTRY_OR_PROCEDURE(d, i) \
	((fst_GetFieldByIndex((d), (i), SYMTAB_OBJECT_CLASS) & \
		(OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG)))

#define FS_IS_IP_VAR(d, i) \
	(FS_IS_DUMMY_PARAM_FOR_ENTRY_OR_PROCEDURE(d, i) || \
	 (fst_GetFieldByIndex((d), (i), SYMTAB_STORAGE_CLASS) & SC_GLOBAL))

#endif
