/* $Id: CallGraphDFProblem.C,v 1.2 1997/03/27 20:40:12 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
//  CallGraphDFProblem.C:  
//  
//  base class for call graph dataflow analysis problems
//
//  Author: 
//    John Mellor-Crummey                                    July 1994
//
//  Copyright 1994, Rice University
//***************************************************************************



#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <libs/support/sets/WordSet.h>
#include <libs/support/sets/DataFlowSet.h>
#include <libs/support/msgHandlers/DumpMsgHandler.h>

#include <libs/support/graphs/directedGraph/DirectedGraph.h>

#include <libs/fileAttrMgmt/module/Module.h>
#include <libs/fileAttrMgmt/module/ModuleProcsIterator.h>

#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/composition/CompositionIterators.h>

#include <libs/ipAnalysis/ipInfo/ModuleLocalInfo.h>
#include <libs/ipAnalysis/ipInfo/ProcLocalInfo.h>
#include <libs/ipAnalysis/ipInfo/CallSitesLocalInfo.h>
#include <libs/ipAnalysis/ipInfo/CallSiteLocalInfo.h>

#include <libs/ipAnalysis/callGraph/CallGraphIterators.h>
#include <libs/ipAnalysis/callGraph/CallGraphDFProblem.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeSet.h>
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>
#include <libs/ipAnalysis/callGraph/ModuleInfoIterator.h>

class ModuleLocalInfo;
class ProcLocalInfo;


//***************************************************************************
// declarations
//***************************************************************************

static int debug = 0;

//****************************
// forward declarations
//****************************
static unsigned int UpdateDataFlowSet(DataFlowSet *&set, DataFlowSet *newSet);



//****************************
// class FixedSizeBitVector
//****************************
class FixedSizeBitVector {
public:
  FixedSizeBitVector(unsigned int numberOfSlots) {
    vector = (unsigned char*) calloc((size_t)numberOfSlots, 1);
    nSlots = numberOfSlots;
  };
  ~FixedSizeBitVector() { free((char*)vector); };
  unsigned char &operator[](unsigned int elemIndex) { 
    assert(elemIndex >= 0 && elemIndex < nSlots);
    return vector[elemIndex];
  };
private:
  unsigned int nSlots;
  unsigned char *vector;
};




struct CallGraphDFProblemState {
  CallGraphDFProblemState() { VALUE = 0; };
  ~CallGraphDFProblemState() { delete VALUE; };
 DataFlowSet *VALUE;
};

//***************************************************************************
// class CallGraphDFProblem interface operations
//***************************************************************************


CallGraphDFProblem::CallGraphDFProblem(const char *_localInfoClassName) :
localInfoClassName(_localInfoClassName)
{
}


CallGraphDFProblem::~CallGraphDFProblem() 
{
  delete top;
  delete bottom;
}


void CallGraphDFProblem::Initialize(DirectedGraph *dg, int)
{
  CallGraph *cg = (CallGraph *) dg;
  CallGraphNodeIterator nodes(cg, Unordered);
  CallGraphNode *node;
  CallGraphNodeSet hasLocalInfo;
  
  //---------------------------------------------------------------------
  // initialize dataflow problem at each node
  //---------------------------------------------------------------------
  if (localInfoClassName) {
    //-----------------------------------------------------------------------
    // node initialization requires local information. iterate over the 
    // modules retrieving their local information as we go ...
    //-----------------------------------------------------------------------
    ModuleInfoIterator modules(cg->program, localInfoClassName);
    for (; modules.module; modules.Advance(true)) {
      const ModuleLocalInfo *mli =  modules.moduleInfo;
      
      //-----------------------------------------------------------------------
      // for each procedure defined in the module ...
      //-----------------------------------------------------------------------
      ModuleLocalInfoIterator procedures(mli);
      for(ProcLocalInfo *pli; pli = procedures.Current(); ++procedures) {
        node = cg->LookupNode(pli->name);
        assert(node);
        //--------------------------------------------------------------------
        // perform node initialization passing local information
        //--------------------------------------------------------------------
        InitializeNode(node, pli);
        hasLocalInfo.Add(node);
      }
    }
  } 

  //--------------------------------------------------------------------
  // perform node initialization for nodes that have no local information
  //--------------------------------------------------------------------
  for (; node = nodes.Current(); ++nodes) {
    nodeSets[node->Id()].VALUE = top->Clone();
    if (hasLocalInfo.IsMember(node)) continue;
    else InitializeNode(node, 0);
  }
  nodes.Reset(Unordered);
  
  //---------------------------------------------------------------------
  // initialize dataflow problem at each edge
  //---------------------------------------------------------------------
  for (; node = nodes.Current(); ++nodes) {
    CallGraphEdgeIterator edges(node, DirectedEdgeIn);
    CallGraphEdge *edge;
    for (; edge = edges.Current(); ++edges) {
      InitializeEdge(edge);
    }
  }
}



void CallGraphDFProblem::InitializeNode (CallGraphNode *, ProcLocalInfo *) 
{
}


void CallGraphDFProblem::InitializeEdge (CallGraphEdge *) 
{
}


void CallGraphDFProblem::FinalizeAnnotation(CallGraphNode *node, 
					    CallGraphDFAnnot *annot)
{
  node->PutAnnotation(annot);
}



void CallGraphDFProblem::Solve(CallGraph *cg)
{
  unsigned int nodeCount = cg->NodeIdHighWaterMark();
  nodeSets = new CallGraphDFProblemState[nodeCount];

  DirectedGraphIterativeDFP::Solve(cg, ((direction  == Forward) ? 
					DirectedEdgeOut : DirectedEdgeIn), 0);

  delete [nodeCount] nodeSets;
}

//===========================================================================
// finalization
//===========================================================================

void CallGraphDFProblem::FinalizeNode(DirectedGraphNode *node) 
{
    FinalizeAnnotation((CallGraphNode *) node, nodeSets[node->Id()].VALUE);
    nodeSets[node->Id()].VALUE = 0;
}




//===========================================================================
// DirectedGraph solver upcalls
//===========================================================================


unsigned int CallGraphDFProblem::AtDirectedGraphNode
(DirectedGraphNode *node, DirectedEdgeDirection incoming)
{
  //--------------------------------------------------------
  // for each incoming edge:
  //   translate edgeOut to nodeIn 
  //   compute meetResult as meet of all nodeIns
  //--------------------------------------------------------
  CallGraphNode *cnode = (CallGraphNode *) node;
  DirectedEdgeDirection outgoing = 
    (incoming == DirectedEdgeIn) ? DirectedEdgeOut : DirectedEdgeIn;

  DataFlowSet *meetPartialResult = top->Clone();
  
  if (debug) {
    dumpHandler.Dump("Starting Processing for node %s\n", cnode->procName);
    dumpHandler.BeginScope();
  }
  // first meet call gets NULL edge
  meetPartialResult = (DataFlowSet *) 
    DFmeet(meetPartialResult, 0, cnode, 0);
  if (debug) {
    dumpHandler.Dump("meetPartialResult:\n");
    meetPartialResult->Dump();
  }

  // meet for each edge
  CallGraphEdgeIterator edges(cnode, incoming);
  for (CallGraphEdge *edge; edge = edges.Current(); ++edges) {
    CallGraphNode *predNode = (CallGraphNode *) edge->Endpoint(outgoing);
    assert(predNode != cnode);
    meetPartialResult = (DataFlowSet *) 
      DFmeet(meetPartialResult,  nodeSets[predNode->Id()].VALUE, cnode, edge);
    if (debug) {
      dumpHandler.Dump("meetPartialResult after processing edge %s\n", 
	edge->callSiteName);
      meetPartialResult->Dump();
    }
  }

  //--------------------------------------------------------
  // update VALUE from meetResult and INIT 
  //--------------------------------------------------------
  unsigned int nodeId = node->Id();
  unsigned char changed = 0;
  DataFlowSet *nodeValue = (DataFlowSet *) 
    DFtrans(meetPartialResult, nodeSets[nodeId].VALUE, cnode, changed);
  delete nodeSets[nodeId].VALUE;
  nodeSets[nodeId].VALUE = nodeValue;
  if (debug) {
    dumpHandler.Dump("trans output:\n");
    nodeValue->Dump();
    dumpHandler.EndScope();
  }
  return changed;
}
