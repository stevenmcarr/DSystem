/* $Id: ComputeProcSummary.C,v 1.1 1997/03/11 14:27:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ComputeProcSummary.C
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#include <libs/frontEnd/ast/AST_Set.h>
#include <libs/frontEnd/ast/AstIterators.h>
#include <libs/frontEnd/ast/treeutil.h>

#include <libs/support/tables/cNameValueTable.h>

#include <libs/ipAnalysis/ipInfo/ProcSummary.h>
#include <libs/ipAnalysis/ipInfo/CallSite.h>
#include <libs/ipAnalysis/ipInfo/iptypes.h>

#include <libs/fileAttrMgmt/fortranModule/ComputeProcSummary.i>


//************************************************************************
// local hacks to make up for deficiencies elsewhere 
// these should be fixed when the files are available 
//************************************************************************

EXTERN(Boolean, isConstantExpr, (SymDescriptor d, AST_INDEX arg));

//=======================================================================
// this function belongs in treeutil.c which was locked by
// someone else at the time this was written. there is a copy of a similarly
// named function in FortTree.C (that does the same thing) that should 
// be removed.
//=======================================================================
static AST_INDEX get_name_from_ref(AST_INDEX ref)
{
  // ref can be a GEN_IDENTIFIER, GEN_SUBSCRIPT, GEN_SUBSTRING
  switch (gen_get_node_type(ref)) {
  case GEN_IDENTIFIER:
    return ref;
    
  case GEN_SUBSCRIPT:
    return (gen_SUBSCRIPT_get_name(ref));
    
  case GEN_SUBSTRING: 
    {
      AST_INDEX tmp = gen_SUBSTRING_get_substring_name(ref);
      if (ast_get_node_type(tmp) == GEN_SUBSCRIPT)
	return (gen_SUBSCRIPT_get_name(tmp));
      else
	return tmp;
    }
  default:
    return AST_NIL;
  }
}


static void ProcSummaryFormalList(FormalParameters *fp, SymDescriptor d, 
			AST_INDEX formals)
{
  if (formals != ast_null_node) {
    AST_INDEX node = list_first(formals);	
    
    while (node != ast_null_node) {
      switch(gen_get_node_type(node)) {
      case GEN_STAR:
	fp->Append(new FormalParameter("*", VTYPE_STAR));
	break;
      case GEN_IDENTIFIER:
	{
	  char *text  = gen_get_text(node);
	  int oc = fst_GetField(d, text, SYMTAB_OBJECT_CLASS);
	  int sc = fst_GetField(d, text, SYMTAB_STORAGE_CLASS);
	  int vtype = VTYPE_NO_ATTRIBUTES; 	
	  
	  if (oc & OC_IS_EXECUTABLE) vtype |= VTYPE_PROCEDURE;
	  
	  // mark if formal parameter is declared (infer used) as an 
	  // array. this is used to sharpen dependence analysis when
	  // an array element is passed to a procedure -- JMC 7/92
	  if ((oc & OC_IS_DATA) && 
	      (fst_GetField(d, text, SYMTAB_NUM_DIMS) != 0))
	    vtype |= VTYPE_USED_AS_ARRAY;
	  
	  fp->Append(new FormalParameter(text, vtype));
	  break;
	}
      default:
	assert(0); /* JMC -- error if here */
      }
      node  = list_next( node );
    }
  }
}


static void ProcSummaryEntryPoint(EntryPoints *entries, FortTree ft, 
				  AST_INDEX entryNode, SymDescriptor d)
{
  char *entryName = gen_get_text(get_name_in_entry(entryNode));
  int nodeId = ft_NodeToNumber(ft, entryNode);
  EntryPoint *entry = new EntryPoint(entryName, nodeId);
  entries->AddEntry(entry);
  ProcSummaryFormalList(&entry->formals, d, get_formals_in_entry(entryNode));
}


