/* $Id: DirectedGraphIterators.h,v 1.1 1997/03/11 14:36:43 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef DirectedGraphIterators_h
#define DirectedGraphIterators_h

//*********************************************************************
// DirectedGraphIterators.h
//
//   iterators for traversing nodes in a directed graph, or 
//   incoming or outgoing edges of a node in a directed graph
//
// Author: John Mellor-Crummey                             July 1994
//
// Copyright 1994, Rice University
//*********************************************************************


#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef DirectedGraph_h
#include <libs/support/graphs/directedGraph/DirectedGraph.h>
#endif



//**************************
// external declarations
//**************************

class DGNodePointerVector;


//*********************************************************************
// class DirectedGraphNodeIterator 
//*********************************************************************
class DirectedGraphNodeIterator {
  DirectedGraph *dgraph;
  DGNodePointerVector *nodeVector;
  unsigned int currentIndex;
public:
  DirectedGraphNodeIterator
    (DirectedGraph *dg, const TraversalOrder torder = Unordered, 
     const DirectedEdgeDirection direction = DirectedEdgeOut,
     const int edgeClass = ALL_EDGE_CLASSES);

  ~DirectedGraphNodeIterator();

  DirectedGraphNode *Current();
  void operator++();

  void Reset(const TraversalOrder torder = Unordered, 
	     const DirectedEdgeDirection direction = DirectedEdgeOut,
	     const int edgeClass = ALL_EDGE_CLASSES);
};



//*********************************************************************
// class DirectedGraphEdgeIterator 
//*********************************************************************
class DirectedGraphEdgeIterator {
public:
  DirectedGraphEdgeIterator(const DirectedGraphNode *node, 
			    DirectedEdgeDirection et, 
			    int edgeClassId = ALL_EDGE_CLASSES);
  ~DirectedGraphEdgeIterator();

  void operator++();
  DirectedGraphEdge *Current();
  void Reset(const DirectedGraphNode *node, DirectedEdgeDirection et, 
	     int edgeClassId = ALL_EDGE_CLASSES);
  void Reset();

private:
  void SelectEdgeClass();

  DirectedEdgeDirection etype;
  DirectedGraphNode *node;
  DirectedGraphEdge *curEdge;

  int edgeClassId;
  unsigned int curEdgeClassId;
};


#endif
