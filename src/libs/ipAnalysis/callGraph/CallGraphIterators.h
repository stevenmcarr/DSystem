/* $Id: CallGraphIterators.h,v 1.6 1997/03/11 14:34:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef CallGraphIterators_h
#define CallGraphIterators_h

//***************************************************************************
// CallGraphIterators.h:  
//
//     iterators for traversing nodes and edges in a call graph
//
// Author: 
//   John Mellor-Crummey                                           May 1993
//
// Copyright 1993, 1994 Rice University
// 
//***************************************************************************


#ifndef DirectedGraphIterators_h
#include <libs/support/graphs/directedGraph/DirectedGraphIterators.h>
#endif


class CallGraph;
class CallGraphNode;
class CallGraphEdge;


//----------------------------------------------------------------------------
// CallGraphNodeIterator: use the underlying DirectedGraphNodeIterator to 
// provide the functionality; this layer only provides a cast from the
// base (DirectedGraphNode *) to the derived (CallGraphNode *)
//----------------------------------------------------------------------------
class CallGraphNodeIterator : public DirectedGraphNodeIterator {
public:
  CallGraphNodeIterator(CallGraph *cg, TraversalOrder order = Unordered, int edgeClass = 0);
  CallGraphNode *Current();
};


//----------------------------------------------------------------------------
// CallGraphEdgeIterator: use the underlying 
// DirectedGraphEdgeIterator to provide the functionality; this layer 
// only provides a cast from the base (DirectedGraphEdge *) to the derived 
// (CallGraphEdge *)
//----------------------------------------------------------------------------
class CallGraphEdgeIterator : public DirectedGraphEdgeIterator { 
public:
  CallGraphEdgeIterator(CallGraphNode *node, DirectedEdgeDirection et, int edgeClass = 0);
  CallGraphEdge *Current();
};


#endif /* CallGraphIterators_h */
