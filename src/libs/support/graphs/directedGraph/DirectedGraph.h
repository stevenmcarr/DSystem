/* $Id: DirectedGraph.h,v 1.1 1997/03/11 14:36:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef DirectedGraph_h
#define DirectedGraph_h

//*********************************************************************
// DirectedGraph.h
//
// the DirectedGraph class supports construction of a directed graph 
// that may contain multiple classes of edges among a single set of 
// nodes. the ability to have different separate edge sets among the 
// same set of nodes could be useful, for instance,  for representing 
// control flow edges as well as control dependence edges among 
// the statements in a program.
// 
// Author: John Mellor-Crummey                             July 1994
//
// Copyright 1994, Rice University
//*********************************************************************


#include <stdarg.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef AnnotatedObject_h
#include <libs/support/annotation/AnnotatedObject.h>
#endif

#ifndef IdGenerator_h
#include <libs/support/misc/IdGenerator.h>
#endif



//***********************************
// forward and external declarations
//***********************************

class PointerMap;
class WordSet;
class FormattedFile;

class DirectedGraphNode;
class DirectedGraphEdge;
class DirectedGraphNodeIterator;
class DirectedGraphEdgeIterator;

class DirectedGraphNodeSet;
class DGNodePointerVector;

typedef void (*DirectedGraphWalkCallBack)(const DirectedGraphNode *node, 
					  va_list args);



//*********************************************************************
// enum DirectedEdgeDirection 
//*********************************************************************
enum DirectedEdgeDirection {DirectedEdgeIn = 0, DirectedEdgeOut = 1};
#define DIRECTED_EDGE_ENDPOINT_TYPE_COUNT 2

#define ALL_EDGE_CLASSES -1




//*********************************************************************
// class DirectedGraph
//
// NOTES:
// (1) edge classes are densely numbered 0..numberOfEdgeClasses-1
//*********************************************************************
class DirectedGraph: public AnnotatedObject {
public:
  DirectedGraph();
  virtual ~DirectedGraph();
  
  void Create(int _numberOfEdgeClasses = 1);
  void Destroy();
  
  //-----------------------------------------------------------------
  // graph traversal is relative to a root. this package allows two
  // separate roots to be specified corresponding to src and 
  // and sink nodes in a flow graph. traversal can be forward along
  // edges from the source, or backward along edges reaching the sink.
  //-----------------------------------------------------------------
  void SetRoot(DirectedGraphNode *node, 
	       DirectedEdgeDirection end = DirectedEdgeOut);
  DirectedGraphNode *GetRoot(DirectedEdgeDirection end = DirectedEdgeOut);
  
  void Walk(const DirectedGraphNode *node, 
	    const DirectedGraphWalkCallBack function,
	    const DirectedEdgeDirection direction, 
	    const int edgeClass, 
	    const TraversalOrder order, ...);
  
  unsigned int NumberOfNodes();	
  unsigned int NumberOfEdges(int edgeClass = ALL_EDGE_CLASSES);	
  unsigned int NumberOfEdgeClasses();	

  unsigned int NodeIdHighWaterMark();	
  unsigned int EdgeIdHighWaterMark();	


  int DirectedGraphRead(FormattedFile *file);
  int DirectedGraphWrite(FormattedFile *file);

  void DirectedGraphDump();

private: 
  //-----------------------------------------------------------------
  // methods
  //-----------------------------------------------------------------
  virtual int DirectedGraphReadUpCall(FormattedFile *file);
  virtual int DirectedGraphWriteUpCall(FormattedFile *file);


  // hooks to allocate an instance of the most derived node/edge types
  virtual DirectedGraphNode *NewNode();
  virtual DirectedGraphEdge *NewEdge();

  virtual void DirectedGraphDumpUpCall();

  //-----------------------------------------------------------------
  // build a vector containing the sequence of nodes reached using the
  // specified traversal order. for Unordered traversals, all nodes are
  // visited. for ordered traversals, the traversal includes all of the
  // nodes reachable from the root of type end along edges of class 
  // edgeClass.
  //-----------------------------------------------------------------
  DGNodePointerVector *BuildWalkVector(const TraversalOrder order,
				       const DirectedEdgeDirection end,
				       const int edgeClass);
  
  void WalkHelper(const DirectedGraphNode *node, 
		  const DirectedGraphWalkCallBack function,
		  const DirectedEdgeDirection direction, 
		  const TraversalOrder order, 
		  const int edgeClass, 
		  DirectedGraphNodeSet *nodeSet,
		  va_list args);	

  int WriteRoot(FormattedFile *file, DirectedEdgeDirection end);
  int ReadRoot(FormattedFile *file, DirectedEdgeDirection end, PointerMap *map);
  
  //-----------------------------------------------------------------
  // data
  //-----------------------------------------------------------------
  DirectedGraphNode *walkRoot[DIRECTED_EDGE_ENDPOINT_TYPE_COUNT];

  unsigned int numberOfNodes;
  DirectedGraphNode *nodeListHead;

  IdGenerator edgeIds;
  IdGenerator nodeIds;

  unsigned int numberOfEdgeClasses;
  unsigned int *numberOfEdges;


friend class DirectedGraphNode;
friend class DirectedGraphEdge;

friend class DirectedGraphNodeIterator;
friend class DirectedGraphEdgeIterator;
};



//*********************************************************************
// class DirectedGraphEdge
//*********************************************************************
class DirectedGraphEdge : public AnnotatedObject {
public:
  
  DirectedGraphEdge(DirectedGraph *dg, DirectedGraphNode *_src, 
		    DirectedGraphNode *_sink, unsigned int _edgeClass = 0);
  virtual ~DirectedGraphEdge();

  DirectedGraph *Graph();

  DirectedGraphNode *Sink();
  DirectedGraphNode *Src();
  DirectedGraphNode *Endpoint(DirectedEdgeDirection end);

  unsigned int Id(); // this id is not preserved across a write/read pair

  unsigned int EdgeClass();

  void DirectedGraphEdgeDump();
  
protected:
  DirectedGraphEdge(DirectedGraph *dg);

private:
  virtual int DirectedGraphEdgeReadUpCall(FormattedFile *file);
  virtual int DirectedGraphEdgeWriteUpCall(FormattedFile *file);

  virtual void DirectedGraphEdgeDumpUpCall();

  // I/O support
  int DirectedGraphEdgeRead(FormattedFile *file, PointerMap *map);
  int DirectedGraphEdgeWrite(FormattedFile *file);
  void Initialize(DirectedGraphNode *_src, DirectedGraphNode *_sink, 
		  unsigned int _edgeClass, unsigned int _edgeId);

  void LinkEdge(DirectedEdgeDirection end, DirectedGraphNode *node);
  void UnLinkEdge(DirectedEdgeDirection end);

  DirectedGraph *graph;

  unsigned int edgeId;
  unsigned int edgeClass;

  DirectedGraphNode *endpoint[DIRECTED_EDGE_ENDPOINT_TYPE_COUNT];
  DirectedGraphEdge *next[DIRECTED_EDGE_ENDPOINT_TYPE_COUNT];
  DirectedGraphEdge *prev[DIRECTED_EDGE_ENDPOINT_TYPE_COUNT];

friend class DirectedGraph;
friend class DirectedGraphEdgeIterator;
};



//*********************************************************************
// class DirectedGraphNode
//*********************************************************************
class DirectedGraphNode: public AnnotatedObject {
public:

  DirectedGraphNode(DirectedGraph *dg);
  virtual ~DirectedGraphNode();

  DirectedGraph *Graph();
  unsigned int Id(); // this id is not preserved across a write/read pair

  void DirectedGraphNodeDump();

private:
  virtual int DirectedGraphNodeReadUpCall(FormattedFile *file);
  virtual int DirectedGraphNodeWriteUpCall(FormattedFile *file);

  virtual void DirectedGraphNodeDumpUpCall();

  // I/O support
  int DirectedGraphNodeRead(FormattedFile *file);
  int DirectedGraphNodeWrite(FormattedFile *file);
  void Initialize(unsigned int _nodeId); 


  void DeleteIncidentEdges(unsigned int incidentEdgeSet);


  DirectedGraph *graph;
  DirectedGraphEdge **edges;

  unsigned int nodeId;

  DirectedGraphNode *next;
  DirectedGraphNode *prev;

friend class DirectedGraphEdge;
friend class DirectedGraph;
friend class DirectedGraphEdgeIterator;
};

#endif