static CallSite *InvocationToCallSite(FortTree ft, SymDescriptor d, 
			       char *caller_name, FortTreeNode invocation)
{
  Boolean callee_is_proc_param; // is callee a procedure parameter?
  
  char *callee_name = gen_get_text(gen_INVOCATION_get_name(invocation));
  int callee_index = fst_QueryIndex(d, callee_name);
  assert(fst_index_is_valid(callee_index));
  
  
  // look up the information about our caller\'s formal parameters
  fst_index_t entry_index = fst_QueryIndex(d, caller_name);
  assert(fst_index_is_valid(entry_index));
  cNameValueTable entry_ht = 
    (cNameValueTable) fst_GetFieldByIndex(d, entry_index, SYMTAB_FORMALS_HT);
  
  // if the callee is a procedure parameter, it must be a parameter of 
  // the caller, otherwise it is not included in callgraph information 
  // since it would be an error to invoke it from this context
  
  int oc = fst_GetFieldByIndex(d, callee_index, SYMTAB_OBJECT_CLASS);
  if (oc & (OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG)) {
    // callee is a parameter of some entry
    int param_position;
    if (!(entry_ht && NameValueTableQueryPair(entry_ht, (Generic) callee_name, 
					 &param_position))) 
      // callee is not a parameter of this entry, so abort
      // handling of this callsite
      return 0;
    else callee_is_proc_param = true;
  } else callee_is_proc_param = false;
  
  CallSite *cs = new CallSite(callee_name, ft_NodeToNumber(ft, invocation),
			      callee_is_proc_param);
  
  ActualList *alist = cs->GetActuals();
  
  AST_INDEX arglist = gen_INVOCATION_get_actual_arg_LIST(invocation);
  AST_INDEX arg;
  
  
  for (arg = list_first (arglist); arg != AST_NIL; arg = list_next (arg)){
    int atype = 0;
    if (is_constant(arg)) {
      // ****** fortran constant ******
      alist->Append(gen_get_text(arg), VTYPE_CONSTANT, 0, 0);
    } else if (is_subscript(arg) || is_substring(arg) || 
	       is_identifier(arg)) {
      /*
	* get the identifier from the reference - this may be a
	* subscript or substring reference 
	*/
      char *pname = gen_get_text(get_name_from_ref(arg));
      
      fst_index_t pindex = fst_QueryIndex(d, pname);
      assert(fst_index_is_valid(pindex));
      int oc = fst_GetFieldByIndex(d, pindex, SYMTAB_OBJECT_CLASS);
      int sc = fst_GetFieldByIndex(d, pindex, SYMTAB_STORAGE_CLASS);
      
      // determine if actual is a procedure parameter 
      // and whether or not it is an intrinsic
      if (sc & SC_EXTERNAL) // actual is an external procedure parameter
	atype = VTYPE_PROCEDURE;
      if (sc & (SC_GENERIC | SC_INTRINSIC)) // actual is an intrinsic function
	atype = (VTYPE_PROCEDURE | VTYPE_INTRINSIC);
      
      if (oc & (OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG)) {
	// ****** formal parameter ******
	int param_position;
	if (!(entry_ht && NameValueTableQueryPair(entry_ht, (Generic) pname, 
					     &param_position))) {
	  // illegal actual, the actual is a formal for an 
	  // entry point other than the current one
	  // abort processing of this callsite 
	  delete cs;
	  return 0;
	} else {
	  atype |= VTYPE_FORMAL_PARAMETER;
	  alist->Append(pname, atype, 0, 0);
	}
      } else if (atype & VTYPE_PROCEDURE) {
	// ****** procedure ******
	alist->Append(pname, atype, 0, 0);
      } else {
	fst_index_t leader;
	int offset;
	unsigned int size;
	int vtype;
	
	fst_Symbol_To_EquivLeader_Offset_Size(d, pindex, 
					      &leader, &offset, &size);
	
	if (fst_GetFieldByIndex(d, leader, SYMTAB_OBJECT_CLASS) 
	    & OC_IS_COMMON_NAME) 
	  vtype = VTYPE_COMMON_DATA;
	else vtype = VTYPE_LOCAL_DATA;
	
	alist->Append((char *) fst_GetFieldByIndex(d, leader, 
						   SYMTAB_NAME), 
		      vtype, offset, size);
      }
    } else if (is_return_label(arg)) {
      alist->Append("*", VTYPE_STAR, 0, 0);
    } else if (isConstantExpr(d, arg)) {
      // a fortran CONSTANT expression 
      // eventually the string will carry the 
      // value of the constant 
      alist->Append("const_expr", VTYPE_CONSTANT_EXPR, 0, 0);
    } else { /* an expression */
      alist->Append("compiler_temp", VTYPE_COMPILER_TEMPORARY, 0, 0);
    }
  }
  return cs;
}



//========================================================================
// accumulate a set of all invocations (except for intrinsics
// and statement functions)
//========================================================================
static void ProcessProc(ProcSummary *ps, FortTree ft, SymDescriptor symTable, 
			AST_INDEX stmtList, char *procName)
{
  
  AST_INDEX node;
  for (AstIterator nodes(stmtList); 
       (node = nodes.Current()) != AST_NIL; nodes++) {
    switch(gen_get_node_type(node)) {
    case GEN_INVOCATION: { 
      fst_index_t findex = 
	fst_QueryIndex(symTable, gen_get_text(gen_INVOCATION_get_name(node)));
      
      int sc = fst_GetFieldByIndex(symTable, findex,SYMTAB_STORAGE_CLASS);
      
      if (!(sc & (SC_STMT_FUNC | SC_INTRINSIC | SC_GENERIC))) {
	// add user function invocation to the set
	CallSite *cs = InvocationToCallSite(ft, symTable, procName, node);
	if (cs != 0) ps->calls->Add(cs);
      }
      break;
    }
    case GEN_ENTRY: 
      ProcSummaryEntryPoint(&ps->entryPoints, ft, node, symTable);
    default:
      break;
    }
  }
}

static ProcType GetProcType(AST_INDEX proc)
{
  NODE_TYPE ntype = gen_get_node_type(proc);

  switch(ntype) {
  case GEN_PROGRAM: 
    return ProcType_PGM;
  case GEN_SUBROUTINE: 
    return ProcType_SUB;
  case GEN_FUNCTION: 
    return ProcType_FUN;
  case GEN_BLOCK_DATA: 
    return ProcType_BDATA;
  default: 
    return ProcType_ILLEGAL;
  }
}


ProcLocalInfo *ComputeProcSummary(FortTree ft, AST_INDEX proc)
{
  char *procName = gen_get_text(get_name_in_entry(proc));
  ProcType type = GetProcType(proc);
  
  SymDescriptor symTable = ft_SymGetTable(ft, procName);
  
  ProcSummary *ps = new ProcSummary(procName, type);

  //----------------------------------------------------------------------
  // add entry point for procedure itself
  //----------------------------------------------------------------------
  ProcSummaryEntryPoint(&ps->entryPoints, ft, proc, symTable);
  
  //----------------------------------------------------------------------
  // collect information about nodes in the procedure
  //----------------------------------------------------------------------
  ProcessProc(ps, ft, symTable, gen_get_stmt_list(proc), procName);
  
  return ps;
}
