/* $Id: InitInfo.C,v 1.18 1997/03/11 14:29:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Header $ */
/****************************************************************/
/* Author : John Mellor-Crummey (Modified by Seema Hiranandani) */
/* Date   : 2/10/92                                             */
/* the functions in this file compute procedure parameters,     */
/* common blocks and mod/ref information                        */
/****************************************************************/

#include <ctype.h>
#include <assert.h>
#include <stdio.h>

#include <libs/frontEnd/fortTree/FortTree.i>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/frontEnd/ast/treeutil.h>

#include <libs/support/tables/cNameValueTable.h>

#include <libs/frontEnd/fortTree/fortsym.i>
#include <libs/frontEnd/fortTree/InitInfo.h>
#include <libs/frontEnd/fortTree/TypeChecker.h>

/* C++ includes for mod/ref information */

#include <libs/frontEnd/fortTree/modrefnametree.h>
#include <libs/ipAnalysis/ipInfo/iptree.h>
#if 0
#include <libs/ipAnalysis/ipInfo/module.h>
#endif
#include <libs/ipAnalysis/ipInfo/iptypes.h>

/* forward declarations */

static CallSite* invocation_to_callsite(FortTree ft, SymDescriptor d, 
				 char *caller_name, FortTreeNode invocation);

static IPinfoTreeNode *iptree_build(FortTree ft, char *entry_name, 
			     SymDescriptor d, ModRefNameTreeNode *I, 
			     IPinfoTreeNode *parent);

// 2/24/94 RvH: loop_tree_info_for_entry() is used elsewhere!
extern IPinfoTree *loop_tree_info_for_entry(Generic LocInfo, AST_INDEX proc);

static void IPinfoTree_build_formal_list(ParameterList *plist, SymDescriptor d,
				  AST_INDEX formals);


/*********************************************************************/
/* Create an IPinfoTree structure for an entry point in the module   */
/* procedure name and parameter information is stored                */
/*********************************************************************/
IPinfoTree *
loop_tree_info_for_entry(Generic LocalInfo, AST_INDEX proc)
{
  
  IPinfoTreeNode *tnode;
  char *entry_name = gen_get_text(get_name_in_entry(proc));
  AST_INDEX formals = get_formals_in_entry(proc);
  tnode = iptree_build(((LInfo*)LocalInfo)->ft, entry_name,
		       ((LInfo*)LocalInfo)->proc_sym_table,
		       ((LInfo*)LocalInfo)->I, 0);
  ((LInfo*)LocalInfo)->tnode = tnode;
  IPinfoTree *ip_info_tree = 
    new IPinfoTree(tnode,entry_name, 
		(gen_get_node_type(proc) == GEN_PROGRAM));

  IPinfoTree_build_formal_list(ip_info_tree->plist,
			       ((LInfo*)LocalInfo)->proc_sym_table , formals);
  return ip_info_tree;
}

/*********************************************************************/
/* Store information on the relationship between MOD and REF and the */
/* parameters and common blocks                                      */
/*********************************************************************/
IPinfoTreeNode *
iptree_build(FortTree ft, char *entry_name, SymDescriptor d, 
	ModRefNameTreeNode *I, IPinfoTreeNode *parent)
{ 
  char *name, *leader;
  int offset; 
  int size;
  unsigned int vtype;
  IPinfoTreeNode *tnode = new IPinfoTreeNode(parent, I->block_id, I->type);
  
#if 0
  for(name = I->refs->first_entry(MODREFTYPE_MOD); name != 0;
      name = I->refs->next_entry(MODREFTYPE_MOD)) {
    if (entry_name_symdesc_name_To_leader_offset_size_vtype(entry_name, d, 
							    name, &leader, 
							    &offset, &size, 
							    &vtype)) {
      if (size == FST_ADJUSTABLE_SIZE) size == INFINITE_INTERVAL_LENGTH;
      tnode->refs->AddModRef(MODREFTYPE_MOD, leader, offset, size, vtype);
    }
  }
  
  for(name = I->refs->first_entry(MODREFTYPE_REF); name != 0;
      name = I->refs->next_entry(MODREFTYPE_REF)) {
    if (entry_name_symdesc_name_To_leader_offset_size_vtype(entry_name, d, 
							    name, &leader, 
							    &offset, &size, 
							    &vtype)) {
      if (size == FST_ADJUSTABLE_SIZE) size == INFINITE_INTERVAL_LENGTH;
      tnode->refs->AddModRef(MODREFTYPE_REF, leader, offset, size, vtype);
    }
  }
#endif
  InvocationListEntry *ile;
  for(ile = I->ilist->First(); ile != 0; ile = ile->Next()){
    CallSite *cs = invocation_to_callsite(ft, d, entry_name, ile->node());
    if (cs != 0) tnode->calls->Append(cs);
  }
  int i = I->ChildCount(); 
  ModRefNameTreeNode *child = (ModRefNameTreeNode *) I->FirstChild(); 
  while (i-- > 0) {
    iptree_build(ft, entry_name, d, child, tnode);
    child = (ModRefNameTreeNode *) child->NextSibling();
  }
  return tnode;
}


/**********************************************************************/
/* Build the parameter list by obtaining the name and the type of the */ 
/* parameter from the symbol table                                    */
/**********************************************************************/
void IPinfoTree_build_formal_list(ParameterList *plist, SymDescriptor d,
	AST_INDEX formals)
{
  if (formals != ast_null_node) {
    AST_INDEX node = list_first(formals);	
    
    while (node != ast_null_node) {
      switch(gen_get_node_type(node)) {
	case GEN_STAR:
	plist->Append(new ParameterListEntry("*", VTYPE_STAR));
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
	  
	  plist->Append(new ParameterListEntry(text, vtype));
	  break;
	}
	default:
	assert(0); /* JMC -- error if here */
      }
      node  = list_next( node );
    }
  }
}

/**********************************************************************/
/* Process a call site. Store information about the entry name,       */
/* if it is a procedure parameter and a list of the actual parameters */
/**********************************************************************/
CallSite *
invocation_to_callsite(FortTree ft, SymDescriptor d, char *caller_name, 
	FortTreeNode invocation)
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
					    (Generic*)&param_position))) 
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
      char *pname = gen_get_text(get_name_astindex_from_ref(arg));
      
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
	if (!(entry_ht && 
	      NameValueTableQueryPair(entry_ht, (Generic) pname, 
				      (Generic*)&param_position))) {
	      // illegal actual, the actual is a formal for an 
	      // entry point other than the current one
	      // abort processing of this callsite 
	  delete cs;
	  return 0;
	} else {
	  atype |= VTYPE_FORMAL_PARAMETER;
	  alist->Append(pname, atype, 0,  INFINITE_INTERVAL_LENGTH);
	}
      } else if (atype & VTYPE_PROCEDURE) {
	    // ****** procedure ******
	alist->Append(pname, atype, 0, 0);
      } else {
	fst_index_t leader;
	int offset = 0;
	unsigned int size = 0;
	int vtype = 0;
	
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














