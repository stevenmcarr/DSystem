/* $Id: ScalarModRefDFProblem.i,v 1.4 1997/03/11 14:35:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ScalarModRefDFProblem_h
#define ScalarModRefDFProblem_h

//***************************************************************************
//    ScalarModRefDFProblem.h
//
//    definition of the ScalarModRef interprocedural data flow problem. the 
//    side-effect of performing ScalarModRef analysis is that each node and
//    edge in the callgraph is annotated with a ScalarModRefAnnot named
//    SCALAR_MOD_ANNOT and SCALAR_REF_ANNOT that describe the set of 
//    referenced or modified variables as appropriate. Aliases are considered
//    when constructing these sets.
//
//    Author:
//     John Mellor-Crummey                                   January 1993
//
//    Copyright 1993, Rice University, as part of the ParaScope Programming 
//    Environment Project
//***************************************************************************


#include <libs/ipAnalysis/ipInfo/iptypes.h>

#include <libs/ipAnalysis/callGraph/CallGraphDFProblem.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>


class CallGraph;      // external declaration
class CallGraphNode;  // external declaration
class CallGraphEdge;  // external declaration


class ScalarModRefDFProblem : public CallGraphDFProblem { 
  CallGraph *cg; 
  ModRefType which;
  char *annot_name;
public: 
  //-------------------------------------------------------------------
  // constructor
  //-------------------------------------------------------------------
  ScalarModRefDFProblem(CallGraph *cg, ModRefType whichType, char *aname); 

private:
  //-------------------------------------------------------------------
  // annotate an edge with a set that is initially empty 
  //-------------------------------------------------------------------
  void InitializeEdge(CallGraphEdge *edge);

  //-------------------------------------------------------------------
  // annotate an edge with a set that is derived from the local 
  // information for the procedure
  //-------------------------------------------------------------------
  void InitializeNode(CallGraphNode *node, ProcLocalInfo *mrinfo);

  //-------------------------------------------------------------------

  //-------------------------------------------------------------------
  // augment incoming edges with a ScalarModRefAnnot set that uses the
  // GMOD/GREF information at a node to construct a summary 
  // representation for accesses to actuals passed at the call
  //-------------------------------------------------------------------
  void *DFtrans(void *in_v, void *prev_out_v, void *node_v, 
				unsigned char &changed);

  //-------------------------------------------------------------------
  // compute the union of annotations from outgoing edges at the 
  // current node
  //-------------------------------------------------------------------
  void *DFmeet(void *partial_result_annot_v, void *, void *self_node_v, 
	       void *edge_v);

  //-------------------------------------------------------------------
  // factor alias information into the GMOD/REF annotation on each node 
  // and propagate that to the MOD/REF annotation on each incoming edge
  //-------------------------------------------------------------------
  virtual void FinalizeAnnotation(CallGraphNode *node, 
	FlowGraphDFAnnot *annot);
};

#if 0
// the definition of Construct... probably should be somewhere else -- JMC
class IPinfoTreeNode; // external declaration
class ScalarModRefAnnot; // external declaration
ScalarModRefAnnot *ConstructScalarModRefAnnotForLoop(CallGraphNode *node, 
						     IPinfoTreeNode *loop,
						     char *annot_name, 
						     ModRefType which);
#endif


#endif /* ScalarModRefDFProblem_h */
