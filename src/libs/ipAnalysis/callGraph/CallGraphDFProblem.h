/* $Id: CallGraphDFProblem.h,v 1.3 1997/03/11 14:34:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef CallGraphDFProblem_h
#define CallGraphDFProblem_h

//****************************************************************************
//
// Class definition for CallGraphDFProblem, an CallGraphedural data flow 
// analysis problem. This class is not useful itself, but rather it is 
// designed to be inherited from. See also the definitions for the classes 
// CallGraph, CallGraphNode, CallGraphAnnot, etc.
//
// Author: 
//    John Mellor-Crummey                                 December 1992
//
//    Copyright 1992, Rice University, as part of the ParaScope Programming 
//    Environment Project
//****************************************************************************

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef DirectedGraphIterativeDFP_h
#include <libs/support/graphs/directedGraph/DirectedGraphIterativeDFP.h>
#endif

#ifndef CallGraphAnnot_h
#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>
#endif

class CallGraphNode;
class CallGraphEdge;
class ProcLocalInfo;
class CallGraphDFProblemState;

typedef enum { Forward, Backward } dfdirection;

class CallGraphDFProblem : private DirectedGraphIterativeDFP {
public:
  DataFlowSet *top;
  DataFlowSet *bottom;
  dfdirection direction;

  CallGraphDFProblem(const char *localInfoClassName);
  ~CallGraphDFProblem();

  void Solve(CallGraph *cg);

private:
  virtual void InitializeNode(CallGraphNode *node, ProcLocalInfo *info);
  virtual void InitializeEdge(CallGraphEdge *edge);
  virtual void FinalizeAnnotation(CallGraphNode *node, 
				  CallGraphDFAnnot *annot);

  virtual void *DFmeet(void *, void *, void *, void *) = 0;
  virtual void *DFtrans(void *, void *, void *, unsigned char &) = 0;
  
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
  
  //--------------------------------------------------------
  // finalization upcalls
  //--------------------------------------------------------
  void FinalizeNode(DirectedGraphNode *node);

private:
  const char *localInfoClassName; 
  CallGraphDFProblemState *nodeSets;
}; 

#endif /* CallGraphDFProblem_h */
