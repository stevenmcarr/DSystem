/* $Id: TypeChecker.ansi.c,v 1.41 93/02/15 18:50:11 */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* The external interface routines:
 *
 *	FortStmtTypeCheck(s,d)  type checks the statement s relative to the
 *				symbol table and type checker represented
 *				by descriptor d.
 *
 *	FortTreeTypeCheck(d,ft) type checks the entire FortTree ft
 *				and fills in the table descriptor.
 *
 *  Modification History
 *
 *    Mar 12, 1993                       Alan Carle
 *      -- Added intramodule interface consistency checking. We now flag
 *         inconsistent defs and uses of procedures.
 *      -- Modified statement list traversals to ensure that statements
 *         in the "supported_smp" dialect occur only in reasonable statement
 *         locations.
 *
 *    Oct  7, 1992                       Alan Carle
 *      -- In addition to numerous other fixes to list_nexting from modified
 *         nodes in lists, added checks to ensure that all label references actually
 *         have corresponding label definitions.
 *
 *    June 3, 1992                       Paul Havlak
 *	-- Made SC_STACK the default storage class, and added code
 *		to set storage class to SC_STATIC for non-global
 *		variables occurring in SAVE and DATA statements.
 *
 *    January 7, 1992                    John Mellor-Crummey 
 *      -- added externally visible symbol table abstraction
 *      -- strengthened type checking
 *      -- added equivalence class based storage mapping
 *      -- added code to appropriately coerce 
 *         statement function definitions <--> assignments 
 *         function invocations <--> array references 
 *
 *    July 19, 1991                     Tim Harvey
 *      -- added check in CheckIf for else and else if labels
 *      -- handle READ_SHORT and READ_LONG stmts similar to
 *         GEN_WRITE statements; they also may read whole array 
 *         by name without subscripts
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <include/bstring.h>

#include <libs/support/database/newdatabase.h>

#include <libs/frontEnd/ast/builtins.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/frontEnd/include/stmt_func.h>

#include <libs/support/tables/cNameValueTable.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/support/strings/rn_string.h>

#include <libs/frontEnd/fortTree/fortsym.i>
#include <libs/frontEnd/fortTree/FortTree.i>
#include <libs/frontEnd/fortTree/TypeChecker.h>


/* maximum dimensions the type checker will permit for a fortran array */
#define MAX_NUM_ARRAY_DIMS 20 

/* some shorthands */

#define srt(node,v) gen_put_real_type(node,v)
#define sct(node,v) gen_put_converted_type(node,v)

#define grt(node) \
	(node == AST_NIL ? TYPE_UNKNOWN : gen_get_real_type(node))
#define gct(node) \
	(node == AST_NIL ? TYPE_UNKNOWN : gen_get_converted_type(node))

/*
 * An implementation of the equivalence handling algorithm loosely based
 * on that given in  "Principles of Compiler Design" by Aho and Ullman
 */

   /* info about the two obj's being equiv'ed   */
struct equiv_struct
{
  AST_INDEX     node;           /* pointer to the equiv'd item              */
  int           name;           /*  - its symbol table index                */
  int           leader;         /*  - its leader                            */
  int           eq_offset;      /*  dist from origin of obj to origin of leader */
  int           height;         /*  - # links to its leader                 */
  int           high;           /*  - upper bound of array rel. to start    */
  int           equiv_pt;       /*  - offset of equivalence from origin     */
  int           leader_is_common; /*  0 if the leader is not a common block */
  int           storage_class;  /*  - its s.c. field                        */
};

static struct equiv_struct objs[2];

struct EQList
{
  AST_INDEX      stmt;
  struct EQList *next;
};

static struct EQList *EquivList = (struct EQList *) 0;
int equivDebug = 0;


/* forward declarations */

STATIC(Boolean, WalkProcedures, (FortTree ft, TableDescriptor td, AST_INDEX root));
STATIC(int, findType, (SymDescriptor d, AST_INDEX TypeLen));
STATIC(void, ArrayDeclLen, (SymDescriptor d, AST_INDEX node, int type, AST_INDEX stmt));
STATIC(void, SetImplicitType, (SymTable t, int i, SymDescriptor d));
STATIC(void, DeclarationCleanUp, (SymDescriptor d));
STATIC(void, TypeDeclarationsCheckDefault, (SymDescriptor d, AST_INDEX stmts));
STATIC(void, CheckDefault, (SymDescriptor d, AST_INDEX node));
STATIC(void, MakeStackStatic, (SymDescriptor d, AST_INDEX ident));
STATIC(int, tcExprCheck, (SymDescriptor d, AST_INDEX node));
STATIC(void, CheckBadProcNameContext, (SymDescriptor d, AST_INDEX node, int oc, int sc));
STATIC(void, tcCheckIdentifier, (SymDescriptor d, AST_INDEX node));
STATIC(void, tcCheckLabelRef, (SymDescriptor d, AST_INDEX node));
STATIC(void, tcCheckLabelDef, (SymDescriptor d, AST_INDEX node));
STATIC(AST_INDEX, tcCheckSubscript, (SymDescriptor d, AST_INDEX node));
STATIC(void, tcCheckSubstring, (SymDescriptor d, AST_INDEX node));
STATIC(void, CheckStatementFunctionInvocation, (SymDescriptor d, fst_index_t findex,
                                             AST_INDEX node, int type));
STATIC(AST_INDEX, tcCheckInvocation, (SymDescriptor d, AST_INDEX node, int context_sc));
STATIC(int, CheckIntrinsicInvocation, (SymDescriptor d, AST_INDEX node));
STATIC(int, CheckGenericInvocation, (SymDescriptor d, AST_INDEX node));
STATIC(void, doFormals, (SymDescriptor d, int pindex, AST_INDEX list));
STATIC(int, tcSymInsert, (SymDescriptor d, char *name, int type));
STATIC(int, tcCommonInsert, (SymDescriptor d, char *name, AST_INDEX common_name_ref));
STATIC(void, tcCommonAddVar, (SymDescriptor d, int common_block, int var_index));
STATIC(void, CleanupVariableObjectClass, (SymTable t, int i, SymDescriptor d));
STATIC(void, CleanupVariableStorageClass, (SymTable t, int i, SymDescriptor d));
STATIC(void, CheckStmtFuncBody, (SymDescriptor d, AST_INDEX defstmt, AST_INDEX node));
STATIC(void, CheckStmtFuncOrdering, (SymTable t, int i, SymDescriptor d));
STATIC(void, CheckStmtFuncsOrdering, (SymDescriptor d));
STATIC(void, CleanupClassesForVariables, (SymDescriptor d));
STATIC(Boolean, isDimExpr, (SymDescriptor d, AST_INDEX node));
STATIC(void, PostProcessArrayDecl, (SymTable t, int index, SymDescriptor d));
STATIC(void, PostProcessArrayDecls, (SymDescriptor d));
STATIC(int, CharExprLength, (SymDescriptor d, AST_INDEX expr));
STATIC(int, ConstCharExprLength, (SymDescriptor d, AST_INDEX expr));
STATIC(int, SymbolElementSizeFromType, (SymDescriptor d, int index));
STATIC(int, VariableSize, (SymDescriptor d, int index));
STATIC(void, CheckLabelRefDef, (SymTable t, int i, SymDescriptor d));
STATIC(void, CheckLabelRefDefs, (SymDescriptor d));
STATIC(void, AssignVariableSize, (SymTable t, int i, SymDescriptor d));
STATIC(void, AssignVariableSizes, (SymDescriptor d));
STATIC(void, LayoutOneCommonBlock, (SymTable t, int common_index, int dummy));
STATIC(void, LayoutCommonBlocks, (SymDescriptor d));
STATIC(void, EnsureOneCommonBlockDefined, (SymTable t, int common_index, SymDescriptor d));
STATIC(void, EnsureAllCommonBlocksDefined, (SymDescriptor d));
STATIC(void, CheckEquivalence, (SymDescriptor d, AST_INDEX stmt));
STATIC(void, AnEquivalence, (SymDescriptor d, AST_INDEX stmt));
STATIC(void, FlattenEquiv, (SymTable t, int i, SymDescriptor d));
STATIC(void, FlattenEquivalences, (SymDescriptor d));
STATIC(void, PostProcessEquivalences, (SymDescriptor d, FortTree ft));
STATIC(void, EquivEltList, (SymDescriptor d, AST_INDEX node));
STATIC(void, find_leader, (SymDescriptor d, struct equiv_struct *obj));
STATIC(void, compute_size, (SymDescriptor d, AST_INDEX node, int *high, int *eq_pt));
STATIC(void, InvertEqMapEntry, (SymTable t, int i, SymDescriptor d));
STATIC(void, InvertEqMap, (SymDescriptor d));
STATIC(void, ReportEquivConflict, (SymDescriptor d));
STATIC(Boolean, isError, (AST_INDEX stmt));
STATIC(void, doBlockDataStmtList, (SymDescriptor d, AST_INDEX stmt_list));
STATIC(void, doProcedureStmtList, (SymDescriptor d, AST_INDEX stmt_list));
STATIC(void, doLogicalIfStmt, (SymDescriptor d, AST_INDEX stmt));
STATIC(void, doGuardStmtList, (SymDescriptor d, AST_INDEX stmt_list));
STATIC(void, doParallelDoStmtList, (SymDescriptor d, AST_INDEX stmt_list));
STATIC(void, doDoStmtList, (SymDescriptor d, AST_INDEX stmt_list));
STATIC(AST_INDEX, StmtTypeCheck, (SymDescriptor d, AST_INDEX stmt));
STATIC(void, CheckData, (SymDescriptor d, AST_INDEX stmt));
STATIC(void, CheckDataList, (SymDescriptor d, AST_INDEX list));
STATIC(void, RegisterImplicit, (SymDescriptor p, AST_INDEX stmt));
STATIC(void, CheckImplicit, (SymDescriptor d, AST_INDEX stmt));
STATIC(void, CheckCommon, (SymDescriptor d, AST_INDEX Stmt));
STATIC(void, CheckExternal, (SymDescriptor d, AST_INDEX Stmt));
STATIC(void, CheckTypeStmt, (SymDescriptor d, AST_INDEX node));
STATIC(void, CheckDimsStmt, (SymDescriptor d, AST_INDEX node));
STATIC(void, CheckSave, (SymDescriptor d, AST_INDEX Stmt));
STATIC(void, CheckParameter, (SymDescriptor d, AST_INDEX stmt));
STATIC(void, CheckIntrinsic, (SymDescriptor d, AST_INDEX Stmt));
STATIC(void, CheckEntryDefinition, (SymDescriptor d, AST_INDEX stmt));
STATIC(void, CheckStatementFunction, (SymDescriptor d, AST_INDEX stmt));
STATIC(void, CheckPrivate, (SymDescriptor d, AST_INDEX stmt));
STATIC(void, CheckAssignment, (SymDescriptor d, AST_INDEX stmt));
STATIC(void, CheckIf, (SymDescriptor d, AST_INDEX stmt));
STATIC(Boolean, AssignmentShouldBeStmtFunction, (SymDescriptor d, AST_INDEX stmt));
STATIC(Boolean, StmtFunctionShouldBeAssignment, (SymDescriptor d, AST_INDEX stmt));
STATIC(AST_INDEX, CoerceAssignmentToStmtFunc, (SymDescriptor d, AST_INDEX stmt));
STATIC(AST_INDEX, CoerceStmtFuncToAssignment, (SymDescriptor d, AST_INDEX stmt));
STATIC(Boolean, is_supported_smp_executable_stmt, (AST_INDEX stmt));
STATIC(Boolean, is_supported_smp_specification_stmt, (AST_INDEX stmt));
STATIC(void, CheckTypeStmt, (SymDescriptor d, AST_INDEX node));
STATIC(void, CheckDimsStmt, (SymDescriptor d, AST_INDEX node));
STATIC(AST_INDEX, identifier_in_var_ref, (AST_INDEX node));
STATIC(char*, get_num, (char *char_ptr, int *num_ptr));
STATIC(Boolean, InvocationIsAnIntrinsicOrGeneric, (AST_INDEX invnode));
STATIC(void, CheckEntryDefinition, (SymDescriptor d, AST_INDEX stmt));
STATIC(void, ProhibitStmtFuncContainsProcCalls, (SymTable t, int i, SymDescriptor d));
STATIC(void, ProhibitStmtFuncsContainProcCalls, (SymDescriptor d));
STATIC(void, ProhibitActualArgContainsProcCalls, (SymDescriptor d, AST_INDEX node));
STATIC(void, PrivateTableDumpOne, (int key, int value, int extra));
STATIC(void, PrivateTableDump, (cNameValueTable ht));
STATIC(Boolean, isAllowableInLogicalIf, (AST_INDEX stmt));



void FortStmtTypeCheck(TableDescriptor d, AST_INDEX stmt)
{
  /* ensure Statement checker invoked with reasonable context */
  assert((d != (TableDescriptor) 0));
  
  /* Need to:
   *
   *	(1) cruise back up the tree to find the right symbol table 
   *
   *	(2) invoke StmtTypeCheck() on the statement, in the right context
   *
   */
  
  assert(0); /* FortStmtTypeCheck not yet implemented */
}

void FortTreeTypeCheck(TableDescriptor td, FortTree ft)
{
  Boolean result;
  
  /* first, get rid of the old symbol tables */
  TableDescriptorHashTableClear(td);
  
  /* initialize count of entry points in module */
  td->NumberOfModuleEntries = 0; 
  
  /* now, walk the tree, type check it, and build new tables */
  result = WalkProcedures(ft, td, ft_Root(ft));
}

static Boolean 
WalkProcedures(FortTree ft, TableDescriptor td, AST_INDEX root)
{
  AST_INDEX 		ProcList;
  AST_INDEX 		procedure, next_procedure;
  AST_INDEX 		StmtList;
  AST_INDEX		FormalList;
  int			ProcType;
  char	*		PName;
  int			index;
  
  Boolean result = true;
  
  /* do the necessary initializations */
  ft_InitNeedProvs(ft);
  
  /* now walk the list of procedures */
  ProcList = gen_GLOBAL_get_subprogram_scope_LIST(root);
  procedure 	  = list_first(ProcList);
  
  while (procedure != AST_NIL)
  {
    int	Type, sc;
      
    next_procedure = list_next(procedure);

    sc = 0;
    PName	= NULL;
    FormalList 	= AST_NIL;
    StmtList	= AST_NIL;
    Type        = TYPE_UNKNOWN;
      
    ProcType	= gen_get_node_type(procedure);
      
    switch (ProcType)
    {
    case GEN_PLACE_HOLDER:
    case GEN_COMMENT:
      break;
	  
    case GEN_PROGRAM:
      PName = (char*) gen_get_text(gen_PROGRAM_get_name(procedure));
      StmtList     = gen_PROGRAM_get_stmt_LIST(procedure);
      sc = SC_PROGRAM;
      Type = TYPE_NONE;
      break;
   
    case GEN_SUBROUTINE:
      PName = (char *) gen_get_text(gen_SUBROUTINE_get_name(procedure));
      FormalList = gen_SUBROUTINE_get_formal_arg_LIST(procedure);
      StmtList     = gen_SUBROUTINE_get_stmt_LIST(procedure);
      sc = SC_SUBROUTINE | SC_CURRENT_PROC;
      Type = TYPE_NONE;
      break;
	  
    case GEN_FUNCTION:
      PName = (char *) gen_get_text(gen_FUNCTION_get_name(procedure));
      FormalList = gen_FUNCTION_get_formal_arg_LIST(procedure);
      StmtList     = gen_FUNCTION_get_stmt_LIST(procedure);
      sc = SC_FUNCTION | SC_CURRENT_PROC;
      break;
	  
    case GEN_BLOCK_DATA:
      PName = (char *) gen_get_text(gen_BLOCK_DATA_get_name(procedure));
      StmtList      = gen_BLOCK_DATA_get_stmt_LIST(procedure);
      sc = SC_BLOCK_DATA;
      Type = TYPE_NONE;
      break;
	  
    default:
      ft_SetSemanticErrorForStatement((FortTree)td->FortTreePtr, procedure, 
				      ft_UNEXPECTED_NODE_TYPE);
      break;
    }
      
    if (sc != 0)	/* process the subprogram */
    {
      SymDescriptor d;
	  
      td->NumberOfModuleEntries++; /* count of entry points in module */
      /* for each subprogram, allocate a SymDescriptor 
       * and fill it appropriately.
       */
	  
      if (SymDescriptorAlloc(td, PName, &d))
	tc_ERROR(d, procedure, ft_DUPLICATE_DECLARATION);

      if (sc & SC_PROGRAM)
	d->dataIsStatic = true;  /* all variables in main are static.  */
	  
      if (sc & SC_FUNCTION)
	Type = findType(d, gen_FUNCTION_get_type_len(procedure));

      index = tcSymInsert(d, PName, Type);
      if (Type != TYPE_UNKNOWN) 
	SymPutFieldByIndex(d->Table, index, SYMTAB_TYPE_STMT,  procedure);

      SymPutFieldByIndex(d->Table, index, SYMTAB_OBJECT_CLASS,  OC_IS_EXECUTABLE);
      SymPutFieldByIndex(d->Table, index, SYMTAB_STORAGE_CLASS, sc);
	  
      /* similarly, if it has formals, its time to add them */
      
      if (FormalList != AST_NIL)
	doFormals(d, index, FormalList);
      
      /* if it has a statement list, walk it */
      if (StmtList != AST_NIL)
	if (sc == SC_BLOCK_DATA) 
	  doBlockDataStmtList(d, StmtList);
	else
	  doProcedureStmtList(d, StmtList);
      
      /* Now, go back and assign types to the declarations 
       * and formals ... 
       * 
       * we can't do this on the fly without enforcing the 
       * standard's ordering restrictions ... so we make a
       * second pass over the declarations ...
       * 
       * This sort of clean-up is needed by ai
       */
      if (ft_GetState(ft) != ft_ERRONEOUS)
        if (FormalList != AST_NIL)
	  CheckDefault(d, FormalList);
      
      if (ft_GetState(ft) != ft_ERRONEOUS)
        if (StmtList != AST_NIL)
	  TypeDeclarationsCheckDefault(d, StmtList);

      if (ft_GetState(ft) != ft_ERRONEOUS)
        if (ProcType == GEN_FUNCTION)
	  CheckDefault(d, gen_FUNCTION_get_name(procedure));

      /* check stmt function ordering and guarantee that stmt functions are
	 not recursively defined */
      if (ft_GetState(ft) != ft_ERRONEOUS) CheckStmtFuncsOrdering(d);

      /* 
       * PARASCOPE LIMITATION -- BULLETPROOFING 
       * If a statement function contains a procedure call, then local
       * analysis will expand the statement function and then record information
       * for the expanded code which it then deallocates, and all is lost.
       */
      if (ft_GetState(ft) != ft_ERRONEOUS) ProhibitStmtFuncsContainProcCalls(d);
      
      /* check that referenced labels have actually been defined */
      if (ft_GetState(ft) != ft_ERRONEOUS) CheckLabelRefDefs(d);
      if (ft_GetState(ft) != ft_ERRONEOUS) CleanupClassesForVariables(d);
      if (ft_GetState(ft) != ft_ERRONEOUS) PostProcessArrayDecls(d);
      if (ft_GetState(ft) != ft_ERRONEOUS) AssignVariableSizes(d);
      if (ft_GetState(ft) != ft_ERRONEOUS) LayoutCommonBlocks(d);
      if (ft_GetState(ft) != ft_ERRONEOUS) EnsureAllCommonBlocksDefined(d);
      PostProcessEquivalences(d, ft);
      /* if (ft_GetState(ft) != ft_ERRONEOUS) AssignStackStatic(d); -- don't set these until here */

      if (ft_GetState(ft) != ft_ERRONEOUS) ft_BuildNeedProvs(ft, procedure, d);
    }
      
    /* and move on to the next procedure */
    procedure	= next_procedure;
  }
  
  /* and perform the necessary finalizations */
  
  return result;
}

