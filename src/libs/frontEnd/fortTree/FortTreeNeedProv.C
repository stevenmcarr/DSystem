/* $Id: FortTreeNeedProv.C,v 1.6 94/05/17 */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/frontEnd/include/NeedProvSet.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/strings/rn_string.h>

#include <libs/frontEnd/fortTree/fortsym.i>
#include <libs/frontEnd/fortTree/FortTree.i>
#include <libs/frontEnd/fortTree/TypeChecker.h>
#include <libs/frontEnd/fortTree/ft.h>

typedef enum {
  USE_AFTER_DEF,
  DEF_AFTER_USE,
  USE_AFTER_USE,
  DEF_AFTER_DEF
  } InterfacePairType;

//*********************************
// forward declarations
//*********************************

static void ft_BuildNeedProvsForProcedure(FortTree ft, FortTreeNode node, 
					  SymDescriptor d, char *proc_name);

static void addToProv(FortTree ft, FortTreeNode elt, SymDescriptor d, 
		      char *proc_name);

static void addToNeed(FortTree ft, FortTreeNode elt, char *entryName, 
		      SymDescriptor d, char *scopeName);

static void checkInterface(SymDescriptor d, AST_INDEX node, 
			   ProcInterface *ent1, 
			   ProcInterface *ent2, InterfacePairType type);

static uint ArgListLength(AST_INDEX list);



//*********************************
// interface operations
//*********************************

void ft_InitNeedProvs(FortTree ft)
{
  /* create the needs and provs lists, these lists may have been allocated on
     previous calls, so delete them first if necessary. */
  if(ft->needs) delete ft->needs;
  ft->needs = new NeedProvSet();

  if(ft->provs) delete ft->provs;
  ft->provs = new NeedProvSet();
}

void ft_BuildNeedProvs(FortTree ft, FortTreeNode node, SymDescriptor d)
{
  int          nodetype;

  assert(d != 0); // valid symbol table to resolve parameter lists

  if (node == AST_NIL) 
    return;

  nodetype = gen_get_node_type(node);
  switch (nodetype)
  {
  case GEN_PROGRAM:
  case GEN_FUNCTION:
  case GEN_SUBROUTINE: 
  case GEN_BLOCK_DATA: 
    {
       char *proc_name = gen_get_text(get_name_in_entry(node));
       addToProv(ft, node, d, proc_name);
       ft_BuildNeedProvsForProcedure(ft, node, d, proc_name);
    }
    break;
  case GEN_COMMENT:
    break;
  default:
    fprintf(stderr,
	    "ft_BuildNeedProvs: invoked with node of type %s\n", 
	    gen_node_type_get_text(nodetype));
    break;
  }
}

void ft_BuildNeedProvsForProcedure(FortTree ft,
				   FortTreeNode node,
				   SymDescriptor d,
				   char *proc_name)
{
  FortTreeNode elt,p;
  int          i,n;
  
  switch(gen_get_node_type(node)) {
  case GEN_LIST_OF_NODES:
    elt = list_first(node);
    while(elt != AST_NIL) {
      ft_BuildNeedProvsForProcedure(ft, elt, d, proc_name);
      elt = list_next(elt);
    }
    break;
    
  case GEN_ENTRY:
    addToProv(ft, node, d, proc_name);
    break;
    
  case GEN_IDENTIFIER:
    p = tree_out(node);
    if (is_invocation(p) && node != gen_INVOCATION_get_name(p))
      {/* an actual argument that might be an external routine*/
	addToNeed(ft, node, gen_get_text(node), d, proc_name);
      }
    break;
    
  case GEN_INVOCATION:
    addToNeed(ft, node, gen_get_text(gen_INVOCATION_get_name(node)), d, proc_name);
    ft_BuildNeedProvsForProcedure(ft, 
				  gen_INVOCATION_get_actual_arg_LIST(node),
				  d, proc_name);
    break;
    
  default:
    n = gen_how_many_sons(gen_get_node_type(node));
    for(i = 1; i <= n; i++)
      ft_BuildNeedProvsForProcedure(ft, gen_get_son_n(node, i), d, proc_name);
    break;
  }
}



//******************************
// private operations
//******************************

