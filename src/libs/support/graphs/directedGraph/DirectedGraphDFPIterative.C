/* $Id: DirectedGraphDFPIterative.C,v 1.1 1997/03/11 14:36:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*********************************************************************
// DirectedGraphIterativeDFP.C
//
// Author: John Mellor-Crummey                             July 1994
//
// Copyright 1994, Rice University
//*********************************************************************


#include <libs/support/graphs/directedGraph/DirectedGraph.h>
#include <libs/support/graphs/directedGraph/DirectedGraphIterators.h>
#include <libs/support/graphs/directedGraph/DirectedGraphIterativeDFP.h>


//*********************************************************************
// class DirectedGraphIterativeDFP
//*********************************************************************

DirectedGraphIterativeDFP::DirectedGraphIterativeDFP()
{
}


DirectedGraphIterativeDFP::~DirectedGraphIterativeDFP()
{
}


//-----------------------------------------------------------------------
// general default solver 
//-----------------------------------------------------------------------
void DirectedGraphIterativeDFP::Solve(DirectedGraph *dg, 
				   DirectedEdgeDirection alongFlow, 
				   int edgeClass)
{
  //---------------------------------------------------------------
  // initialize dataflow information at each of the nodes and edges
  //---------------------------------------------------------------
  Initialize(dg, edgeClass);

  //---------------------------------------------------------------
  // Kildall style iterative solver: iterate until dataflow 
  // information at each node and edge stabilizes
  //---------------------------------------------------------------
  unsigned int changed;
  DirectedGraphNode *node;
  DirectedGraphEdge *edge;
  DirectedEdgeDirection againstFlow = 
    (alongFlow == DirectedEdgeIn) ? DirectedEdgeOut : DirectedEdgeIn;
  DirectedGraphNodeIterator nodes(dg, ReversePostOrder, alongFlow, edgeClass);
  do {
    changed = 0;
    for (; node = nodes.Current(); nodes++) {
      //--------------------------------------------------
      // compute dataflow information at node
      //--------------------------------------------------
      changed |= AtDirectedGraphNode(node, againstFlow);

      //--------------------------------------------------
      // compute dataflow information at outgoing edges 
      //--------------------------------------------------
      DirectedGraphEdgeIterator edges(node, alongFlow, edgeClass);
      for (; edge = edges.Current(); edges++) {
	changed |= AtDirectedGraphEdge(edge, alongFlow);
      }
    }
    nodes.Reset(ReversePostOrder, alongFlow, edgeClass);
  } while (changed);


  //---------------------------------------------------------------
  // finalize dataflow information at each of the nodes and edges
  //---------------------------------------------------------------
  for (; node = nodes.Current(); nodes++) {
    FinalizeNode(node);
    DirectedGraphEdgeIterator edges(node, alongFlow, edgeClass);
    for (; edge = edges.Current(); edges++) FinalizeEdge(edge);
  }
}


//-----------------------------------------------------------------------
// solver callbacks
//-----------------------------------------------------------------------
unsigned int DirectedGraphIterativeDFP::AtDirectedGraphNode
(DirectedGraphNode *, DirectedEdgeDirection)
{
  return 0;
}


unsigned int DirectedGraphIterativeDFP::AtDirectedGraphEdge
(DirectedGraphEdge *, DirectedEdgeDirection)
{
  return 0;
}


//-----------------------------------------------------------------------
// finalization callbacks
//-----------------------------------------------------------------------
void DirectedGraphIterativeDFP::FinalizeEdge(DirectedGraphEdge *)
{
}


void DirectedGraphIterativeDFP::FinalizeNode(DirectedGraphNode *)
{
}

