/* $Id: CallGraphFlowInsensitiveDFP.h,v 1.3 1997/03/11 14:34:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef CallGraphFlowInsensitiveDFP_h
#define CallGraphFlowInsensitiveDFP_h

//****************************************************************************
//
// CallGraphFlowInsensitiveDFP.h
//
// template for a flow-insensitive interprocedural data flow analysis problem. 
//
// Author: John Mellor-Crummey                                 July 1994
//
// Copyright 1994, Rice University
//****************************************************************************


#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef DirectedGraphIterativeDFP_h
#include <libs/support/graphs/directedGraph/DirectedGraphIterativeDFP.h>
#endif


class CallGraphNode;
class CallGraphEdge;
class ProcLocalInfo;
class CallSiteLocalInfo;
class DataFlowSet;
struct CallGraphFlowInsensDataFlowSets;


typedef FUNCTION_POINTER(DataFlowSet *, AnnotConstructor, (void));


//****************************************************************************
// class CallGraphFlowInsensitiveDFP 
//****************************************************************************
class CallGraphFlowInsensitiveDFP: private DirectedGraphIterativeDFP {
public:
  CallGraphFlowInsensitiveDFP(const char *_localInfoClassName, 
			      AnnotConstructor newTopAnnot,
			      DirectedEdgeDirection direction);

  ~CallGraphFlowInsensitiveDFP();
  
  void Solve(CallGraph *cg);
  
private:
  //========================================================
  // interface upcalls 
  //========================================================
  
  //--------------------------------------------------------
  // initialization upcall interface
  //--------------------------------------------------------
  virtual DataFlowSet *InitializeNode(CallGraphNode *node, ProcLocalInfo *info);
  virtual DataFlowSet *InitializeEdge(CallGraphEdge *edge, CallSiteLocalInfo *info);
  
  //--------------------------------------------------------
  // maps edgeOut to nodeIn
  //--------------------------------------------------------
  virtual DataFlowSet *EdgeToNode
    (CallGraphEdge *edge, CallGraphNode *node, const DataFlowSet *edgeOut);
  
  //--------------------------------------------------------
  // returns meetPartialResult MEET nodeIn
  // implementation contract: 
  //   may return meetPartialResult (possibly modified), but 
  //   may not delete meetPartialResult
  //--------------------------------------------------------
  virtual DataFlowSet *Meet
    (CallGraphEdge *edge, CallGraphNode *node, const DataFlowSet *nodeIn, 
     DataFlowSet *meetPartialResult) = 0;
  
  //--------------------------------------------------------
  // maps meetResult to nodeOut
  // implementation contract: 
  //   may return meetResult (possibly modified), but may 
  //   not delete meetResult
  //--------------------------------------------------------
  virtual DataFlowSet *AtNode(CallGraphNode *node, const DataFlowSet *nodeInit,
			      DataFlowSet *meetResult); 

  //--------------------------------------------------------
  // maps nodeOut to edgeIn
  //--------------------------------------------------------
  virtual DataFlowSet *NodeToEdge
    (CallGraphNode *node, CallGraphEdge *edge, const DataFlowSet *nodeOut);
  
  //--------------------------------------------------------
  // maps edgeIn to edgeOut
  // implementation contract: 
  //   may return edgeIn (possibly modified), but may not 
  //   delete edgeIn
  //--------------------------------------------------------
  virtual DataFlowSet *AtEdge(CallGraphEdge *edge, const DataFlowSet *edgeInit,
			      DataFlowSet *edgeIn);
  
  //--------------------------------------------------------
  // finalization upcall interface
  //   the DataFlowSet returned as a result of finalization
  //   will be added as an annotation to the graph component
  //   default versions of these routines return 0, causing
  //   all DataFlowSets to be discarded 
  // implementation contract: 
  //   may return annot (possibly modified), but may not 
  //   delete annot
  //--------------------------------------------------------
  virtual DataFlowSet *FinalizeNodeAnnotation
    (CallGraphNode *node, DataFlowSet *annot);

  virtual DataFlowSet *FinalizeEdgeAnnotation
    (CallGraphEdge *edge, DataFlowSet *annot);
  
private:
  //========================================================
  // base class virtual functions
  //========================================================
  
  //--------------------------------------------------------
  // initialization upcall 
  //--------------------------------------------------------
  void Initialize(DirectedGraph *dg, int edgeClass);
  
  //--------------------------------------------------------
  // solver upcalls
  //--------------------------------------------------------
  unsigned int AtDirectedGraphNode(DirectedGraphNode *node, 
				   DirectedEdgeDirection incoming);
  unsigned int AtDirectedGraphEdge(DirectedGraphEdge *edge, 
				   DirectedEdgeDirection src);
  
  //--------------------------------------------------------
  // finalization upcalls
  //--------------------------------------------------------
  void FinalizeNode(DirectedGraphNode *node);
  void FinalizeEdge(DirectedGraphEdge *edge);

private:
  const char *localInfoClassName; 
  AnnotConstructor newTopAnnotation;
  DirectedEdgeDirection flowDirection;
  CallGraphFlowInsensDataFlowSets *nodeSets;
  CallGraphFlowInsensDataFlowSets *edgeSets;
}; 

#endif /* CallGraphFlowInsensitiveDFP_h */
