/* $Id: CallGraphNodeEdge.h,v 1.5 1997/03/27 20:40:12 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef CallGraphNodeEdge_h
#define CallGraphNodeEdge_h

//***************************************************************************
//   CallGraphNodeEdge.h
//
//      Definitions for nodes and edges in the callgraph
//
//		ProcParBinding defines the mapping across a callsite
//
//		CallGraphNode defines the node structure
//
//		CallGraphEdge defines the edge structure
//
//    Author: 
//      John Mellor-Crummey                                    January 1993
//
//    this implementation evolved from an earlier prototype developed by 
//    Mary Hall and and Rene Rodriguez in January 1992
//
//    Copyright 1993, Rice University
//
//***************************************************************************

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef DirectedGraph_h
#include <libs/support/graphs/directedGraph/DirectedGraph.h>
#endif

#ifndef CallSiteParamBindings_h
#include <libs/ipAnalysis/callGraph/CallSiteParamBindings.h>
#endif

#include <libs/support/strings/StringSet.h>

//--------------------------------------------------------------------------
// external declarations
//--------------------------------------------------------------------------
class ClassInstanceRegistry; // external definition
class FormattedFile;         // external definition
class Annotation;            // external definition

class CallGraph;
class CallSite; 
class ProcSummary;
class ProcParBindingsSet;

//--------------------------------------------------------------------------
// forward declarations
//--------------------------------------------------------------------------
class CallGraphEdge;
class ProcParBindingsSet;
class FormalParameterSet;
class EntryPoints;

typedef enum {CGNT_Program, CGNT_Subroutine, CGNT_Function, CGNT_BlockData, 
	      CGNT_ProcParCaller, CGNT_Start, CGNT_Entry, CGNT_Exit, 
	      CGNT_PlaceHolder } CallGraphNodeType;

typedef enum {CGET_CallSite, CGET_FromEntry, CGET_ToExit, CGET_FromStart,
	      CGET_ProcParBinding, CGET_PlaceHolder } CallGraphEdgeType;

#define FIRST_FORMAL 0


//--------------------------------------------------------------------------
// class CallGraphNode 
// 
//    a node in a call graph represents either a user procedure, or a
//    (user procedure, procedure variable) pair that represents invocations
//    of "procedure variable" from "user procedure"
//
//    JMC 1/93
//--------------------------------------------------------------------------
class CallGraphNode : public DirectedGraphNode {
  FormalParameterSet *formalParameterSet;
  EntryPoints *entryPoints;
  ClassInstanceRegistry *GetAnnotMgrRegistry();
public:
  static ClassInstanceRegistry *annotMgrRegistry;
  //----------------------------------------------------------------------
  // public data fields 
  //----------------------------------------------------------------------
  CallGraphNodeType           type;		
  char                        *procName; 	// procedure name

  //----------------------------------------------------------------------
  // constructors 
  //----------------------------------------------------------------------
  // constructor for a standard procedure node
  CallGraphNode(CallGraph *cg, ProcSummary *summary);
  
  // constructor for synthetic nodes used to represent either a 
  // (procedure parameter, caller) pair, or the start node
  CallGraphNode(CallGraph *cg, const char *syntheticNodeName, 
		CallGraphNodeType _type, int nparameters); 
  
  // constructor for a placeholder node such as START, ENTRY, or EXIT 
  // (each of which must specify a procName) or a placeholder to be filled
  // in by a read, which has an empty name 
  CallGraphNode(CallGraph *cg, CallGraphNodeType _type = CGNT_PlaceHolder,
	      const char *_procName = 0);
  
  //----------------------------------------------------------------------
  // destructor 
  //----------------------------------------------------------------------
  ~CallGraphNode();

  //--------------------------------------------------------------
  // virtual function that supplies class name
  //--------------------------------------------------------------
  CLASS_NAME_FDEF(CallGraphNode);
  
  
  //----------------------------------------------------------------------
  // Formal parameter handling:
  //
  // formal parameter name <--> position mapping
  // formal parameter positions numbered starting at FIRST_FORMAL -- JMC 11/92
  //----------------------------------------------------------------------
  Boolean IsFormal(const char *name);         // test if name is formal

  // -1 --> not found
  int FormalNameToPosition(const char *entryPoint, const char *formal);

  // 0 --> not found
  const char *FormalPositionToName(const char *entryPoint, unsigned int);

  //----------------------------------------------------------------------
  // access to callgraph
  //----------------------------------------------------------------------
  CallGraph *GetCallGraph();
  
  //----------------------------------------------------------------------
  // procedure-valued variable handling
  //----------------------------------------------------------------------
  ProcParBindingsSet  *GetProcParBindings(const char *procName);
  Boolean ProcParHasBinding(const char *formal, const char *procName);
  void AddProcParBinding(const char *formal, const char *procName);
  
  //----------------------------------------------------------------------
  // Access to annotations 
  //----------------------------------------------------------------------
  void PutAnnotation(Annotation *a);
  void DeleteAnnotation(const char *aname);
  Annotation *GetAnnotation(const char *aname, Boolean demand = false);
  
  //----------------------------------------------------------------------
  // I/O
  //----------------------------------------------------------------------
  int DirectedGraphNodeReadUpCall(FormattedFile *file);
  int DirectedGraphNodeWriteUpCall(FormattedFile  *file);

  void DirectedGraphNodeDumpUpCall();
  
  
// CallGraphEdge as friend to avoid exposing internal representation
// details in the external interface -- JMC 11/92
friend class CallGraphEdge;
  
// call graph node iterators
friend class CallGraphNodeOutEdgeIterator;  
friend class CallGraphNodeInEdgeIterator;  
};


#define INVALID_CALLSITE_INDEX   -1

//--------------------------------------------------------------------------
// class CallGraphEdge 
// 
//    an edge in a call graph represents either a direct invocation of one 
//    user procedure by another, an invocation by a user procedure through
//    a particular procedure variable, or a binding of an invoked procedure 
//    variable to a particular callee. typically, users only care about 
//    information along edges directed out of user procedures. the other
//    types exist to avoid the need for special case handling of 
//    invocations of procedure variables during interprocedural data flow
//    analysis.
//
//    JMC 1/93
//--------------------------------------------------------------------------
class CallGraphEdge : public DirectedGraphEdge {
  ClassInstanceRegistry *GetAnnotMgrRegistry();
public:
  static ClassInstanceRegistry *annotMgrRegistry;
  //----------------------------------------------------------------------
  // public data fields 
  //----------------------------------------------------------------------
  CallGraphEdgeType type;

  char *callSiteName; 		       
  int callSiteIndexInCaller; 

  char *calleeEntryName;
  CallSiteParamBindings paramBindings;	

  //----------------------------------------------------------------------
  // constructors
  //----------------------------------------------------------------------

  // Empty edge for read routine 
  CallGraphEdge(CallGraph *cg);

  // used to build an edge from start node
  CallGraphEdge(CallGraph *cg, CallGraphNode *caller, CallGraphNode *callee,
	      CallGraphEdgeType _type);

  // used to build edges from entry and exit nodes for flow analysis
  CallGraphEdge(CallGraph *cg, CallGraphNode *caller, CallGraphNode *callee);

  // add an edge using local information about a callsite
  CallGraphEdge(CallGraph *cg, CallGraphNode *caller, CallGraphNode *callee,
		const char *calleeName, CallSite *cs);

  // add an edge from a node representing a (procedure parameter, caller) pair
  // to a callee resolved from the procedure parameter binding
  CallGraphEdge(CallGraph *cg, CallSite *cs, CallGraphNode *caller, 
		CallGraphNode *callee, const char *calleeName); 

  //----------------------------------------------------------------------
  // destructor
  //----------------------------------------------------------------------
  ~CallGraphEdge();

  //--------------------------------------------------------------
  // virtual function that supplies class name
  //--------------------------------------------------------------
  CLASS_NAME_FDEF(CallGraphEdge);
  
  //----------------------------------------------------------------------
  // access to callgraph
  //----------------------------------------------------------------------
  CallGraph *GetCallGraph();  

  //----------------------------------------------------------------------
  // access to edge source and sink 
  //----------------------------------------------------------------------
  CallGraphNode *Caller(); 
  CallGraphNode *Callee();
  
  //----------------------------------------------------------------------
  // Access to annotations 
  //----------------------------------------------------------------------
  void PutAnnotation(Annotation *a);
  void DeleteAnnotation(const char *aname);
  Annotation *GetAnnotation(const char *aname, Boolean demand = false);
  
  //----------------------------------------------------------------------
  // I/O
  //----------------------------------------------------------------------
  int DirectedGraphEdgeReadUpCall(FormattedFile *file);
  int DirectedGraphEdgeWriteUpCall(FormattedFile *file);

  void DirectedGraphEdgeDumpUpCall();
  
// CallGraphNode as friend to avoid exposing internal representation
// details in the external interface -- JMC 11/92
friend class CallGraphNode;
};

//*************************************************************************
// class ProcParBindingsSetIterator interface
//*************************************************************************

class ProcParBindingsIteratorS {
public:
  ProcParBindingsIteratorS(StringSet *t) : iterator(t) {};
  StringSetIterator iterator;
};



//--------------------------------------------------------------------------
// class ProcParBindingsIterator  
// 
//    an iterator that enumerates the names of all procedure names in
//    the binding set of a particular procedure variable
//
//    JMC 1/93
//--------------------------------------------------------------------------
class ProcParBindingsIterator { 
  ProcParBindingsIteratorS *hidden;
public:
  ProcParBindingsIterator(ProcParBindingsSet *s); 
  ~ProcParBindingsIterator(); 

  const char *Current();     // current value of the iterator
  void Reset();        // restart the iterator
  void operator ++();  // advance the iterator
};

char *CallGraphEdgeName(const char *caller_procName, int callsiteid);
char *CallGraphNodeNameForCallerProcParPair(const char *callerName, 
					    const char *proc_varName);

CLASS_NAME_EDEF(CallGraphNode);
CLASS_NAME_EDEF(CallGraphEdge);

#endif /* CallGraphNodeEdge_h */
