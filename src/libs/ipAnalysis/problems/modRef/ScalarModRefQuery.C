/* $Id: ScalarModRefQuery.C,v 1.4 1997/03/11 14:35:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <assert.h>

#include <libs/support/misc/general.h>

#include <libs/ipAnalysis/ipInfo/EqClassPairs.h>

#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>

#include <libs/ipAnalysis/problems/modRef/ScalarModRefAnnot.h>
#include <libs/ipAnalysis/problems/modRef/ScalarModRefQuery.h>

//////////////////////////////////////////////////////////////////////////////
//
//  For variables at call sites (edges)
//
static Boolean IPQuery_IsScalarModRef(char *aname, Generic callGraph, 
				      char *caller, int callsite, char *vname, 
				      int offset, int length)
{
  CallGraph *cg = (CallGraph *) callGraph;
  if (cg == NULL) return true;

  CallGraphEdge *edge = cg->LookupEdge (caller, callsite);
  if (edge == NULL) return true;

  ScalarModRefAnnot *smr = 
    (ScalarModRefAnnot *) edge->GetAnnotation(aname, true);
  EqClassPairs *entry = smr->globals.QueryEntry(vname);
  
  if (entry == 0) return false;

  return BOOL(entry->Overlaps(offset, length));
}

Boolean IPQuery_IsScalarRef(Generic callGraph, char *caller, int callsite, 
			    char *vname, int offset, int length)
{
  return IPQuery_IsScalarModRef(SCALAR_REF_ANNOT, callGraph, caller, 
				callsite, vname, offset, length);
}


Boolean IPQuery_IsScalarMod(Generic callGraph, char *caller, int callsite, 
			    char *vname, int offset, int length)
{
  return IPQuery_IsScalarModRef(SCALAR_MOD_ANNOT, callGraph, caller, 
				callsite, vname, offset, length);
}

//////////////////////////////////////////////////////////////////////////////
//
//  For actual parameters at call sites (look at sink of edge)
//
static Boolean IPQuery_IsScalarModRefArg(char *aname, Generic callGraph, 
					 char *caller, int callsite, 
					 int actual_pos)
{
  CallGraph *cg = (CallGraph *) callGraph;
  if (cg == NULL) return true;

  CallGraphEdge *edge = cg->LookupEdge (caller, callsite);
  if (edge == NULL) return true;

  const char *formal = 
    edge->Callee()->FormalPositionToName(edge->calleeEntryName, actual_pos);

  if (!formal) return false; // position not found

  ParamBinding *boundActual = edge->paramBindings.GetReverseBinding(formal);
  
  if (!boundActual) return false; // position not found

  ScalarModRefAnnot *smr = 
    (ScalarModRefAnnot *) edge->GetAnnotation(aname, true);

  int result = 0;
  if (boundActual->a_class & APC_DataFormal) {
    result  = smr->formals.IsMember(boundActual->actual);
  } else {
    EqClassPairs *entry;
    if (boundActual->a_class & APC_DataGlobal) {
      entry = smr->globals.QueryEntry(boundActual->actual);
    } else if (boundActual->a_class & APC_DataLocal) {
      entry = smr->locals.QueryEntry(boundActual->actual);
    } else { 
      assert(0); // should never get here -- jmc
    }
    if (entry)
      result = entry->Overlaps(boundActual->a_offset, boundActual->a_length);
  }

  return result ? true : false;
}

Boolean IPQuery_IsScalarRefArg(Generic callGraph, char *caller, int callsite, 
			       int actual_pos)
{
  return IPQuery_IsScalarModRefArg(SCALAR_REF_ANNOT, callGraph, 
				   caller, callsite, actual_pos);
}

Boolean IPQuery_IsScalarModArg(Generic callGraph, char *caller, int callsite, 
			       int actual_pos)
{
  return IPQuery_IsScalarModRefArg(SCALAR_MOD_ANNOT, callGraph, 
				   caller, callsite, actual_pos);
}

//////////////////////////////////////////////////////////////////////////////
//
//  For variables in whole procedures (nodes)
//
static Boolean IPQuery_IsScalarModRefNode(char *aname, Generic callGraph, 
					  char *proc, char *vname, 
					  int offset, int length)
{
  CallGraph *cg = (CallGraph *) callGraph;
  if (cg == NULL) return true;

  CallGraphNode *node = cg->LookupNode (proc);
  if (node == NULL) return true;

  ScalarModRefAnnot *smr = 
    (ScalarModRefAnnot *) node->GetAnnotation(aname, true);
  EqClassPairs *entry = smr->globals.QueryEntry(vname);

  if (entry == 0) return false;

  return BOOL(entry->Overlaps(offset, length));
}

Boolean IPQuery_IsScalarRefNode(Generic callGraph, char *proc, 
				char *vname, int offset, int length)
{
  return IPQuery_IsScalarModRefNode(SCALAR_GREF_ANNOT, callGraph, proc, 
				    vname, offset, length);
}


Boolean IPQuery_IsScalarModNode(Generic callGraph, char *proc, 
				char *vname, int offset, int length)
{
  return IPQuery_IsScalarModRefNode(SCALAR_GMOD_ANNOT, callGraph, proc, 
				    vname, offset, length);
}