static int 
findType(SymDescriptor d, AST_INDEX TypeLen)
{
  int type, len /* can't be a register */;
  char *s;
  
  if (TypeLen == AST_NIL)	/* this comes up in an implicitly-typed	*/
    return TYPE_UNKNOWN;	/*  function's header			*/
  
  if (gen_get_node_type(TypeLen) != GEN_TYPE_LEN)
    tc_ERROR(d, TypeLen, ft_TYPE_FAULT);
  
  type = gen_get_node_type(gen_TYPE_LEN_get_type(TypeLen));
  
  if (gen_TYPE_LEN_get_length(TypeLen) != AST_NIL) {
    s = gen_get_text(gen_TYPE_LEN_get_length(TypeLen));
    s = get_num( s, &len );
  }
  else len = 0;
  
  switch(type) {
  case GEN_CHARACTER:
    type = TYPE_CHARACTER;
    break;
  case GEN_LOGICAL:
    if (!(len == 0 || len == 1))
      tc_ERROR(d, TypeLen, ft_INVALID_TYPE_LENGTH);
    type = TYPE_LOGICAL;
    break;
  case GEN_INTEGER:
    if (!(len == 0 || len == 2 || len == 4))
      tc_ERROR(d, TypeLen, ft_INVALID_TYPE_LENGTH);
    type = TYPE_INTEGER;
    break;
  case GEN_REAL:
    if (!(len == 0 || len == 4 || len == 8))
      {
	tc_ERROR(d, TypeLen, ft_INVALID_TYPE_LENGTH);
	type = TYPE_REAL;
      }
    else if (len == 8)
      type = TYPE_DOUBLE_PRECISION;
    else
      type = TYPE_REAL;
    break;
  case GEN_COMPLEX:
    if (len != 0)
      tc_ERROR(d, TypeLen, ft_INVALID_TYPE_LENGTH);
    type = TYPE_COMPLEX;
    break;
  case GEN_DOUBLE_PRECISION:
    if (!(len == 0 || len == 8))
      tc_ERROR(d, TypeLen, ft_INVALID_TYPE_LENGTH);
    type = TYPE_DOUBLE_PRECISION;
    break;
  case GEN_SEMAPHORE:
    if (len != 0)
      tc_ERROR(d, TypeLen, ft_INVALID_TYPE_LENGTH);
    type = TYPE_SEMAPHORE;
    break;
  case GEN_EVENT:
    if (len != 0)
      tc_ERROR(d, TypeLen, ft_INVALID_TYPE_LENGTH);
    type = TYPE_EVENT;
    break;
  case GEN_BARRIER:
    if (len != 0)
      tc_ERROR(d, TypeLen, ft_INVALID_TYPE_LENGTH);
    type = TYPE_BARRIER;
    break;
  default:
    tc_ERROR(d, TypeLen, ft_TYPE_FAULT);
    type = TYPE_INTEGER;
    break;
  }
  return type;
}

static void 
ArrayDeclLen(SymDescriptor d, AST_INDEX node, int type, AST_INDEX stmt)
/*stmt:  node for statement being checked */
{
  AST_INDEX Var;
  AST_INDEX dimensions, dimension, next_dimension;
  int Index;
  
  Var		= gen_ARRAY_DECL_LEN_get_name(node);
  dimensions	= gen_ARRAY_DECL_LEN_get_dim_LIST(node);
  
  Index = tcSymInsert(d, gen_get_text(Var), type);
  if (type == TYPE_UNKNOWN)
    type = SymGetFieldByIndex(d->Table, Index, SYMTAB_TYPE);
  else /* this statement is trying to set its type */
    {
      /* so there are three cases */
      if (SymGetFieldByIndex(d->Table, Index, SYMTAB_TYPE_STMT) 
	  == (AST_INDEX) TYPE_STMT_IMPLICIT) {
	/* it's implicitly typed, so we should fix it up		*/
	SymPutFieldByIndex(d->Table, Index, SYMTAB_TYPE, type);
	SymPutFieldByIndex(d->Table, Index, SYMTAB_TYPE_STMT, stmt);
      } else { 
	if (SymGetFieldByIndex(d->Table, Index, SYMTAB_TYPE_STMT) == 
	    (AST_INDEX) TYPE_STMT_UNKNOWN) {
	  /* we just set its type, so we must record that fact	*/
	  SymPutFieldByIndex(d->Table, Index, SYMTAB_TYPE_STMT, stmt);
	} else {
	  /* this statement is a duplicate declaration!		*/
	  tc_ERROR(d, node, ft_DUPLICATE_DECLARATION);
	}
      }
    }
  
  if (dimensions != AST_NIL) {
    Boolean SawAStar = false;
    int ndims = 0;
    
    SymPutFieldByIndex(d->Table, Index, SYMTAB_DIM_LIST, dimensions);
    dimension = list_first( dimensions );
    
    while( dimension != AST_NIL )
      {
	AST_INDEX lbound;
	AST_INDEX ubound;
	
	next_dimension = list_next(dimension);

	ndims++;
	if (ndims >= MAX_NUM_ARRAY_DIMS) {
	  tc_ERROR(d, node, ft_TOO_MANY_DIMS);
	  ndims--;
	  break;
	}
	
	if (SawAStar != false) {
	  tc_ERROR(d, node, ft_ADJUSTABLE_DIMENSION_ERROR);
	  ndims--;
	  break;
	}
	
	lbound = gen_DIM_get_lower(dimension);
	ubound = gen_DIM_get_upper(dimension);
	
	/* and some plain old fashioned error checking */
	if (lbound == AST_NIL && ubound == AST_NIL)
	  tc_ERROR(d, node, ft_BAD_DIMENSION_SPECIFIER);
	
	/* Note:  
	 * PostProcessArrayDecl 
	 * (1) performs dimension bounds checking of ubound 
	 * (2) reports error if array is not a dummy argument of 
	 *     the procedure or one of its entries. (this error
	 *     cannot be reported until all entries have been
	 *     seen, hence we report it in PostProcessArrayDecl
	 *     instead of here)
	 */
	
	if (gen_get_node_type(ubound) == GEN_STAR) SawAStar = true;
	
	dimension = next_dimension;
      }
    
    if (SymGetFieldByIndex(d->Table, Index, SYMTAB_NUM_DIMS) == 0) {
      SymPutFieldByIndex(d->Table, Index, SYMTAB_NUM_DIMS, ndims);
      SymPutFieldByIndex(d->Table, Index, SYMTAB_DIM_STMT, stmt);
    }
    else {
      /* always report error (see X3.9-1978, sect 8.1) */
      tc_ERROR(d, node, ft_MULT_ARRAY_DECLARATORS);
    }
  }
}

static void
SetImplicitType(SymTable t, int i, SymDescriptor d)
{
  if (SymGetFieldByIndex(t, i, SYMTAB_TYPE_STMT) == TYPE_STMT_IMPLICIT) {
    char *name = (char *) SymGetFieldByIndex(t, i, SYMTAB_NAME);
    SymPutFieldByIndex(t, i, SYMTAB_TYPE, d->implicits.typedecl[name[0]].type);
  }
}


static void 
DeclarationCleanUp(SymDescriptor d)
{
  register ImplicitList	chain;
  register ImplicitList	temp;
  
  /* First, make sure we have all the implict stuff right */
  
  SymDescriptorInitializeImplicits(d);	/* start from scratch */
  
  chain = d->implicits.IList;	/* re-apply all the active ones */
  while(chain != (ImplicitList) 0) {
    if (gen_get_node_type(chain->stmt) != GEN_IMPLICIT) { 
      /* stmt on the list has changed (via editing!) */
      if (chain->next != (ImplicitList) 0)
	{ /* compress the list */
	  temp = chain->next;
	  chain->stmt = temp->stmt;
	  chain->next = temp->next;
	  free_mem((void*)temp);
	}
      /* if its the last element, we'll ignore it */
    }
    else CheckImplicit(d, chain->stmt);
    
    chain = chain->next;
  }
  SymForAll(d->Table, (SymIteratorFunc)SetImplicitType, (Generic) d);
}

static void TypeDeclarationsCheckDefault(SymDescriptor d, AST_INDEX stmts)
{
  AST_INDEX stmt, next_stmt;

  for (stmt = list_first(stmts);
       stmt != AST_NIL && (is_comment(stmt) ||
			   !is_supported_smp_executable_stmt(stmt));
       stmt = next_stmt)
  {
    next_stmt = list_next(stmt);
    CheckDefault(d, stmt);
  }
}

static void 
CheckDefault(SymDescriptor d, AST_INDEX node) /* Default case for type checking */
                                        /*      Identifier => check it!   */
{					/*	Expression => check it!	  */
  int 	    i, n;			/*	has sons?  => recurse!	  */
  AST_INDEX list;
  AST_INDEX list_elt, next_list_elt;

  switch(gen_get_node_type(node)) {
  case GEN_BINARY_AND:	
  case GEN_BINARY_CONCAT:
  case GEN_BINARY_DIVIDE:	
  case GEN_BINARY_EQ:
  case GEN_BINARY_EQV:	
  case GEN_BINARY_EXPONENT:
  case GEN_BINARY_GE:		
  case GEN_BINARY_GT:
  case GEN_BINARY_LE:		
  case GEN_BINARY_LT:
  case GEN_BINARY_MINUS:	
  case GEN_BINARY_NE:
  case GEN_BINARY_NEQV:	
  case GEN_BINARY_OR:
  case GEN_BINARY_PLUS:	
  case GEN_BINARY_TIMES:
  case GEN_COMPLEX_CONSTANT:	
  case GEN_CONSTANT:
  case GEN_IDENTIFIER:	
  case GEN_INVOCATION:
  case GEN_LABEL_DEF:		
  case GEN_LABEL_REF:
  case GEN_PLACE_HOLDER:	
  case GEN_RETURN_LABEL:
  case GEN_SUBSCRIPT:		
  case GEN_SUBSTRING:
  case GEN_TEXT:		
  case GEN_UNARY_MINUS:
  case GEN_UNARY_NOT:
    (void) tcExprCheck(d, node);
    break;
    
  case GEN_COMMON_ELT:
    /* default handling of GEN_COMMON_ELT causes common name to be
     * inserted in symbol table by a call to checkIdentifier. by 
     * treating it specially here, we avoid the insertion so the
     * common name is in the common table only 
     */
    list_elt = list_first(gen_COMMON_ELT_get_common_vars_LIST(node));
    while(list_elt != AST_NIL) {
      next_list_elt = list_next(list_elt);
      CheckDefault(d, list_elt);
      list_elt = next_list_elt;
    }
    break;
    
  default:
    if (is_list(node)) {
      list_elt = list_first(node);
      while(list_elt != AST_NIL) {
	next_list_elt = list_next(list_elt);
	CheckDefault(d, list_elt);
	list_elt = next_list_elt;
      }
    } else {
      n = gen_how_many_sons(gen_get_node_type(node));
      for (i=1;i<=n;i++)
	CheckDefault(d,ast_get_son_n(node,i));
    }
  }
}


static void
MakeStackStatic(SymDescriptor d, AST_INDEX ident) /* used for vars in DATA and SAVE */
                                                  /* -- paco 3 June 1992            */
{
    int         sc;
    STR_TEXT    text;
    fst_index_t Index;
    SymTable    t = d->Table;

    text = gen_get_text(ident);
    Index = tcSymInsert(d, text, TYPE_UNKNOWN);

    if (IS_DUMMY_PARAM_FOR_ENTRY_OR_PROCEDURE(t, Index))
	tc_ERROR(d, ident, ft_FORMAL_IN_STATIC);

    sc = SymGetFieldByIndex(d->Table, Index, SYMTAB_STORAGE_CLASS);

    if (sc & SC_STACK)
        SymPutFieldByIndex(d->Table, Index, SYMTAB_STORAGE_CLASS, SC_STATIC);
    /*
     *  else should be an error sometimes...
     *
     *  e.g., variables in DATA statements *must* be in COMMON blocks
     *  in BLOCK DATA subprograms, and must be static in other subprograms.
     */
     
}

void
ConstantSetType(AST_INDEX node)
{
  int rt;
  switch(str_get_type(gen_get_symbol(node))) {
  case STR_CONSTANT_INTEGER:
  case STR_CONSTANT_HEX:
    rt = TYPE_INTEGER;
    break;
  case STR_CONSTANT_REAL:
    rt = TYPE_REAL;
    break;
  case STR_CONSTANT_EXACT:
    rt = TYPE_EXACT;
    break;
  case STR_CONSTANT_DOUBLE_PRECISION:
    rt = TYPE_DOUBLE_PRECISION;
    break;
  case STR_CONSTANT_LOGICAL:
    rt = TYPE_LOGICAL;
    break;
  case STR_CONSTANT_CHARACTER:
    rt = TYPE_CHARACTER;
    break;
  default:
    rt = TYPE_UNKNOWN;
    break;
  }
  srt(node,rt);
  sct(node,rt);
}

static int 
tcExprCheck(SymDescriptor d, AST_INDEX node)
{
  int lht, rht, rt;
  
  if (node == AST_NIL)
    return TYPE_UNKNOWN;
  
  switch(gen_get_node_type(node)) {
  case GEN_PLACE_HOLDER:
    srt(node, TYPE_UNKNOWN);
    sct(node, TYPE_UNKNOWN);
    break;
  case GEN_SUBSCRIPT:
    /* check for subscript - invocation ambiguity 
     * node may change if a coercion is applied
     */
    node = tcCheckSubscript(d, node);
    break;
  case GEN_SUBSTRING:
    tcCheckSubstring(d, node);
    break;
  case GEN_IDENTIFIER:
    tcCheckIdentifier(d, node);
    break;
  case GEN_INVOCATION:
    /* check for subscript - invocation ambiguity 
     * node may change if a coercion is applied
     */
    node = tcCheckInvocation(d, node, SC_FUNCTION);
    break;
  case GEN_BINARY_EXPONENT:
    lht = tcExprCheck(d, gen_BINARY_EXPONENT_get_rvalue1(node));
    rht = tcExprCheck(d, gen_BINARY_EXPONENT_get_rvalue2(node));
    rt  = exp_type_map[lht][rht];
    if (rt == TYPE_ERROR)		       	/* mark the tree	*/
      {						/* and set some types	*/
	tc_ERROR(d, node, ft_TYPE_FAULT);
	srt(node, TYPE_ERROR);
	sct(node, TYPE_UNKNOWN);
      }
    else
      {
	srt(node,rt);			       	/* what we've learned	*/
	sct(node,rt);			       	/* may get changed from	*/
      }						/*   above ...		*/
    break;
  case GEN_BINARY_TIMES:
  case GEN_BINARY_DIVIDE:
  case GEN_BINARY_PLUS:
  case GEN_BINARY_MINUS:
    lht = tcExprCheck(d, gen_get_son_n(node, 1));
    rht = tcExprCheck(d, gen_get_son_n(node, 2));
    rt  = mult_type_map[lht][rht];
    if (rt == TYPE_ERROR)		       	/* mark the tree	*/
      {						/* and set some types	*/
	tc_ERROR(d, node, ft_TYPE_FAULT);
	srt(node, TYPE_ERROR);
	sct(node, TYPE_UNKNOWN);
      }
    else
      {
	sct(gen_get_son_n(node, 1),rt);	/* may coerce the sons	*/
	sct(gen_get_son_n(node, 2),rt);
	srt(node,rt);			       	/* what we've learned	*/
	sct(node,rt);			       	/* may get changed from */
      }						/*   above ...		*/
    break;
  case GEN_BINARY_CONCAT:
    lht = tcExprCheck(d, gen_BINARY_CONCAT_get_rvalue1(node));
    rht = tcExprCheck(d, gen_BINARY_CONCAT_get_rvalue2(node));
    rt  = concat_type_map[lht][rht];
    if (rt == TYPE_ERROR)			/* mark the tree     */
      {						/* and set some types	*/
	tc_ERROR(d, node, ft_TYPE_FAULT);
	srt(node, TYPE_ERROR);
	sct(node, TYPE_UNKNOWN);
      }
    else
      {
	/* may coerce the sons	*/
	sct(gen_BINARY_CONCAT_get_rvalue1(node),rt);	
	sct(gen_BINARY_CONCAT_get_rvalue2(node),rt);
	srt(node,rt);				 /* what we've learned*/
	sct(node,rt);			         /* may get changed from */
      }						 /*   above ...		*/
    /* Add a check for variable length substrings - they're 
       invalid here --JMC ???
       */
    break;
  case GEN_BINARY_AND:
  case GEN_BINARY_OR:
  case GEN_BINARY_EQV:
  case GEN_BINARY_NEQV:
    lht = tcExprCheck(d, gen_BINARY_AND_get_rvalue1(node));
    rht = tcExprCheck(d, gen_BINARY_AND_get_rvalue2(node));
    rt  = and_type_map[lht][rht];
    if (rt == TYPE_ERROR)			/* mark the tree     */
      {						/* and set some types	*/
	tc_ERROR(d, node, ft_TYPE_FAULT);
	srt(node, TYPE_ERROR);
	sct(node, TYPE_UNKNOWN);
      }
    else
      {
	sct(gen_BINARY_CONCAT_get_rvalue1(node),rt);   /* may coerce the sons*/
	sct(gen_BINARY_CONCAT_get_rvalue2(node),rt);
	srt(node,rt);				      /* what we've learned  */
	sct(node,rt);				      /* may get changed from*/
      }						/*   above ...		*/
    break;
  case GEN_BINARY_EQ:
  case GEN_BINARY_NE:
    lht = tcExprCheck(d, gen_BINARY_EQ_get_rvalue1(node));
    rht = tcExprCheck(d, gen_BINARY_EQ_get_rvalue2(node));
    rt  = eq_type_map[lht][rht];
    if (rt == TYPE_ERROR)			/* mark the tree    */
      {						/* and set some types	*/
	tc_ERROR(d, node, ft_TYPE_FAULT);
	srt(node, TYPE_ERROR);
	sct(node, TYPE_UNKNOWN);
      }
    else
      {
	lht = eq_arg_type_map[lht][rht];
	sct(gen_BINARY_TIMES_get_rvalue1(node),lht);  /* may coerce the sons */
	sct(gen_BINARY_TIMES_get_rvalue2(node),lht);
	srt(node,rt);				     /* what we've learned   */
	sct(node,rt);				     /* may get changed from */
      }						     /*   above ...	     */
    break;
  case GEN_BINARY_GE:
  case GEN_BINARY_GT:
  case GEN_BINARY_LE:
  case GEN_BINARY_LT:
    lht = tcExprCheck(d, gen_BINARY_GE_get_rvalue1(node));
    rht = tcExprCheck(d, gen_BINARY_GE_get_rvalue2(node));
    rt  = ge_type_map[lht][rht];
    if (rt == TYPE_ERROR)			    /* mark the tree    */
      {						    /* and set some types   */
	tc_ERROR(d, node, ft_TYPE_FAULT);
	srt(node, TYPE_ERROR);
	sct(node, TYPE_UNKNOWN);
      }
    else
      {
	lht = ge_arg_type_map[lht][rht];
	sct(gen_BINARY_TIMES_get_rvalue1(node),lht); /* may coerce the sons  */
	sct(gen_BINARY_TIMES_get_rvalue2(node),lht);
	srt(node,rt);				     /* what we've learned   */
	sct(node,rt);				     /* may get changed from */
      }					             /*   above ...	     */
    break;
  case GEN_UNARY_MINUS:
    lht = tcExprCheck(d, gen_UNARY_MINUS_get_rvalue(node));
    rt  = minus_type_map[lht];
    if (rt == TYPE_ERROR)				/* mark the tree     */
      {						/* and set some types	*/
	tc_ERROR(d, node, ft_TYPE_FAULT);
	srt(node, TYPE_ERROR);
	sct(node, TYPE_UNKNOWN);
      }
    else
      {
	sct(gen_UNARY_MINUS_get_rvalue(node), rt);	/* may coerce the son*/
	srt(node, rt);				/* set the real type	*/
	sct(node, rt);				/* may get changed ...	*/
      }
    break;
  case GEN_UNARY_NOT:
    lht = tcExprCheck(d, gen_UNARY_NOT_get_rvalue(node));
    rt  = not_type_map[lht];
    if (rt == TYPE_ERROR)
      {
	tc_ERROR(d, node, ft_TYPE_FAULT);
	srt(node, TYPE_ERROR);
	sct(node, TYPE_UNKNOWN);
      }
    else
      {
	sct(gen_UNARY_NOT_get_rvalue(node), rt);
	srt(node, rt);
	sct(node, rt);
      }
    break;
  case GEN_LABEL_DEF:
    tcCheckLabelDef(d, node);
    break;
  case GEN_LABEL_REF:
    tcCheckLabelRef(d, node);
    break;
  case GEN_COMPLEX_CONSTANT:
    lht = tcExprCheck(d, gen_COMPLEX_CONSTANT_get_real_const(node));
    rht = tcExprCheck(d, gen_COMPLEX_CONSTANT_get_imag_const(node));
    
    if (isConstantExpr(d, gen_COMPLEX_CONSTANT_get_real_const(node)) &&
	isConstantExpr(d, gen_COMPLEX_CONSTANT_get_imag_const(node)) )
      {
	srt(node, TYPE_COMPLEX);
	sct(node, TYPE_COMPLEX);
      }
    else
      {
	tc_ERROR(d, node, ft_INVALID_COMPLEX_CONSTANT);
	srt(node, TYPE_ERROR);
	sct(node, TYPE_COMPLEX);
      }
    /* and force the subparts to be single precision reals	*/
    sct(gen_COMPLEX_CONSTANT_get_real_const(node), TYPE_REAL);
    sct(gen_COMPLEX_CONSTANT_get_imag_const(node), TYPE_REAL);
    break;
  case GEN_CONSTANT:
    ConstantSetType(node);
    break;
  case GEN_TEXT:
  case GEN_RETURN_LABEL:
    break;
  default:
    tc_ERROR(d, node, ft_LAST_SEMANTIC_ERROR);
    break;
  }
  return gct(node);
}

