/* $Id: LocalModRef.C,v 1.16 1997/03/11 14:29:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/* Author : Seema Hiranandani                                           */
/* This file contains a rewrite of the mod/ref problem                  */
/************************************************************************/
#include <ctype.h>
#include <assert.h>
#include <stdio.h>

#include <libs/frontEnd/fortTree/FortTree.i>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/frontEnd/ast/treeutil.h>

#include <libs/frontEnd/fortTree/fortsym.i>
#include <libs/frontEnd/fortTree/TypeChecker.h>
#include <libs/frontEnd/fortTree/InitInfo.h>

/* C++ includes for mod/ref information */

#include <libs/frontEnd/fortTree/modrefnametree.h>
#include <libs/ipAnalysis/ipInfo/iptree.h>
#if 0
#include <libs/ipAnalysis/ipInfo/module.h>
#endif
#include <libs/ipAnalysis/ipInfo/iptypes.h>

#include <libs/frontEnd/include/stmt_func.h>

//-------------------------------------------------------------------------
// ModRefProblem constructor and destructor
//-------------------------------------------------------------------------
ModRefProblem::ModRefProblem()
{
   modreftype = MODREFTYPE_NEITHER;
   I = (ModRefNameTreeNode*)0;
}

ModRefProblem::~ModRefProblem()
{
   // should we delete I?????

   // delete I;
}

/***************************************************************************/
/* This function walks the entire function and collects information for    */
/*   ModRef                                                                */
/***************************************************************************/
void ModRefProblem::ModRef(AST_INDEX node, Generic LocInfo)
{
  NODE_TYPE       node_type = gen_get_node_type (node);
  AST_INDEX       elt;
  int             i,n;
  
  ModRefProblem P;
  P.ProblemInfo(GetModRefType(), GetModRefNameTreeNode());
  
  switch (node_type) {
    
  case GEN_ASSIGN:
    P.SetModRefType(MODREFTYPE_MOD); 
    P.ModRef(gen_ASSIGN_get_name(node), LocInfo);
    break;
    
  case GEN_ASSIGNMENT:
    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_ASSIGNMENT_get_rvalue(node), LocInfo);
    
    P.SetModRefType(MODREFTYPE_MOD);
    P.ModRef(gen_ASSIGNMENT_get_lvalue(node), LocInfo);
    break;
    
  case GEN_READ_SHORT:
    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_READ_SHORT_get_format_identifier(node), LocInfo);
    
    P.SetModRefType(MODREFTYPE_MOD);
    P.ModRef(gen_READ_SHORT_get_data_vars_LIST(node), LocInfo);
    break;
    
  case GEN_READ_LONG:
    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_READ_LONG_get_kwd_LIST(node), LocInfo);
    
    P.SetModRefType(MODREFTYPE_MOD);
    P.ModRef(gen_READ_LONG_get_io_LIST(node),  LocInfo);
    break;

  case GEN_WRITE:
    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_WRITE_get_kwd_LIST(node), LocInfo);
    P.ModRef(gen_WRITE_get_data_vars_LIST(node), LocInfo);
    
  case GEN_PRINT:
    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_PRINT_get_format_identifier(node), LocInfo);
    
    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_PRINT_get_data_vars_LIST(node),  LocInfo);
    break;
    
  case GEN_SUBSCRIPT:
    P.ModRef(gen_SUBSCRIPT_get_name(node), LocInfo);

    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_SUBSCRIPT_get_rvalue_LIST(node), LocInfo);
    break;
    
  case GEN_INVOCATION: {
	// accumulate a list of all invocations (except for intrinsics
	// and statement functions) here as part of collection of loop 
	// based interprocedural information -- JMC 
    
    fst_index_t findex = 
      fst_QueryIndex(((LInfo*)LocInfo)->proc_sym_table, 
		     gen_get_text(gen_INVOCATION_get_name(node)));
    
    int sc = fst_GetFieldByIndex(((LInfo*)LocInfo)->proc_sym_table,
				 findex,SYMTAB_STORAGE_CLASS);
    
    if (sc & SC_STMT_FUNC) {
      AST_INDEX sf_expr = 
	stmt_func_invocation_to_expression(((LInfo*)LocInfo)->proc_sym_table,
					   node);
      
      P.ModRef(sf_expr, LocInfo);
      tree_free(sf_expr);
      
    } else {
      if (sc & (SC_INTRINSIC | SC_GENERIC)) {
	P.SetModRefType(MODREFTYPE_REF); 
      } else {
	    // variables passed as actuals to user functions 
	    // are not added to IREF  
	P.SetModRefType(MODREFTYPE_NEITHER);
	P.GetModRefNameTreeNode()->ilist->Append(node);
      }
      P.ModRef(gen_INVOCATION_get_actual_arg_LIST(node),
	       LocInfo);
    }
  }
    
    break;
  case GEN_SUBSTRING:
    P.ModRef(gen_SUBSTRING_get_substring_name(node), LocInfo);
    
    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_SUBSTRING_get_rvalue1(node), LocInfo);
    
    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_SUBSTRING_get_rvalue2(node), LocInfo);
    break;
    
  case GEN_IMPLIED_DO:
    
    P.ModRef(gen_IMPLIED_DO_get_imp_elt_LIST(node), LocInfo);
    
    P.SetModRefType(MODREFTYPE_MOD);
    P.ModRef(gen_IMPLIED_DO_get_name(node), LocInfo);
    
    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_IMPLIED_DO_get_rvalue1(node), LocInfo);
    
    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_IMPLIED_DO_get_rvalue2(node), LocInfo);
    
    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_IMPLIED_DO_get_rvalue3(node), LocInfo);
    break;
    
  case GEN_INDUCTIVE:
    
    P.SetModRefType(MODREFTYPE_MOD);
    P.ModRef(gen_INDUCTIVE_get_name(node), LocInfo);
    
    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_INDUCTIVE_get_rvalue1(node), LocInfo);
    
    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_INDUCTIVE_get_rvalue2(node), LocInfo);
    
    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_INDUCTIVE_get_rvalue3(node), LocInfo);
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
    P.SetModRefType(MODREFTYPE_MOD);
    P.ModRef(gen_get_son_n(node, 1), LocInfo);
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
    
    P.SetModRefType(MODREFTYPE_REF); 
    P.ModRef(gen_ERR_SPECIFY_get_lbl_ref(node), LocInfo);
    break;
    
  case GEN_IDENTIFIER: {
#if 0
    char *text = gen_get_text(node);
    int index = fst_QueryIndex(((LInfo*)LocInfo)->proc_sym_table,  text);
    
    assert(fst_index_is_valid(index));
    int oc = fst_GetFieldByIndex(((LInfo*)LocInfo)->proc_sym_table,
				 index, SYMTAB_OBJECT_CLASS);
    if (oc & OC_IS_DATA)
      GetModRefNameTreeNode()->refs->Add(text, GetModRefType());	
    break;
#endif
  }
    
  case GEN_LIST_OF_NODES:
    elt = list_first(node);
    while (elt != AST_NIL) {
      P.ModRef(elt, LocInfo);
      elt = list_next(elt);
    }
    break;
    
  case GEN_COMMON:
  case GEN_DIMENSION:
  case GEN_EQUIVALENCE:
	/* First node is label, second is list of elements */
    P.SetModRefType(MODREFTYPE_NEITHER);
    P.ModRef(gen_get_son_n(node, 2), LocInfo);
    break;
    
	/* Only refs to formals or manifest constants in subscript expressions 
	  * are of interest -- JMC
	  */
    
  case GEN_TYPE_STATEMENT:
    P.SetModRefType(MODREFTYPE_NEITHER);
    P.ModRef(gen_TYPE_STATEMENT_get_array_decl_len_LIST(node),
	     LocInfo);
    break;
    
  case GEN_ARRAY_DECL_LEN:
	/* Only looking at list of dimensions */
    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_ARRAY_DECL_LEN_get_dim_LIST(node), LocInfo);
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
    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_get_son_n(node, 1), LocInfo);
    
    P.SetModRefType(MODREFTYPE_REF);
    P.ModRef(gen_get_son_n(node, 2), LocInfo);
    break;
    
  case GEN_PARALLELLOOP:
  case GEN_DO:
  case GEN_DO_ALL:
    {
      
      ModRefNameTreeNode *Loop = 
	new ModRefNameTreeNode(((node_type == GEN_DO) ? LOOP : PARLOOP), 
			       ft_NodeToNumber(((LInfo*)LocInfo)->ft, node),
			       GetModRefNameTreeNode());
      
      n = gen_how_many_sons(node_type);
      P.SetModRefNameTreeNode(Loop);
      P.SetModRefType(MODREFTYPE_NEITHER);
      for(i = 1; i <= n; i++)
	P.ModRef(gen_get_son_n(node, i), LocInfo);
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
      P.ModRef(gen_get_son_n(node, i), LocInfo);
    break;
  }
}

