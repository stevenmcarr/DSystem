/* $Id: CallGraph.h,v 1.2 1997/03/11 14:34:29 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef CallGraph_h
#define CallGraph_h

//***************************************************************************
//    CallGraph.h:  
//
//      definition of a representation of for a call graph for a program.
//      nodes in the graph represent procedures, edges represent callsites.
//      call graphs are the basis for interprocedural analysis.
//
//    Author: 
//      John Mellor-Crummey                                    
//
//        based in part on an early prototype by Mary Hall and 
//        Rene Rodriguez
//
//    Copyright 1994, Rice University
//***************************************************************************

#ifndef DirectedGraph_h
#include <libs/support/graphs/directedGraph/DirectedGraph.h>
#endif

#ifndef Attribute_h
#include <libs/fileAttrMgmt/attributedFile/Attribute.h>
#endif

class ClassInstanceRegistry; // external definition
class FormattedFile;         // external definition
class Annotation;            // external definition

class Composition;           // external definition

class CallGraphEdge;         // external definition
class CallGraphNode;         // external definition
class ProcSummary;           // external definition


//--------------------------------------------------------------------------
// class CallGraph 
//
//    the representation of a call graph for a program. this representation 
//    is the basis for interprocedural analysis.
//
//--------------------------------------------------------------------------
class CallGraph : public DirectedGraph, public Attribute {
public:
  static ClassInstanceRegistry *annotMgrRegistry;
  const Composition *program;  

  CallGraphNode *entryNode; // usual flow graph entry node
  CallGraphNode *exitNode;  // usual flow graph exit node
  CallGraphNode *startNode; // execution starts here

  //--------------------------------------------------------------
  // constructor and destructor
  //--------------------------------------------------------------
  CallGraph();
  ~CallGraph();

  //--------------------------------------------------------------
  // virtual function that supplies class name
  //--------------------------------------------------------------
  CLASS_NAME_FDEF(CallGraph);
  
  //--------------------------------------------------------------
  // construct a callgraph from initial information about modules in 
  // the program. build is separate from the constructor since a
  // callgraph can also be built by reading in an external 
  // representation from a file. returns 0 on success.
  //--------------------------------------------------------------
  int Build();		
  
  //--------------------------------------------------------------
  // lookup the representation of a node and or edge in the graph
  //--------------------------------------------------------------
  CallGraphNode *LookupNode(const char *name);
  CallGraphEdge *LookupEdge(const char *caller, int id); 

  // the LookupEdge interface below is present for completeness, but 
  // is not intended to be used for anything other than incremental
  // callgraph construction -- JMC 1/93 
  CallGraphEdge *LookupEdge(const char *callSite);  

  //----------------------------------------------------------------------
  // Access to annotations 
  //----------------------------------------------------------------------
  void PutAnnotation(Annotation *a);
  void DeleteAnnotation(const char *aname);
  Annotation *GetAnnotation(const char *aname, Boolean demand = false);
  
  //--------------------------------------------------------------
  // demand a specified annotation be computed everywhere
  // it is appropriate in the callgraph. this is an interface
  // suitable for testing computation of annotations that
  // as of yet have no consumers that demand their computation
  //--------------------------------------------------------------
  void DemandNodeAnnotations(char *aname);
  void DemandEdgeAnnotations(char *aname);

  void Dump();
  
private: // member functions
  virtual DirectedGraphNode *NewNode();
  virtual DirectedGraphEdge *NewEdge();

  int AddStaticCallEdges(ProcSummary *summary);

  void AddNodeNameMapEntry(CallGraphNode *node, const char *name);
  void AddEdgeNameMapEntry(CallGraphEdge *edge, const char *name);
  void AddEntryExitEdgesForNode(CallGraphNode *node);


  int Create();
  void Destroy();

  //--------------------------------------------------------------
  // Attribute interface
  //--------------------------------------------------------------
  int ReadUpCall(File *file);
  int WriteUpCall(File *file);
  int ComputeUpCall();

  void DirectedGraphDumpUpCall();

  ClassInstanceRegistry *GetAnnotMgrRegistry();

private: // data
  struct CallGraphS *hidden;   // keep other representation details  hidden 
  
friend class CallGraphNode;
friend class CallGraphEdge;
};

CLASS_NAME_EDEF(CallGraph);

#endif /* CallGraph_h */