static void
CheckBadProcNameContext(SymDescriptor d, AST_INDEX node, int oc, int sc)
{
  if (oc & OC_IS_EXECUTABLE) {
    if ((sc & SC_FUNCTION) && ((sc & SC_ENTRY) ||
			       (sc & SC_CURRENT_PROC)))
      /* ok for name of the enclosing function, or one
       * of its entries to appear as a variable
       */
      return;
    
    tc_ERROR(d, node, ft_PROC_NAME_INVALID_IN_EXPR);
    
    /* prevent cryptic errors from happening up one level
     * that arise from having TYPE_NONE for the converted
     * type for a subroutine name
     */
    sct(node, TYPE_UNKNOWN);
  }
}

static void
tcCheckIdentifier(SymDescriptor d, AST_INDEX node)
{
  int	temp;
  register int i;
  register AST_INDEX stmt;
  register Boolean ANIsLegal = false;
  Boolean inlist = false;
  int type;
  int oc, sc;
  
  i = tcSymInsert(d, gen_get_text(node), TYPE_UNKNOWN);

  type = SymGetFieldByIndex(d->Table, i, SYMTAB_TYPE);
  srt(node, type);
  sct(node, type);

  oc = SymGetFieldByIndex(d->Table, i, SYMTAB_OBJECT_CLASS);
  sc = SymGetFieldByIndex(d->Table, i, SYMTAB_STORAGE_CLASS);

  /* infer object class is data for identifiers that don't already have
   * an object class assigned to them.
   */
  if (!(oc & (OC_IS_EXECUTABLE | OC_IS_DATA | OC_IS_COMMON_NAME)))
    SymPutFieldByIndex(d->Table, i, SYMTAB_OBJECT_CLASS, oc | OC_IS_DATA);
  
  stmt = node;	 		/* now, we need to find out where we are */
  while(stmt != AST_NIL &&	/* for error checking ...		 */
	BOOL(is_statement(stmt) == false))
  {
    if (in_list(stmt)) 
    {
      stmt = list_head(stmt);
      inlist = true;
    } 
    else 
    {
      stmt = ast_get_father(stmt);
      temp = gen_get_node_type(stmt);
      switch (temp) 
      {
      case GEN_INVOCATION:
	if (inlist && (oc & OC_IS_EXECUTABLE)) 
	{
	  /* bad procedure parameter if
	   * (1) subroutine but not external
	   * (2) function and not external, or
	   *     not current function (if it is the current
	   *     function, it refers to the function variable,
	   *     not the function itself)
	   * (3) statement function
	   */
	  if ((sc & SC_SUBROUTINE) && (~sc & SC_EXTERNAL))
	    tc_ERROR(d, node, ft_PROC_PARAM_NO_EXTERNAL);
	  else if ((sc & SC_FUNCTION) && (~sc & SC_EXTERNAL) &&
		   /* and not current function ... */
		   !(sc & SC_CURRENT_PROC)) 
	    tc_ERROR(d, node, ft_PROC_PARAM_NO_EXTERNAL);
	  else if (sc & SC_STMT_FUNC)
	    tc_ERROR(d, node, ft_PARAM_IS_STFUNC);
	}
	return;
	break;
      case GEN_STMT_FUNCTION:
	if (node != gen_STMT_FUNCTION_get_name(stmt)) 
	  CheckBadProcNameContext(d, node, oc, sc);
	break;
      case GEN_INTRINSIC:
      case GEN_SUBROUTINE:
      case GEN_FUNCTION:
      case GEN_EXTERNAL:
      case GEN_ENTRY:
      case GEN_ARRAY_DECL_LEN:
	/* (1) array name is legal for any of these statements
	 * (2) a procedure name can legally appear in any of these 
	 *     contexts
	 */
	return;
      case GEN_PRINT:
      case GEN_WRITE:
      case GEN_READ_SHORT:
      case GEN_READ_LONG:
	/* assume that any variable reference in a READ or WRITE may
	   be a "whole array reference" -- this is, of course,
	   incorrect. */
	ANIsLegal = true;
      default:
	CheckBadProcNameContext(d, node, oc, sc);
	break;
      }
    }
  }
  
  if (SymGetFieldByIndex(d->Table, i, SYMTAB_NUM_DIMS) != 0)
  { /* declared as an array */
    if (!(ANIsLegal || is_supported_smp_specification_stmt(stmt)))
    {
      /* and its not in a location deemed to be "ok" -- this assumes
         that any variable reference in a specification statement can
         be a "whole array reference" -- this is, of course, incorrect. */

      tc_ERROR(d, node, ft_WRONG_NO_SUBSCRIPTS);
    }
  }
}

static void
tcCheckLabelRef(SymDescriptor d, AST_INDEX node)
{
  register int i;

  i = tcSymInsert(d, gen_get_text(node), TYPE_LABEL);

  SymPutFieldByIndex(d->Table, i, SYMTAB_REF_STMT, out(node));
  SymPutFieldByIndex(d->Table, i, SYMTAB_OBJECT_CLASS, OC_IS_EXECUTABLE);
  SymPutFieldByIndex(d->Table, i, SYMTAB_STORAGE_CLASS, SC_STMT_LABEL);
  /* SYMTAB_TYPE_STMT is either set to the defining statement, or
     is TYPE_STMT_UNKNOWN */

  srt(node, TYPE_LABEL);
  sct(node, TYPE_LABEL);
}

static void
tcCheckLabelDef(SymDescriptor d, AST_INDEX node)
{
  register int i;
  
  i = tcSymInsert(d, gen_get_text(node), TYPE_LABEL);
  if ((SymGetFieldByIndex(d->Table, i, SYMTAB_TYPE_STMT) != 
       (AST_INDEX) TYPE_STMT_UNKNOWN)
      && (out(node) != 
	  SymGetFieldByIndex(d->Table, i, SYMTAB_TYPE_STMT))) {
    tc_ERROR(d, node, ft_DUPLICATE_LABEL_DECLARATION);
  }
  
  SymPutFieldByIndex(d->Table, i, SYMTAB_TYPE_STMT, out(node));
  SymPutFieldByIndex(d->Table, i, SYMTAB_OBJECT_CLASS, OC_IS_EXECUTABLE);
  SymPutFieldByIndex(d->Table, i, SYMTAB_STORAGE_CLASS, SC_STMT_LABEL);
  
  srt(node, TYPE_LABEL);
  sct(node, TYPE_LABEL);
}

static  AST_INDEX
tcCheckSubscript(SymDescriptor d, AST_INDEX node)
{
  register int i, t, count;
  AST_INDEX list, name, args, New, parent;
  AST_INDEX rvalue, next_rvalue;

  name = gen_SUBSCRIPT_get_name(node);
  i = tcSymInsert(d, gen_get_text(name), TYPE_UNKNOWN);

  t = SymGetFieldByIndex(d->Table, i, SYMTAB_TYPE);
  srt(node, t);
  sct(node, t);
  srt(name, t);
  sct(name, t);

  count = SymGetFieldByIndex(d->Table, i, SYMTAB_NUM_DIMS);

  if (count == 0) /* Oops ... this is either an error, or it should be an invocation! */
  {
    parent = out(node);
    if ((is_assignment(parent) && gen_ASSIGNMENT_get_lvalue(parent) == node) 
	|| (is_equivalence(parent)))    /* there are missing cases here ! */
    {
      /* illegally subscripted variable */
      tc_ERROR(d, node, ft_WRONG_NO_SUBSCRIPTS);
      return node;
    }
    else
    {
      args = gen_SUBSCRIPT_get_rvalue_LIST(node);
      tree_replace(args, AST_NIL);
      tree_replace(name, AST_NIL);
      
      New = gen_INVOCATION(name,args);
      tree_replace(node, New);
      tree_free(node);
      
      New = tcCheckInvocation(d, New, SC_FUNCTION);	/* type check it */
      return New;					/* and get out of here */
    }
  }
  
  for (rvalue = list_first(gen_SUBSCRIPT_get_rvalue_LIST(node));
       rvalue != AST_NIL;
       rvalue = next_rvalue)
  {  
    next_rvalue = list_next(rvalue);
    count --;
    if (tcExprCheck(d, rvalue) != TYPE_INTEGER)
      sct(rvalue, TYPE_INTEGER);	          /* coerce indices to INTEGER */
  }

  if (count != 0)
    tc_ERROR(d, node, ft_WRONG_NO_SUBSCRIPTS);

  return node;
}

static void
tcCheckSubstring(SymDescriptor d, AST_INDEX node)
{
  AST_INDEX subs = gen_SUBSTRING_get_substring_name(node);
  int ntype = gen_get_node_type(subs);
  int i;
  
  if (ntype == GEN_IDENTIFIER)
    i = tcSymInsert(d, gen_get_text(subs), TYPE_UNKNOWN);
  else if (ntype == GEN_SUBSCRIPT) {
    tcCheckSubscript(d, subs);
    i = tcSymInsert(d, gen_get_text(
				    gen_SUBSCRIPT_get_name(subs)), TYPE_UNKNOWN);
  } else {
    tc_ERROR(d, node, ft_INVALID_SUBSTRING);
    return;
  }
  
  if (SymGetFieldByIndex(d->Table, i, SYMTAB_TYPE) != TYPE_CHARACTER)
    tc_ERROR(d, node, ft_BAD_TYPE);
  
  srt(node, SymGetFieldByIndex(d->Table, i, SYMTAB_TYPE));
  sct(node, SymGetFieldByIndex(d->Table, i, SYMTAB_TYPE));
  
  if (tcExprCheck(d,gen_SUBSTRING_get_rvalue1(node)) != TYPE_INTEGER)
    sct(gen_SUBSTRING_get_rvalue1(node), TYPE_INTEGER);
  
  if (tcExprCheck(d,gen_SUBSTRING_get_rvalue2(node)) != TYPE_INTEGER)
    sct(gen_SUBSTRING_get_rvalue2(node), TYPE_INTEGER);
}

static void
CheckStatementFunctionInvocation(SymDescriptor d, fst_index_t findex, 
                                 AST_INDEX node, int type)
{
  int arg_count = 0;
  AST_INDEX flist_elt = list_first(SymGetFieldByIndex(d->Table, findex, SYMTAB_FORMALS_LIST));
  AST_INDEX alist_elt  = list_first(gen_INVOCATION_get_actual_arg_LIST(node));
  AST_INDEX next_flist_elt;
  AST_INDEX next_alist_elt;

  while(alist_elt != AST_NIL)
  {
    int atype = tcExprCheck(d, alist_elt);
    
    next_flist_elt = list_next(flist_elt);
    next_alist_elt = list_next(alist_elt);

    if (flist_elt != AST_NIL) {
      int formal_index = SymQueryIndex(d->Table, gen_get_text(flist_elt));
      int ftype = 
	SymGetFieldByIndex(d->Table, formal_index, SYMTAB_TYPE);
      if (atype != ftype)
	tc_ERROR(d, node, ft_STFUNC_ARG_TYPE_MISMATCH);
      else if (atype == TYPE_CHARACTER) {
	int flen = SymbolElementSizeFromType(d, formal_index);
	if ((flen != CHAR_LEN_STAR) && 
	    (CharExprLength(d, alist_elt) != flen))
	  tc_ERROR(d, node, ft_STFUNC_CHAR_LEN_MISMATCH);
      }
    }

    /* another ParaScope restriction */
    ProhibitActualArgContainsProcCalls(d, alist_elt);
    arg_count++;

    alist_elt = next_alist_elt;
    flist_elt = next_flist_elt;
  }
  
  if (arg_count != SymGetFieldByIndex(d->Table, findex, SYMTAB_NARGS))
    tc_ERROR(d, node, ft_WRONG_STFUNC_ARG_COUNT);
}

static AST_INDEX
tcCheckInvocation(SymDescriptor d, AST_INDEX node, int context_sc)
{
  register int i, type;
  AST_INDEX list, list_elt, name, args;
  AST_INDEX next_elt;
  int external;
  int sc;
  
  name = gen_INVOCATION_get_name(node);
  
  /* subroutines get TYPE_NONE, functions are implicitly typed if
   * no type statement is present 
   */
  if (context_sc & SC_SUBROUTINE) {
    int sc;
    i = tcSymInsert(d, gen_get_text(name), TYPE_NONE);
    sc = SymGetFieldByIndex(d->Table, i, SYMTAB_STORAGE_CLASS);
    if (sc & SC_FUNCTION) {
      tc_ERROR(d, node, ft_NO_FUNC_INVOC_IN_CALL);
      srt(name, TYPE_ERROR);
      sct(name, TYPE_UNKNOWN);
      srt(node, TYPE_ERROR);
      sct(node, TYPE_UNKNOWN);
      return node;
    }
  } else {
    i = tcSymInsert(d, gen_get_text(name), TYPE_UNKNOWN);
    
    if (SymGetFieldByIndex(d->Table, i, SYMTAB_NUM_DIMS) != 0) {
      AST_INDEX New;
      /* Oops ... this should be a subscript */
      args = gen_INVOCATION_get_actual_arg_LIST(node);
      tree_replace(args, AST_NIL);
      tree_replace(name, AST_NIL);
      
      New = gen_SUBSCRIPT(name,args);
      tree_replace(node, New);
      tree_free(node);
      
      New = tcCheckSubscript(d, New);			/* type check it */
      return New;					/* and get out of here */
    } else {
      int sc = SymGetFieldByIndex(d->Table, i, SYMTAB_STORAGE_CLASS);
      if (sc & SC_SUBROUTINE) {
	tc_ERROR(d, node, ft_NO_SUBR_CALL_IN_EXPR);
	srt(name, TYPE_ERROR);
	sct(name, TYPE_UNKNOWN);
	srt(node, TYPE_ERROR);
	sct(node, TYPE_UNKNOWN);
	return node;
      }
      
    }
  }
  
  /* coerce class of object to executable */
  {
    int Class = SymGetFieldByIndex(d->Table, i, SYMTAB_OBJECT_CLASS);
    if (Class & ~(OC_IS_EXECUTABLE | OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG))
      tc_ERROR(d, node, ft_INVOCATION_NAME_CONFLICT);
    SymPutFieldByIndex(d->Table, i, SYMTAB_OBJECT_CLASS, 
		       OC_IS_EXECUTABLE | 
		       (Class & (OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG)));
  }
  
  sc = SymGetFieldByIndex(d->Table, i, SYMTAB_STORAGE_CLASS);
  external = sc & SC_EXTERNAL;
  
  /* should now go check to see if name is an intrinsic ...	*/
  /* should also set the type fields of the name node ...	*/
  
  type = TYPE_UNKNOWN; /* initially */
  
  /* only check for generic if not external (see X3.9-1978, sect 8.8) */
  if (!external)
    type = CheckGenericInvocation(d,node);  /* generic?	*/
  
  if (type != TYPE_UNKNOWN) {
    SymPutFieldByIndex(d->Table, i, SYMTAB_STORAGE_CLASS, 
		       SC_GENERIC);
  } else {
    /* only check for intrinsic if not external (see X3.9-1978, sect 8.8)*/
    if (!external)
      type = CheckIntrinsicInvocation(d,node); /* intrinsic? */
    if (type != TYPE_UNKNOWN) {
      SymPutFieldByIndex(d->Table, i, SYMTAB_STORAGE_CLASS, 
			 SC_INTRINSIC);
    } else {
      /* we expect this case: user defined routine */
      
      /* if not statement function, set type to function or subroutine 
       * (clear from the context of the invocation).
       */
      type = SymGetFieldByIndex(d->Table, i, SYMTAB_TYPE);
      
      if (sc & SC_STMT_FUNC) 
	CheckStatementFunctionInvocation(d, i, node, type);
      else {
	
	SymPutFieldByIndex(d->Table, i, SYMTAB_STORAGE_CLASS, 
			   context_sc | external);
	
	if ((type == TYPE_CHARACTER) && (context_sc & SC_FUNCTION) && 
	    (SymGetFieldByIndex(d->Table, i, SYMTAB_CHAR_LENGTH) 
	     == CHAR_LEN_STAR))
	  tc_ERROR(d, node, ft_CHAR_FN_LEN_UNDEFINED);
	
	list_elt  = list_first(gen_INVOCATION_get_actual_arg_LIST(node));
	while(list_elt != AST_NIL)
	  {
	    next_elt = list_next(list_elt);
	    (void) tcExprCheck(d,list_elt);
	    list_elt = next_elt;
	  }
      }
    }
  }
  
  srt(name, type);
  sct(name, type);
  srt(node, type);
  sct(node, type);

  return node;
}

Boolean
isConstantExpr(SymDescriptor d, AST_INDEX node)
{
  int index;
  
  switch (gen_get_node_type(node)) {
    case GEN_IDENTIFIER:       
    index = SymQueryIndex(d->Table, gen_get_text(node)); /* look up name */
    if (index == SYM_INVALID_INDEX) return false; /* not found */
    else return BOOL(IS_MANIFEST_CONSTANT(d->Table ,index)); /* defined constant? */
    case GEN_CONSTANT:
    return true;
    case GEN_BINARY_EXPONENT:
    case GEN_BINARY_TIMES:
    case GEN_BINARY_DIVIDE:
    case GEN_BINARY_PLUS:
    case GEN_BINARY_MINUS:
    case GEN_BINARY_CONCAT:
    case GEN_BINARY_AND:
    case GEN_BINARY_OR:
    case GEN_BINARY_EQ:
    case GEN_BINARY_NE:
    case GEN_BINARY_GE:
    case GEN_BINARY_GT:
    case GEN_BINARY_LE:
    case GEN_BINARY_LT:
    case GEN_BINARY_EQV:
    case GEN_BINARY_NEQV:
    return BOOL(isConstantExpr(d, gen_get_son_n(node, 1)) &&
		isConstantExpr(d, gen_get_son_n(node, 2)));
    case GEN_UNARY_MINUS:
    case GEN_UNARY_NOT:
    return BOOL(isConstantExpr(d, gen_UNARY_NOT_get_rvalue(node)));
    default:
    return false;
  }
}

static int 
CheckIntrinsicInvocation(SymDescriptor d, AST_INDEX node)
{
  intrinsic_descriptor *iinfo;
  static intrinsic_descriptor *ichar_desc = (intrinsic_descriptor *) 0;
  
  AST_INDEX	list;
  AST_INDEX	id;
  char *name;
  int i, ArgType, type;
  int TypeFault = ft_NO_ERROR;
  AST_INDEX elt, next_elt;
  
  /* get descriptor for intrinsic ichar which must be handled
   * specially
   */
  if (ichar_desc == (intrinsic_descriptor *) 0)
    ichar_desc = builtins_intrinsicFunctionInfo("ichar");
  
  id = gen_INVOCATION_get_name(node);
  name = gen_get_text(id);
  
  iinfo = builtins_intrinsicFunctionInfo(name);
  if (iinfo == (intrinsic_descriptor *) 0) /* not an intrinsic */
    return TYPE_UNKNOWN;
  
  /* check for Argument types and count */
  elt = list_first(gen_INVOCATION_get_actual_arg_LIST(node));
  
  /* special test for intrinsic ICHAR */
  if ((iinfo == ichar_desc) && (gen_get_node_type(elt) == GEN_IDENTIFIER)) {
    if (SymGetField(d->Table, gen_get_text(elt), SYMTAB_TYPE) == 
	TYPE_CHARACTER) {
      if (SymGetField(d->Table, gen_get_text(elt), SYMTAB_CHAR_LENGTH) 
	  != 1)
	tc_ERROR(d, node, ft_ICHAR_ARG_LEN);
    } else TypeFault = 1;
  }
  
  i = 0;
  ArgType = TYPE_UNKNOWN;

  for (; elt != AST_NIL; elt = next_elt)
  {/* walk carefully -- CARLE */
    next_elt = list_next(elt);
    i++;
    type = tcExprCheck(d,elt);
    if (ArgType == TYPE_UNKNOWN)
      ArgType = type;
    else if (type != ArgType) {
      ArgType = TYPE_UNKNOWN;
      TypeFault = ft_BUILTIN_INCOMPAT_ARG_TYPES;
    }
  }
  
  switch(iinfo->numArgs) {
  case NARGS_MORE_THAN_2: 
    if (i < 2) TypeFault = ft_BUILTIN_WRONG_NARGS;
    break;
  default:
    if (iinfo->numArgs != i) TypeFault = ft_BUILTIN_WRONG_NARGS;
    break;
  }

  if (iinfo->argType != ArgType)
      TypeFault = ft_BUILTIN_INVALID_ARG_TYPE;

  if (TypeFault == ft_NO_ERROR) return iinfo->resType;
  else {
    tc_ERROR(d, node, TypeFault);
    return TYPE_ERROR;
  }
  
}

