/* $Id: DirectedGraphIterators.C,v 1.1 1997/03/11 14:36:43 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*********************************************************************
//  DirectedGraphIterators.C:
//
//  Author:
//    John Mellor-Crummey                                    July 1994
//
//  Copyright 1994, Rice University
//*********************************************************************


#include <assert.h>

#include <libs/support/graphs/directedGraph/DirectedGraphIterators.h>
#include <libs/support/graphs/directedGraph/DGNodePointerVector.h>
#include <libs/support/graphs/directedGraph/DirectedGraph.h>



//*********************************************************************
// class DirectedGraphNodeIterator interface operations
//*********************************************************************

DirectedGraphNodeIterator::DirectedGraphNodeIterator
(DirectedGraph *dg, const TraversalOrder order, 
 const DirectedEdgeDirection direction, const int edgeClass) : 
dgraph(dg), nodeVector(0)
{
  Reset(order, direction, edgeClass);
}


DirectedGraphNodeIterator::~DirectedGraphNodeIterator()
{
  delete nodeVector;
}


DirectedGraphNode *DirectedGraphNodeIterator::Current()
{
  return ((currentIndex < nodeVector->NumberOfEntries()) ? 
	  nodeVector->Entry(currentIndex) : 0);
}


void DirectedGraphNodeIterator::Reset
(const TraversalOrder order, const DirectedEdgeDirection direction, 
 const int edgeClass)
{
  delete nodeVector;
  nodeVector = dgraph->BuildWalkVector(order, direction, edgeClass);
  currentIndex = 0;
}


void DirectedGraphNodeIterator:: operator ++()
{ 
  ++currentIndex;
}



//*********************************************************************
// class DirectedGraphEdgeIterator interface operations
//*********************************************************************

DirectedGraphEdgeIterator::DirectedGraphEdgeIterator
(const DirectedGraphNode *_node, DirectedEdgeDirection et, int edgeClassId)  
{
  Reset(_node, et, edgeClassId);
}


DirectedGraphEdgeIterator::~DirectedGraphEdgeIterator()
{
}


void DirectedGraphEdgeIterator::Reset
(const DirectedGraphNode *_node, DirectedEdgeDirection et, int _edgeClassId)
{
  etype = et;
  edgeClassId = _edgeClassId;
  node = (DirectedGraphNode *) _node;
  Reset();
}

void DirectedGraphEdgeIterator::Reset()
{
  curEdgeClassId = (edgeClassId == ALL_EDGE_CLASSES) ? 0 : edgeClassId;
  SelectEdgeClass();
}

void DirectedGraphEdgeIterator::SelectEdgeClass()
{
  curEdge = node->edges[curEdgeClassId * DIRECTED_EDGE_ENDPOINT_TYPE_COUNT + etype];
}


DirectedGraphEdge *DirectedGraphEdgeIterator::Current()
{
  return curEdge;
}


void DirectedGraphEdgeIterator::operator++()
{
  if (curEdge) {
    if ((curEdge = curEdge->next[etype]) == 0) {
      if (edgeClassId == ALL_EDGE_CLASSES) {
	while (curEdge == 0 && 
	       ++curEdgeClassId < node->graph->numberOfEdgeClasses) {
	  SelectEdgeClass();
	}
      }
    }
  }
}

