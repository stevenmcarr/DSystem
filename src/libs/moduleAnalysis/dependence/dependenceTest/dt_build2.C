/* $Id: dt_build2.C,v 1.6 1997/03/11 14:35:53 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

//****************************************************************************
// dt_build2.C                                    John Mellor-Crummey 10/92
//
// this file provides support for 
// (1) building dependence reference lists in the presence of callsites
// (2) building strings that conservatively describe portion of an array 
//     accessed as a side-effect of a callsite
//****************************************************************************

#include <stdarg.h>
#include <stdlib.h>

// shared declarations for the dependence graph builder
#include <libs/moduleAnalysis/dependence/dependenceTest/dt_build.i>

#include <libs/support/tables/cNameValueTable.h>

// interface for interprocedural information
#include <libs/ipAnalysis/ipInfo/iptypes.h>
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/problems/modRef/ScalarModRefAnnot.h>

//****************************
// forward declarations
//****************************

STATIC(void, dg_invoc_globals_add_refs, 
          (SymDescriptor d, CallGraphEdge *e, Dg_ref_params *r, char *aname, 
           int ref_type, AST_INDEX invoc));

STATIC(void, dg_add_global_refV, 
          (fst_index_t nindex, Generic value, va_list args));

STATIC(void, dg_invoc_globals_conservative,
          (SymDescriptor d, int callsite_id, Dg_ref_params *r, AST_INDEX invoc));



//************************************************************************
// Interface operations
//************************************************************************


//--------------------------------------------------------------------------
// dg_invoc_globals                               John Mellor-Crummey 9/92
//
// make entries in the reference list for side effects to arguments and
// global variables
//
// if there is relevant interprocedural information in the callgraph,
// use it to augment the reference lists; if not, make conservative
// assumptions
//--------------------------------------------------------------------------
void dg_invoc_globals(SymDescriptor d, int callsite_id, Dg_ref_params *r, 
		 AST_INDEX invoc)
{
  CallGraph *cg = (CallGraph *) r->program_callgraph;
  char *caller = r->current_proc;

  if (cg != NULL) {
    CallGraphEdge *e = cg->LookupEdge(caller, callsite_id);
    if (e != NULL) {
      dg_invoc_globals_add_refs(d, e, r, SCALAR_MOD_ANNOT, T_DEF, invoc);
      dg_invoc_globals_add_refs(d, e, r, SCALAR_REF_ANNOT, T_USE, invoc);
      return;
    }
  }
  dg_invoc_globals_conservative(d, callsite_id, r, invoc);
}


//--------------------------------------------------------------------------
// dg_conservative_rsd_string                      John Mellor-Crummey 9/92
//
// construct an RSD for a conservative approximation of the data
// that may be modified by a procedure invocation
//
// name may be the name of a common block unknown to this scope, a scalar, 
// or an array
//
// for an unknown common, the RSD string is just the common name 
//
// for a scalar, the RSD string is just the scalar name
//
// for an array, the RSD contains a * for each dimension of the array 
// indicating that our conservative approximation assumes that the entire 
// array may be modified as a side-effect of the procedure invocation
//--------------------------------------------------------------------------

#define MAX_STRING 128
char *dg_conservative_rsd_string(FortTree ft, AST_INDEX node, char *name)
{
  static char buffer[MAX_STRING];
  int ndims = 0; /* default -- no array dimensions */

  // get the symbol table for the reference from the reference itself
  // by brute force 
  // WARNING: this may result in poor performance 
  AST_INDEX scope = find_scope(node);
  SymDescriptor d = 
    ft_SymGetTable(ft, gen_get_text(get_name_in_entry(scope)));

  int nindex = fst_QueryIndex(d, name);

  // nindex will be invalid for an unknown common block name
  if (fst_index_is_valid(nindex)) { 
    // determine number of array dimensions if any
    ndims = fst_GetFieldByIndex(d, nindex, SYMTAB_NUM_DIMS); 
  }
  
  // build the RSD string -- variable name with a * for each array 
  // dimension
  char *bp = &buffer[0];
  while (*bp = *name++) bp++; /* copy name string */
  if (ndims > 0) {
    *bp++ = '('; *bp++ = '*';
    while (--ndims > 0) {
      *bp++ = ','; *bp++ = '*';
    }
    *bp++ = ')';
  }
  *bp = 0;

  return buffer;
}



//************************************************************************
// Private operations
//************************************************************************



//--------------------------------------------------------------------------
// dg_invoc_globals_add_refs                      John Mellor-Crummey 9/92
//
// for each triple (common name, offset, size) in a
// the side-effect summary set for a callsite. 
//   if the common is known in the current procedure 
//     translate the triple into a list of symbol table indices for
//     common variables in this scope. these indices are stored in a 
//     hashtable to eliminate duplicate indices since more than one triple
//     might overlap a particular common variable (since common block
//     definitions may not always be consistent among procedures, a triple
//     may not exactly align with a sequence of common variables in the
//     current procedure. thus, the variables at the ends of the sequence 
//     may arise again for some other non-overlapping triple.)
//
//     add each symbol in the hashtable to the reference list
//--------------------------------------------------------------------------