static int 
CheckGenericInvocation(SymDescriptor d, AST_INDEX node)
/* passed the invocation node */
{
  generic_descriptor *ginfo;
  AST_INDEX list;
  AST_INDEX id;
  char *name;
  int i;
  int ArgType, type, ResultType;
  int TypeFault = ft_NO_ERROR;
  AST_INDEX elt, next_elt;
  Boolean   type_valid;

  id   = gen_INVOCATION_get_name(node);
  name = gen_get_text(id);
  
  ginfo = builtins_genericFunctionInfo(name);
  
  if (ginfo == (generic_descriptor *) 0) return TYPE_UNKNOWN;
  
  /* check for Argument types and count */
  list = gen_INVOCATION_get_actual_arg_LIST(node);
  i = 0;
  ArgType = TYPE_UNKNOWN;

  for (elt = list_first(list); elt != AST_NIL; elt = next_elt)
  {
    next_elt = list_next(elt);
    i++;
    type = tcExprCheck(d,elt);
    if (ArgType == TYPE_UNKNOWN) 
      ArgType = type;
    else if (type != ArgType) 
      TypeFault = ft_BUILTIN_INCOMPAT_ARG_TYPES;
  }
  
  switch(ginfo->numArgs) {
  case NARGS_MORE_THAN_2:
    if (i < 2) TypeFault = ft_BUILTIN_WRONG_NARGS;
    break;
  case NARGS_CMPLX_1_OR_2:
    if ((ArgType == TYPE_COMPLEX) && (i != 1)) 
	  TypeFault = ft_BUILTIN_WRONG_NARGS;
    else if (i != 2) 
	  TypeFault = ft_BUILTIN_WRONG_NARGS;
    break;
  default:
    if (i != ginfo->numArgs) TypeFault = ft_BUILTIN_WRONG_NARGS;
    break;
  }

  /* 
   * check if argument type is valid, if so, save it in ResultType 
   * since it may be needed as the result type
   */
  type_valid = false;
  for (i=0; i<GENERIC_DESC_NARGS; i++) {
    if (ginfo->args[i] == ArgType)
      type_valid = true;
  }

  if (!type_valid)
    TypeFault = ft_BUILTIN_INVALID_ARG_TYPE;
  else
  {
    /* determine the result type for an invocation of this generic */
    ResultType = generic_result_type(ginfo, ArgType);
  }

  if (TypeFault == ft_NO_ERROR) return ResultType;
  else {
    tc_ERROR(d, node, TypeFault);
    return TYPE_ERROR;
  }
}

static void 
doFormals(SymDescriptor d, int pindex, AST_INDEX list) 
{
  STR_TEXT	text;
  int	        index;
  AST_INDEX     list_elt, next_list_elt;
  int           arg_position = 1; /* first arg in position 1 */
  
  /* allocate a hash table for formal parameters */
  cNameValueTable ht = NameValueTableAlloc(8, (NameCompareCallback)strcmp, 
                                           (NameHashFunctCallback)hash_string);
  
  /* associate the hash table with the entry point */
  SymPutFieldByIndex(d->Table, pindex, SYMTAB_FORMALS_HT, (Generic) ht);
  
  SymPutFieldByIndex(d->Table, pindex, SYMTAB_FORMALS_LIST, list);
  
  list_elt = list_first(list);	/* arg checked for == NIL before call */
  
  while (list_elt != AST_NIL) {
    Generic oldvalue;
    int oc;

    next_list_elt  = list_next( list_elt );
    
    switch(gen_get_node_type(list_elt)) {
      case GEN_STAR:
      arg_position++;
      break;
      case GEN_IDENTIFIER:
      text  = gen_get_text( list_elt );
      index = tcSymInsert(d, text, TYPE_UNKNOWN);
      
      if (NameValueTableAddPair(ht, (Generic)text, (Generic)arg_position++, &oldvalue))
	tc_ERROR(d, list_elt, ft_DUPLICATE_ARGUMENTS);
      
      oc = SymGetFieldByIndex(d->Table, index, SYMTAB_OBJECT_CLASS);
      SymPutFieldByIndex(d->Table, index, SYMTAB_OBJECT_CLASS, 
			 oc | OC_IS_FORMAL_PAR);
      break;
      default:
      assert(0); /* JMC -- I think we shouldn't ever be here */
    }
    
    list_elt  = next_list_elt;
  }
}

void 
tc_ERROR(SymDescriptor d, AST_INDEX node, int code) 
/* a simplified error recording routine */
{
  if (code < ft_NO_ERROR || ft_LAST_SEMANTIC_ERROR < code)
    code = ft_BAD_ERROR_CODE;
  
  d->Errors++;
  ft_SetSemanticErrorForStatement((FortTree) ((TableDescriptor)(d->parent_td))->FortTreePtr, 
				  node, code );
}

static int 
tcSymInsert(SymDescriptor d, char *name, int type)
{
  SymTable table = d->Table;
  
  /*  query the name in the symbol table  */
  int index = SymQueryIndex(table,name);
  
  if (index == SYM_INVALID_INDEX) { /* not found */
    
    index = SymIndex(table, name); /* create new entry */
    
    /* set SYMTAB_TYPE and SYMTAB_TYPE_STMT as appropriate */
    if (type <= TYPE_UNKNOWN || TYPE_LAST <= type) {
      SymPutFieldByIndex(table, index, SYMTAB_TYPE, 
			 d->implicits.typedecl[name[0]].type);
      SymPutFieldByIndex(table, index, SYMTAB_TYPE_STMT, TYPE_STMT_IMPLICIT);

    } else {
      SymPutFieldByIndex(table, index, SYMTAB_TYPE, type);
      SymPutFieldByIndex(table, index, SYMTAB_TYPE_STMT, TYPE_STMT_UNKNOWN);
    }
    
    /* initialize SYMTAB_OBJECT_CLASS */ 
    SymPutFieldByIndex(table, index, SYMTAB_OBJECT_CLASS, OC_UNDEFINED);

    /* initialize SYMTAB_STORAGE_CLASS -- paco */
    SymPutFieldByIndex(table, index, SYMTAB_STORAGE_CLASS, SC_STACK);
  }
  else { /* found --  adjust type as needed */
    if (TYPE_UNKNOWN < type && type < TYPE_LAST &&
	SymGetFieldByIndex(table, index, SYMTAB_TYPE) != type) {
      if (SymGetFieldByIndex(table, index, SYMTAB_TYPE_STMT) == 
	  TYPE_STMT_IMPLICIT) {
	SymPutFieldByIndex(table, index, SYMTAB_TYPE, type);
	SymPutFieldByIndex(table, index, SYMTAB_TYPE_STMT, TYPE_STMT_UNKNOWN);
      }
    }
  }
  return index;
}


static int 
tcCommonInsert(SymDescriptor d, char *name, AST_INDEX common_name_ref)
{
  SymTable table = d->Table;
  register int index;
  int oc;  

  index = SymQueryIndex(table, name);
  
  if (index == SYM_INVALID_INDEX)  /* not found, so allocate a new one */
    {
    index = SymIndex(table, name);
 
    SymPutFieldByIndex(table, index, SYMTAB_SIZE, 0);
    SymPutFieldByIndex(table, index, SYMTAB_OBJECT_CLASS, OC_IS_COMMON_NAME);
    SymPutFieldByIndex(table, index, SYMTAB_FIRST_NAME, SYM_INVALID_INDEX);
    SymPutFieldByIndex(table, index, SYMTAB_LAST_NAME, SYM_INVALID_INDEX);
    SymPutFieldByIndex(table, index, SYMTAB_NUM_COMMON_VARS, 0);
    SymPutFieldByIndex(table, index, SYMTAB_COMMON_STMT, AST_NIL);
    SymPutFieldByIndex(table, index, SYMTAB_COMMON_NAME_FIRST_USE, common_name_ref);

    /* the default for a blank common is static, otherwise it is stack */
    if (strcmp(name, BLANK_COMMON))
       SymPutFieldByIndex(table, index, SYMTAB_STORAGE_CLASS, SC_STACK);
    else
       SymPutFieldByIndex(table, index, SYMTAB_STORAGE_CLASS, SC_STATIC);
  }
  
  return index;
}


static void 
tcCommonAddVar(SymDescriptor d, int common_block, int var_index)
{
  assert(common_block != SYM_INVALID_INDEX);
  
  /* record info at variable identifying that it is in a common block */
  SymPutFieldByIndex(d->Table, var_index, SYMTAB_PARENT, common_block);
  SymPutFieldByIndex(d->Table, var_index, SYMTAB_OBJECT_CLASS, OC_IS_DATA);
  SymPutFieldByIndex(d->Table, var_index, SYMTAB_NEXT_COMMON, 
		     SYM_INVALID_INDEX);
  
  /* update list of variables for the common block */
  if (SymGetFieldByIndex(d->Table, common_block, SYMTAB_FIRST_NAME) == 
      SYM_INVALID_INDEX) {
    SymPutFieldByIndex(d->Table, common_block, SYMTAB_FIRST_NAME, 
		       var_index);
    SymPutFieldByIndex(d->Table, common_block, SYMTAB_LAST_NAME, 
		       var_index);
  } else {
    SymPutFieldByIndex(d->Table, 
		       SymGetFieldByIndex(d->Table, common_block, 
					  SYMTAB_LAST_NAME),
		       SYMTAB_NEXT_COMMON, var_index);
    SymPutFieldByIndex(d->Table, common_block, SYMTAB_LAST_NAME, 
		       var_index);
  }
  SymPutFieldByIndex(d->Table, common_block, SYMTAB_NUM_COMMON_VARS, 
		     SymGetFieldByIndex(d->Table, common_block, 
					SYMTAB_NUM_COMMON_VARS) + 1);
}



static void 
CleanupVariableObjectClass(SymTable t, int i, SymDescriptor d)
{
  int oc = SymGetFieldByIndex(t, i, SYMTAB_OBJECT_CLASS);
  int sc = SymGetFieldByIndex(t, i, SYMTAB_STORAGE_CLASS);
  
  
  /*
   * in this implementation, common names are entered in the symbol
   * table as /name/, so common name space is distinct from variable
   * name space
   */
  if (oc & OC_IS_COMMON_NAME) return;
  
  /* statement label can't be a variable name either */
  if (sc & SC_STMT_LABEL) return;
  
  /* a symbolic name is the name of a variable if it meets all of the 
   * following conditions:
   */
   
   /*  (1)  doesn't appear in a PARAMETER, INTRINSIC, or EXTERNAL statement 
    *       (in this case, variable already has an object class) 
    */
    
    if (sc & (SC_CONSTANT |						/* parameter */
	      (SC_INTRINSIC | SC_GENERIC) | 		/* intrinsic */
	      SC_EXTERNAL)) 						/* external */
      return;
  
  /*  (2)  it is not the name of an array, subroutine, main program, or
   *       block data subprogram
   */
  
  if (IS_ARRAY(t, i) | 						/* array */
      (sc & (SC_SUBROUTINE | 					/* subroutine */
	     SC_PROGRAM | 						/* main program */
	     SC_BLOCK_DATA))) 					/* block data subprogram */
    return;
  
  /*  (3)  it appears other than as the name of a common block, the name of 
   *       an external function in a FUNCTION statement, or an entry name
   *       in an ENTRY statement in an external function
   *
   *   note: we don't set the OBJECT_CLASS to OC_IS_DATA for functions
   *         and function entries to avoid confusion. these two cases
   *         can be handled separately.
   *
   *         since the common block name space in the symbol table is 
   *         distinct from that of variables (as described above), no
   *         test is required for common blocks 
   */
  
  /*  (4)  it is never immediately followed by a left paren unless it is
   *       immediately preceded by the FUNCTION or ENTRY keyword or is
   *       at the beginning of a character substring name
   *
   *  note: a variable cannot have the same name as a function called by the 
   *        current function, nor can it have the same name as a statement
   *        function
   *
   *        character substrings will not be thought of as arrays without
   *        an array declaration, so we don't have to worry about character
   *        substrings at all here
   */
  
  if (sc & SC_FUNCTION) return;
  
  if (sc & SC_STMT_FUNC) return;
  
  SymPutFieldByIndex(t, i, SYMTAB_OBJECT_CLASS, oc | OC_IS_DATA);
  
}

static void 
CleanupVariableStorageClass(SymTable t, int i, SymDescriptor d)
{
  int oc, sc;

  if (d->dataIsStatic)
  {
     oc = SymGetFieldByIndex(t, i, SYMTAB_OBJECT_CLASS);
     sc = SymGetFieldByIndex(t, i, SYMTAB_STORAGE_CLASS);

     if  (oc & OC_UNDEFINED) return;
     if  (oc & OC_IS_EXECUTABLE) return;
     if  (oc & OC_IS_FORMAL_PAR) return;
     if  (oc & OC_IS_STFUNC_ARG) return;
     if  (oc & OC_IS_ENTRY_ARG) return;
     if  (sc & SC_CONSTANT) return;
     if  (sc & SC_NO_MEMORY) return;
     if  (sc & SC_STATIC) return;
     if (((oc & OC_IS_DATA) == 0) && ((oc & OC_IS_COMMON_NAME) == 0)) return; 

     if  (sc & SC_STACK) sc = sc - SC_STACK;
     sc = sc | SC_STATIC;

     SymPutFieldByIndex(t, i, SYMTAB_STORAGE_CLASS, sc);
     
  }
}

static void
CheckStmtFuncBody(SymDescriptor d, AST_INDEX defstmt, AST_INDEX node)
{
  int sc;
  int sym_index;

  if (is_identifier(node))
  {
    sym_index = SymIndex(d->Table, gen_get_text(node));
    sc = SymGetFieldByIndex(d->Table, sym_index, SYMTAB_STORAGE_CLASS);  
    if (sc & SC_STMT_FUNC)
    {/* stmt func referenced by body of statement function
	was defined too late */
      tc_ERROR(d, defstmt, ft_STMT_FUNC_ORDER);
    }
  }
  else if (is_list(node))
  {
    AST_INDEX elt, next_elt;
    for (elt = list_first(node); elt != AST_NIL; elt = next_elt)
    {
      next_elt = list_next(elt);
      CheckStmtFuncBody(d, defstmt, elt);
    }
  }
  else 
  {
    int n = gen_how_many_sons(gen_get_node_type(node));
    int i;
    for (i=1;i<=n;i++)
      CheckStmtFuncBody(d, defstmt, ast_get_son_n(node,i));      
  }
}

static void 
CheckStmtFuncOrdering(SymTable t, int i, SymDescriptor d)
{
  AST_INDEX body, expr, defstmt;
  int sc = SymGetFieldByIndex(t, i, SYMTAB_STORAGE_CLASS);

  if (sc & SC_STMT_FUNC)
  {
    body = SymGetFieldByIndex(t, i, SYMTAB_SF_EXPR);
    /* get rhs of statement function definition */
    expr = SymGetFieldByIndex(t, i, SYMTAB_EXPR);
    /* get the statement itself */
    defstmt = out(expr);
    CheckStmtFuncBody(d, defstmt, body);
  }
}  

static void
CheckStmtFuncsOrdering(SymDescriptor d)
{
  SymForAll(d->Table, (SymIteratorFunc)CheckStmtFuncOrdering, (Generic) d);
}

static void
CleanupClassesForVariables(SymDescriptor d)
{
  SymForAll(d->Table, (SymIteratorFunc)CleanupVariableObjectClass, (Generic) d);
  SymForAll(d->Table, (SymIteratorFunc)CleanupVariableStorageClass, (Generic) d);
}

static Boolean 
isDimExpr(SymDescriptor d, AST_INDEX node)
{
  int  i;
  
  switch(gen_get_node_type(node))
    {
      case GEN_IDENTIFIER:
      i = SymQueryIndex(d->Table, gen_get_text(node));
      if (i != SYM_INVALID_INDEX) { /* found */
	int sc = SymGetFieldByIndex(d->Table, i, SYMTAB_STORAGE_CLASS);
	int oc = SymGetFieldByIndex(d->Table, i, SYMTAB_OBJECT_CLASS);
	int isScalar = (SymGetFieldByIndex(d->Table, i, SYMTAB_DIM_LIST)
			== AST_NIL);
	
	/* must be either
	 * (1) constant  or 
	 * (2) scalar dummy argument
	 * (3) scalar common variable 
	 */
	if (sc & SC_CONSTANT) return true;
	else if (isScalar) {
	  if ((oc & (OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG))) return true;
	  else {
	    /* check if variable is in common */
	    int leader = SymGetFieldByIndex(d->Table, i,SYMTAB_PARENT);
	    if ((leader != SYM_INVALID_INDEX) && 
		(SymGetFieldByIndex(d->Table, leader, 
				    SYMTAB_OBJECT_CLASS) &
		 OC_IS_COMMON_NAME)) 
	      return true;
	  }
	}
      }
      return false;
      case GEN_CONSTANT:
      return true;
      case GEN_BINARY_TIMES:
      case GEN_BINARY_DIVIDE:
      case GEN_BINARY_PLUS:
      case GEN_BINARY_MINUS:
      case GEN_BINARY_EXPONENT:
      return BOOL(isDimExpr(d, gen_BINARY_TIMES_get_rvalue1(node)) &&
		  isDimExpr(d, gen_BINARY_TIMES_get_rvalue2(node)));
      case GEN_UNARY_MINUS:
      return isDimExpr(d, gen_UNARY_MINUS_get_rvalue(node));
      default:
      return false; 
    }
}


static void 
PostProcessArrayDecl(SymTable t, int index, SymDescriptor d)
{
  AST_INDEX dimensions, dimension, next_dimension;
  ArrayBound dims[MAX_NUM_ARRAY_DIMS];
  Boolean DimError = false;
  Boolean SawAStar = false;
  int ndims = 0;
  Boolean arrayNotFormal = 
    NOT(SymGetFieldByIndex(t, index, SYMTAB_OBJECT_CLASS) & 
	(OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG));
  
  if (SymGetFieldByIndex(t, index, SYMTAB_DIM_STMT) == 0)
    return; /* not an array */
  
  dimensions = SymGetFieldByIndex(t, index, SYMTAB_DIM_LIST);
  dimension = list_first( dimensions );
  
  assert(dimension != AST_NIL);
  
  for( ; dimension != AST_NIL; dimension = next_dimension) 
  {
    AST_INDEX lbound;
    AST_INDEX ubound;

    next_dimension = list_next(dimension);
    ndims++;
    if (ndims >= MAX_NUM_ARRAY_DIMS) {
      DimError = true; /* error reported during initial pass */
      continue;
    }
    
    if (SawAStar != false) {
      DimError = true; /* error reported during initial pass */
      continue;
    }
    
    lbound = gen_DIM_get_lower(dimension);
    ubound = gen_DIM_get_upper(dimension);
    
    /* and some plain old fashioned error checking */
    if (lbound == AST_NIL && ubound == AST_NIL)
      tc_ERROR(d, lbound, ft_BAD_DIMENSION_SPECIFIER);
    
    if (lbound != AST_NIL) {
      if (isConstantExpr(d, lbound) && !evalConstantIntExpr(d, lbound, 
							    &dims[ndims-1].lb.value.const_val)) {
	dims[ndims-1].lb.type = constant;
      } else if (arrayNotFormal || !isDimExpr(d, lbound)) {
	dims[ndims-1].lb.type = dim_error;
	tc_ERROR(d, lbound, ft_ADJUSTABLE_DIMENSION_ERROR);
	DimError = true;
      } else {
	dims[ndims-1].lb.type = symbolic_expn_ast_index;
	dims[ndims-1].lb.value.ast = lbound;
      }
    } else {
      dims[ndims-1].lb.type = constant;
      dims[ndims-1].lb.value.const_val = 1;
    }
    
    if (gen_get_node_type(ubound) == GEN_STAR)
      {
	dims[ndims-1].ub.type = star;
	if (arrayNotFormal) {
	  tc_ERROR(d, ubound, ft_ADJUSTABLE_DIMENSION_ERROR);
	  DimError = true;
	} else  SawAStar = true;
      }
    else if (ubound != AST_NIL) {
      if (isConstantExpr(d, ubound) && !evalConstantIntExpr(d, ubound, 
							    &dims[ndims-1].ub.value.const_val)) {
	dims[ndims-1].ub.type = constant;
      } else if (arrayNotFormal || !isDimExpr(d, ubound)) {
	dims[ndims-1].ub.type = dim_error;
	tc_ERROR(d, ubound, ft_ADJUSTABLE_DIMENSION_ERROR);
	DimError = true;
      } else {
	dims[ndims-1].ub.type = symbolic_expn_ast_index;
	dims[ndims-1].ub.value.ast = ubound;
      }
    }
    
    if ((dims[ndims-1].ub.type == constant) && 
	(dims[ndims-1].lb.type == constant) && 
	(dims[ndims-1].ub.value.const_val < 
	 dims[ndims-1].lb.value.const_val)) {
      tc_ERROR(d, dimension, ft_ARRAY_UPPER_LT_LOWER);
      DimError = true;
    }
  }
  
  if (DimError == false) {
    int size = sizeof(ArrayBound) * ndims;
    ArrayBound *dimarray = 
      (ArrayBound *) get_mem(size, "Array dimension bounds");
    bcopy(dims, dimarray, size);
    SymPutFieldByIndex(d->Table, index, SYMTAB_DIM_BOUNDS, (Generic) dimarray);
  }
}

/* ensure that all arrays that are not formal parameters have solid dimensions.
 * arrays that are formals may have symbolic expressions in terms of scalar
 * parameters, or a '*' as the upper bound in the last dimension
 *
 * this checking is deferred so that all PARAMETER statements are processed
 * before array dimensions are checked so all manifest constants are
 * known when the dimension expressions are checked 
 */
static void 
PostProcessArrayDecls(SymDescriptor d)
{
  SymForAll(d->Table, (SymIteratorFunc)PostProcessArrayDecl, (Generic) d);
}


