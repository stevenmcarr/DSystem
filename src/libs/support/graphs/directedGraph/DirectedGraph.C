/* $Id: DirectedGraph.C,v 1.3 1997/06/25 15:16:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*******************************************************************
// DirectedGraph.C
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
//*******************************************************************

// #define DEBUG 1


#include <assert.h>
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>

#include <include/bstring.h>
#include <libs/support/file/FormattedFile.h>
#include <libs/support/msgHandlers/DumpMsgHandler.h>
#include <libs/support/tables/PointerMap.h>
#include <libs/support/sets/WordSet.h>

#include <libs/support/graphs/directedGraph/DirectedGraph.h>
#include <libs/support/graphs/directedGraph/DirectedGraphIterators.h>
#include <libs/support/graphs/directedGraph/DirectedGraphNodeSet.h>
#include <libs/support/graphs/directedGraph/DGNodePointerVector.h>



//*******************************************************************
// declarations 
//*******************************************************************

//***********************
// forward declarations 
//***********************

static void BuildNodeVector(const DirectedGraphNode *node, va_list args);



//*******************************************************************
// class DirectedGraph interface operations
//*******************************************************************

DirectedGraph::DirectedGraph() : nodeListHead(0), numberOfEdges(0), 
numberOfNodes(0), numberOfEdgeClasses(0) 
{
  walkRoot[DirectedEdgeIn] = 0;
  walkRoot[DirectedEdgeOut] = 0;
}


DirectedGraph::~DirectedGraph()
{
  Destroy();
}


void DirectedGraph::Create(int _numberOfEdgeClasses)
{
  Destroy();
  numberOfEdgeClasses = _numberOfEdgeClasses;
  numberOfEdges = new unsigned int[numberOfEdgeClasses];

  edgeIds.Create();
  nodeIds.Create();

  // initialize typed edge counts to zero
  bzero(numberOfEdges, sizeof(unsigned int) * numberOfEdgeClasses);
}


void DirectedGraph::Destroy()
{
  if (numberOfEdges) { // active instance
    for (DirectedGraphNode *node; node = nodeListHead;) delete node;
    walkRoot[DirectedEdgeIn] = 0;
    walkRoot[DirectedEdgeOut] = 0;
    
    delete numberOfEdges;
    numberOfEdges = 0;
    
    edgeIds.Destroy();
    nodeIds.Destroy();
  }
}


void DirectedGraph::SetRoot(DirectedGraphNode *node, DirectedEdgeDirection end)
{
  assert(node == 0 || node->graph == this);
  walkRoot[end] = node;
}


DirectedGraphNode *DirectedGraph::GetRoot(DirectedEdgeDirection end)
{
  return walkRoot[end];
}


unsigned int DirectedGraph::NumberOfNodes()
{
  return numberOfNodes;
}


unsigned int DirectedGraph::NumberOfEdges(int edgeClass)
{
  if ((edgeClass >= 0) && (edgeClass < numberOfEdgeClasses)) {
    return numberOfEdges[edgeClass];
  } else {
    if (edgeClass == ALL_EDGE_CLASSES)  {
      unsigned int sum = 0;
      for (int i=0; i < numberOfEdgeClasses; i++) 
	sum += numberOfEdges[i];
      return sum;
    }
  }
  assert(0);
  return 0;
}

unsigned int DirectedGraph::NumberOfEdgeClasses()
{
  return numberOfEdgeClasses;
}


unsigned int DirectedGraph::NodeIdHighWaterMark()
{
  return nodeIds.GetHighWaterMark();
}


unsigned int DirectedGraph::EdgeIdHighWaterMark()
{
  return edgeIds.GetHighWaterMark();
}


void DirectedGraph::Walk(const DirectedGraphNode *node, 
			 const DirectedGraphWalkCallBack function,
			 const DirectedEdgeDirection direction, 
			 const int edgeClass, 
			 const TraversalOrder order, ...)
{
  assert(((edgeClass >= 0) && (edgeClass < numberOfEdgeClasses)) ||
	 (edgeClass == ALL_EDGE_CLASSES));

  va_list args;
  va_start(args, order);

  assert(node != NULL);

  if (order == ReversePostOrder) {
    DGNodePointerVector *rpo = 
      BuildWalkVector(ReversePostOrder, direction, edgeClass); 
    for(unsigned  i = 0; i < rpo->NumberOfEntries(); i++) 
      function(rpo->Entry(i), args);
    delete rpo;
  } else {
    DirectedGraphNodeSet nodeSet; 
    WalkHelper(node, function, direction, order, edgeClass, &nodeSet, args);
  }
}


int DirectedGraph::DirectedGraphWrite(FormattedFile *file)
{
  int code;

  //-------------------------------------------------------
  // write numberOfEdgeClasses
  //-------------------------------------------------------
  code = file->Write(numberOfEdgeClasses);
  if (code) return code;

  //-------------------------------------------------------
  // save state of id generators
  //-------------------------------------------------------
  code = nodeIds.Write(file) || edgeIds.Write(file);
  if (code) return code;

  //-------------------------------------------------------
  // write node count
  //-------------------------------------------------------
  code = file->Write(numberOfNodes);
  if (code) return code;

  //-------------------------------------------------------
  // write nodes and their pointers which will be used
  // to build a translation table during read so that edges
  // can be reattached properly
  //-------------------------------------------------------
  DirectedGraphNodeIterator nodes(this);
  DirectedGraphNode *node;
  for(; node = nodes.Current(); ++nodes) {
    code = file->Write(node) || node->DirectedGraphNodeWrite(file);
    if (code) return code;
  }

  //-------------------------------------------------------
  // save root nodes 
  //-------------------------------------------------------
  WriteRoot(file, DirectedEdgeOut);
  WriteRoot(file, DirectedEdgeIn);
  
  //-------------------------------------------------------
  // write total number of edges
  //-------------------------------------------------------
  code = file->Write(NumberOfEdges(ALL_EDGE_CLASSES));
  if (code) return code;

  //-------------------------------------------------------
  // write edges
  //-------------------------------------------------------
  nodes.Reset();
  for (; node = nodes.Current(); ++nodes) {
    DirectedGraphEdgeIterator edges(node, DirectedEdgeIn, ALL_EDGE_CLASSES);
    DirectedGraphEdge *edge;
    for (; edge = edges.Current(); ++edges) {
      //-------------------------------------------------------
      // write incoming graph edges
      //-------------------------------------------------------
      code = edge->DirectedGraphEdgeWrite(file);
      if (code) return code;
    }
  }

  return AnnotatedObject::WriteAnnotations(file) || 
    DirectedGraphWriteUpCall(file); 
}


int DirectedGraph::DirectedGraphRead(FormattedFile *file)
{
  PointerMap nodeMap;
  unsigned int count;
  int code;

  //-------------------------------------------------------
  // initialize with numberOfEdgeClasses
  //-------------------------------------------------------
  code = file->Read(count);
  if (code) return code;
  Create(count);

  //-------------------------------------------------------
  // restore state of id generators
  //-------------------------------------------------------
  code = nodeIds.Read(file) || edgeIds.Read(file);
  if (code) return code;

  //-------------------------------------------------------
  // read node count
  //-------------------------------------------------------
  code = file->Read(count);
  if (code) return code;

  //-------------------------------------------------------
  // read nodes, enter pointers in translation table
  // so edges can be properly be reattached
  //-------------------------------------------------------
  for(; count-- > 0;) {
    void *oldNodePtr; 
    DirectedGraphNode *node = NewNode();
    code = file->Read(oldNodePtr) || node->DirectedGraphNodeRead(file);
    
    if (code) return code;
    nodeMap.InsertMapping(oldNodePtr, node);
  }

  //-------------------------------------------------------
  // recover root nodes 
  //-------------------------------------------------------
  ReadRoot(file, DirectedEdgeOut, &nodeMap);
  ReadRoot(file, DirectedEdgeIn, &nodeMap);
  
  
  //-------------------------------------------------------
  // read edge count
  //-------------------------------------------------------
  code = file->Read(count);
  if (code) return code;

  //-------------------------------------------------------
  // read graph edges, mapping src and sink using 
  // node pointer translation table 
  //-------------------------------------------------------
  for(; count-- > 0;) {
    DirectedGraphEdge *edge = NewEdge();
    code = edge->DirectedGraphEdgeRead(file, &nodeMap);
    if (code) return code;
  }

  return AnnotatedObject::ReadAnnotations(file) || 
    DirectedGraphReadUpCall(file); 
}


void DirectedGraph::DirectedGraphDump()
{
  dumpHandler.Dump("DirectedGraph %x\n", this);
  dumpHandler.BeginScope();
  DirectedGraphDumpUpCall();
  DumpAnnotations();

  DirectedGraphNodeIterator nodes(this);
  DirectedGraphNode *node;

  //-------------------------------------------------------
  // dump nodes
  //-------------------------------------------------------
  for (; node = nodes.Current(); ++nodes) {
    node->DirectedGraphNodeDump();
  }

  //-------------------------------------------------------
  // dump edges
  //-------------------------------------------------------
  nodes.Reset();
  for (; node = nodes.Current(); ++nodes) {
    DirectedGraphEdgeIterator edges(node, DirectedEdgeIn, ALL_EDGE_CLASSES);
    DirectedGraphEdge *edge;
    for (; edge = edges.Current(); ++edges) {
      //-------------------------------------------------------
      // dump incoming graph edges
      //-------------------------------------------------------
      edge->DirectedGraphEdgeDump();
    }
  }
  dumpHandler.EndScope();
}

//********************************************************
// class DirectedGraph private operations
//********************************************************

void DirectedGraph::DirectedGraphDumpUpCall()
{
}

DirectedGraphNode *DirectedGraph::NewNode()
{
  return new DirectedGraphNode(this);
}


DirectedGraphEdge *DirectedGraph::NewEdge()
{
  return new DirectedGraphEdge(this);
}

void DirectedGraph::WalkHelper(const DirectedGraphNode *node, 
			       const DirectedGraphWalkCallBack function,
			       const DirectedEdgeDirection direction, 
			       const TraversalOrder order, 
			       const int edgeClass, 
			       DirectedGraphNodeSet *nodeSet, 
			       va_list args)
{
  assert(order == PreOrder || order == PostOrder || 
	 order == PreAndPostOrder);
  
  if (nodeSet->IsMember(node)) return; // ignore if already visited
  nodeSet->Add(node);
  
  if (order == PreOrder || order == PreAndPostOrder) function(node, args);
  
  DirectedEdgeDirection opposite = 
    (direction == DirectedEdgeIn) ? DirectedEdgeOut : DirectedEdgeIn;
  DirectedGraphEdgeIterator edges(node, direction, edgeClass);
  for (DirectedGraphEdge *edge; edge = edges.Current(); ++edges) {
    WalkHelper(edge->Endpoint(opposite), function, direction, order, 
	       edgeClass, nodeSet, args);
  }
  
  if (order == PostOrder || order == PreAndPostOrder) function(node, args);
}


DGNodePointerVector *DirectedGraph::BuildWalkVector
(const TraversalOrder order, const DirectedEdgeDirection direction, 
 const int edgeClass)
{
  unsigned int vectorSize = numberOfNodes;
  if (order == PreAndPostOrder) vectorSize << 1; // two visits per node

  DGNodePointerVector *nodeVector = 
    new DGNodePointerVector(vectorSize); // traversal order storage

  TraversalOrder torder = (order == ReversePostOrder) ? PostOrder : order;

  if (order == Unordered) {
    for(DirectedGraphNode *node = nodeListHead; node; node = node->next) {
      nodeVector->Append(node);
    }
  } else {
    Walk(walkRoot[direction], BuildNodeVector, direction, edgeClass, torder, 
	 nodeVector); 
    if (order == ReversePostOrder) nodeVector->Reverse();
  }

  return nodeVector;
}


int DirectedGraph::ReadRoot(FormattedFile *file, DirectedEdgeDirection end,
			    PointerMap *map)
{  
  void *oldRoot;
  int code = file->Read(oldRoot);
  if (code) return code;
  DirectedGraphNode *newRoot = 
    (oldRoot ? (DirectedGraphNode *) map->Map((DirectedGraphNode *) oldRoot) : 0);
  SetRoot(newRoot, end);
  return 0;
}


int DirectedGraph::WriteRoot(FormattedFile *file, DirectedEdgeDirection end)
{  
  return file->Write(GetRoot(end));
}


int DirectedGraph::DirectedGraphReadUpCall(FormattedFile *)
{
  return 0;
}


int DirectedGraph::DirectedGraphWriteUpCall(FormattedFile *)
{
  return 0;
}


//*******************************************************************
// class DirectedGraphNode interface operations
//*******************************************************************
 
DirectedGraphNode::DirectedGraphNode(DirectedGraph *dg)
{
  graph = dg;

  // allocate space for the incident edge lists for each edge class
  assert(graph->numberOfEdgeClasses > 0);
  edges = new DirectedGraphEdge *[graph->numberOfEdgeClasses * 
		     DIRECTED_EDGE_ENDPOINT_TYPE_COUNT];
  bzero(edges, 
	(graph->numberOfEdgeClasses * DIRECTED_EDGE_ENDPOINT_TYPE_COUNT * 
	 sizeof(DirectedGraphEdge *)));

  // link at head of nodelist
  prev = 0;
  next = graph->nodeListHead;
  if (next) next->prev = this;
  graph->nodeListHead = this;

  ++graph->numberOfNodes; // correct node count

  Initialize(dg->nodeIds.AcquireId());
}


#if 0
DirectedGraphNode::DirectedGraphNode() 
{
}
#endif

void DirectedGraphNode::Initialize(unsigned int _nodeId) 
{
  nodeId = _nodeId;
}


DirectedGraphNode::~DirectedGraphNode()
{
#if DEBUG
  cout << "deleting node " << (void *) this << "\n";
  cout.flush();
#endif

  int incidentEdgeSets = 
    graph->numberOfEdgeClasses * DIRECTED_EDGE_ENDPOINT_TYPE_COUNT;
  for (int i = 0; i < incidentEdgeSets; i++) DeleteIncidentEdges(i);
  
  // unlink from nodelist
  if (prev) prev->next = next;
  else graph->nodeListHead = next;
  if (next) next->prev = prev;
  next = 0;
  prev = 0;

  delete edges;

  graph->nodeIds.ReleaseId(nodeId);
  graph->numberOfNodes--; // correct node count
}


unsigned int DirectedGraphNode::Id()
{
  return nodeId;
}

DirectedGraph *DirectedGraphNode::Graph()
{
  return graph;
}


int DirectedGraphNode::DirectedGraphNodeRead(FormattedFile *file)
{
  int code = file->Read(nodeId);
  if (code) return code;
  Initialize(nodeId);

  return ReadAnnotations(file) || DirectedGraphNodeReadUpCall(file);
}


int DirectedGraphNode::DirectedGraphNodeWrite(FormattedFile *file)
{
  return file->Write(nodeId) || WriteAnnotations(file) ||
    DirectedGraphNodeWriteUpCall(file);
}

void DirectedGraphNode::DirectedGraphNodeDump()
{
  dumpHandler.Dump("DirectedGraphNode[%x, %d]\n",this, nodeId);
  dumpHandler.BeginScope();
  DirectedGraphNodeDumpUpCall();
  DumpAnnotations();
  dumpHandler.EndScope();
}


//********************************
// private operations
//********************************

void DirectedGraphNode::DirectedGraphNodeDumpUpCall()
{
}

int DirectedGraphNode::DirectedGraphNodeReadUpCall(FormattedFile *)
{
  return 0;
}


int DirectedGraphNode::DirectedGraphNodeWriteUpCall(FormattedFile *)
{
  return 0;
}


void DirectedGraphNode::DeleteIncidentEdges(unsigned int incidentEdgeSet)
{
#if DEBUG
  cout << "deleting edges incident on node " << (void *) this << "\n";
  cout.flush();
#endif

  DirectedGraphEdge *edge;
  while (edge = edges[incidentEdgeSet]) delete edge;
}


//*******************************************************************
// class DirectedGraphEdge interface operations
//*******************************************************************

DirectedGraphEdge::DirectedGraphEdge
(DirectedGraph *dg, DirectedGraphNode *src, DirectedGraphNode *sink,
 unsigned int _edgeClass)
{
  graph = dg;

  assert(((_edgeClass >= 0) && (_edgeClass < graph->NumberOfEdgeClasses())) ||
	 (_edgeClass == ALL_EDGE_CLASSES));

  Initialize(src, sink, _edgeClass, dg->edgeIds.AcquireId());
}


DirectedGraphEdge::DirectedGraphEdge(DirectedGraph *dg)
{
  graph = dg;
}


void DirectedGraphEdge::Initialize
(DirectedGraphNode *src, DirectedGraphNode *sink, 
 unsigned int _edgeClass, unsigned int _edgeId)
{
#if DEBUG
  cout << "adding edge " << (void *) this << " from " << (void*) src << " to " << (void *) sink << "\n";
  cout.flush();
#endif

  edgeClass = _edgeClass;
  edgeId = _edgeId;
  LinkEdge(DirectedEdgeIn, sink);
  LinkEdge(DirectedEdgeOut, src);
  ++graph->numberOfEdges[edgeClass];
}


// when deleteing a node, update edge lists associated with
// predecessor and successor
DirectedGraphEdge::~DirectedGraphEdge()
{
#if DEBUG
  cout << "deleting edge " << (void *) this << "\n";
  cout.flush();
#endif

  UnLinkEdge(DirectedEdgeIn);
  UnLinkEdge(DirectedEdgeOut);

  graph->nodeIds.ReleaseId(edgeId);
  graph->numberOfEdges[edgeClass]--;
}
  

unsigned int DirectedGraphEdge::EdgeClass()
{
  return edgeClass;
}


DirectedGraph *DirectedGraphEdge::Graph()
{
  return graph;
}

DirectedGraphNode *DirectedGraphEdge::Sink()
{
  return endpoint[DirectedEdgeIn];
}


DirectedGraphNode *DirectedGraphEdge::Src()
{
  return endpoint[DirectedEdgeOut];
}


DirectedGraphNode *DirectedGraphEdge::Endpoint(DirectedEdgeDirection end)
{
  return endpoint[end];
}


unsigned int DirectedGraphEdge::Id()
{
  return edgeId;
}


int DirectedGraphEdge::DirectedGraphEdgeRead
(FormattedFile *file, PointerMap *map)
{
  void *src, *sink;
  unsigned int tmpClass, tmpId;
  int code = file->Read(src) || file->Read(sink) ||  file->Read(tmpClass) || 
    file->Read(tmpId);
  if (code) return code;

#if DEBUG
  cout << "read edge " << (void *) this << " from " << src << " to " << sink << "\n";
  cout << "read (mapped) edge " << (void *) this << " from " << map->Map(src) << " to " << map->Map(sink) << "\n";
  cout.flush();
#endif

  Initialize((DirectedGraphNode *) map->Map(src), 
	     (DirectedGraphNode *) map->Map(sink), tmpClass, tmpId);

  return  ReadAnnotations(file) || DirectedGraphEdgeReadUpCall(file);
}


int DirectedGraphEdge::DirectedGraphEdgeWrite(FormattedFile *file)
{ 
#if DEBUG
  cout << "writing edge " << (void *) this << " from " << (void*) Src() << " to " << (void *) Sink() << "\n";
  cout.flush();
#endif

  return file->Write(Src()) || file->Write(Sink()) ||  file->Write(edgeClass) || 
    file->Write(edgeId) ||  WriteAnnotations(file) ||
    DirectedGraphEdgeWriteUpCall(file);
}


void DirectedGraphEdge::DirectedGraphEdgeDump()
{
  dumpHandler.Dump("DirectedGraphEdge[%x, %d] from %d to %d\n",this, edgeId, 
		   Src()->Id(), Sink()->Id());
  dumpHandler.BeginScope();
  DirectedGraphEdgeDumpUpCall();
  DumpAnnotations();
  dumpHandler.EndScope();
}

//********************************
// private operations
//********************************

void DirectedGraphEdge::DirectedGraphEdgeDumpUpCall()
{
}

int DirectedGraphEdge::DirectedGraphEdgeReadUpCall(FormattedFile *)
{
  return 0;
}


int DirectedGraphEdge::DirectedGraphEdgeWriteUpCall(FormattedFile *)
{
  return 0;
}


void DirectedGraphEdge::LinkEdge(DirectedEdgeDirection end, 
				 DirectedGraphNode *node)
{
  endpoint[end] = node;

  // link edge at head of edge list
  prev[end] = 0;
  next[end] = node->edges[edgeClass * DIRECTED_EDGE_ENDPOINT_TYPE_COUNT + end];
  if (next[end]) next[end]->prev[end] = this;
  node->edges[edgeClass * DIRECTED_EDGE_ENDPOINT_TYPE_COUNT + end] = this;
}


void DirectedGraphEdge::UnLinkEdge(DirectedEdgeDirection end) 
{
  // splice self out of edge list
  if (prev[end]) prev[end]->next[end] = next[end];
  else {
    endpoint[end]->edges[edgeClass * DIRECTED_EDGE_ENDPOINT_TYPE_COUNT + end] =
      next[end];
  }
  if (next[end]) next[end]->prev[end] = prev[end];

  // clear edge list pointers
  next[end] = 0;
  prev[end] = 0;
}



//*******************************************************************
// private operations
//*******************************************************************


static void BuildNodeVector(const DirectedGraphNode *node, va_list args)
{ 
  DGNodePointerVector *nodeVector = va_arg(args, DGNodePointerVector *);
  nodeVector->Append(node); 
}


