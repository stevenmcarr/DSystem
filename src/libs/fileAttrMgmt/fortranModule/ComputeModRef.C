/* $Id: ComputeModRef.C,v 1.3 1997/06/24 17:38:03 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/ipAnalysis/ipInfo/ProcScalarModRefInfo.h>


#include <ctype.h>
#include <assert.h>
#include <stdio.h>

#include <libs/support/strings/StringSet.h>
#include <libs/support/tables/cNameValueTable.h>

#include <libs/frontEnd/ast/forttypes.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/ast/treeutil.h>

#include <libs/ipAnalysis/ipInfo/iptypes.h>

#include <libs/frontEnd/include/stmt_func.h>

/*
 * BUGS:
 *   stmt_funcs should be walked for ref information
 *   data and type initialization statements need to be walked
 *	for ref / mod information
 */

/************************************************************************
 This module contains the routines used in the generation of initial 
 information in the module editor.
************************************************************************/
static ProcScalarModRefInfo *BuildProcScalarModRefInfo
(char *entry_name, SymDescriptor d, StringSet *vars);

static void visit(FortTree ft, SymDescriptor proc_sym_table, AST_INDEX node, 
	StringSet *vars, ModRefType vt);

static AST_INDEX get_name_astindex_from_ref(AST_INDEX evar);


/*
 * This entry point computes IMOD and IUSE information for the each module in
 * the program version specified by the context.  (This information differs
 * slightly from that described in the papers; local variables are excluded
 * from the sets!)
 *
 * The IMOD and IUSE information is written into a file named "initial" in the
 * appropriate module version directory. 
 */


ProcLocalInfo *ComputeProcScalarModRefInfo(FortTree ft, AST_INDEX node)
{
  ProcScalarModRefInfo *summary; 
  char *procName = gen_get_text(get_name_in_entry(node));
  
  AST_INDEX stmt_list  = get_stmts_in_scope(node);
  
  StringSet vars[2]; // index with ModRefType
  
  // Entering the active part of the procedure, so we can assume that any
  // reference is a USE.  Turn on the using flag, walk the list, and reset
  // the flag.
  
  SymDescriptor proc_sym_table = ft_SymGetTable(ft, procName);
  
  assert(proc_sym_table != 0);
  
  visit(ft, proc_sym_table, stmt_list, vars, MODREFTYPE_REF);
  
  
  
  switch (gen_get_node_type(node)) {
  case GEN_FUNCTION:
  case GEN_SUBROUTINE:
    //-------------------------------------------
    // multiple entrypoints handled transparently 
    // fall through to handle the routine itself
    //-------------------------------------------
  case GEN_PROGRAM:
  case GEN_BLOCK_DATA: 
    summary = BuildProcScalarModRefInfo(procName, proc_sym_table, vars);
    return summary;
  default:
    break;
  }
  return 0;
}


static Boolean 
entry_name_symdesc_name_To_leader_offset_size_vtype(char *entry_name,
	SymDescriptor d, const char *name, char **leader, int *offset, 
	unsigned int *size, unsigned int *vtype)
{
  fst_index_t name_index, leader_index;
  unsigned int type = 0;
  Boolean leader_set = false;
  
  name_index = fst_QueryIndex(d, (char *) name);
  fst_Symbol_To_EquivLeader_Offset_Size(d, name_index, &leader_index,
					offset, size);
  assert(fst_index_is_valid(leader_index));
  *leader = (char *) fst_GetFieldByIndex(d, leader_index, SYMTAB_NAME);
  
  int oc = fst_GetFieldByIndex(d, name_index, SYMTAB_OBJECT_CLASS);
  
  // if symbol is a parameter, it must be a parameter of the current
  // entry point, otherwise it is not included in mod/ref information
  // (it is an error to reference a parameter of entry e1 in an 
  // invocation of entry e2)
  if (oc & (OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG)) {
    Generic param_position;
    int entry_index = fst_QueryIndex(d, entry_name);
    assert(fst_index_is_valid(entry_index));
    cNameValueTable ht = 
      (cNameValueTable) fst_GetFieldByIndex(d, entry_index, SYMTAB_FORMALS_HT);
    if (ht && NameValueTableQueryPair(ht, (Generic) name, &param_position)) {
      *offset = (int)param_position;
      type |= VTYPE_FORMAL_PARAMETER;
    }
    else return false;
  }
  
  int sc = fst_GetFieldByIndex(d, name_index, SYMTAB_STORAGE_CLASS);
  if (sc & SC_EXTERNAL) type |= VTYPE_PROCEDURE;
  else {
    int ocl = fst_GetFieldByIndex(d, leader_index, SYMTAB_OBJECT_CLASS);
    if (ocl & OC_IS_COMMON_NAME) type |= VTYPE_COMMON_DATA;
    else type |= VTYPE_LOCAL_DATA;
  }
  *vtype = type;
  return true;
}