static int
CharExprLength(SymDescriptor d, AST_INDEX expr)
{
  switch(gen_get_node_type(expr))	{
    case GEN_BINARY_CONCAT:
    return (CharExprLength(d, gen_BINARY_CONCAT_get_rvalue1(expr)) +
	    CharExprLength(d, gen_BINARY_CONCAT_get_rvalue2(expr)));
    case GEN_IDENTIFIER:
    {
      int index = SymQueryIndex(d->Table, gen_get_text(expr));
      return SymbolElementSizeFromType(d, index);
    }
    case GEN_CONSTANT:
    if (gen_get_real_type(expr) == TYPE_CHARACTER)
      {
	/* character constants stored as "'...'". therefore, the
	 * length must be adjusted to account for the presence of 
	 * apostrophes 
	 */
	char *s = gen_get_text(expr);
	return strlen(s) - 2;
      }
    default:
    assert(0);
    return 0; /* return value to avoid compiler warnings */
  }
}


static int
ConstCharExprLength(SymDescriptor d, AST_INDEX expr)
{
  switch(gen_get_node_type(expr))	{
    case GEN_BINARY_CONCAT:
    return (ConstCharExprLength(d, gen_BINARY_CONCAT_get_rvalue1(expr)) +
	    ConstCharExprLength(d, gen_BINARY_CONCAT_get_rvalue2(expr)));
    case GEN_IDENTIFIER:
    {
      SymTable t = d->Table;
      int index = SymQueryIndex(t, gen_get_text(expr));
      if (!IS_MANIFEST_CONSTANT(t, index)) {
	tc_ERROR(d, expr, ft_BAD_CHAR_CONST_EXPR);
	return 0;
      } else {
	if (SymGetFieldByIndex(t, index, SYMTAB_PARAM_STATUS) ==
	    PARAM_IN_EVALUATION) {
	  tc_ERROR(d, expr, ft_RECURSIVE_PARAMETER_DEFN);
	} else {
	  int len;
	  SymPutFieldByIndex(t, index, SYMTAB_PARAM_STATUS, 
			     PARAM_IN_EVALUATION);
	  len = ConstCharExprLength(d, 
				    SymGetFieldByIndex(t, index, SYMTAB_EXPR));
	  SymPutFieldByIndex(t, index, SYMTAB_PARAM_STATUS, 
			     PARAM_UNDEFINED);
	  return len;
	}
      }
    }
    case GEN_CONSTANT:
    if (gen_get_real_type(expr) == TYPE_CHARACTER)
      {
	/* character constants stored as "'...'". therefore, the
	 * length must be adjusted to account for the presence of 
	 * apostrophes 
	 */
	char *s = gen_get_text(expr);
	return strlen(s) - 2;
      }
    default:
    tc_ERROR(d, expr, ft_BAD_CHAR_CONST_EXPR);
    return 0;
  }
}

static int
SymbolElementSizeFromType(SymDescriptor d, int index)
{
  SymTable t = d->Table;
  int size, len; 
  int type = SymGetFieldByIndex(t, index, SYMTAB_TYPE);
  
  /* initially determine size by data type */ 
  if(type == TYPE_CHARACTER) {
    /* size of character string type depends on its length */
    len = SymGetFieldByIndex(t, index, SYMTAB_CHAR_LENGTH);
    if (len == CHAR_LEN_STAR) {
      int oc = SymGetFieldByIndex(t, index, SYMTAB_OBJECT_CLASS);
      int sc = SymGetFieldByIndex(t, index, SYMTAB_STORAGE_CLASS);
      int expr = SymGetFieldByIndex(t, index, SYMTAB_EXPR);
      
      /* character constant with length specified implicitly by 
       * a constant character expression
       */
      if (sc & SC_CONSTANT) {
	len = ConstCharExprLength(d, expr);
	SymPutFieldByIndex(t, index, SYMTAB_CHAR_LENGTH, len);
      }
      else {
	/* STAR length OK if
	 *  (1) dummy argument (of statement function, or external 
	 *      procedure/entry)
	 *  (2) name of external entry procedure
	 *  (3) name of current procedure
	 */
	if (!((oc & (OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG | 
		     OC_IS_STFUNC_ARG)) ||
	      ((oc & OC_IS_EXECUTABLE) & (sc & SC_ENTRY)) ||
	      (sc & SC_CURRENT_PROC))) {
	  tc_ERROR(d, SymGetFieldByIndex(t, index, SYMTAB_TYPE_STMT),
		   ft_CHAR_LEN_SPEC_REQUIRED);
	}
      }
    }
    size = SIZE_PER_CHAR * len;
  } else {
    assert((type > 0) && (type < TYPE_LAST)); /* one we know about ... */
    size = type_to_size_in_bytes[type];
  }
  return size;
}

/* compute the size of a variable based on its type and dimensions (if any) 
 * do not report any errors -- type checker will already have done so
 */
static int 
VariableSize(SymDescriptor d, int index)
{
  SymTable t = d->Table;
  int elements = 1;
  int ndims;
  ArrayBound *bounds;
  
  int size = SymbolElementSizeFromType(d, index);
  
  ndims =  SymGetFieldByIndex(t, index, SYMTAB_NUM_DIMS);
  bounds = (ArrayBound *) SymGetFieldByIndex(t, index, SYMTAB_DIM_BOUNDS);
  
  /* adjust the number of elements if variable is an array */
  if ((ndims != 0) && (bounds != (ArrayBound *) 0)) {
    for(; --ndims >= 0;) {
      if (bounds[ndims].lb.type == constant && 
	  bounds[ndims].ub.type == constant) {
	elements *= (bounds[ndims].ub.value.const_val - 
		     bounds[ndims].lb.value.const_val) + 1;
      }
      else 
	/* dim_error cases filtered before this stage; 
	   if dimension is non-constant here, it is a legal
	   adjustable array -- JMC 3/93 */
	return FST_ADJUSTABLE_SIZE; 
    }
  }
  return elements * size; /* total size */
}

/* assign variable size to each firmly sized object */
static void
CheckLabelRefDef(SymTable t, int i, SymDescriptor d)
{
  int sc = SymGetFieldByIndex(t, i, SYMTAB_STORAGE_CLASS);
  if (sc & SC_STMT_LABEL)
  {
    /* if this is AST_NIL, then the label has not been defined, but it has been referenced */
    if (SymGetFieldByIndex(t, i, SYMTAB_TYPE_STMT)
	== (AST_INDEX) TYPE_STMT_UNKNOWN)
      tc_ERROR(d,
	       SymGetFieldByIndex(t, i, SYMTAB_REF_STMT),
	       ft_LABEL_REF_NOT_DEFINED);
  }
}

/* assign sizes to each variable in a procedure's symbol table */
static void
CheckLabelRefDefs(SymDescriptor d)
{
  SymForAll(d->Table, (SymIteratorFunc)CheckLabelRefDef, 
            (Generic) d);/* for each variable */
}


/* assign variable size to each firmly sized object */
static void
AssignVariableSize(SymTable t, int i, SymDescriptor d)
{
  int oc = SymGetFieldByIndex(t, i, SYMTAB_OBJECT_CLASS);
  int sc = SymGetFieldByIndex(t, i, SYMTAB_STORAGE_CLASS);
  
  /* don't set size for generics since they may not be the same everywhere */
  if (oc & OC_IS_DATA || (sc & (SC_FUNCTION | SC_INTRINSIC)))
    SymPutFieldByIndex(t, i, SYMTAB_SIZE, VariableSize(d, i));
}
	
/* assign sizes to each variable in a procedure's symbol table */
static void
AssignVariableSizes(SymDescriptor d)
{
  SymForAll(d->Table, (SymIteratorFunc)AssignVariableSize, 
            (Generic) d);/* for each variable */
}


/* compute offset and size fields for each variable in a common block */
static void
LayoutOneCommonBlock(SymTable t, int common_index, Generic dummy)
{
  int  var_index;
  int offset = /* initial offset of first common block variable */ 0; 
  
  /* if symbol is not name of common block  we are done */
  if (!(SymGetFieldByIndex(t, common_index, SYMTAB_OBJECT_CLASS) & 
	OC_IS_COMMON_NAME)) 
    return;
  
  /* symbol table index of first variable in common block */
  var_index = SymGetFieldByIndex(t, common_index, SYMTAB_FIRST_NAME);
  
  /* for each variable in the common block */
  while(var_index != SYM_INVALID_INDEX) { 
    /* assign offsets based on variable size */
    int size = SymGetFieldByIndex(t, var_index, SYMTAB_SIZE);
    assert(size > 0); /* test for valid size -- JMC */
    SymPutFieldByIndex(t, var_index, SYMTAB_EQ_OFFSET, offset);
    offset += size; /* advance offset past current variable */
    /* next variable in the common block */
    var_index = SymGetFieldByIndex(t, var_index, SYMTAB_NEXT_COMMON);
  }
  
  /* record size of common block */
  SymPutFieldByIndex(t, common_index, SYMTAB_SIZE, offset);
}


/* for all common blocks in a procedure's symbol table */
static void
LayoutCommonBlocks(SymDescriptor d)
{
  SymForAll(d->Table, LayoutOneCommonBlock, 0); /* for each common block */
}

/* compute offset and size fields for each variable in a common block */
static void
EnsureOneCommonBlockDefined(SymTable t, int common_index, SymDescriptor d)
{
  int  var_index;
  int offset = /* initial offset of first common block variable */ 0; 
  
  /* if symbol is not name of common block  we are done */
  if (!(SymGetFieldByIndex(t, common_index, SYMTAB_OBJECT_CLASS) & 
	OC_IS_COMMON_NAME)) 
    return;

  /* an error if the common block is not defined */
  if (SymGetFieldByIndex(t, common_index, SYMTAB_COMMON_STMT) == AST_NIL)
    tc_ERROR(d,
	     SymGetFieldByIndex(t, common_index, SYMTAB_COMMON_NAME_FIRST_USE),
	     ft_COMMON_BLOCK_UNDEFINED);    

  return;  

}

/* for all common blocks in a procedure's symbol table */
static void
EnsureAllCommonBlocksDefined(SymDescriptor d)
{
  SymForAll(d->Table, (SymIteratorFunc)EnsureOneCommonBlockDefined, 
            (Generic) d); /* for each common block */
}

/* An implementation of the equivalence handling algorithm loosely based
 * on that given in  "Principles of Compiler Design" by Aho and Ullman
 *
 *
 * A field to watch throughout the code 
 *
 * SYMTAB_EQ_OFFSET
 *
 *  -- for any variable declared in a common block, it contains the 
 *     offset from the origin of the common block. for all other symbols,
 *     it is initially 0.
 *  -- changed whenever a leader becomes a follower (variables declared
 *     in common blocks can never become followers) 
 *  -- offsets are updated when the tree is flattened
 *
 */


static void
CheckEquivalence(SymDescriptor d, AST_INDEX stmt)
{
  struct EQList *p;
  AST_INDEX      equiv_elt_list, next_equiv_elt_list;
  AST_INDEX      equiv_elt, next_equiv_elt;
  AST_INDEX      name;
  int            index;
  SymTable       t = d->Table;
  int            count;

  p = (struct EQList *) get_mem(sizeof(struct EQList), "Equivalence EQList");
  
  for (equiv_elt_list = list_first(gen_EQUIVALENCE_get_equiv_elt_LIST(stmt));
       equiv_elt_list != AST_NIL;
       equiv_elt_list = next_equiv_elt_list)
  {
    next_equiv_elt_list = list_next(equiv_elt_list);

    for (equiv_elt = list_first(gen_EQUIV_ELT_get_lvalue_LIST(equiv_elt_list)); 
	 equiv_elt != AST_NIL;
	 equiv_elt = next_equiv_elt)
    {
      next_equiv_elt = list_next(equiv_elt);

      name = identifier_in_var_ref(equiv_elt);
      index = tcSymInsert(d, gen_get_text(name), TYPE_UNKNOWN);

      if (IS_DUMMY_PARAM_FOR_ENTRY_OR_PROCEDURE(t, index)) 
        tc_ERROR(d, name, ft_FORMAL_IN_EQUIVALENCE);
      else if (SymGetFieldByIndex(t, index, SYMTAB_STORAGE_CLASS) & (SC_CONSTANT | SC_NO_MEMORY))
    	tc_ERROR(d, name, ft_INVALID_VARIABLE_IN_EQUIV);
      else
      {
      	/* SymPutFieldByIndex(t, index, SYMTAB_EQUIV_STMT, stmt); */
      	tcExprCheck(d, equiv_elt);
      }
    }
  }
  
  p->stmt   = stmt;
  p->next   = EquivList;
  EquivList = p;
}


static void
AnEquivalence(SymDescriptor d, AST_INDEX stmt)
{
  AST_INDEX equiv_elt, next_equiv_elt;
  
  equiv_elt = list_first(gen_EQUIVALENCE_get_equiv_elt_LIST(stmt));
  while (equiv_elt != AST_NIL)
  {
    next_equiv_elt = list_next(equiv_elt);
    EquivEltList(d, equiv_elt);
    equiv_elt = next_equiv_elt;
  }
}


static void
FlattenEquiv(SymTable t, int i, SymDescriptor d)
{
  int grandfather, oc, sc, sc_father;
  int father = SymGetFieldByIndex(t, i, SYMTAB_PARENT);
  
  if (father != SYM_INVALID_INDEX) {

    FlattenEquiv(t, father, d);
    grandfather = SymGetFieldByIndex(t, father, SYMTAB_PARENT);
    if (grandfather != SYM_INVALID_INDEX) {
      SymPutFieldByIndex(t, i, SYMTAB_PARENT, grandfather);
      SymPutFieldByIndex(t, i, SYMTAB_EQ_OFFSET, 
			 SymGetFieldByIndex(t, i, SYMTAB_EQ_OFFSET) + 
			 SymGetFieldByIndex(t, father, SYMTAB_EQ_OFFSET));
      father = grandfather;
    }
    
    /* set storage class to that of leader for symbols with OC_IS_DATA 
     * for executables, setting the storage class to that of its leader 
     * causes problems for function entry variables since they lose
     * the information that they are an entry
     */
    if (SymGetFieldByIndex(t,i, SYMTAB_OBJECT_CLASS) & OC_IS_DATA)
    {
      sc = SymGetFieldByIndex(t, i, SYMTAB_STORAGE_CLASS);
      if (SymGetFieldByIndex(t,father,SYMTAB_OBJECT_CLASS) & OC_IS_COMMON_NAME)
	{
          sc_father = SymGetFieldByIndex(t,father,SYMTAB_STORAGE_CLASS);
          if (sc & SC_STATIC) sc = sc - SC_STATIC;
          if (sc & SC_STACK) sc = sc - SC_STACK;
          sc = sc | SC_GLOBAL | sc_father;
	  
	  SymPutFieldByIndex(t, i, SYMTAB_STORAGE_CLASS, sc);
        }
      else
	  SymPutFieldByIndex(t, i, SYMTAB_STORAGE_CLASS, 
			     SymGetFieldByIndex(t, father,
						SYMTAB_STORAGE_CLASS));
    }
    
    
    /* see if root of equiv tree is in a common block */
    oc = SymGetFieldByIndex(t, father, SYMTAB_OBJECT_CLASS);
    if (oc & OC_IS_COMMON_NAME) {
      int offset, cblocksize, varsize;
      
      /* error check -- illegal to negatively extend common block
       */
      offset = SymGetFieldByIndex(t, i, SYMTAB_EQ_OFFSET);
      if (offset < 0) {
	AST_INDEX dims = SymGetFieldByIndex(t, i, SYMTAB_DIM_LIST);
	if (dims != AST_NIL) {
	  tc_ERROR(d, dims, ft_EQUIV_NEG_EXTEND_COMMON);
	}
      }
      
      /* if the end of a storage sequence equivalenced to a common 
       * variable extends past the end of the common block, extend 
       * the size of the common block to include it
       */
      cblocksize = SymGetFieldByIndex(t, father, SYMTAB_SIZE);
      /* test for valid size (0 is valid common size) -- JMC 3/93 */
      assert(cblocksize >= 0); 

      varsize = SymGetFieldByIndex(t, i, SYMTAB_SIZE);
      assert(varsize > 0);   /* test for valid size -- JMC 3/93 */
      if (cblocksize < (varsize + offset))
	SymPutFieldByIndex(t, father, SYMTAB_SIZE, varsize + offset);
    }
    
  }
}


static void 
FlattenEquivalences(SymDescriptor d)
{
  SymForAll(d->Table, (SymIteratorFunc)FlattenEquiv, (Generic) d);
}

static void
PostProcessEquivalences(SymDescriptor d, FortTree ft)
{
  if (EquivList != 0) {
    struct EQList *p = EquivList;
    while (p != (struct EQList *) 0) {
      struct EQList *q = p;
      if (ft_GetState(ft) != ft_ERRONEOUS) AnEquivalence(d, p->stmt);
      p = p->next;
      free_mem((void*)q);
    }
    EquivList = (struct EQList *) 0;
  }
  /* since common blocks use the equivalence tree representation,
   * flattening the equivalence tree and inverting the equivalence map 
   * is mandatory for proper treatment
   * of common blocks
   */
  if (ft_GetState(ft) != ft_ERRONEOUS) {
    FlattenEquivalences(d);
    InvertEqMap(d);
  }
}

static void 
EquivEltList(SymDescriptor d, AST_INDEX node)
{
  AST_INDEX lvalue, next_lvalue;
  int parent, s_parent, child, s_child, p_dist, c_dist;
  
  /* We iterate through the list, considering pairs of names.
   * Each pair consists of the first name in the list and one 
   * other name.  
   */
  lvalue = list_first(gen_EQUIV_ELT_get_lvalue_LIST(node));
  
  if (lvalue != AST_NIL)
    {
      objs[0].node = lvalue;
      lvalue = list_next(lvalue);
      
      while (lvalue != AST_NIL) {
	
	next_lvalue = list_next(lvalue);

	find_leader(d, &objs[0]); /* not loop invariant! */
	
	objs[1].node   = lvalue;
	
	find_leader(d, &objs[1]);
	
	/* check for conflicting equivalence between common blocks */
	if (objs[0].leader_is_common && objs[1].leader_is_common) {
	  if (objs[0].leader != objs[1].leader) 
	    tc_ERROR(d, lvalue, ft_EQUIV_TWO_COMMON_BLOCKS);
	  else /* same common block */
	    ReportEquivConflict(d);
	}
	else if (objs[0].leader == objs[1].leader) /* same equiv class */
	  ReportEquivConflict(d);
	else { 
	  
	  /* merge the sets  -- if a variable is in common, make sure
	   * it is the new leader, otherwise leader with highest tree
	   * becomes new leader
	   */
	  if (objs[0].leader_is_common) parent = 0; 
	  else if (objs[1].leader_is_common) parent = 1; 
	  else if (objs[0].height >= objs[1].height) parent = 0; 
	  else parent = 1; 
	  
	  child = 1 - parent;
	  s_child = objs[child].leader;
	  s_parent = objs[parent].leader; 
	  
	  /*
	   *  Fix storage class of parent -- the key thing is that 
	   *  GLOBAL wins out over STATIC wins out over STACK.
	   *  Therefore if the child is not STACK, and the parent
	   *  is not GLOBAL, then the child's storage class is the
	   *  same as or better than the parent's.
	   *
	   *  Flattening will propagate this storage class to all the kids.
	   */
	  if (!(objs[parent].leader_is_common))
	      if ((!(objs[child].storage_class & SC_STACK)) &&
		  (!(objs[parent].storage_class & SC_GLOBAL)))

		  SymPutFieldByIndex(d->Table, s_parent,
				     SYMTAB_STORAGE_CLASS,
				     objs[child].storage_class);
	  
	  /* *_DIST is the distance of the equivalence point from the
	   * origin of the leader.
	   * It is used to compute the equivalence point of the added
	   * data item from its new leader.  It is also used to update
	   * the low and high fields of the newly aggregated group.
	   */
	  p_dist = objs[parent].eq_offset + objs[parent].equiv_pt;
	  c_dist = objs[child].eq_offset  + objs[child].equiv_pt;
	  
	  /* fix up the parental relationships */
	  
	  /* if (child == 0) this will change find_leader(d,0) */
	  SymPutFieldByIndex(d->Table, s_child, SYMTAB_PARENT, s_parent);
	  
	  SymPutFieldByIndex(d->Table, s_child, SYMTAB_EQ_OFFSET, 
			     p_dist - c_dist);
	} /* remember that indentation is off by one level */
	
	lvalue = next_lvalue;
      }
    }
}


