/* $Id: AFormalDFProblem.i,v 1.1 1997/03/11 14:35:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef AFormalDFProblem_i
#define AFormalDFProblem_i

//***************************************************************************
//    AFormalDFProblem.i
//
//    determine if a formal parameter is ever used as an array 
//    by the current procedure or any of the procedures it calls
//
//    Author:
//     John Mellor-Crummey                                   September 1994
//
//    Copyright 1994, Rice University. All rights reserved.
//***************************************************************************

#ifndef CallGraphFlowInsensitiveDFP_h
#include <libs/ipAnalysis/callGraph/CallGraphFlowInsensitiveDFP.h>
#endif


class CallGraphNode; // external declaration
class CallGraphEdge; // external declaration


class AFormalDFProblem : public  CallGraphFlowInsensitiveDFP { 
public: 
  //-------------------------------------------------------------------
  // constructor
  //-------------------------------------------------------------------
  AFormalDFProblem(); 
  
  //-------------------------------------------------------------------
  // annotate a node with information about which formals declared as 
  // arrays
  //-------------------------------------------------------------------
  DataFlowSet *InitializeNode(CallGraphNode *node, ProcLocalInfo *);
  
  //-------------------------------------------------------------------
  // compute the union of annotations along incoming edges at a the 
  // current node
  //-------------------------------------------------------------------
  DataFlowSet *Meet(CallGraphEdge *edge, CallGraphNode *node, 
		    const DataFlowSet *nodeIn, DataFlowSet *meetPartialResult);

  //--------------------------------------------------------
  // combines meetResult with initial information
  //--------------------------------------------------------
  virtual DataFlowSet *AtNode(CallGraphNode *node, const DataFlowSet *nodeInit,
			      DataFlowSet *meetResult); 

  //--------------------------------------------------------
  // maps nodeOut to edgeIn
  //--------------------------------------------------------
  virtual DataFlowSet *NodeToEdge
    (CallGraphNode *node, CallGraphEdge *edge, const DataFlowSet *nodeOut);
private:
  static DataFlowSet *newAFormalTop();
};


#endif /* AFormalDFProblem_i */