static EqClassScalarModRefInfo *GetEntry(ProcScalarModRefInfo *summary, 
					 const char *leader, unsigned vtype)
{
  EqClassScalarModRefInfo *info = summary->GetEntry(leader);
  if (!info) {
    info = new EqClassScalarModRefInfo(leader, vtype);
    summary->AddEntry(info);
  }
  return info;
}

static void CollectPairs(char *entry_name, SymDescriptor d, StringSet *vars, 
			 ProcScalarModRefInfo *summary, ModRefType which)
{
  const char *name; 
  char *leader; 
  int offset; 
  unsigned size, vtype;
  
  StringSetIterator mods(&vars[which]);
  for(; name = mods.Current(); ++mods) {
    if (entry_name_symdesc_name_To_leader_offset_size_vtype
	(entry_name, d, name, &leader, &offset, &size, &vtype)) {
      (GetEntry(summary, leader, vtype))->pairs[which].AddPair
	(new OffsetLengthPair(offset, size));
    }
  }
}


static ProcScalarModRefInfo *BuildProcScalarModRefInfo
(char *entry_name, SymDescriptor d, StringSet *vars) 
{ 
  ProcScalarModRefInfo *summary = new ProcScalarModRefInfo(entry_name);

  CollectPairs(entry_name, d, vars, summary, MODREFTYPE_REF);
  CollectPairs(entry_name, d, vars, summary, MODREFTYPE_MOD);

  return summary;
}


