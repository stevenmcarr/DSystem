/* $Id: CallGraphFlowInsensitiveDFP.C,v 1.3 1999/03/31 21:55:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
//  CallGraphFlowInsensitiveDFP.C:  
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
#include <libs/support/vectors/PointerVector.h>

#include <libs/fileAttrMgmt/module/Module.h>
#include <libs/fileAttrMgmt/module/ModuleProcsIterator.h>

#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/composition/CompositionIterators.h>

#include <libs/ipAnalysis/ipInfo/ModuleLocalInfo.h>
#include <libs/ipAnalysis/ipInfo/ProcLocalInfo.h>
#include <libs/ipAnalysis/ipInfo/CallSitesLocalInfo.h>
#include <libs/ipAnalysis/ipInfo/CallSiteLocalInfo.h>

#include <libs/ipAnalysis/callGraph/CallGraphIterators.h>
#include <libs/ipAnalysis/callGraph/CallGraphFlowInsensitiveDFP.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeSet.h>
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/ModuleInfoIterator.h>

class ModuleLocalInfo;



//****************************************************************************
// declarations
//****************************************************************************


struct CallGraphFlowInsensDataFlowSets {
  CallGraphFlowInsensDataFlowSets() { INIT = 0; VALUE = 0; };
  ~CallGraphFlowInsensDataFlowSets() { delete INIT; delete VALUE; };
 DataFlowSet *INIT;
 DataFlowSet *VALUE;
};



//************************
// forward declarations
//************************
static unsigned int UpdateDataFlowSet(DataFlowSet *&set, DataFlowSet *newSet);



//***************************************************************************
// class FixedSizeBitVector
//***************************************************************************
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


//***************************************************************************
// class CallGraphFlowInsensitiveDFP interface operations
//***************************************************************************
CallGraphFlowInsensitiveDFP::CallGraphFlowInsensitiveDFP
(const char *_localInfoClassName, AnnotConstructor newTopAnnot,
 DirectedEdgeDirection direction) 
{
  localInfoClassName = _localInfoClassName;
  newTopAnnotation = newTopAnnot,
  flowDirection = direction;
}


CallGraphFlowInsensitiveDFP::~CallGraphFlowInsensitiveDFP()
{
}


void CallGraphFlowInsensitiveDFP::Solve(CallGraph *cg)
{
  unsigned int nodeCount = cg->NodeIdHighWaterMark();
  unsigned int edgeCount = cg->EdgeIdHighWaterMark();
  nodeSets = new CallGraphFlowInsensDataFlowSets[nodeCount];
  edgeSets = new CallGraphFlowInsensDataFlowSets[edgeCount];

  DirectedGraphIterativeDFP::Solve(cg, flowDirection, 0);

  delete [nodeCount] nodeSets;
  delete [edgeCount] edgeSets;
}

//===========================================================================
// initialization user-level upcalls
//===========================================================================
DataFlowSet *CallGraphFlowInsensitiveDFP::InitializeNode
(CallGraphNode *, ProcLocalInfo *) 
{
  return newTopAnnotation();
}


DataFlowSet *CallGraphFlowInsensitiveDFP::InitializeEdge
(CallGraphEdge *, CallSiteLocalInfo *) 
{
  return newTopAnnotation();
}


//===========================================================================
// CallGraph data-flow solver user-level upcalls
//===========================================================================
DataFlowSet *CallGraphFlowInsensitiveDFP::NodeToEdge
(CallGraphNode *, CallGraphEdge *, const DataFlowSet *nodeOut)
{
  return nodeOut->Clone();
}


DataFlowSet *CallGraphFlowInsensitiveDFP::EdgeToNode
(CallGraphEdge *, CallGraphNode *, const DataFlowSet *edgeOut)
{
  return edgeOut->Clone();
}


DataFlowSet *CallGraphFlowInsensitiveDFP::AtNode
(CallGraphNode *, const DataFlowSet *, DataFlowSet *meetResult)
{
  return meetResult;
}


DataFlowSet *CallGraphFlowInsensitiveDFP::AtEdge
(CallGraphEdge *, const DataFlowSet *, DataFlowSet *edgeIn)
{
  return edgeIn;
}

//===========================================================================
// finalization user-level upcalls
//===========================================================================
DataFlowSet *CallGraphFlowInsensitiveDFP::FinalizeNodeAnnotation
(CallGraphNode *, DataFlowSet *) 
{
  return 0;
}


DataFlowSet *CallGraphFlowInsensitiveDFP::FinalizeEdgeAnnotation
(CallGraphEdge *, DataFlowSet *) 
{
  return 0;
}


//*************************************
// private operations
//*************************************

//===========================================================================
// initialization
//===========================================================================

void CallGraphFlowInsensitiveDFP::Initialize(DirectedGraph *dg, int edgeClass)
{
  CallGraph *cg = (CallGraph *) dg;
  CallGraphNodeIterator nodes(cg, Unordered);
  CallGraphNode *node;

  FixedSizeBitVector nodeInitialized(dg->NodeIdHighWaterMark());
  FixedSizeBitVector edgeInitialized(dg->EdgeIdHighWaterMark());
  
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
        node = cg->LookupNode(pli->NamedObject::name);
        assert(node);
        //--------------------------------------------------------------------
        // perform node initialization passing local information
        //--------------------------------------------------------------------
	unsigned int nodeId = node->Id();
	nodeSets[nodeId].INIT = InitializeNode(node, pli);
        nodeInitialized[nodeId] = 1;
        //--------------------------------------------------------------------
        // for each callsite noted in the procedure ...
        //--------------------------------------------------------------------
	CallSitesLocalInfoIterator callsites(pli);
	for(CallSiteLocalInfo *cli; cli = callsites.Current(); ++callsites) {
	  CallGraphEdge *edge = cg->LookupEdge(pli->NamedObject::name, cli->id);
	  //------------------------------------------------------------------
	  // perform edge initialization passing local information
	  //------------------------------------------------------------------
	  unsigned int edgeId = edge->Id();
	  edgeSets[edgeId].INIT = InitializeEdge(edge, cli);
	  edgeInitialized[edgeId] = 1;
	}
      }
    }
  } 

  //--------------------------------------------------------------------
  // set value to top for each node and edge and perform
  // initialization for nodes and edges that have no local 
  // information 
  //--------------------------------------------------------------------
  for (; node = nodes.Current(); ++nodes) {
    unsigned int nodeId = node->Id();
    nodeSets[nodeId].VALUE = newTopAnnotation();
    if (!nodeInitialized[nodeId]) 
      nodeSets[nodeId].INIT = InitializeNode(node, 0);
    CallGraphEdgeIterator edges(node, DirectedEdgeOut, edgeClass);
    for (CallGraphEdge *edge; edge = edges.Current(); ++edges) {
      unsigned int edgeId = edge->Id();
      edgeSets[edgeId].VALUE = newTopAnnotation();
      if (!edgeInitialized[edge->Id()]) 
	  edgeSets[edgeId].INIT = InitializeEdge(edge, 0);
    }
  }
}


//===========================================================================
// finalization
//===========================================================================

void CallGraphFlowInsensitiveDFP::FinalizeNode(DirectedGraphNode *node) 
{
  DataFlowSet *nodeAnnot = 
    FinalizeNodeAnnotation((CallGraphNode *) node, nodeSets[node->Id()].VALUE);

  // prevent destruction of a data flow result that will live on as an annotation
  if (nodeAnnot == nodeSets[node->Id()].VALUE) nodeSets[node->Id()].VALUE = 0;
  if (nodeAnnot) node->AddAnnot(nodeAnnot);
}


void CallGraphFlowInsensitiveDFP::FinalizeEdge(DirectedGraphEdge *edge)
{
  DataFlowSet *edgeAnnot =
    FinalizeEdgeAnnotation((CallGraphEdge *) edge, edgeSets[edge->Id()].VALUE);

  // prevent destruction of a data flow result that will live on as an annotation
  if (edgeAnnot == edgeSets[edge->Id()].VALUE) edgeSets[edge->Id()].VALUE = 0;
  if (edgeAnnot) edge->AddAnnot(edgeAnnot);
}


//===========================================================================
// DirectedGraph solver upcalls
//===========================================================================


unsigned int CallGraphFlowInsensitiveDFP::AtDirectedGraphNode
(DirectedGraphNode *node, DirectedEdgeDirection incoming)
{
  CallGraphNode *cnode = (CallGraphNode *) node;

  if (cnode->type == CGNT_Entry || cnode->type == CGNT_Exit) return 0;

  //--------------------------------------------------------
  // for each incoming edge:
  //   translate edgeOut to nodeIn 
  //   compute meetResult as meet of all nodeIns
  //--------------------------------------------------------
  DataFlowSet *meetPartialResult = newTopAnnotation();
  CallGraphEdgeIterator edges(cnode, incoming);
  for (CallGraphEdge *edge; edge = edges.Current(); ++edges) {
    if (edge->type == CGET_FromEntry || edge->type == CGET_ToExit) continue;

    DataFlowSet *nodeIn = EdgeToNode(edge, cnode, edgeSets[edge->Id()].VALUE); 
    DataFlowSet *meetOut = Meet(edge, cnode, nodeIn, meetPartialResult);
    if (meetOut != meetPartialResult) {
      delete meetPartialResult;
      meetPartialResult = meetOut;
    }
    delete nodeIn;
  }

  //--------------------------------------------------------
  // update VALUE from meetResult and INIT 
  //--------------------------------------------------------
  unsigned int nodeId = node->Id();
  DataFlowSet *nodeOut = 
    AtNode(cnode, nodeSets[nodeId].INIT, meetPartialResult);
  if (nodeOut != meetPartialResult) delete meetPartialResult;
  return UpdateDataFlowSet(nodeSets[nodeId].VALUE, nodeOut);
}


unsigned int CallGraphFlowInsensitiveDFP::AtDirectedGraphEdge
(DirectedGraphEdge *edge, DirectedEdgeDirection src)
{
  CallGraphEdge *cedge = (CallGraphEdge *) edge;

  if (cedge->type == CGET_FromEntry || cedge->type == CGET_ToExit) return 0;
  
  CallGraphNode *node = (CallGraphNode *) cedge->Endpoint(src);
  DataFlowSet *edgeIn = NodeToEdge(node, cedge, nodeSets[node->Id()].VALUE);
  unsigned int edgeId = cedge->Id();

  DataFlowSet *edgeOut = AtEdge(cedge, edgeSets[edgeId].INIT, edgeIn);
  if (edgeOut != edgeIn) delete edgeIn;
  return UpdateDataFlowSet(edgeSets[edgeId].VALUE, edgeOut); 
}

//****************************************************************************
// private operations
//****************************************************************************

static unsigned int UpdateDataFlowSet(DataFlowSet *&set, DataFlowSet *newSet)
{
  unsigned int unchanged = *set == *newSet;
  delete set;
  set = newSet;
  return !unchanged;
}



