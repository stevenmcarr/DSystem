/* $Id: AFormalQuery.C,v 1.1 1997/03/11 14:35:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*************************************************************************
//
//  AFormalQuery.C 
//
//    query interface for whether a parameter to a procedure is ever 
//    accessed as an array by that procedure or any of its descendants 
//    in the callgraph
//
//    author: John Mellor-Crummey                            July 1992
//
//    modification history:
//      John Mellor-Crummey                               January 1993
//        updated to use new CallGraph interface
//
//    Copyright 1992, 1993, Rice University, as part of the ParaScope 
//    Programming Environment Project.
//
//*************************************************************************

#include <stdio.h>
#include <assert.h>

#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/problems/rsd/AFormalAnnot.h>
#include <libs/ipAnalysis/problems/rsd/AFormalQuery.h>

//*************************************************************************
//
// Boolean IPQuery_ParamIsAFormal
//         a query interface for interprocedural analysis of whether a 
//         formal parameter of a procedure is ever indexed as an array by 
//         the procedure itself, or any procedures reachable from it.
//         
//         this capability will eventally be subsumed by regular section
//         analysis
//         
//*************************************************************************
Boolean IPQuery_ParamIsAFormal(Generic cg, const char *procEntry, 
			       unsigned int paramIndex)
{
  CallGraph *callGraph = (CallGraph *) cg;
  if (callGraph == NULL) return true; // no callgraph --> conservative reply
  
  CallGraphNode *node = callGraph->LookupNode(procEntry);
  if (node == NULL) return true; // stale callgraph --> conservative reply
  
  AFormalAnnot *annot = (AFormalAnnot *) 
    node->GetAnnotation(AFORMAL_ANNOT, true); // demand annotation
  assert(annot != NULL);
  
  const char *paramName = node->FormalPositionToName(procEntry, paramIndex);
  if (paramName == NULL) 
    // mismatched interface (stale callgraph?) --> conservative reply
    return true; 
  
  return BOOL(annot->IsMember(paramName));
}


//*************************************************************************
// 
// Boolean IPQuery_CalleeParamIsAFormal
//         a query interface for interprocedural analysis of whether an 
//         actual parameter to a procedure is ever indexed as an array by 
//         the procedure itself, or any procedures reachable from it.
//         
//         if an actual parameter is only accessed as a scalar and never as
//         an array, then some dependences can be identified as loop
//         independent rather than loop carried. 
//
//         an actual parameter to a procedure is never indexed as an array
//         by a procedure or any of that procedure's descendants in the call 
//         chain only if the corresponding formal parameter of the procedure
//         is never indexed as an array.
//     
//         this capability will eventally be subsumed by regular section
//         analysis
//         
//*************************************************************************
Boolean IPQuery_CalleeParamIsAFormal(Generic cg, const char *caller, 
				     int callsiteId, unsigned int paramIndex)
{
  CallGraph *callGraph = (CallGraph *) cg;
  if (callGraph == NULL) return true; // no callgraph --> conservative reply
  
  CallGraphEdge *edge = callGraph->LookupEdge(caller, callsiteId);
  
  if (edge == NULL) return true; // stale callgraph --> conservative reply
  
  // actual used as array iff corresponding callee formal
  // used as an array -- JMC 7/92
  return IPQuery_ParamIsAFormal(cg, edge->calleeEntryName, paramIndex);
}