static void
find_leader(SymDescriptor d, struct equiv_struct *obj)
{
  SymTable t = d->Table;
  int type, leader, temp;
  AST_INDEX node_name, son_node;
  
  type  = gen_get_node_type(obj->node);
  switch(type) {
    case GEN_IDENTIFIER:
    node_name = obj->node;
    break;
    case GEN_SUBSCRIPT:
    node_name = gen_SUBSCRIPT_get_name(obj->node);
    break;

    case GEN_SUBSTRING:
       /* see if node also has subscripts */
       son_node   = gen_SUBSTRING_get_substring_name(obj->node);
       if (gen_get_node_type(son_node) == GEN_SUBSCRIPT ) {
            node_name = gen_SUBSCRIPT_get_name(son_node);
       }
       else {
            node_name = gen_SUBSTRING_get_substring_name(obj->node);
       }

       break;

    default:
    tc_ERROR(d, obj->node, ft_TYPE_FAULT);
    break;
  }
  
  leader      = SymQueryIndex(t, gen_get_text(node_name)); 
  obj->name   = leader;
  obj->height = 1;
  obj->eq_offset = 0;
  obj->storage_class = SymGetFieldByIndex(t, leader, SYMTAB_STORAGE_CLASS);
  
  /* constants defined with PARAMETER stmt and stmt function
   * dummy arguments are not legal in EQUIVALENCE
   */
  if (SymGetFieldByIndex(t, leader, SYMTAB_STORAGE_CLASS) & 
      (SC_CONSTANT | SC_NO_MEMORY)) {
    tc_ERROR(d, obj->node, ft_INVALID_VARIABLE_IN_EQUIV);
  }
  
  compute_size(d, obj->node, &obj->high, &obj->equiv_pt);
  
  if (IS_DUMMY_PARAM_FOR_ENTRY_OR_PROCEDURE(t, leader)) 
    tc_ERROR(d, obj->node, ft_FORMAL_IN_EQUIVALENCE);
  
  temp = SymGetFieldByIndex(t, leader, SYMTAB_PARENT);
  
  /* now run up the tree to a leader */
  while(temp != SYM_INVALID_INDEX) { 
    obj->eq_offset += SymGetFieldByIndex(t, leader, SYMTAB_EQ_OFFSET);
    obj->height++;	/* used to pick leader above	*/
    if (SymGetFieldByIndex(t, leader, SYMTAB_EQ_OFFSET) > 0) {
      /* extend the high field */
      obj->high += SymGetFieldByIndex(t, leader, SYMTAB_EQ_OFFSET);
    }
    
    leader = temp;			/* and advance the pointers	*/
    temp   = SymGetFieldByIndex(t, leader, SYMTAB_PARENT);
  }
  obj->leader = leader;
  obj->leader_is_common = SymGetFieldByIndex(t, leader, SYMTAB_OBJECT_CLASS) &
    OC_IS_COMMON_NAME;
}


static void
compute_size(SymDescriptor d, AST_INDEX node, int *high, int *eq_pt)
/* *high: its upper bound (#elts * size) */
/* *eq_pt: distance from equiv pt to low pt */
{
  SymTable t = d->Table;  
  int index, elt_size, total_size, dims;
  AST_INDEX identifier, subscript, substring, son_node;
  int  substring_index=0,  cum_index = 0;
  
  switch(gen_get_node_type(node)) {
    case GEN_IDENTIFIER:
    identifier = node;
    subscript  = AST_NIL;
    break;
    
    case GEN_SUBSCRIPT:
    identifier = gen_SUBSCRIPT_get_name(node);
    subscript  = gen_SUBSCRIPT_get_rvalue_LIST(node);
    break;
    
    case GEN_SUBSTRING:
    substring  = gen_SUBSTRING_get_rvalue1(node);
    if (evalConstantIntExpr(d, substring, &substring_index)) {
	  tc_ERROR(d, node, ft_EQUIV_NON_CONST_SUBSCRIPT);
	  return;
    }	

    /* see if node also has subscripts */
    son_node   = gen_SUBSTRING_get_substring_name(node);
    if (gen_get_node_type(son_node) == GEN_SUBSCRIPT ) {
         subscript  = gen_SUBSCRIPT_get_rvalue_LIST(son_node);
         identifier = gen_SUBSCRIPT_get_name(son_node);
    }
    else {
         subscript  = AST_NIL;
         identifier = gen_SUBSTRING_get_substring_name(node);
    }

    break;
    
    default:
    tc_ERROR(d, node, ft_INVALID_VARIABLE_IN_EQUIV);
    return;
  }
  
  index = SymQueryIndex(t, gen_get_text(identifier));
  dims  = SymGetFieldByIndex(t, index, SYMTAB_NUM_DIMS);
  
  *eq_pt = 0;  /* eq_pt starts out as a zero. 			*/
  
  elt_size = SymbolElementSizeFromType(d, index);
  
  *high = total_size = SymGetFieldByIndex(t, index, SYMTAB_SIZE);
  assert(total_size > 0); /* test for valid size -- JMC 3/93 */
  
  if (dims != 0) {
    /* get the information about array's dimensions */
    ArrayBound *dimarray = (ArrayBound *) 
      SymGetFieldByIndex(t, index, SYMTAB_DIM_BOUNDS);
    int currentdim = dims - 1;  /* last dimension of dimarray */
    
    if (subscript != AST_NIL) { /* scope out offset from the front */
      AST_INDEX list = list_last(subscript);
      
      while(list != AST_NIL) {
	int current_index;
	if (evalConstantIntExpr(d, list, &current_index)) {
	  tc_ERROR(d, node, ft_EQUIV_NON_CONST_SUBSCRIPT);
	  return;
	}
	
	/* N.B. total_size == 0 indicates an earlier declaration 
	 * error or an adjustable size array (which isn't allowed 
	 * to be used in an EQUIVALENCE anyway).
	 * therefore, we can punt on the subscript
	 * offset calculation. it is however worthwhile to go
	 * through this loop anyway because we may detect a
	 * mismatch in the number of declared dimensions of
	 * an array and the number of subscripts used.
	 */
	
	if (total_size != 0) {
	  cum_index +=  (current_index - 
			 dimarray[currentdim].lb.value.const_val);
	  if (currentdim-- > 0) {
	    cum_index = cum_index * 
	      (dimarray[currentdim].ub.value.const_val - 
	       dimarray[currentdim].lb.value.const_val + 1);
	  }
	}
	dims--;
	list = list_prev(list);
      }
      
      if (dims != 0) 
	tc_ERROR(d, node, ft_WRONG_NO_SUBSCRIPTS);
      
   
    }
  }
  /* and adjust the equivalence point */
  *eq_pt = cum_index * elt_size + substring_index; 
}


/* and the code to invert the equivalence map ...
 * used to interpret the interprocedural information
 * produced by the program compiler
 *
 */

static void 
InvertEqMapEntry(SymTable t, int i, SymDescriptor d)
{
  int oc = SymGetFieldByIndex(t, i, SYMTAB_OBJECT_CLASS);
  int sc = SymGetFieldByIndex(t, i, SYMTAB_STORAGE_CLASS);
  
  int *EqNext = d->EQmap.EqChainNext;
  LeaderTable_t *LTable = d->EQmap.LeaderTable;
  
  /* all data objects 
   * (1) local variables --> oc & OC_IS_DATA
   * (2) common variables --> oc & OC_IS_DATA
   * (3) function variable --> (sc & SC_FUNCTION) & (sc & SC_CURRENT_PROC)
   * (4) function entry variable --> (sc & SC_FUNCTION) & (sc & SC_ENTRY)
   */
  if ((oc & OC_IS_DATA) || 
      ((sc & SC_FUNCTION) && 
       ((sc & SC_ENTRY) ||
	(sc & SC_CURRENT_PROC)))) {
    
    /* N.B. FlattenEquivalences guarantees tree is at most height 1 */
    int leader = SymGetFieldByIndex(t, i, SYMTAB_PARENT); 
    
    if (leader != SYM_INVALID_INDEX) {  /* is a leaf in an equiv tree */
      
      LTable[i].leader = leader;	/* Set it's leader table entry	*/
      LTable[leader].varcount++;	/* update count of vars for leader */	
      EqNext[i] = EqNext[leader];	/* and add it to class' chain	*/
      EqNext[leader] = i;			/* from leader's EqNext slot    */
      /*
       *  This assertion is false if the leader has OC_IS_COMMON_NAME
       */
      if (SymGetFieldByIndex(t, leader, SYMTAB_OBJECT_CLASS) & OC_IS_DATA) {
	assert(SymGetFieldByIndex(t, i, SYMTAB_STORAGE_CLASS) ==
	       SymGetFieldByIndex(t, leader, SYMTAB_STORAGE_CLASS));
      }
    }
    else LTable[i].leader = i; /* It's a leader or a singleton	*/
  } else if (oc & OC_IS_COMMON_NAME) {
    LTable[i].leader = i; /* a common block is always its own leader */
    /* don't need to update the leaders varcount since we haven't added 
     * another var to the chain
     */
  }
}


static void 
InvertEqMap(SymDescriptor d)
{
  int i;
  
  int symbols = SymMaxIndex(d->Table) + 1;
  
  d->EQmap.tablesize  = symbols;
  d->EQmap.LeaderTable  = (LeaderTable_t *) 
    get_mem(sizeof(LeaderTable_t) * symbols, "LTable");
  d->EQmap.EqChainNext  = (int *) get_mem(sizeof(int) * symbols, "EqNext");
  
  for(i = 0; i < symbols; i++) {
    d->EQmap.LeaderTable[i].leader = SYM_INVALID_INDEX;
    d->EQmap.LeaderTable[i].varcount = 0;
    d->EQmap.EqChainNext[i] = SYM_INVALID_INDEX;
  }
  
  SymForAll(d->Table, (SymIteratorFunc)InvertEqMapEntry, (Generic) d);
}


static void
ReportEquivConflict(SymDescriptor d)
{
  if (objs[0].eq_offset + objs[0].equiv_pt == 
      objs[1].eq_offset + objs[1].equiv_pt) 
    /* could lead to circularity in equivalence tree */
    tc_ERROR(d, objs[0].node, ft_EQUIV_MULT_DEFINED);
  else tc_ERROR(d, objs[0].node, ft_INCONSISTENT_EQUIVALENCES);
}

static Boolean
isError(AST_INDEX stmt)
{
  /* node type is GEN_ERROR, or we have set a semantic error at that
   * statement and the node type has not yet been adjusted
   */
  return BOOL(is_error(stmt) ||  (ft_GetSemanticErrorCode(stmt) != ft_NO_ERROR));
}

static void PrivateTableDumpOne(Generic key, Generic value, Generic extra)
{
  fprintf(stderr,"%l	%l\n",key,value);
}

static void PrivateTableDump(cNameValueTable ht)
{
  fprintf(stderr,"PRIVATE SCOPES HASHTABLE\n");
  NameValueTableForAll(ht,PrivateTableDumpOne,0);
}


/************************************************************************************/
/* Statement List traversers.                                                       */
/************************************************************************************/

static void doBlockDataStmtList(SymDescriptor d, AST_INDEX stmt_list)
{
  AST_INDEX stmt, next_stmt;

  for (stmt = list_first(stmt_list); stmt != AST_NIL; stmt = next_stmt)
  {
    next_stmt = list_next(stmt);
    
    switch(gen_get_node_type(stmt))
    {
    case GEN_COMMENT:
      /* okay */
      break;
    case GEN_IMPLICIT:
    case GEN_PARAMETER:
    case GEN_DIMENSION:
    case GEN_COMMON:
    case GEN_SAVE:
    case GEN_EQUIVALENCE:
    case GEN_DATA:
    case GEN_TYPE_STATEMENT:
      stmt = StmtTypeCheck(d, stmt);
      break;
    default:
      tc_ERROR(d, stmt, ft_BAD_STMT_IN_BLOCK_DATA);
      break;
    }
  }  
}

#define IN_PROCEDURE_SPECS 0
#define IN_PROCEDURE_EXECS 1

static void doProcedureStmtList(SymDescriptor d, AST_INDEX stmt_list)
{
  AST_INDEX stmt, next_stmt;
  int State = IN_PROCEDURE_SPECS;
  Boolean stmts_out_of_order = false;

  for (stmt = list_first(stmt_list); stmt != AST_NIL; stmt = next_stmt)
  {
    next_stmt = list_next(stmt);
    
    if (is_comment(stmt))
      continue;

    /* no state transitions implied here */
    if (is_data(stmt) || is_entry(stmt))
      stmt = StmtTypeCheck(d, stmt);

    else if (State == IN_PROCEDURE_SPECS)
    {
      /* should an assignment be statement function ? */
      if (is_assignment(stmt) && AssignmentShouldBeStmtFunction(d, stmt))
	stmt = CoerceAssignmentToStmtFunc(d, stmt);

      /* should a statement function be an assignment statement */
      else if (is_stmt_function(stmt) && StmtFunctionShouldBeAssignment(d, stmt))
	stmt = CoerceStmtFuncToAssignment(d, stmt);

      if (is_f77_specification_stmt(stmt))
	stmt = StmtTypeCheck(d, stmt);
      else if (is_supported_smp_executable_stmt(stmt))
      {
	State = IN_PROCEDURE_EXECS;
	stmt = StmtTypeCheck(d, stmt);
      }
      else
	tc_ERROR(d, stmt, ft_STMT_NOT_IN_SMP_DIALECT);
    }
    else
    {
      if (is_stmt_function(stmt))
	stmt = CoerceStmtFuncToAssignment(d, stmt);

      if (is_supported_smp_executable_stmt(stmt))
	stmt = StmtTypeCheck(d, stmt);
      else if (is_f77_specification_stmt(stmt))
      {
	if (!stmts_out_of_order)
	{
	  tc_ERROR(d, stmt, ft_STMT_OUT_OF_ORDER);
	  stmts_out_of_order = true;
	}
      }
      else
	tc_ERROR(d, stmt, ft_STMT_NOT_IN_SMP_DIALECT);
    }
  }  
}

static Boolean isAllowableInLogicalIf(AST_INDEX stmt)
{
  /* what's valid here? */
  if (stmt == AST_NIL)
    return false;

  switch (gen_get_node_type(stmt))
  {
    /* these are all executable statements that must be excluded from
       logical ifs */
  case GEN_ENTRY:
  case GEN_DATA:
  case GEN_FORMAT:
  case GEN_DO:
  case GEN_DO_ALL:
  case GEN_PARALLELLOOP:
  case GEN_IF:
  case GEN_LOGICAL_IF:
    return false;
  default:
    /* This works, since PARALLELLOOP and DO_ALL have already been
       filtered out. */
    return BOOL(is_supported_smp_executable_stmt(stmt));
  }
}

static void doLogicalIfStmt(SymDescriptor d, AST_INDEX stmt)
{
  if (is_stmt_function(stmt))
    stmt = CoerceStmtFuncToAssignment(d, stmt);

  if (isAllowableInLogicalIf(stmt))
    stmt = StmtTypeCheck(d, stmt);

  else
    tc_ERROR(d, stmt, ft_STMT_NOT_ALLOWED_IN_LOGICAL_IF);
}

static void doGuardStmtList(SymDescriptor d, AST_INDEX stmt_list)
{
  /* same as doDoStmtList */
  doDoStmtList(d, stmt_list);
}

#define IN_PARALLEL_LOOP_SPECS 0
#define IN_PARALLEL_LOOP_EXECS 1

static void doParallelDoStmtList(SymDescriptor d, AST_INDEX stmt_list)
{
  AST_INDEX stmt, next_stmt;
  int State = IN_PARALLEL_LOOP_SPECS;
  Boolean   stmts_out_of_order = false;

  for (stmt = list_first(stmt_list); stmt != AST_NIL; stmt = next_stmt)
  {
    next_stmt = list_next(stmt);

    /* first coerce stmt func to assignment if necessary */
    if (is_stmt_function(stmt))
      stmt = CoerceStmtFuncToAssignment(d, stmt);

    if (is_comment(stmt))
      continue;

    if (is_data(stmt))
    {/* okay in this context, leaves us in the same statement list state as before */
      stmt = StmtTypeCheck(d, stmt);
      continue;
    }

    if (is_private(stmt))
    {
      if (State == IN_PARALLEL_LOOP_SPECS)
      {/* fine, check it */
	stmt = StmtTypeCheck(d, stmt);	
      }
      else
      {/* out of order -- mark it as erroneous, only if its the first out
	  of order statement in order to limit cascading complaints. Don't
	  bother checking the statement for other errors, in any case. */
	if (!stmts_out_of_order)
	{
	  tc_ERROR(d, stmt, ft_STMT_OUT_OF_ORDER);
	  stmts_out_of_order = true;
	}
      }
    }
    else if (is_supported_smp_executable_stmt(stmt) && !is_entry(stmt))
    {
      stmt = StmtTypeCheck(d, stmt);
      State = IN_PARALLEL_LOOP_EXECS;
    }
    else
      tc_ERROR(d, stmt, ft_STMT_NOT_ALLOWED_IN_CONTEXT);
  }
}

static void doDoStmtList(SymDescriptor d, AST_INDEX stmt_list)
{
  AST_INDEX stmt, next_stmt;

  for (stmt = list_first(stmt_list); stmt != AST_NIL; stmt = next_stmt)
  {
    next_stmt = list_next(stmt);

    /* first coerce stmt func to assignment if necessary */
    if (is_stmt_function(stmt))
      stmt = CoerceStmtFuncToAssignment(d, stmt);

    if (is_comment(stmt))
      continue;

    if (is_supported_smp_executable_stmt(stmt) && !is_entry(stmt))
      stmt = StmtTypeCheck(d, stmt);
    else
    {/* out of order, don't bother checking it for errors */
      tc_ERROR(d, stmt, ft_STMT_NOT_ALLOWED_IN_CONTEXT);
    }
  }
}

/************************************************************************************/
/*                                                                                  */
/************************************************************************************/

static AST_INDEX 
StmtTypeCheck(SymDescriptor d, AST_INDEX stmt)
{
  int lht, rht, nodeType;
  AST_INDEX son;
  AST_INDEX gson;
  
  nodeType = gen_get_node_type(stmt);
  
  /* Handle the label definition */
  son = gen_ASSIGNMENT_get_lbl_def(stmt);
  if (son != AST_NIL)
    (void) tcExprCheck(d, son);
  
  switch(nodeType) {
  case GEN_ASSIGNMENT:
    CheckAssignment(d, stmt);
    break;
  case GEN_CALL:
    son = gen_CALL_get_invocation(stmt);
    (void) tcCheckInvocation(d, son, SC_SUBROUTINE);
    break;
  case GEN_COMMON:
    CheckCommon(d, stmt);
    break;
  case GEN_DATA:
    /* Make defined variables STATIC */
    CheckData(d, stmt);
    break;
  case GEN_DIMENSION:
    CheckDimsStmt(d, stmt);
    break;

  case GEN_PARALLELLOOP:		/* IBM PF Extension */
  case GEN_DO_ALL:
  case GEN_DO:
    (void) tcExprCheck(d, gen_DO_get_lbl_ref(stmt));
    (void) tcExprCheck(d, gen_DO_get_close_lbl_def(stmt));
    
    switch(gen_get_node_type(gen_DO_get_control(stmt)))
    {
    case GEN_INDUCTIVE:
      son  = gen_DO_get_control(stmt);
      gson = gen_INDUCTIVE_get_name(son);
      lht  = tcExprCheck(d, gson);
	
      gson = gen_INDUCTIVE_get_rvalue1(son);	/* check limits */
      if (gson != AST_NIL &&
	  gen_get_node_type(gson) != GEN_PLACE_HOLDER)
      {
	rht = tcExprCheck(d,gson);
	if (rht != TYPE_INTEGER && rht != TYPE_REAL &&
	    rht != TYPE_DOUBLE_PRECISION)
	  tc_ERROR(d, stmt, ft_BAD_TYPE_IN_DO);
	if (lht != rht)
	  sct(gson, lht);
      }
      gson = gen_INDUCTIVE_get_rvalue2(son);
      if (gson != AST_NIL &&
	  gen_get_node_type(gson) != GEN_PLACE_HOLDER)
      {
	rht = tcExprCheck(d,gson);
	if (rht != TYPE_INTEGER && rht != TYPE_REAL &&
	    rht != TYPE_DOUBLE_PRECISION)
	  tc_ERROR(d, stmt, ft_BAD_TYPE_IN_DO);
	if (lht != rht)
	  sct(gson, lht);
      }
      gson = gen_INDUCTIVE_get_rvalue3(son);
      if (gson != AST_NIL &&
	  gen_get_node_type(gson) != GEN_PLACE_HOLDER)
      {
	rht = tcExprCheck(d,gson);
	if (rht != TYPE_INTEGER && rht != TYPE_REAL &&
	    rht != TYPE_DOUBLE_PRECISION)
	  tc_ERROR(d, stmt, ft_BAD_TYPE_IN_DO);
	if (lht != rht)
	  sct(gson, lht);
      }
      break;
    case GEN_CONDITIONAL:
      son = gen_CONDITIONAL_get_rvalue(gen_DO_get_control(stmt));
      if (tcExprCheck(d, son) != TYPE_LOGICAL)
	sct(son, TYPE_LOGICAL);
      break;
    case GEN_REPETITIVE:
      son = gen_REPETITIVE_get_rvalue(gen_DO_get_control(stmt));
      if (tcExprCheck(d, son) != TYPE_INTEGER)
	sct(son, TYPE_LOGICAL);
      break;
    default:
      tc_ERROR(d, gen_DO_get_control(stmt), ft_INCOMPLETE);
      break;
    }

    if (nodeType == GEN_PARALLELLOOP)
      doParallelDoStmtList(d, gen_DO_get_stmt_LIST(stmt));
    else if (nodeType == GEN_DO_ALL || nodeType == GEN_DO)
      doDoStmtList(d, gen_DO_get_stmt_LIST(stmt));
    break;
  case GEN_ENTRY:
    d->parent_td->NumberOfModuleEntries++; 
    /* count of entry points in module */
    CheckEntryDefinition(d, stmt);
    break;
  case GEN_EQUIVALENCE:
    CheckEquivalence(d, stmt);
    break;
  case GEN_EXTERNAL:
    CheckExternal(d, stmt);
    break;
  case GEN_IF:
    CheckIf(d, stmt);
    break;
  case GEN_IMPLICIT:
    CheckImplicit(d, stmt);
    RegisterImplicit(d, stmt);
    DeclarationCleanUp(d);
    break;
  case GEN_INTRINSIC:
    CheckIntrinsic(d, stmt);
    break;
  case GEN_LOGICAL_IF:
    (void) tcExprCheck(d, gen_LOGICAL_IF_get_rvalue(stmt));
    doLogicalIfStmt(d, list_first(gen_LOGICAL_IF_get_stmt_LIST(stmt)));
    break;
  case GEN_PARAMETER:
    CheckParameter(d, stmt);
    break;
  case GEN_PRIVATE:		/* IBM PF Extension */
    CheckPrivate(d, stmt);  
    break;
  case GEN_STMT_FUNCTION:
    CheckStatementFunction(d, stmt);
    break;
  case GEN_SAVE:
    CheckSave(d, stmt);
    break;
  case GEN_TYPE_STATEMENT:
    CheckTypeStmt(d, stmt);
    break;

  case GEN_ARITHMETIC_IF:
  case GEN_ASSIGN:
  case GEN_ASSIGNED_GOTO:
  case GEN_COMPUTED_GOTO:
  case GEN_CONTINUE:
  case GEN_GOTO:
  case GEN_PAUSE:
  case GEN_PLACE_HOLDER:
  case GEN_RETURN:
  case GEN_STOP:
  case GEN_READ_SHORT:
  case GEN_READ_LONG:
  case GEN_WRITE:
  case GEN_PRINT:
  case GEN_OPEN:
  case GEN_CLOSE:
  case GEN_INQUIRE:
  case GEN_BACKSPACE_SHORT:
  case GEN_BACKSPACE_LONG:
  case GEN_ENDFILE_SHORT:
  case GEN_ENDFILE_LONG:
  case GEN_REWIND_SHORT:
  case GEN_REWIND_LONG:
  case GEN_FORMAT:
    CheckDefault(d, stmt);
    break;
    
  default:
    tc_ERROR(d, stmt, ft_LAST_SEMANTIC_ERROR);
    break;
  }

  return stmt;
}