/*****************************************************************************/
/* write the information for the ModRefProblem                               */
/*****************************************************************************/
void ModRefProblem::WriteLocalInfo(Generic LocInfo)
{
#if 0
  ((LInfo*)LocInfo)->m->Write(*((LInfo*)LocInfo)->dbport);
#endif
}

/*****************************************************************************/
/* this function initiates the mod ref computation                           */
/*****************************************************************************/
void ModRefProblem::ComputeLocalInfo(AST_INDEX stmt_list, Generic LocalInfo)
{
  AST_INDEX node, current;
  ProblemInfo(MODREFTYPE_NEITHER, ((LInfo*)LocalInfo)->I);
  ModRef(stmt_list, LocalInfo);
  
  node =   ((LInfo*)LocalInfo)->node;
  
  switch (gen_get_node_type(node))
    {
    case GEN_FUNCTION:
    case GEN_SUBROUTINE:
      
#if 0
	  // information for entry points 
      current = list_first(stmt_list);
      while (current != AST_NIL) {
	if (is_entry(current)) {
	  ((LInfo*)LocalInfo)->m->Append(loop_tree_info_for_entry
					       (LocalInfo, current)); 
	}
	current = list_next(current);
      }
#endif
      
	  // fall through to handle the routine itself
    case GEN_PROGRAM:
    case GEN_BLOCK_DATA:
	  // information for the routine
      ((LInfo*)LocalInfo)->ipt = loop_tree_info_for_entry
					   (LocalInfo, node);
      break;
    }
}