static void
visit(FortTree ft, SymDescriptor proc_sym_table,  AST_INDEX node, 
      StringSet *vars, ModRefType vt)
{
  NODE_TYPE       node_type = gen_get_node_type (node);
  AST_INDEX       elt;
  int             i,n;
  
  switch (node_type) {
  case GEN_ASSIGN:
    visit(ft, proc_sym_table, gen_ASSIGN_get_name(node), vars, MODREFTYPE_MOD);
    break;
  case GEN_ASSIGNMENT:
    visit(ft, proc_sym_table, gen_ASSIGNMENT_get_rvalue(node), vars, 
	  MODREFTYPE_REF);
    visit(ft, proc_sym_table, gen_ASSIGNMENT_get_lvalue(node), vars, 
	  MODREFTYPE_MOD);
    break;
  case GEN_READ_SHORT:
    visit(ft, proc_sym_table, gen_READ_SHORT_get_format_identifier(node), vars, 
	  MODREFTYPE_REF);
    visit(ft, proc_sym_table, gen_READ_SHORT_get_data_vars_LIST(node), vars, 
	  MODREFTYPE_MOD);
    break;
  case GEN_READ_LONG:
    visit(ft, proc_sym_table, gen_READ_LONG_get_kwd_LIST(node), vars, 
	  MODREFTYPE_REF);
    visit(ft, proc_sym_table, gen_READ_LONG_get_io_LIST(node), vars, 
	  MODREFTYPE_MOD);
    break;
  case GEN_SUBSCRIPT:
    visit(ft, proc_sym_table, gen_SUBSCRIPT_get_name(node), vars, vt);
    visit(ft, proc_sym_table, gen_SUBSCRIPT_get_rvalue_LIST(node), vars, 
	  MODREFTYPE_REF);
    break;
  case GEN_INVOCATION:
    {
      // accumulate a list of all invocations (except for intrinsics
      // and statement functions) here as part of collection of loop 
      // based interprocedural information -- JMC 
      
      fst_index_t findex = fst_QueryIndex(proc_sym_table, 
					  gen_get_text(gen_INVOCATION_get_name(node)));
      int sc = fst_GetFieldByIndex(proc_sym_table, findex, 
				   SYMTAB_STORAGE_CLASS);
      
      if (sc & SC_STMT_FUNC) {
	AST_INDEX sf_expr = 
	  stmt_func_invocation_to_expression(proc_sym_table, node);
	
	visit(ft, proc_sym_table, sf_expr, vars, MODREFTYPE_REF);
	tree_free(sf_expr);
      } else {
	ModRefType mutype;
	if (sc & (SC_INTRINSIC | SC_GENERIC)) {
	  mutype = MODREFTYPE_REF;
	} else {
	  // note: USE as an actual is not in IUSE since it 
	  // would cause spurious recompilation    
	  mutype = MODREFTYPE_NEITHER;
	}
	visit(ft, proc_sym_table, 
	      gen_INVOCATION_get_actual_arg_LIST(node), vars, mutype);
      }
    }
    break;
  case GEN_SUBSTRING:
    visit(ft, proc_sym_table, gen_SUBSTRING_get_substring_name(node), vars, vt);
    visit(ft, proc_sym_table, gen_SUBSTRING_get_rvalue1(node), vars, 
	  MODREFTYPE_REF);
    visit(ft, proc_sym_table, gen_SUBSTRING_get_rvalue2(node), vars, 
	  MODREFTYPE_REF);
    break;
  case GEN_IMPLIED_DO:
    visit(ft, proc_sym_table, gen_IMPLIED_DO_get_imp_elt_LIST(node), vars, vt);
    visit(ft, proc_sym_table, gen_IMPLIED_DO_get_name(node), vars, 
	  MODREFTYPE_MOD);
    visit(ft, proc_sym_table, gen_IMPLIED_DO_get_rvalue1(node), vars, 
	  MODREFTYPE_REF);
    visit(ft, proc_sym_table, gen_IMPLIED_DO_get_rvalue2(node), vars, 
	  MODREFTYPE_REF);
    visit(ft, proc_sym_table, gen_IMPLIED_DO_get_rvalue3(node), vars, 
	  MODREFTYPE_REF);
    break;
  case GEN_INDUCTIVE:
    visit(ft, proc_sym_table, gen_INDUCTIVE_get_name(node), vars, 
	  MODREFTYPE_MOD);
    visit(ft, proc_sym_table, gen_INDUCTIVE_get_rvalue1(node), vars, 
	  MODREFTYPE_REF);
    visit(ft, proc_sym_table, gen_INDUCTIVE_get_rvalue2(node), vars, 
	  MODREFTYPE_REF);
    visit(ft, proc_sym_table, gen_INDUCTIVE_get_rvalue3(node), vars, 
	  MODREFTYPE_REF);
    break;
  case GEN_EXIST_QUERY:
  case GEN_OPENED_QUERY:
  case GEN_NUMBER_QUERY:
  case GEN_NAMED_QUERY:
  case GEN_NAME_QUERY:
  case GEN_ACCESS_QUERY:
  case GEN_SEQUENTIAL_QUERY:
  case GEN_DIRECT_QUERY:
  case GEN_FORM_QUERY:
  case GEN_FORMATTED_QUERY:
  case GEN_UNFORMATTED_QUERY:
  case GEN_RECL_QUERY:
  case GEN_BLANK_QUERY:
  case GEN_NEXTREC_QUERY:
  case GEN_IOSTAT_QUERY:
    /* get the lvalue */
    visit(ft, proc_sym_table, gen_get_son_n(node, 1), vars, MODREFTYPE_MOD);
    break;
  case GEN_FILE_SPECIFY:
  case GEN_FMT_SPECIFY:
  case GEN_REC_SPECIFY:
  case GEN_END_SPECIFY:
  case GEN_STATUS_SPECIFY:
  case GEN_ACCESS_SPECIFY:
  case GEN_FORM_SPECIFY:
  case GEN_RECL_SPECIFY:
  case GEN_BLANK_SPECIFY:
  case GEN_UNIT_SPECIFY:
  case GEN_ERR_SPECIFY:
    visit(ft, proc_sym_table, gen_ERR_SPECIFY_get_lbl_ref(node), vars, 
	  MODREFTYPE_REF);
    break;
  case GEN_IDENTIFIER:
    {
      char *text = gen_get_text(node);
      int index = fst_QueryIndex(proc_sym_table,  text);
      
      assert(fst_index_is_valid(index));
      int oc = fst_GetFieldByIndex(proc_sym_table, index, 
				   SYMTAB_OBJECT_CLASS);
      
      if (oc & OC_IS_DATA) {
	switch(vt) {
	case MODREFTYPE_REF:
	case MODREFTYPE_MOD: 
	  vars[vt].Add(text); break;
	default: break;
	}
      }
      break;
    }
    
  case GEN_LIST_OF_NODES:
    elt = list_first(node);
    while (elt != AST_NIL) {
      visit(ft, proc_sym_table, elt, vars, vt);
      elt = list_next(elt);
    }
    break;
  case GEN_COMMON:
  case GEN_DIMENSION:
  case GEN_EQUIVALENCE:
    /* First node is label, second is list of elements */
    visit(ft, proc_sym_table, gen_get_son_n(node, 2), vars, MODREFTYPE_NEITHER);
    break;
    
    /* Only refs to formals or manifest constants in subscript expressions 
      * are of interest -- JMC
      */
  case GEN_TYPE_STATEMENT:
    visit(ft, proc_sym_table, gen_TYPE_STATEMENT_get_array_decl_len_LIST(node),
	  vars, MODREFTYPE_NEITHER);
    break;
  case GEN_ARRAY_DECL_LEN:
    /* Only looking at list of dimensions */
    visit(ft, proc_sym_table, gen_ARRAY_DECL_LEN_get_dim_LIST(node), vars, 
	  MODREFTYPE_REF);
    break;
    
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
    /* REF in an expression is always in IREF. this case is necessary
      * to ensure that all components of expressions passed as actuals 
      * in a function invocation are noted as referenced -- JMC
      */
    visit(ft, proc_sym_table, gen_get_son_n(node, 1), vars, MODREFTYPE_REF);
    visit(ft, proc_sym_table, gen_get_son_n(node, 2), vars, MODREFTYPE_REF);
    break;
  case GEN_PARALLELLOOP:
  case GEN_DO:
  case GEN_DO_ALL:
    {
      n = gen_how_many_sons(node_type);
      for(i = 1; i <= n; i++)
	visit(ft, proc_sym_table, gen_get_son_n(node, i), vars, 
	      MODREFTYPE_NEITHER);
    }
    break;
    
  case GEN_EXTERNAL:
  case GEN_IMPLICIT:
  case GEN_INTRINSIC:
  case GEN_STMT_FUNCTION:
  case GEN_ENTRY:
    /* these constructs have no affect on mod/use info */
    break;
    
    /* skip these for today */
  case GEN_PARAMETER:
  case GEN_SAVE:
  case GEN_DATA:
    break;
    
  default:
    n = gen_how_many_sons(node_type);
    for(i = 1; i <= n; i++)
      visit(ft, proc_sym_table, gen_get_son_n(node, i), vars, vt);
    break;
  }
}