/************************************************************************************/
/* Check specification statements.                                                  */
/************************************************************************************/

static void
CheckData(SymDescriptor d, AST_INDEX stmt)   /* Check DATA statement           */
                                             /* mark init'd vars as static     */
                                             /* -- paco 3 June 1992            */
{
    AST_INDEX node, next;

    for (node = list_first(gen_DATA_get_data_elt_LIST(stmt));
         !is_null_node(node);
         node = next)
    {
        next = list_next(node);
        CheckDefault(d, gen_DATA_ELT_get_init_LIST(node));
        CheckDataList(d, gen_DATA_ELT_get_data_vars_LIST(node));
        CheckDefault(d, gen_DATA_ELT_get_data_vars_LIST(node));
    }
}


static void 
CheckDataList(SymDescriptor d, AST_INDEX list)	/* Check var list in DATA stmt    */
                                                /* mark init'd vars as static     */
                                                /* -- paco 3 June 1992            */
{
    AST_INDEX   node, next;

    for (node = list_first(list);
         !is_null_node(node);
         node = next)
    {
      next = list_next(node);

      if (is_identifier(node))
	MakeStackStatic(d, node);
      else if (is_subscript(node))
	MakeStackStatic(d, gen_SUBSCRIPT_get_name(node));
      else if (is_substring(node))
	MakeStackStatic(d, gen_SUBSTRING_get_substring_name(node));
      else if (is_implied_do(node))
	CheckDataList(d, gen_IMPLIED_DO_get_imp_elt_LIST(node));
    }
}

static void 
RegisterImplicit(SymDescriptor p, AST_INDEX stmt)
{
  ImplicitList chain;
  
  chain = (ImplicitList) get_mem(sizeof(struct ImplicitList_t), 
				 "Implicit statement list");
  chain->stmt = stmt;
  chain->next = p->implicits.IList;
  p->implicits.IList = chain;
}

static void 
CheckImplicit(SymDescriptor d, AST_INDEX stmt)
{
  AST_INDEX imp_list;
  AST_INDEX elt_list;
  int 	    type;
  char *p, *q, c;
  
  imp_list = list_first(gen_IMPLICIT_get_implicit_elt_LIST(stmt));
  
  while(imp_list != AST_NIL) {
    type = findType(d, gen_IMPLICIT_ELT_get_type_len(imp_list));
    elt_list = list_first(
			  gen_IMPLICIT_ELT_get_implicit_refs_LIST(imp_list));
    
    while(elt_list != AST_NIL)
      {
	switch(gen_get_node_type(elt_list))
	  {
	    case GEN_IMPLICIT_PAIR:
	    p = gen_get_text(gen_IMPLICIT_PAIR_get_start(elt_list));
	    q = gen_get_text(gen_IMPLICIT_PAIR_get_end(elt_list));
	    c = *p;
	    if ((('a' <= *p && *p <= 'z') ||
		 ('A' <= *p && *p <= 'Z'))&&
		(('a' <= *q && *q <= 'z') ||
		 ('A' <= *q && *q <= 'Z'))&&
		(*p <= *q) )
	      {
		for (c=p[0]; c<= q[0]; c++) {
		  if (d->implicits.typedecl[c].seen == true)
		    tc_ERROR(d, elt_list, ft_IMPLICIT_OVERLAP);
		  else d->implicits.typedecl[c].seen = true;
		  d->implicits.typedecl[c].type = type;
		}
	      }
	    else
	      tc_ERROR(d, elt_list, ft_BAD_IMPLICIT_SPECIFIER);
	    break;
	    case GEN_LETTER:
	    p = gen_get_text(elt_list);
	    if (('a' <= *p && *p <= 'z') ||
		('A' <= *p && *p <= 'Z') )
	      {
		if (d->implicits.typedecl[*p].seen == true)
		  tc_ERROR(d, elt_list, ft_IMPLICIT_OVERLAP);
		else d->implicits.typedecl[*p].seen = true;
		d->implicits.typedecl[*p].type = type;
	      }
	    else
	      tc_ERROR(d, elt_list, ft_BAD_IMPLICIT_SPECIFIER);
	    break;
	    default:
	    tc_ERROR(d, elt_list, ft_BAD_IMPLICIT_SPECIFIER);
	    break;
	  }
	elt_list = list_next(elt_list);
      }

    imp_list = list_next(imp_list);
  }
}

static void 
CheckCommon(SymDescriptor d, AST_INDEX Stmt)
{
  AST_INDEX 	name, next_name;
  AST_INDEX	common_elt, next_common_elt;
  AST_INDEX     id;
  STR_TEXT	text;
  int 		Index;
  AST_INDEX     common_name;
  int           common_index;
  int           parent_index;
  
  /* get the first common element */
  common_elt = list_first(gen_COMMON_get_common_elt_LIST(Stmt));
 
  /* For now, all we need to do is to mark each variable with 
   * the statement number of the COMMON statement
   */
  while(common_elt != AST_NIL) {

    next_common_elt = list_next(common_elt);

    /* insert common block name into common table */
    common_name = gen_COMMON_ELT_get_name(common_elt);
    common_index = tcCommonInsert(d, gen_get_text(common_name), common_name);

    /* record the common statement in which this common block is defined*/
    SymPutFieldByIndex(d->Table, common_index, SYMTAB_COMMON_STMT, Stmt);
    
    /* walk the list of names and mark them in the table */
    name = list_first(gen_COMMON_ELT_get_common_vars_LIST(common_elt));
    
    while(name != AST_NIL)
      {
	int oc, sc;
	int common_name;

	next_name = list_next(name);

	ArrayDeclLen(d, name, TYPE_UNKNOWN, Stmt);
	
	id    = gen_ARRAY_DECL_LEN_get_name(name);
	text  = gen_get_text(id);
	Index = tcSymInsert(d, text, TYPE_UNKNOWN);
	
	oc = SymGetFieldByIndex(d->Table, Index, SYMTAB_OBJECT_CLASS);
	sc = SymGetFieldByIndex(d->Table, Index, SYMTAB_STORAGE_CLASS);
	
	/* the only use of the SYMTAB_PARENT field until after equivalence
	 * processing is to tie common variables to their common block
	 * thus if SYMTAB_PARENT is not SYM_INVALID_INDEX, then it
	 * names a common block
	 */
	parent_index = SymGetFieldByIndex(d->Table, Index, SYMTAB_PARENT);

	/* error if name is
	 *   already used as a common var (parent_index != SYM_INVALID_INDEX)
	 *   a formal parameter
	 *   a function name
         *   a constant
	 */
	
	if ((parent_index != SYM_INVALID_INDEX) || 
	    (oc & (OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG)) ||  
	    ((oc & OC_IS_EXECUTABLE) &&  (sc & SC_FUNCTION)))
	  tc_ERROR(d, Stmt, ft_COMMON_VAR_NAME_CONFLICT);

	if (sc & SC_CONSTANT) tc_ERROR(d, Stmt, ft_CONSTANT_IN_COMMON);

	/* record the common statement in which this variable occurs */
	SymPutFieldByIndex(d->Table, Index, SYMTAB_COMMON_STMT, Stmt);
	SymPutFieldByIndex(d->Table, Index, SYMTAB_OBJECT_CLASS, OC_IS_DATA);
	
	/* storage class for common variables:   default = STACK, GLOBAL */   
        assert((sc & SC_NO_MEMORY) == 0);
        if (sc == 0) sc = SC_STACK; 
        sc = sc | SC_GLOBAL;

	SymPutFieldByIndex(d->Table, Index, SYMTAB_STORAGE_CLASS, sc);

	
	/* associate var name with common block */
	tcCommonAddVar(d, common_index, Index);
	
	name = next_name;
      }
    common_elt = next_common_elt;
  }
}

static void 
CheckExternal(SymDescriptor d, AST_INDEX Stmt)
{
  AST_INDEX name, next_name;
  STR_TEXT  text;
  int	    Index;
  
  name = list_first(gen_EXTERNAL_get_name_LIST(Stmt));
  
  while(name != AST_NIL) {
    int sc, oc;

    next_name = list_next(name);

    text  = gen_get_text(name);
    Index = tcSymInsert(d, text, TYPE_UNKNOWN);
    
    sc = SymGetFieldByIndex(d->Table, Index, SYMTAB_STORAGE_CLASS);
    oc = SymGetFieldByIndex(d->Table, Index, SYMTAB_OBJECT_CLASS);
    
    if (sc & (SC_GENERIC | SC_INTRINSIC))
      tc_ERROR(d, Stmt, ft_INTRINSIC_AND_EXTERNAL);
    
    /* signal external in storage class */
    SymPutFieldByIndex(d->Table, Index, SYMTAB_STORAGE_CLASS, 
		       SC_EXTERNAL | sc);
    
    /* signal executable in object class 
     * note: careful here since a formal parameter defaults to 
     *       object class of OC_IS_DATA until proven otherwise
     */
    SymPutFieldByIndex(d->Table, Index, SYMTAB_OBJECT_CLASS, 
		       OC_IS_EXECUTABLE | (oc & (OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG)));
    
    name = next_name;
  }
}

static void 
CheckTypeStmt(SymDescriptor d, AST_INDEX node)
{
  int 	    type;
  AST_INDEX TypeLen = gen_TYPE_STATEMENT_get_type_len(node);
  AST_INDEX TypeLenLen = gen_TYPE_LEN_get_length(TypeLen);
  AST_INDEX decl_len, next_decl_len;
  
  type = findType(d, TypeLen);
  
  decl_len = list_first(gen_TYPE_STATEMENT_get_array_decl_len_LIST(node));
  
  while (decl_len != AST_NIL) {
    int index;

    next_decl_len = list_next(decl_len);
    (void) ArrayDeclLen( d, decl_len, type, node );
    
    index = SymQueryIndex(d->Table, 
			  gen_get_text(gen_ARRAY_DECL_LEN_get_name(decl_len)));
    
    
    /* if array declarator, we know object is data */
    if (SymGetFieldByIndex(d->Table, index, SYMTAB_DIM_STMT)) {
      int oc = SymGetFieldByIndex(d->Table, index, SYMTAB_OBJECT_CLASS);
      SymPutFieldByIndex(d->Table, index, SYMTAB_OBJECT_CLASS, 
			 (oc & (OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG)) | OC_IS_DATA);
    }
    
    /* for character vars: add length specifier to symbol table */
    if (type == TYPE_CHARACTER) {
      int len = 1; /* default  number of chars if no length specifier */
      AST_INDEX ownlen = gen_ARRAY_DECL_LEN_get_len(decl_len);
      AST_INDEX local_len;
      
      if (ownlen != AST_NIL) local_len = ownlen;
      else local_len = TypeLenLen;
      if (local_len != AST_NIL) {
	if (gen_get_node_type(local_len) == GEN_STAR) {
	  len = CHAR_LEN_STAR; 
	  /* JMC -- error checking needed for length STAR <TO DO> */
	}
	else {
	  /*
	    char *s = gen_get_text(local_len);
	    s = get_num(s, &len );
	    */
	  if (evalConstantIntExpr(d, local_len, &len))
	    tc_ERROR(d, local_len, ft_LEN_MUST_BE_INT_CONST_EXP);
	}
      }
      SymPutFieldByIndex(d->Table, index, SYMTAB_CHAR_LENGTH, len);
    }
    
    decl_len = next_decl_len;
  }
}

static void 
CheckDimsStmt(SymDescriptor d, AST_INDEX node)
{
  AST_INDEX	decl_len, next_decl_len;
  
  decl_len = list_first(gen_DIMENSION_get_array_decl_len_LIST(node));
  
  while (decl_len != AST_NIL) {
    int index;

    next_decl_len = list_next(decl_len);

    (void) ArrayDeclLen( d, decl_len, TYPE_UNKNOWN, node );
    
    index = SymQueryIndex(d->Table, 
			  gen_get_text(gen_ARRAY_DECL_LEN_get_name(decl_len)));
    
    /* if array declarator, we know object is data */
    if (SymGetFieldByIndex(d->Table, index, SYMTAB_DIM_STMT)) {
      int oc = SymGetFieldByIndex(d->Table, index, SYMTAB_OBJECT_CLASS);
      SymPutFieldByIndex(d->Table, index, SYMTAB_OBJECT_CLASS, 
			 (oc & (OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG)) | OC_IS_DATA);
    }
    else 
      /* not array declarator -- bad dims statement */
      tc_ERROR(d, node, ft_BAD_DIMENSION_SPECIFIER);
    
    decl_len = next_decl_len;
  }
}

static void 
CheckSave(SymDescriptor d, AST_INDEX Stmt)
{
  AST_INDEX name, next_name;
  int	    Index, str_type;
  char*     name_str;
  
  name = list_first(gen_SAVE_get_name_LIST(Stmt));

  if (is_null_node(name))
  {
     d->dataIsStatic = true; 
  }
  else {
     while(name != AST_NIL) {
       next_name = list_next(name);

       name_str = string_table_get_text(ast_get_symbol(name));
       str_type = str_get_type(ast_get_symbol(name));
       if (str_type == STR_COMMON_NAME) 
	 Index = tcCommonInsert(d, name_str, name); 
       else {
	 Index = tcSymInsert(d, name_str, TYPE_UNKNOWN);
       }
       MakeStackStatic(d, name);
       SymPutFieldByIndex(d->Table, Index, SYMTAB_SAVE_STMT, Stmt);

       name = next_name;
     }
  }
}



static void 
CheckParameter(SymDescriptor d, AST_INDEX stmt)
{
  AST_INDEX param_elt, next_param_elt, expr;
  int lht, rht;
  
  param_elt = list_first(gen_PARAMETER_get_param_elt_LIST(stmt));
  
  while (param_elt != AST_NIL) {
    AST_INDEX name = gen_PARAM_ELT_get_name(param_elt);
    int index  = tcSymInsert(d, gen_get_text(name), TYPE_UNKNOWN);
    int oc = SymGetFieldByIndex(d->Table, index, SYMTAB_OBJECT_CLASS);
    int sc = SymGetFieldByIndex(d->Table, index, SYMTAB_STORAGE_CLASS);

    next_param_elt = list_next(param_elt);

    if (oc & (OC_IS_EXECUTABLE | OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG |
	      OC_IS_STFUNC_ARG))
      tc_ERROR(d, stmt, ft_PARAM_BAD_ENTITY);
    
    if (sc & SC_CONSTANT)
      tc_ERROR(d, stmt, ft_MANIFEST_CONST_REDEFINITION);
    
    SymPutFieldByIndex(d->Table, index, SYMTAB_OBJECT_CLASS, OC_IS_DATA);
    SymPutFieldByIndex(d->Table, index, SYMTAB_STORAGE_CLASS, SC_CONSTANT);
    
    lht  = tcExprCheck(d, name);
    
    expr = gen_PARAM_ELT_get_rvalue(param_elt);
    
    SymPutFieldByIndex(d->Table, index, SYMTAB_EXPR, expr);
    rht  = tcExprCheck(d, expr);
    
    if (lht != rht)
      sct(expr, lht);
    
    param_elt = next_param_elt;
  }
}

static void 
CheckIntrinsic(SymDescriptor d, AST_INDEX Stmt)
{
  AST_INDEX name, next_name;
  STR_TEXT  text;
  int	    Index;
  
  name = list_first(gen_INTRINSIC_get_name_LIST(Stmt));
  
  while(name != AST_NIL) {
    generic_descriptor *ginfo;
    intrinsic_descriptor *iinfo;

    next_name = list_next(name);

    text  = gen_get_text(name);
    
    Index = tcSymInsert(d, text, TYPE_UNKNOWN);
    
    /* a name declared intrinsic may not also be external */
    if (SymGetFieldByIndex(d->Table, Index, SYMTAB_STORAGE_CLASS) & SC_EXTERNAL)
      tc_ERROR(d, Stmt, ft_INTRINSIC_AND_EXTERNAL);
    
    /* some prior declaration */
    if (SymGetFieldByIndex(d->Table, Index, SYMTAB_STORAGE_CLASS) & 
	(SC_GENERIC | SC_INTRINSIC))
      tc_ERROR(d, Stmt, ft_DUPL_INTRINSIC);
    
    if (ginfo = builtins_genericFunctionInfo(text)) {  /* generic */
      SymPutFieldByIndex(d->Table, Index, SYMTAB_STORAGE_CLASS, SC_GENERIC);
      SymPutFieldByIndex(d->Table, Index, SYMTAB_OBJECT_CLASS, 
			 OC_IS_EXECUTABLE);
      
      /* use generic table to set function result type if it is fixed */
      if (ginfo->resType != TYPE_UNKNOWN)
	SymPutFieldByIndex(d->Table, Index, SYMTAB_TYPE, 
			   ginfo->resType);
    } else if (iinfo = builtins_intrinsicFunctionInfo(text)) {  
      /* intrinsic */
      SymPutFieldByIndex(d->Table, Index, SYMTAB_STORAGE_CLASS, SC_INTRINSIC);
      SymPutFieldByIndex(d->Table, Index, SYMTAB_OBJECT_CLASS, 
			 OC_IS_EXECUTABLE);
      
      /* use intrinsic table to set function result type */
      SymPutFieldByIndex(d->Table, Index, SYMTAB_TYPE, iinfo->resType);
    }
    else tc_ERROR(d, Stmt, ft_NOT_INTRINSIC); /* neither */
    
    SymPutFieldByIndex(d->Table, Index, SYMTAB_INTRINSIC_STMT, Stmt);
    name = next_name;
  }
}

