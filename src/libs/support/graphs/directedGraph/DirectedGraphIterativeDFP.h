/* $Id: DirectedGraphIterativeDFP.h,v 1.1 1997/03/11 14:36:43 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef DirectedGraphIterativeDFP_h
#define DirectedGraphIterativeDFP_h

//*********************************************************************
// DirectedGraphIterativeDFP.h
//
// Author: John Mellor-Crummey                             July 1994
//
// Copyright 1994, Rice University
//*********************************************************************


#ifndef DirectedGraph_h
#include <libs/support/graphs/directedGraph/DirectedGraph.h>
#endif



//*********************************************************************
// class DirectedGraphIterativeDFP
//*********************************************************************
class DirectedGraphIterativeDFP {
public:
  //--------------------------------------------------------
  // constructor/destructor
  //--------------------------------------------------------
  DirectedGraphIterativeDFP();
  virtual ~DirectedGraphIterativeDFP();

  //--------------------------------------------------------
  // Kildall iterative solver 
  //--------------------------------------------------------
  void Solve(DirectedGraph *dg, DirectedEdgeDirection alongFlow,
	     int edgeClass);

private:
  //--------------------------------------------------------
  // initialization callback
  //--------------------------------------------------------
  virtual void Initialize(DirectedGraph *dg, int edgeClass)  = 0;

  //--------------------------------------------------------
  // solver callbacks 
  //--------------------------------------------------------
  //   AtDirectedGraphNode and AtDirectedGraphEdge return non-zero flag if change
  //   in dataflow information at the graph element
  virtual unsigned int AtDirectedGraphNode(DirectedGraphNode *node, 
					   DirectedEdgeDirection incoming);
  virtual unsigned int AtDirectedGraphEdge(DirectedGraphEdge *edge, 
					   DirectedEdgeDirection src);

  //--------------------------------------------------------
  // finalization callbacks
  //--------------------------------------------------------
  virtual void FinalizeEdge(DirectedGraphEdge *edge);
  virtual void FinalizeNode(DirectedGraphNode *node);
};

#endif