// 
// an entry, function, program, subroutine, block data
//
static void addToProv(FortTree ft, FortTreeNode node, SymDescriptor d, 
		      char *proc_name)
{
  ProcInterfaceType pusage;
  FortTreeNode args, a;
  
  char       *name  = gen_get_text(get_name_in_entry(node));
  fst_index_t index = fst_QueryIndex(d, name);
  int         sc    = fst_GetFieldByIndex(d, index, SYMTAB_STORAGE_CLASS);
  
  // Note: any identifier that appears as the name of an ENTRY statement has
  // storage class bit SC_SUBROUTINE or SC_FUNCTION set
  
  if (sc & SC_FUNCTION) {
    pusage = PI_FUNCTION;
    
    // FUNCTION entry 
    if (sc & SC_ENTRY) args = gen_ENTRY_get_formal_arg_LIST(node);
    else args = gen_FUNCTION_get_formal_arg_LIST(node);
    
  } else if (sc & SC_SUBROUTINE) {
    pusage = PI_SUBROUTINE;
    
    // SUBROUTINE entry 
    if (sc & SC_ENTRY) args = gen_ENTRY_get_formal_arg_LIST(node);
    else args = gen_SUBROUTINE_get_formal_arg_LIST(node);
    
  } else {
    args = AST_NIL;
    if (sc & SC_PROGRAM) pusage = PI_PROGRAM;
    else pusage = PI_BLOCK_DATA;
  }
  
  ProcInterface *npEntry = 
    new ProcInterface(proc_name, name, pusage, (pusage == PI_FUNCTION) ?
		      fst_GetFieldByIndex(d, index, SYMTAB_TYPE) : TYPE_NONE);
  
  npEntry->AllocateAllArgs(ArgListLength(args));
  
  for (a=list_first(args); a != AST_NIL; a = list_next(a)) {
    if (is_star(a)) {
      npEntry->InitNextArg(PI_LABEL, TYPE_LABEL, "");
    } else {
      char *aname = gen_get_text(a);
      fst_index_t a_index = fst_QueryIndex(d, aname);
      int a_sc = fst_GetFieldByIndex(d, a_index, SYMTAB_STORAGE_CLASS);
      
      if (a_sc & (SC_FUNCTION | SC_SUBROUTINE)) {
	npEntry->InitNextArg(PI_PROCEDURE, 
			  fst_GetFieldByIndex(d, a_index, SYMTAB_TYPE),
			  aname);
      } else if (a_sc & SC_EXTERNAL) { 
	npEntry->InitNextArg(PI_PROCEDURE, TYPE_UNKNOWN, aname);
      } else {
	
	// something is wrong if formal not a VARIABLE 
	assert(fst_GetFieldByIndex(d, a_index, SYMTAB_OBJECT_CLASS) & 
	       OC_IS_DATA);
	
	npEntry->InitNextArg(PI_VARIABLE, 
			  fst_GetFieldByIndex(d, a_index, SYMTAB_TYPE),
			  aname);
      }
    }
  }
  
  
  ProcInterface *found;
  
  if (found = ft->needs->GetEntry(name)) {
    /* Definition of already referenced procedure */
    checkInterface(d, node, found, npEntry, DEF_AFTER_USE);
  }
  
  if (found = ft->provs->GetEntry(name)) {
    /* Redefinition of procedure within module */
    checkInterface(d, node, found, npEntry, DEF_AFTER_DEF);
  } else {
    /* First definition of procedure */
    ft->provs->AddEntry(npEntry);
  }
}