static void dg_invoc_globals_add_refs(SymDescriptor d, CallGraphEdge *e, 
				      Dg_ref_params *r, char *aname, 
				      int ref_type, AST_INDEX invoc)
{
  ScalarModRefAnnot *sma = 
    (ScalarModRefAnnot *) e->GetAnnotation(aname, true);
  
  // create a hashtable to hold symbol table indices of globals in the
  // side-effect summary set for the callsite edge 
  cNameValueTable globals = NameValueTableAlloc(8, 
                                                NameValueTableIntCompare, 
                                                NameValueTableIntHash);
  
#if 0
  // interate through all globals 
  char *name; 
  VarScope scope; 
  int offset, length;
  for (ScalarModRefAnnotIterator smri(sma); 
       name = smri.Current(scope, offset, length); smri++) {
    if (scope != GlobalScope) continue;
    fst_index_t cindex = fst_QueryIndex(d, name);
    
    // if common block is known in this procedure
    if (fst_index_is_valid(cindex)) { 
      // translate a (common name, offset, length) triple into a sequence
      // of symbol table indices for the common variables known to 
      // this procedure that overlap the triple

      if (length == INFINITE_INTERVAL_LENGTH) {
	// set length to cover entire common block -- JMC 3/93
	length = fst_GetFieldByIndex(d, cindex, SYMTAB_SIZE);
      }

      EquivalenceClass *symbols = 
	fst_Symbol_Offset_Size_To_VarSymbols(d, cindex, offset, length);
      
      // add all symbols overlapping the triple to the hash table
      for(int i=0; i < symbols->members; i++) {
	Generic dummy;
	NameValueTableAddPair(globals, (Generic)symbols->member[i], (Generic)0, 
			      &dummy);
      }
      
      // after adding the symbols to the hashtable, this sequence of indices 
      // is no longer needed, discard it 
      free(symbols); 
    } else {
      // add a reference to an unknown common block
      dg_add_ref(r, invoc, name, T_IP_GLOBAL | T_IP_WHOLE_COMMON | ref_type);
    }
  }
#else
  assert(0);
#endif

  // add each symbol in the hashtable to the reference list
  NameValueTableForAllV(globals, 
                        (NameValueTableForAllCallbackV)dg_add_global_refV, 
                        d, r, ref_type, invoc);

  NameValueTableFree(globals);
}


//--------------------------------------------------------------------------
// dg_add_global_ref                              John Mellor-Crummey 9/92
//
// add a reference to a global to the reference list
// the reference is a conservative approximation: it indicates 
// the whole array is assumed to be accessed 
//--------------------------------------------------------------------------
static void dg_add_global_ref(SymDescriptor d, fst_index_t nindex, 
			      Dg_ref_params *r, AST_INDEX invoc, int ref_type)
{
  char *name = (char *) fst_GetFieldByIndex(d, nindex, SYMTAB_NAME); 
  dg_add_ref(r, invoc, name, T_IP_GLOBAL | T_IP_WHOLE_ARRAY | ref_type);
}


//--------------------------------------------------------------------------
// dg_add_global_refV                             John Mellor-Crummey 9/92
//
// see dg_global_add_ref for description of real functionality. this 
// routine is just an interface stub so that all of the globals in 
// a hashtable can be added by iterating over the hashtable contents.
//--------------------------------------------------------------------------
static void dg_add_global_refV(fst_index_t nindex, Generic value, 
			       va_list args)
{
  SymDescriptor d  = va_arg(args, SymDescriptor);
  Dg_ref_params *r = va_arg(args, Dg_ref_params *);
  int ref_type = va_arg(args, int);
  AST_INDEX invoc = va_arg(args, AST_INDEX);

  dg_add_global_ref(d, nindex, r, invoc, ref_type);
}


//--------------------------------------------------------------------------
// add_common_refs                               John Mellor-Crummey 9/92
//
// if the symbol is a common block, conservatively treat all variables in the 
// common block (both declared common variables and variables in the common 
// block through equivalencing) as having been modified at the current
// callsite by adding a definition entry for the variable to the 
// variable reference list to be used by the dependence tester 
//--------------------------------------------------------------------------
static void add_common_refs(SymDescriptor d, fst_index_t sindex, va_list args)
{
  AST_INDEX invoc = va_arg(args, AST_INDEX);
  int callsite_id = va_arg(args, int);
  Dg_ref_params *r = va_arg(args, Dg_ref_params *);
  
  int oc = fst_GetFieldByIndex(d, sindex, SYMTAB_OBJECT_CLASS);
  
  // if the current symbol is a common block
  if (oc & OC_IS_COMMON_NAME) {	
    EquivalenceClass *cvars = fst_OverlappingSymbols(d, sindex);
    for(int i = 0; i < cvars->members; i++) {
      dg_add_global_ref(d, cvars->member[i], r, invoc, T_DEF | 
			T_IP_CONSERVATIVE);
    }
  }
}


//--------------------------------------------------------------------------
// dg_invoc_globals_conservative                   John Mellor-Crummey 9/92
//
// conservatively assume that all common variables are modified at the 
// current callsite
//--------------------------------------------------------------------------
static void dg_invoc_globals_conservative(SymDescriptor d, int callsite_id, 
					  Dg_ref_params *r, AST_INDEX invoc)
{
  fst_ForAllV(d, add_common_refs, invoc, callsite_id, r);
}

