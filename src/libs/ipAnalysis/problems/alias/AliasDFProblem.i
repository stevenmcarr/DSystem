/* $Id: AliasDFProblem.i,v 1.1 1997/03/11 14:34:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef AliasDFProblem_i
#define AliasDFProblem_i

//***************************************************************************
//    AliasDFProblem.i
//
//    definition of the alias interprocedural data flow problem. the 
//    side-effect of performing alias analysis is that each node and
//    edge in the callgraph is annotated with an AliasAnnot that
//    contains pairs of aliases.
// 
//
//   Author: 
//     John Mellor-Crummey                                   August 1994
//  
//    Copyright 1994, Rice University
//***************************************************************************

#ifndef CallGraphFlowInsensitiveDFP_h
#include <libs/ipAnalysis/callGraph/CallGraphFlowInsensitiveDFP.h>
#endif


class CallGraphNode; // external declaration
class CallGraphEdge; // external declaration


class AliasDFProblem : public CallGraphFlowInsensitiveDFP { 
public: 
  //-------------------------------------------------------------------
  // constructor
  //-------------------------------------------------------------------
  AliasDFProblem(); 

private:
  //-------------------------------------------------------------------
  // annotate an edge with pairs of aliases causes directly by aliases
  // among actuals passed at a callsite
  //-------------------------------------------------------------------
  DataFlowSet *InitializeEdge(CallGraphEdge *edge, CallSiteLocalInfo *info);

  //-------------------------------------------------------------------
  // compute the union of annotations along incoming edges at a the 
  // current node
  //-------------------------------------------------------------------
  DataFlowSet *Meet(CallGraphEdge *edge, CallGraphNode *node, 
		    const DataFlowSet *nodeIn, DataFlowSet *meetPartialResult);	
  //-------------------------------------------------------------------
  // compute the aliases pairs passing from a node to an edge 
  //-------------------------------------------------------------------
  DataFlowSet *AliasDFProblem::NodeToEdge
    (CallGraphNode *node, CallGraphEdge *edge, const DataFlowSet *nodeOut);

  //-------------------------------------------------------------------
  // augment aliases incoming on an edge with aliases introduced by this
  // edge
  //-------------------------------------------------------------------
  DataFlowSet *AtEdge(CallGraphEdge *edge, const DataFlowSet *edgeInit, 
	DataFlowSet *edgeIn);

  DataFlowSet *FinalizeNodeAnnotation(CallGraphNode *node, DataFlowSet *annot);

  DataFlowSet *FinalizeEdgeAnnotation(CallGraphEdge *edge, DataFlowSet *annot);
private:
  static DataFlowSet *newAliasTop();
};


#endif /* AliasDFProblem_h */
