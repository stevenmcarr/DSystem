/* $Id: AFormalDFProblem.C,v 1.2 1997/03/27 20:41:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//****************************************************************************
//
//    AFormalDFProblem.C
//
//    determine if a formal parameter is ever used as an array 
//    by the current procedure or any of the procedures it calls
//
//    author: John Mellor-Crummey                             September 1994
//
//    Copyright 1994, Rice University. All rights reserved.
//****************************************************************************


#include <stdio.h>
#include <assert.h>

#include <libs/fileAttrMgmt/fortranModule/AFormalModAttr.h>

#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotReg.h>
#include <libs/ipAnalysis/problems/rsd/AFormalDFProblem.i>
#include <libs/ipAnalysis/problems/rsd/AFormalAnnot.h>



//****************************************************************************
// class AFormalDFProblem interface operations
//****************************************************************************

//----------------------------------------------------------------------------
// AFormalDFProblem::AFormalDFProblem 
//    compute a top and bottom element for the formal usage dataflow problem
//----------------------------------------------------------------------------
AFormalDFProblem::AFormalDFProblem() : 
CallGraphFlowInsensitiveDFP(CLASS_NAME(AFormalModAttr), newAFormalTop, 
			    DirectedEdgeIn) 
{
}


DataFlowSet *AFormalDFProblem::newAFormalTop()
{
  return new AFormalAnnot();
}


//----------------------------------------------------------------------------
// void AFormalDFProblem::InitializeNode
//   initial set for formal usage dataflow problem contains the formals that
//   are declared as an array in the procedure
//----------------------------------------------------------------------------
DataFlowSet *AFormalDFProblem::InitializeNode
(CallGraphNode *, ProcLocalInfo *pli)
{
  ProcAFormalInfo *afi = (ProcAFormalInfo *) pli;
  AFormalAnnot *annot = new AFormalAnnot;

  if (afi) *annot |= *(StringSet *) afi;

  return annot;
}


DataFlowSet *AtNode
(CallGraphNode *, const DataFlowSet *nodeInit, DataFlowSet *meetResult)
{
  *meetResult |= *nodeInit;
  return meetResult;
}


DataFlowSet *NodeToEdge(CallGraphNode *node, CallGraphEdge *edge, 
			const DataFlowSet *nodeOut)
{
  AFormalAnnot *edgeIn = new AFormalAnnot;
  AFormalAnnot *calleeAnnot = (AFormalAnnot *) nodeOut;
  
  ParamNameIterator fnames(edge->paramBindings, FormalNameSet); 
  const char *formal;
  for( ; formal = fnames.Current(); ++fnames) {
    if (calleeAnnot->IsMember(formal)) {
      ParamBinding *binding = 
	edge->paramBindings.GetReverseBinding(formal);
      if (binding->a_class & APC_DataFormal) edgeIn->Add(binding->actual);
    }
  }
  
  return edgeIn;
}


DataFlowSet *Meet(CallGraphEdge *, CallGraphNode *, const DataFlowSet *nodeIn,
		  DataFlowSet *meetPartialResult)
{
  *meetPartialResult |= *nodeIn;
  return meetPartialResult;
}
