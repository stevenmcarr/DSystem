/* $Id: CallGraphIterators.C,v 1.7 1997/03/11 14:34:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// CallGraphIterators.C:  
//
//     iterators for traversing nodes and edges in a call graph
//
// Author: 
//   John Mellor-Crummey                                           May 1993
//
// Copyright 1993, Rice University
// 
//***************************************************************************

#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphIterators.h>

//----------------------------------------------------------------------------
// CallGraphNodeIterator
//----------------------------------------------------------------------------
CallGraphNodeIterator::CallGraphNodeIterator
(CallGraph *cg, TraversalOrder to, int edgeClass) : 
DirectedGraphNodeIterator(cg, to, DirectedEdgeOut, edgeClass)
{
}

CallGraphNode *CallGraphNodeIterator::Current()
{
  return (CallGraphNode *) DirectedGraphNodeIterator::Current();
}


//----------------------------------------------------------------------------
// CallGraphNodeEdgeIterator 
//----------------------------------------------------------------------------
CallGraphEdgeIterator::CallGraphEdgeIterator
(CallGraphNode *node, DirectedEdgeDirection et, int edgeClass) : 
DirectedGraphEdgeIterator(node, et, edgeClass)
{
}

CallGraphEdge *CallGraphEdgeIterator::Current()
{
  return (CallGraphEdge *) DirectedGraphEdgeIterator::Current();
}