static void
CheckEntryDefinition(SymDescriptor d, AST_INDEX stmt)
{
  int entry; 		/* symbol table index of entry name */
  int index; 		/* symbol table index used for entry parameters */
  int	counter = 0; 	/* number of parameters */
  int eclass; 		/* procedure or subroutine ? */
  int oc; 		/* object class */
  int sc; 		/* storage class */
  int arg_position = 1; /* first arg in position 1 */
  int enclosing_proc;

  char *ename = gen_get_text(gen_ENTRY_get_name(stmt));
  AST_INDEX formals = gen_ENTRY_get_formal_arg_LIST(stmt);
  AST_INDEX formal, next_formal;
  
  /* allocate a hash table for formal parameters */
  cNameValueTable ht = NameValueTableAlloc(8, (NameCompareCallback)strcmp, 
                                           (NameHashFunctCallback)hash_string);
  
  /*  add the entry name to the symbol table  */
  entry = tcSymInsert(d, ename, TYPE_UNKNOWN);
  
  /* associate the hash table with the entry point */
  SymPutFieldByIndex(d->Table, entry, SYMTAB_FORMALS_HT, (Generic) ht);
  
  /* list of the formals for the entry point */
  SymPutFieldByIndex(d->Table, entry, SYMTAB_FORMALS_LIST, (Generic) formals);
  
  if (SymDescriptorHashTableAddEntryPoint(d->parent_td->SubPgmHt, ename, d))
    tc_ERROR(d, stmt, ft_DUPLICATE_DECLARATION);
  
  oc = SymGetFieldByIndex(d->Table, entry, SYMTAB_OBJECT_CLASS);
  sc = SymGetFieldByIndex(d->Table, entry, SYMTAB_STORAGE_CLASS);
  
  if (oc & (OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG | OC_IS_STFUNC_ARG))
    tc_ERROR(d, stmt, ft_ENTRY_IS_DUMMY_ARG);
  else if (oc != OC_UNDEFINED)
    tc_ERROR(d, stmt, ft_DUPLICATE_DECLARATION);
  
  if (sc & SC_EXTERNAL)
    tc_ERROR(d, stmt, ft_ENTRY_IS_EXTERNAL);
  
  SymPutFieldByIndex(d->Table, entry, SYMTAB_OBJECT_CLASS,
		     OC_IS_EXECUTABLE);
  
  enclosing_proc = SymQueryIndex(d->Table, d->name);
  
  if (enclosing_proc != SYM_INVALID_INDEX) {
    eclass = 
      SymGetFieldByIndex(d->Table, enclosing_proc, SYMTAB_STORAGE_CLASS) &
	(SC_SUBROUTINE | SC_FUNCTION);
  } else eclass = 0;
  
  if (!eclass) tc_ERROR(d, stmt, ft_ENTRY_CONTEXT);
  
  SymPutFieldByIndex(d->Table, entry, SYMTAB_STORAGE_CLASS, SC_ENTRY | eclass);
  
  /* if function entry --> equivalence entry name variable with 
   * function variable 
   */
  if (eclass & SC_FUNCTION) {
    SymPutFieldByIndex(d->Table, entry, SYMTAB_PARENT, enclosing_proc);
    SymPutFieldByIndex(d->Table, entry, SYMTAB_EQ_OFFSET, 0);
  }
  
  /* walk the parameter list */
  formal = list_first(formals);
  
  while (formal != AST_NIL)
  {
    Generic oldvalue;
    int oc;
    char *text;

    next_formal = list_next(formal);

    /* skip alternate return specifiers */
    if (!is_star(formal))
    {
      text = gen_get_text(formal);
      
      index = tcSymInsert(d, text, TYPE_UNKNOWN);
      
      if (NameValueTableAddPair(ht, (Generic)text, (Generic)arg_position++, &oldvalue))
	tc_ERROR(d, stmt, ft_DUPLICATE_ARGUMENTS);
      
      oc = SymGetFieldByIndex(d->Table, index, SYMTAB_OBJECT_CLASS);
      
      SymPutFieldByIndex(d->Table, index, SYMTAB_OBJECT_CLASS,
			 (oc | OC_IS_ENTRY_ARG));
    }

    counter++;
    formal = next_formal;
  }
  
  SymPutFieldByIndex(d->Table, entry, SYMTAB_NARGS, counter);
  
  return;
}

static void
CheckStatementFunction(SymDescriptor d, AST_INDEX stmt)
{
  AST_INDEX formal_arg, next_formal_arg, expr, sf_expr;
  AST_INDEX name = gen_STMT_FUNCTION_get_name(stmt);
  char *text = gen_get_text(name);
  Boolean args_are_all_identifiers;
  
  /*  add the name to the symbol table  */
  int index = tcSymInsert(d, text, TYPE_UNKNOWN);
  
  cNameValueTable ht; /* a hash table for parameters */
  int		arg_position = 1;
  
  /* if the name has been used previously, and the name is not
     one that has been determined implicitly to be the name
     of an external entry, then this is an error. If the name was 
     determined implicitly to be an external entry, and is now seen
     to be a stmt func name, then we should define it to be a stmt 
     function, and then catch it later as being out-of-order -- CARLE */
  if (SymGetFieldByIndex(d->Table, index, SYMTAB_OBJECT_CLASS) != OC_UNDEFINED) {
    if (!((SymGetFieldByIndex(d->Table,
		index, SYMTAB_STORAGE_CLASS) | SC_EXTERNAL) &&
	  (SymGetFieldByIndex(d->Table,
		index, SYMTAB_TYPE_STMT) == TYPE_STMT_IMPLICIT)))
      tc_ERROR(d, stmt, ft_DUPLICATE_DECLARATION);
  }

  SymPutFieldByIndex(d->Table, index, SYMTAB_OBJECT_CLASS, OC_IS_EXECUTABLE);
  SymPutFieldByIndex(d->Table, index, SYMTAB_STORAGE_CLASS, SC_STMT_FUNC);
  
  /* record the list of formals  */
  SymPutFieldByIndex(d->Table, index, SYMTAB_FORMALS_LIST,
		     gen_STMT_FUNCTION_get_formal_arg_LIST(stmt));
  
  formal_arg = list_first(gen_STMT_FUNCTION_get_formal_arg_LIST(stmt));
  
  /* make sure that each of the args is an identifier */
  args_are_all_identifiers = true;
  for (; formal_arg != AST_NIL; formal_arg = next_formal_arg)
  {
    next_formal_arg = list_next(formal_arg);

    if (!is_identifier(formal_arg))
    {
      args_are_all_identifiers = false;
      tc_ERROR(d, stmt, ft_STMT_FUNC_ARGS_NOT_IDENTS);
      break;
    }
  }

  if (args_are_all_identifiers)
  {
    formal_arg = list_first(gen_STMT_FUNCTION_get_formal_arg_LIST(stmt));

    if (formal_arg != AST_NIL) {
      /* allocate a hash table for formal parameters */
      ht = NameValueTableAlloc(8, (NameCompareCallback)strcmp, 
                               (NameHashFunctCallback)hash_string);
    
      /* associate the hash table with the stmt func */
      SymPutFieldByIndex(d->Table, index, SYMTAB_FORMALS_HT, ht);
    }

    while (formal_arg != AST_NIL)
    {
      int arg_index;
      int oc;
      int oldvalue;
      char *text = gen_get_text(formal_arg);

      next_formal_arg = list_next(formal_arg);

      arg_index = tcSymInsert(d, text, TYPE_UNKNOWN);
      
      oc = SymGetFieldByIndex(d->Table, arg_index, SYMTAB_OBJECT_CLASS);
      
      /* previous conflicting definition? */
      if (oc & OC_IS_EXECUTABLE)
	tc_ERROR(d, stmt, ft_DUPLICATE_DECLARATION);
      
      if (IS_INTRINSIC_OR_GENERIC(d->Table, arg_index))
	tc_ERROR(d, stmt, ft_INTRINSIC_AS_STFUNC_ARG);
      
      SymPutFieldByIndex(d->Table, arg_index, SYMTAB_OBJECT_CLASS,
			 (oc | OC_IS_DATA | OC_IS_STFUNC_ARG));
      
      if (NameValueTableAddPair(ht, (Generic)text, (Generic)arg_position++, 
                                (Generic*)&oldvalue))
	tc_ERROR(d, stmt, ft_DUPLICATE_ARGUMENTS);
      
      formal_arg = next_formal_arg;
    }
  
    /* save number of arguments */
    SymPutFieldByIndex(d->Table, index, SYMTAB_NARGS,
		       arg_position - 1);
  
    tcExprCheck(d, gen_STMT_FUNCTION_get_rvalue(stmt));

    /* now record body of STMT_FUNCTION in symbol table */
    expr = gen_STMT_FUNCTION_get_rvalue(stmt);
    SymPutFieldByIndex(d->Table, index, SYMTAB_EXPR, expr);
    sf_expr = expand_stmt_funcs(d, tree_copy_with_type(expr));

    SymPutFieldByIndex(d->Table, index, SYMTAB_SF_EXPR, sf_expr);
  }
}

static void 
CheckPrivate(SymDescriptor d, AST_INDEX stmt)
{
  AST_INDEX enclosing_scope;
  Generic dummy;
  char *text;
  AST_INDEX next_pvar;
    
  enclosing_scope = tree_out(stmt);
  switch(gen_get_node_type(enclosing_scope)) 
  {
    case GEN_PARALLEL_CASE:
    case GEN_PARALLELLOOP:
    case GEN_PARALLEL:
    case GEN_DO_ALL:
    /* add private variables to symbol table as needed and
       add annotation about scope of private declaration
       
       N.B. should use just a name table rather than a 
       name-value table -- JMC
       */
    {
      AST_INDEX pvar  = list_first(gen_PRIVATE_get_name_LIST(stmt));
      while (pvar != AST_NIL) {
	cNameValueTable ht;
	int index;
	int oc;

	next_pvar = list_next(pvar);

	text  = gen_get_text(pvar);
	index = tcSymInsert(d, text, TYPE_UNKNOWN);
	oc = SymGetFieldByIndex(d->Table, index, 
				    SYMTAB_OBJECT_CLASS);
	
	if (oc & OC_UNDEFINED)
	  SymPutFieldByIndex(d->Table, index, SYMTAB_OBJECT_CLASS, OC_IS_DATA);
	else if (oc & OC_IS_EXECUTABLE) {
	  tc_ERROR(d, stmt, ft_PRIVATE_DECL_FOR_NON_DATA);
	} else if (oc & (OC_IS_FORMAL_PAR | 
			 OC_IS_ENTRY_ARG)) {
	  tc_ERROR(d, stmt, ft_PRIVATE_DECL_FOR_PARAM);
	} else {
	  int parent = SymGetFieldByIndex(d->Table, index,
					  SYMTAB_PARENT);
	  if (parent != SYM_INVALID_INDEX && 
	      (SymGetFieldByIndex(d->Table, parent, 
				  SYMTAB_OBJECT_CLASS) 
	       & OC_IS_COMMON_NAME)) {
	    tc_ERROR(d, stmt, 
		     ft_PRIVATE_DECL_FOR_COMMON);
	  }
	}
	
	ht = (cNameValueTable)SymGetFieldByIndex(d->Table, index,
					    SYMTAB_PRIVATE_SCOPES);
	if (!ht) {
	  ht = NameValueTableAlloc(8, (NameCompareCallback)NameValueTableIntCompare,
                                   (NameHashFunctCallback)NameValueTableIntHash);
	  SymPutFieldByIndex(d->Table, index, 
			     SYMTAB_PRIVATE_SCOPES, (Generic) ht);
	}
	if (NameValueTableAddPair(ht, (Generic)enclosing_scope, (Generic)0, &dummy))
	  tc_ERROR(d, stmt, ft_PRIVATE_DUPLICATE_DEF);
	
	pvar = next_pvar;
      }
    }
    break;
    default:
    tc_ERROR(d, stmt, ft_PRIVATE_IN_BAD_CONTEXT);
    break;
  }
}

/************************************************************************************/
/* Check executable statements.                                                     */
/************************************************************************************/

static void
CheckAssignment(SymDescriptor d, AST_INDEX stmt)
{
  int lht, rht;
  AST_INDEX lval = gen_ASSIGNMENT_get_lvalue(stmt);
  AST_INDEX rval = gen_ASSIGNMENT_get_rvalue(stmt);
  
  /* check the assignment statement */
  
  lht = tcExprCheck(d, lval);
  rht = tcExprCheck(d, rval);
  if (assign_type_map[lht][rht] == TYPE_ERROR) { 
    /* no point setting the statement's type */
    tc_ERROR(d, stmt, ft_INCOMPAT_TYPES_IN_ASSIGN);
  }
  else { /* else, assign it a converted type from the map */
    sct(gen_ASSIGNMENT_get_rvalue(stmt),rhs_assign_type_map[lht][rht]);
  }
}

static void 
CheckIf(SymDescriptor d, AST_INDEX stmt)
{
  AST_INDEX guard_list, guard, next_guard;
  AST_INDEX son;
  int	t;
  
  guard_list = gen_IF_get_guard_LIST(stmt);
  if (guard_list == AST_NIL)
    return;
  
  guard = list_first(guard_list);
  while(guard != AST_NIL) {

    next_guard = list_next(guard);

    /* check for else and else if labels */
    /* 7/19/91 harv */
    (void) tcExprCheck(d, gen_GUARD_get_lbl_def(guard));
    
    son = gen_GUARD_get_rvalue(guard);
    t = tcExprCheck(d,son);
    if (t != TYPE_LOGICAL)
      sct(son, TYPE_LOGICAL);
    
    son = gen_GUARD_get_stmt_LIST(guard);
    if (son != AST_NIL)
      doGuardStmtList(d,son);
    
    guard = next_guard;
  }
}

/************************************************************************************/
/* Predicates to determine whether Assignments should be Stmt Functions, or Stmt    */
/* Functions should be Assignments.                                                 */
/************************************************************************************/

static Boolean AssignmentShouldBeStmtFunction(SymDescriptor d, AST_INDEX stmt)
{
  AST_INDEX lval;
  int       dims;
  fst_index_t lindex;
  char     *lname;

  assert(is_assignment(stmt));

  lval = gen_ASSIGNMENT_get_lvalue(stmt);

  if (!is_subscript(lval))
    return false;

  lname = gen_get_text(gen_SUBSCRIPT_get_name(lval));
  lindex = SymQueryIndex(d->Table, lname);

  /* previously undeclared variable, or variable not declared as array */
  return BOOL(lindex == SYM_INVALID_INDEX || 
     (SymGetFieldByIndex(d->Table, lindex, SYMTAB_NUM_DIMS) <= 0));
}

static Boolean StmtFunctionShouldBeAssignment(SymDescriptor d, AST_INDEX stmt)
{
  AST_INDEX sf_name;
  char     *text;
  int       index;

  assert(is_stmt_function(stmt));

  sf_name = gen_STMT_FUNCTION_get_name(stmt);
  text = gen_get_text(sf_name);
  index = SymQueryIndex(d->Table, text);

  /* variable not declared ... this must be a statement function */
  if (index == SYM_INVALID_INDEX)
    return false;
 
  /* should be assignment if name on stmt func is name of array variable,
     i.e., has more than 0 dimensions */
  return BOOL(SymGetFieldByIndex(d->Table, index, SYMTAB_NUM_DIMS) > 0);
}

/************************************************************************************/
/* Coercion functions, stmt funcs <=> assignment statements.                        */
/************************************************************************************/

static AST_INDEX CoerceAssignmentToStmtFunc(SymDescriptor d, AST_INDEX stmt)
{
  AST_INDEX lval;
  AST_INDEX rval;
  AST_INDEX lbl, name, formals, New;

  assert(is_assignment(stmt));

  lval = gen_ASSIGNMENT_get_lvalue(stmt);
  rval = gen_ASSIGNMENT_get_rvalue(stmt);

  tree_replace(lval, AST_NIL);
  lbl = gen_ASSIGNMENT_get_lbl_def(stmt);
  tree_replace(lbl, AST_NIL);
  tree_replace(rval, AST_NIL);
	
  /* coerce node to be a statment function */
  name = gen_SUBSCRIPT_get_name(lval);
  tree_replace(name, AST_NIL);
  formals = gen_SUBSCRIPT_get_rvalue_LIST(lval);
  tree_replace(formals, AST_NIL);
  tree_free(lval);
  
  New = gen_STMT_FUNCTION(lbl, name, formals, rval);
  tree_replace(stmt, New);
  tree_free(stmt);
  return New;
}

static AST_INDEX CoerceStmtFuncToAssignment(SymDescriptor d, AST_INDEX stmt)
{
  AST_INDEX lbl, formals, lval, rval, New, name;

  assert(is_stmt_function(stmt));

  /* remove the pieces of the statement function */
  lbl = gen_STMT_FUNCTION_get_lbl_def(stmt);
  name = gen_STMT_FUNCTION_get_name(stmt);
  formals = gen_STMT_FUNCTION_get_formal_arg_LIST(stmt);
  rval = gen_STMT_FUNCTION_get_rvalue(stmt);

  tree_replace(lbl, AST_NIL);
  tree_replace(name, AST_NIL);
  tree_replace(formals, AST_NIL);
  tree_replace(rval, AST_NIL);
    
  /* coerce node to be an assignment statement */
  lval = gen_SUBSCRIPT(name, formals);
  New = gen_ASSIGNMENT(lbl, lval, rval);
  tree_replace(stmt, New);
  tree_free(stmt);
  
  return New;  
}

/*******************************************************************************/
/* Auxiliary functions that should be defined elsewhere. - groups.c, in fact   */
/*******************************************************************************/

static Boolean
is_supported_smp_executable_stmt(AST_INDEX stmt)
{
  return BOOL(is_f77_executable_stmt(stmt) 
	      || is_do(stmt)   /* is_do(stmt) permits all three forms of loops to be passed
				  through as part of the smp dialect */
	      || is_parallelloop(stmt) 
	      || is_do_all(stmt));
	      
}

static Boolean
is_supported_smp_specification_stmt(AST_INDEX stmt)
{
  return BOOL(is_f77_specification_stmt(stmt) || is_private(stmt));
}

/*******************************************************************************/
/* Trivial auxiliary functions that are probably defined elsewhere.            */
/*******************************************************************************/

static AST_INDEX identifier_in_var_ref(AST_INDEX node)
{
  if (is_identifier(node))
    return node;
  else if (is_subscript(node))
    return gen_SUBSCRIPT_get_name(node);
  else if (is_substring(node))
  {
    node = gen_SUBSTRING_get_substring_name(node);
    if (is_subscript(node))
      return gen_SUBSCRIPT_get_name(node);
    else
      return node;
  }
}

/* a simple routine to pick an integer out of a string */
static char *
get_num(char *char_ptr, int *num_ptr)
{
  int sign, num;
  
  while (*char_ptr == ' ') /* skip leading blanks */
    char_ptr++;
  
  if (*char_ptr != '-')
    sign = 1;
  else
    {
      sign = -1;
      char_ptr++;
    }
  
  num = 0;
  while(*char_ptr >= '0' && *char_ptr <= '9')
    num = num * 10 + (*char_ptr++ - '0');
  
  if (sign == -1)
    num = -num;
  
  *num_ptr = num;
  return char_ptr;
}

static Boolean InvocationIsAnIntrinsicOrGeneric(AST_INDEX invnode)
{
  AST_INDEX id;
  char *name;
  
  id = gen_INVOCATION_get_name(invnode);
  name = gen_get_text(id);
  return ((builtins_intrinsicFunctionInfo(name) ||
	  builtins_genericFunctionInfo(name)) ? true : false);
}

/*
 * PARASCOPE RESTRICTION ENFORCEMENT:
 */

static void
ProhibitStmtFuncBodyContainsProcCalls(SymDescriptor d, AST_INDEX defstmt, 
                                      AST_INDEX node)
{
  int sc;
  int sym_index;

  if (is_invocation(node) && (!InvocationIsAnIntrinsicOrGeneric(node)))
    tc_ERROR(d, defstmt, ft_RESTRICTION_STMT_FUNC_NO_PROC_CALLS);
  else if (is_list(node))
  {
    AST_INDEX elt, next_elt;
    for (elt = list_first(node); elt != AST_NIL; elt = next_elt)
    {
      next_elt = list_next(elt);
      ProhibitStmtFuncBodyContainsProcCalls(d, defstmt, elt);
    }
  }
  else 
  {
    int n = gen_how_many_sons(gen_get_node_type(node));
    int i;
    for (i=1;i<=n;i++)
      ProhibitStmtFuncBodyContainsProcCalls(d, defstmt, ast_get_son_n(node,i));      
  }
}

static void 
ProhibitStmtFuncContainsProcCalls(SymTable t, int i, SymDescriptor d)
{
  AST_INDEX body, expr, defstmt;
  int sc = SymGetFieldByIndex(t, i, SYMTAB_STORAGE_CLASS);

  if (sc & SC_STMT_FUNC)
  {
    body = SymGetFieldByIndex(t, i, SYMTAB_SF_EXPR);
    /* get rhs of statement function definition */
    expr = SymGetFieldByIndex(t, i, SYMTAB_EXPR);
    /* get the statement itself */
    defstmt = out(expr);
    ProhibitStmtFuncBodyContainsProcCalls(d, defstmt, body);
  }
}

static void
ProhibitStmtFuncsContainProcCalls(SymDescriptor d)
{
  SymForAll(d->Table, (SymIteratorFunc)ProhibitStmtFuncContainsProcCalls,
                       (Generic) d);
}

static void
ProhibitActualArgContainsProcCalls(SymDescriptor d, AST_INDEX node)
{
  int sc;
  int sym_index;

  if (is_invocation(node) && (!InvocationIsAnIntrinsicOrGeneric(node)))
    tc_ERROR(d, node, ft_RESTRICTION_STMT_FUNC_ARG_CONTAINS_PROC_CALL);
  else if (is_list(node))
  {
    AST_INDEX elt, next_elt;
    for (elt = list_first(node); elt != AST_NIL; elt = next_elt)
    {
      next_elt = list_next(elt);
      ProhibitActualArgContainsProcCalls(d, elt);
    }
  }
  else 
  {
    int n = gen_how_many_sons(gen_get_node_type(node));
    int i;
    for (i=1;i<=n;i++)
      ProhibitActualArgContainsProcCalls(d, ast_get_son_n(node,i));
  }
}