// 
// an identifier, or an invocation 
//
static void addToNeed(FortTree ft, FortTreeNode node, char *entryName, 
		      SymDescriptor d, char *scopeName)
{
  FortTreeNode args, a;
  
  fst_index_t index = fst_QueryIndex(d, entryName);
  int sc            = fst_GetFieldByIndex(d, index, SYMTAB_STORAGE_CLASS);
  int oc            = fst_GetFieldByIndex(d, index, SYMTAB_OBJECT_CLASS);
  
  // never add the name of a procedure parameter to the needs map
  if (oc & (OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG)) return;
  
  
  // check storage class here to ensure that entries in the needs
  // list are not created for identifiers that are not external 
  // routines 
  if (sc & (SC_FUNCTION | SC_SUBROUTINE | SC_EXTERNAL)) { // a new definition 
    
    ProcInterfaceType pusage;
    int ptype;
    
    if (sc & SC_FUNCTION) {
      pusage = PI_FUNCTION;
      ptype  = fst_GetFieldByIndex(d, index, SYMTAB_TYPE);
    }
    else if (sc & SC_SUBROUTINE) {
      pusage = PI_SUBROUTINE;
      ptype = TYPE_NONE;
    }
    else {
      // arises from an EXTERNAL declaration for a routine that is
      // used only as an actual 
      pusage = PI_UNKNOWN_ROUTINE;
      ptype = TYPE_UNKNOWN;
    }
    
    ProcInterface *npEntry = 
      new ProcInterface(scopeName, entryName, pusage, ptype);
    
    if ( is_invocation(node) ) {
      /* count args and types, allocate args ... */
      args = gen_INVOCATION_get_actual_arg_LIST(node);
      
      npEntry->AllocateAllArgs(ArgListLength(args));
      
      for ( a=list_first(args); a!=AST_NIL; a=list_next(a) ) {
	if (is_identifier(a)) {
	  char *aname = gen_get_text(a);
	  fst_index_t a_index = fst_QueryIndex(d, aname);
	  int a_sc = fst_GetFieldByIndex(d, a_index, SYMTAB_STORAGE_CLASS);
	  
	  if (a_sc & (SC_FUNCTION | SC_SUBROUTINE)) {
	    
	    npEntry->InitNextArg(PI_PROCEDURE, 
			      fst_GetFieldByIndex(d, a_index, SYMTAB_TYPE),
			      aname);
	    
	  } else if (a_sc & SC_EXTERNAL) {
	    
	    npEntry->InitNextArg(PI_PROCEDURE, TYPE_UNKNOWN, aname);
	    
	  } else if (a_sc & SC_INTRINSIC) {
	    
	    npEntry->InitNextArg(PI_PROCEDURE, 
			      fst_GetFieldByIndex(d, a_index, SYMTAB_TYPE),
			      aname);
	    
	  } else if (fst_GetFieldByIndex(d, a_index, SYMTAB_OBJECT_CLASS) & 
		     OC_IS_DATA) {
	    
	    npEntry->InitNextArg(PI_VARIABLE, 
			      fst_GetFieldByIndex(d, a_index, SYMTAB_TYPE),
			      aname);
	  } else {
	    assert(0);
	    // old code claimed to get here for statement 
	    // functions and constants. I dont see how 
	    // that could be the case -- JMC
	  }
	} 
	else if( is_return_label(a)) {
	  npEntry->InitNextArg(PI_LABEL, TYPE_LABEL, "");
	} else {
	  npEntry->InitNextArg(PI_EXPRESSION, gen_get_real_type(a), "");
	}
      }
    }
    
    ProcInterface *found;
    
    if (found = ft->provs->GetEntry(entryName)) {
      // Reference to entry that has previously been defined in module
      checkInterface(d, node, found, npEntry, USE_AFTER_DEF);
    }
    
    if (found = ft->needs->GetEntry(entryName)) {
      // Reference to already referenced entry 
      checkInterface(d, node, found, npEntry, USE_AFTER_USE);
      delete npEntry; 
    } else {
      // First reference to procedure. add entry to needs map 
      ft->needs->AddEntry(npEntry);
    }
  }
}

static void checkInterface(SymDescriptor d, AST_INDEX node, 
			   ProcInterface *ent1, ProcInterface *ent2,
			   InterfacePairType type)
{
  if ( ent1->Consistent(ent2) != TYPE_MATCH_OK || type == DEF_AFTER_DEF) {
    switch (type) {
    case DEF_AFTER_DEF:
      // should already have been detected in WalkProcedures 
      tc_ERROR(d, node, ft_DUPLICATE_DECLARATION);
      break;
    case DEF_AFTER_USE:
      tc_ERROR(d, node, ft_DEF_CONFLICTS_WITH_PRIOR_USE);
      break;
    case USE_AFTER_DEF:
      tc_ERROR(d, node, ft_USE_CONFLICTS_WITH_PRIOR_DEF);
      break;
    case USE_AFTER_USE:
      tc_ERROR(d, node, ft_USE_CONFLICTS_WITH_PRIOR_USE);
      break;
    }
  }
}


static uint ArgListLength(AST_INDEX list)
{
  return (list != AST_NIL) ? list_length(list) : 0;
}
      
