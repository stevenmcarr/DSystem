/* $Id: CallGraph.C,v 1.8 1999/03/31 21:55:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
//    CallGraph.C:  The class CallGraph defines the calling structure of a 
//                  program which is used as the basis for solving 
//                  interprocedural problems.
//
//    Author: 
//      John Mellor-Crummey                                    January 1993
//
//    this implementation evolved from an early prototype written by 
//    Mary Hall and and Rene Rodriguez in January 1992
//
//    Copyright 1993, 1994 Rice University
//
//***************************************************************************

#include <stdarg.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifdef LINUX
#include <assert.h>
#endif

#include <libs/support/misc/dict.h>
#include <libs/support/tables/NameValueTable.h>
#include <libs/support/file/FormattedFile.h>
#include <libs/support/msgHandlers/ErrorMsgHandler.h>
#include <libs/support/msgHandlers/DumpMsgHandler.h>

#include <libs/ipAnalysis/ipInfo/CallSite.h>
#include <libs/ipAnalysis/ipInfo/ProcSummary.h>

#include <libs/fileAttrMgmt/module/Module.h>
#include <libs/fileAttrMgmt/module/ModuleProcsIterator.h>
#include <libs/fileAttrMgmt/module/ProcSummariesModAttr.h>

#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/composition/CompositionIterators.h>

#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphIterators.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotMgrs.h>
#include <libs/ipAnalysis/callGraph/ModuleInfoIterator.h>

#include <libs/ipAnalysis/callGraph/CallGraphAnnotReg.h>


//*******************************************************************
// declarations 
//*******************************************************************


REGISTER_COMPOSITION_ATTRIBUTE(CallGraph);


//---------------------------------------------------------------------------
// class WorkListEntry, WorkList
//
//    representation of a worklist data structure needed for processing of
//    procedure variable bindings
//
//    JMC 1/93
//---------------------------------------------------------------------------
class WorkListEntry {
public:
  const char *binding;
  const char *formalName;
  const char *calleeName;
  WorkListEntry  *next;
  WorkListEntry(const char *actual, const char *callee, const char *formal) : 
  binding(actual), calleeName(callee), formalName(formal), next(NULL) {};
};

class WorkList {
  WorkListEntry *head;
public:
  WorkList () : head(NULL) {};

  int NonEmpty() { return head != NULL; };

  void Insert(const char *actual, const char *callee, const char *formal) { 
    WorkListEntry *elt = new WorkListEntry(actual, callee, formal); 
    elt->next = head;
    head = elt;
  };

  WorkListEntry *Extract() {
    WorkListEntry *elt = head;
    if (elt != NULL) head = elt->next;
    return elt;
  };
};


class ProcSummaryTable : private NameValueTable {
public:
  ProcSummaryTable() { Create(8); }
  ~ProcSummaryTable() { Destroy(); };
  ProcSummary *GetSummary(CallGraphNode *node) {
    ProcSummary *ps;
    if (QueryNameValue((Generic) node, (Generic *) &ps) == false)
      assert(0);
    return ps;
  };
  void AddSummary(CallGraphNode *node, ProcSummary *ps) {
    AddNameValue((Generic) node, (Generic) ps, 0);
  };
protected:
    uint NameHashFunct (const Generic name, const uint size) {
      return name % size;
    };
    int  NameCompare (const Generic name1, const Generic name2) {
      return name1 - name2;
    };
friend class ProcSummaryTableIterator;
};


class ProcSummaryTableIterator : private NameValueTableIterator {
public:
  ProcSummaryTableIterator(ProcSummaryTable *pst) : 
  NameValueTableIterator(pst){ };
  ProcSummary *Current() { return (ProcSummary *) value; };
  NameValueTableIterator::operator++;
  NameValueTableIterator::Reset;
};





//=========================================================================
// class CallGraphS 
//=========================================================================

//-------------------------------------------------------------------------
// CallGraphS::CallGraphS()  
//
//    constructor for hidden representation: initialize node and edge 
//    dictionaries 
//
//    JMC 1/93
//-------------------------------------------------------------------------
CallGraphS::CallGraphS() : 
nameToNodeMap(cmpstr, hashstr, 0), nameToEdgeMap(cmpstr, hashstr, 0) 
{
} 


//**********************
// forward declarations
//**********************


static CallGraphNode *GetProcParCallNode(CallGraph *cg, 
					 const char *procParmName, 
					 const char *callerName, int parms);

static int ResolveProcedureVars(CallGraph *cg, ProcSummaryTable *pst);



//***************************************************************************
// class CallGraph interface operations
//***************************************************************************

ClassInstanceRegistry *CallGraph::annotMgrRegistry = 0;



CLASS_NAME_FIMPL(CallGraph)


//-------------------------------------------------------------------------
// CallGraph::CallGraph(Composition *_program)  
//
//    initialize the callgraph by creating dictionaries of the nodes and
//    edges, recording a handle for the program itself, and 
//    creating an underlying directed graph representation of the callgraph
//
//    JMC 1/93
//-------------------------------------------------------------------------
CallGraph::CallGraph()
{
} 


//-------------------------------------------------------------------------
// CallGraph::~CallGraph()
//
//    delete the underlying representation of the callgraph itself 
//    nameToNodeMap and nameToEdgeMap contents will be deleted when the 
//    nodes and edges are deleted by the underlying FlowGraph destructor.
//    
//    JMC 6/93
//-------------------------------------------------------------------------
CallGraph::~CallGraph()
{
  Destroy();
}


//-------------------------------------------------------------------------
// int CallGraph::Create()  
//-------------------------------------------------------------------------
int CallGraph::Create()  
{
  program = (Composition *) uplinkToFile;
  DirectedGraph::Create();
  hidden = new struct CallGraphS;
  return hidden ? 0 : -1;
}


//-------------------------------------------------------------------------
// void CallGraph::Destroy()  
//-------------------------------------------------------------------------
void CallGraph::Destroy()  
{
  if (hidden) {
    delete hidden;
    hidden = 0;
    DirectedGraph::Destroy();
  }
}


void CallGraph::AddNodeNameMapEntry(CallGraphNode *node, const char *theName)
{
  hidden->nameToNodeMap.Insert(theName, node); // augment node dictionary
}


void CallGraph::AddEdgeNameMapEntry(CallGraphEdge *edge, const char *theName)
{
  hidden->nameToEdgeMap.Insert(theName, edge); // augment edge dictionary
}


//-------------------------------------------------------------------------
// void CallGraph::Build() 
//
//    construct the callgraph using initial information about each user
//    module in the program. the initial information contains a 
//    representation of each user procedure, its formal parameters, and
//    representations of all of its callsites. the callsite representation
//    includes which procedure constant or procedure variable was invoked,
//    and the actual parameters passed. 
//
//    construction occurs in 3 phases:
//    (1) build a node to represent each user procedure
//    (2) iterate through the callsites of each user procedure and add
//        edges to the graph between nodes representing user procedures
//    
//    JMC 1/93
//-------------------------------------------------------------------------
int CallGraph::Build() 
{
  int returncode = 0;
  ProcSummaryTable pst;

  entryNode = new CallGraphNode(this, CGNT_Entry, "__ENTRY__");
  SetRoot(entryNode, DirectedEdgeOut);
  
  exitNode = new CallGraphNode(this, CGNT_Exit, "__EXIT__");
  SetRoot(exitNode, DirectedEdgeIn);

  startNode = new CallGraphNode(this, CGNT_Start, "__START__");
  AddEntryExitEdgesForNode(startNode);

  //-----------------------------------------------------------------------
  // for each module ...
  //-----------------------------------------------------------------------
  ModuleInfoIterator modules(program, CLASS_NAME(ProcSummariesModAttr));
  for (; modules.module; modules.Advance(false)) {
    ProcSummariesModAttr *psAttr = (ProcSummariesModAttr *) modules.moduleInfo;

    //-----------------------------------------------------------------------
    // for each procedure defined in the module ...
    //-----------------------------------------------------------------------
    ModuleLocalInfoIterator summaries(psAttr);
    for(ProcSummary *summary; summary = (ProcSummary *) summaries.Current(); 
	++summaries) {
      CallGraphNode *node = new CallGraphNode(this, summary);

      switch(node->type) { 
      case CGNT_Program:   // add empty edge to main program from startNode
      case CGNT_BlockData: // add empty edge to block data from startNode
	(void) new CallGraphEdge(this, startNode, node, CGET_FromStart);
	break;
      }

      pst.AddSummary(node, summary);
    }
  }

  ProcSummaryTableIterator psti(&pst);
  ProcSummary *summary;
  for (; summary = psti.Current(); ++psti) {
    returncode |= AddStaticCallEdges(summary);
  }
  
  //-------------------------------------------------------------------------
  // post-conditions of the preceeding code in Build:
  //     the callgraph will contain a node for each user procedure, and
  //     a synthetic procedure node for each (user procedure, procedure 
  //     variable invoked in that procedure) pair. 
  //
  //     an edge from the user procedure node
  //     to the synthetic procedure node will be present for
  //     each callsite at which the procedure variable is 
  //     invoked from the user procedure.
  //-------------------------------------------------------------------------

  //-------------------------------------------------------------------------
  // add edges that result from bindings of procedure variables 
  //-------------------------------------------------------------------------
  returncode |= ResolveProcedureVars(this, &pst);

  //-------------------------------------------------------------------------
  // release ProcSummariesModAttr for each module
  //-------------------------------------------------------------------------
  modules.Reset();
  for (; modules.module; modules.Advance(true)) {
    ProcSummariesModAttr *psAttr = (ProcSummariesModAttr *) modules.moduleInfo;
    psAttr->uplinkToFile->DetachAttribute(psAttr);
  }

  return returncode;
}


//----------------------------------------------------------------------
// void CallGraph::AddStaticCallEdges(ProcSummary *summary)
//
//    add edges to callgraph that correspond to invocations of 
//    procedure constants
//----------------------------------------------------------------------
int CallGraph::AddStaticCallEdges(ProcSummary *summary)
{
  int returncode = 0;
  CallGraphNode *caller = LookupNode(summary->NamedObject::name);
  
  //---------------------------------------------------------------------
  // I do not think this condition can ever be true here 
  // previously it caused a continue -- JMC 1/93
  //---------------------------------------------------------------------
  assert(caller->type != CGNT_ProcParCaller);
  
  //---------------------------------------------------------------------
  // for each callsite in the user procedure
  //---------------------------------------------------------------------
  CallSitesIterator csites(summary->calls);
  CallSite *cs;
  for ( ; (cs = csites.Current()) != NULL; ++csites) {
    
    //---------------------------------------------------------------------
    // if the callsite is an invocation using a procedure variable ...
    //---------------------------------------------------------------------
    if (cs->CalleeIsProcParameter()) {
      
      //-------------------------------------------------------------------
      // ensure that a synthetic procedure node exists to represent the
      // (caller, procedure parameter) pair -- JMC 1/93
      //-------------------------------------------------------------------
      (void) GetProcParCallNode(this, cs->Name(), caller->procName, 
				cs->GetActuals()->Count());
    }
    
    //---------------------------------------------------------------------
    // add edge appropriate for callsite. if the callee is a procedure
    // parameter, the sink of the edge is a synthetic procedure node, 
    // else it is the node for the called procedure -- JMC 11/92
    //---------------------------------------------------------------------
    
    const char *calleeName = 
      ((cs->CalleeIsProcParameter()) ? 
       CallGraphNodeNameForCallerProcParPair(caller->procName, cs->Name()) :
       cs->Name());
    
    CallGraphNode *callee = LookupNode(calleeName);

    if (callee) 
      (void) new CallGraphEdge(this, caller, callee, calleeName, cs);
    else {
      errorMsgHandler.HandleMsg("Callgraph construction error: call to unknown procedure %s from %s.\n", calleeName, caller->procName);
      returncode = -1;
    }
  } 
  return returncode;
}


//-------------------------------------------------------------------------
// CallGraphEdge *CallGraph::LookupEdge(const char *caller, int id) 
//
//    look up a callsite edge by the name of the caller and the id of the
//    callsite
//
//    JMC 1/93
//-------------------------------------------------------------------------
CallGraphEdge *CallGraph::LookupEdge(const char *caller, int id) 
{
  return (CallGraphEdge *) hidden->nameToEdgeMap[CallGraphEdgeName(caller, id)];
} 


//-------------------------------------------------------------------------
// CallGraphEdge *CallGraph::LookupEdge(const char *callSite) 
//
//    look up a callsite edge by name 
//
//    JMC 1/93
//-------------------------------------------------------------------------
CallGraphEdge *CallGraph::LookupEdge(const char *callSite) 
{
  return (CallGraphEdge *) hidden->nameToEdgeMap[callSite];
} 


//-------------------------------------------------------------------------
// CallGraphNode *CallGraph::LookupNode(const char *theName) 
//
//    look up a procedure node edge by name 
//
//    JMC 1/93
//-------------------------------------------------------------------------
CallGraphNode *CallGraph::LookupNode(const char *theName) 
{
  return (CallGraphNode *) hidden->nameToNodeMap[theName];
}


void CallGraph::AddEntryExitEdgesForNode(CallGraphNode *node)
{
  (void) new CallGraphEdge(this, this->entryNode, node, CGET_FromEntry);
  (void) new CallGraphEdge(this, node, this->exitNode, CGET_ToExit);
}


//------------------------------------------------------------------------
// annotation access
//------------------------------------------------------------------------
  
void CallGraph::PutAnnotation(Annotation *a) 
{ 
  AddAnnot(a);	
}
 

void CallGraph::DeleteAnnotation(const char *aname)
{ 
  DeleteAnnot(aname);	
}


Annotation *CallGraph::GetAnnotation(const char *name, Boolean demand) 
{
  // see if the annotation exists 
  Annotation *annot = QueryAnnot(name); 
  // if no such annotation exists and demand is true, construct it 
  if (annot == NULL && demand != false) {
    // look up the manager for the annotation
    CallGraphAnnotMgr *mgr = 
      LOOKUP_STATIC_CLASS_INSTANCE(name, CallGraphAnnotMgr,
				   GetAnnotMgrRegistry());
    
    assert(mgr != 0); // make sure we have a valid manager
    // have the manager construct the requested annotation 
    annot = mgr->Compute(this);
  }
  return annot;
}



//-------------------------------------------------------------------------
// void CallGraph::DemandNodeAnnotations(char *aname)
//
//    force computation of a named annotation for each node (edge) in 
//    the callgraph. the structure of the interprocedural analysis system
//    is designed to produce annotations in a demand driven manner. the
//    primary purpose of this function is for debugging. it can be used
//    to demand that a set of annotations be computed before a
//    consumer that will demand the annotations exists.
//
//    JMC 7/94
//-------------------------------------------------------------------------
void CallGraph::DemandNodeAnnotations(char *aname)
{ 
  // demand an annotation for each node 
  CallGraphNodeIterator nodes(this);
  CallGraphNode *node;
  for ( ; node = nodes.Current(); ++nodes) {
    node->GetAnnotation(aname, true); // demand annotation for node
  }
}


//-------------------------------------------------------------------------
// void CallGraph::DemandEdgeAnnotations(char *aname)
//
//    same as above, except for edges rather than nodes
//
//    JMC 7/94
//-------------------------------------------------------------------------
void CallGraph::DemandEdgeAnnotations(char *aname)
{ 
  // demand an annotation for each edge 
  CallGraphNodeIterator nodes(this);
  CallGraphNode *node;
  for ( ; node = nodes.Current(); ++nodes) {
    CallGraphEdgeIterator edges(node, DirectedEdgeOut);
    CallGraphEdge *edge; 
    for ( ; edge = edges.Current(); ++edges) {
      edge->GetAnnotation(aname, true); // demand annotation for edge
    }
  }
}


//----------------------------------------------------------------------
// int CallGraph::ComputeUpCall()
//
//    build a callgraph as an attribute of a composition 
//
//    JMC 1/94
//----------------------------------------------------------------------
int CallGraph::ComputeUpCall()
{
  if (program->IsCompleteAndConsistent() != true) return -1;
  return Build();
}


//----------------------------------------------------------------------
// int CallGraph::WriteUpCall(File *file) 
//
//    write an external representation of an annotated callgraph to 
//    a file.
//
//    JMC 1/94
//----------------------------------------------------------------------
int CallGraph::WriteUpCall(File *file) 
{
  FormattedFile ffile(file);
  return DirectedGraphWrite(&ffile);
}


//----------------------------------------------------------------------
// int CallGraph::ReadUpCall(File *file) 
//
//    read an external representation of an annotated callgraph from 
//    a file.
//
//    JMC 1/94
//----------------------------------------------------------------------
int CallGraph::ReadUpCall(File *file) 
{
  FormattedFile ffile(file);
  return DirectedGraphRead(&ffile);
}


//----------------------------------------------------------------------
// int CallGraph::DirectedGraphDumpUpCall() 
//
//    JMC 1/94
//----------------------------------------------------------------------
void CallGraph::DirectedGraphDumpUpCall() 
{
}

//----------------------------------------------------------------------
// int CallGraph::Dump() 
//
//    JMC 1/94
//----------------------------------------------------------------------
void CallGraph::Dump() 
{
  dumpHandler.Dump("CallGraph for program %s\n", 
		   ((Composition *) program)->ReferenceFilePathName());
  dumpHandler.BeginScope();
  Attribute::Dump();
  DirectedGraph::DirectedGraphDump();
  dumpHandler.EndScope();
}


//--------------------------------------------------------------------------
// virtual DirectedGraphNode *CallGraph::NewNode()
//--------------------------------------------------------------------------
DirectedGraphNode *CallGraph::NewNode()
{
  return new CallGraphNode(this);
}

//--------------------------------------------------------------------------
// virtual DirectedGraphEdge *CallGraph::NewEdge()
//--------------------------------------------------------------------------
DirectedGraphEdge *CallGraph::NewEdge()
{
  return new CallGraphEdge(this);
}


ClassInstanceRegistry *CallGraph::GetAnnotMgrRegistry()
{
  return annotMgrRegistry;
}



//*******************************************************************
// private operations
//*******************************************************************


//-------------------------------------------------------------------------
// static CallGraphNode *GetProcParCallNode( ... )
//
//    return a pointer to a synthetic procedure node representing a caller,
//    procedure variable pair. if a node is not already present for this
//    pair, create a new one
//
//    JMC 1/93
//-------------------------------------------------------------------------
static CallGraphNode *GetProcParCallNode(CallGraph *cg, 
					 const char *proc_varName, 
					 const char *callerName, int parms)
{
  const char *dummyName = 
    CallGraphNodeNameForCallerProcParPair(callerName, proc_varName);

  // look for a node representing the (caller, procedure parameter) pair
  CallGraphNode *node = cg->LookupNode(dummyName);       
  if (node) return node;  // if one already exists, return it
  else {
    // return a new node
    return new CallGraphNode(cg, dummyName, CGNT_ProcParCaller, parms); 
  }
}



//----------------------------------------------------------------------
// static int ResolveProcedureVars(ProcSummaryTable *pst)
//
//    construct synthetic procedure nodes that arise through invocations
//    through procedure variables. this routine adds edges from nodes
//    representing user procedures to these synthetic nodes, and from the
//    synthetic nodes to the procedures that are bound to the procedure
//    variable that the synthetic node represents.
//----------------------------------------------------------------------
static int ResolveProcedureVars(CallGraph *cg, ProcSummaryTable *pst)
{
  int returncode = 0;
  WorkList  worklist;   
  CallSite *cs;
  
  //-------------------------------------------------------------------------
  // INITIALIZATION                                             
  //
  //     for each node in the callgraph that represents a user 
  //     procedure, look for procedure constants passed at 
  //     callsites in the user procedure  -- JMC 1/93
  //-------------------------------------------------------------------------
  
  CallGraphNodeIterator nodes(cg);
  CallGraphNode *node;
  for (; node = nodes.Current(); ++nodes) {
    
    //---------------------------------------------------------------------
    // ignore any node that does not represent a user procedure -- JMC 1/93
    //---------------------------------------------------------------------
    if ((node->type != CGNT_Program) && (node->type != CGNT_Subroutine) &&
	(node->type != CGNT_Function)) continue; 
    
    //-------------------------------------------------------------------
    // walk callsites in the user procedure; for each callsite that is
    // not an invocation of a procedure variable, and add an entry to the
    // WorkList for each procedure constant passed as an actual parameter
    // -- JMC 1/93
    //-------------------------------------------------------------------
    CallSitesIterator csi(pst->GetSummary(node)->calls);
    for ( ; (cs = csi.Current()) != NULL; ++csi) {
      
      //--------------------------------------------------------------
      // ignore invocations through procedure variables -- JMC 1/93 
      //--------------------------------------------------------------
      if (NOT(cs->CalleeIsProcParameter())) { 
	CallGraphEdge *edge = cg->LookupEdge(node->procName, cs->Id());    
	
	//--------------------------------------------------------------
	// edge may be 0 if missing a procedure was detected in 
	// CallGraph::AddStaticCallEdges. no need to report that error here 
	// again. we simply ignore any procedure variables that may be
	// bound at this callsite and move on so we can detect further
	// errors if any.
	//--------------------------------------------------------------
	if (edge != 0) {  
	  //--------------------------------------------------------------
	  // for all actual parameters passed at the callsite ...
	  //--------------------------------------------------------------
	  for (ParamNameIterator ani(edge->paramBindings, ActualNameSet); 
	       ani.Current(); ++ani) {
	    const char *actual = ani.Current();
	    ParamBindingsSetIterator bi(edge->paramBindings.
					GetForwardBindings(actual));
	    ParamBinding *binding = bi.Current();
	    
	    //------------------------------------------------------------
	    // if an actual is a procedure constant ...
	    //------------------------------------------------------------
	    if (binding != NULL && binding->a_class == APC_ProcConstant) {
	      for (; binding = bi.Current(); ++bi) 
		
		//---------------------------------------------------------
		// add an entry to the worklist for each formal that the 
		// procedure constant is bound to in the callee -- JMC 1/93
		//---------------------------------------------------------
		worklist.Insert(actual, cs->Name(), binding->formal);
	    }
	  } 
	} else returncode = -1;
      } 
    } 
  }
  
  //-------------------------------------------------------------------------
  // MAIN ALGORITHM  (documentation added -- JMC 1/93)         
  //
  // pre-condition: 
  //     the worklist contains triples of the form 
  //     (procedure constant passed as an actual, name of callee
  //      node, formal parameter that the procedure constant is 
  //      bound to in the callee)
  //
  // invariant:
  //     each callee node name in a triple in the worklist is the
  //     name of a user procedure. synthetic procedure node names
  //     never appear in the worklist. 
  //
  // algorithm:  
  //     process each new binding in the worklist; add new bindings as they
  //     become known. iterate until all bindings have been examined
  //
  // post-conditions:
  //     from each synthetic procedure node representing uses of
  //     a procedure variable from a user procedure, there will 
  //     be an edge to each real procedure to which the procedure
  //     variable can be bound during execution of the program.
  //     
  //-------------------------------------------------------------------------
  
  while (worklist.NonEmpty()) {
    ParamBindingsSet *bindings;
    WorkListEntry *elt = worklist.Extract();
    
    //--------------------------------------------------------------
    // node represents a user procedure that receives a procedure 
    // constant as an argument -- JMC 1/93
    //--------------------------------------------------------------
    CallGraphNode *node = cg->LookupNode(elt->calleeName);
    
    //--------------------------------------------------------------
    // if the procedure constant specified in elt->binding is 
    // in the set of bindings that have already been considered 
    // for the formal parameter elt->formalName, no further 
    // processing is needed for cg worklist entry. -- JMC 1/93
    //--------------------------------------------------------------
    if (node->ProcParHasBinding(elt->formalName, elt->binding)) continue; 
    
    //--------------------------------------------------------------
    // add the procedure constant specified by elt->binding to the
    // set of procedure constant bindings considered for 
    // elt->formalName -- JMC 11/93
    //--------------------------------------------------------------
    node->AddProcParBinding(elt->formalName, elt->binding);
    
    //--------------------------------------------------------------
    // iterate over all of the callsites in the user procedure ...
    //--------------------------------------------------------------
    CallSitesIterator csi(pst->GetSummary(node)->calls);
    for( ; (cs = csi.Current()) != NULL; ++csi) {
      CallGraphEdge *edge = cg->LookupEdge(node->procName, cs->Id());
      
      //--------------------------------------------------------------
      // if the callsite is an invocation of a procedure constant 
      //--------------------------------------------------------------
      if (NOT(cs->CalleeIsProcParameter())) {
	
	//--------------------------------------------------------------
	// for each formal in the callee to which elt->formalName is bound,
	// add a new binding triple to the worklist that reflects that 
	// through cg callsite, the formal gets bound to the procedure
	// constant specified by elt->binding -- JMC 1/93
	//--------------------------------------------------------------
	bindings = edge->paramBindings.GetForwardBindings(elt->formalName);
	for (ParamBindingsSetIterator bi(bindings); bi.Current(); ++bi) {
	  worklist.Insert (elt->binding, cs->Name(), bi.Current()->formal);
	}
      }
      
      //--------------------------------------------------------------
      // else, if the callsite is an invocation of a procedure variable
      // other than elt->formalName -- JMC 1/93
      //--------------------------------------------------------------
      else if (strcmp(cs->Name(), elt->formalName) != 0) {
	
	//----------------------------------------------------------------
	// get the bindings of the procedure variable invoked at cg 
	// callsite -- JMC 1/93
	//----------------------------------------------------------------
	ProcParBindingsIterator ppbi(node->GetProcParBindings(cs->Name()));
	const char *procName;
				    
	//----------------------------------------------------------------
	// for each callee binding of the invoked procedure variable 
	// -- JMC 1/93
	//----------------------------------------------------------------
	for (; procName = ppbi.Current(); ++ppbi) {
	  bindings = edge->paramBindings.GetForwardBindings(elt->formalName);
	  
	  //----------------------------------------------------------------
	  // for each formal of the callee that is bound to elt->formalName,
	  // add a new entry to the worklist that reflects a binding of 
	  // that formal to the procedure constant elt->binding -- JMC 1/93
	  //----------------------------------------------------------------
	  for (ParamBindingsSetIterator bi(bindings); bi.Current(); ++bi) {
	    worklist.Insert(elt->binding, procName, bi.Current()->formal);
	  }
	}
      } else {
	//--------------------------------------------------------------
	// the callsite is an invocation of elt->formalName; the triple 
	// last extracted from the worklist describes a new binding of
	// elt->formalName to the procedure constant elt->binding -- JMC 1/93
	//--------------------------------------------------------------
	
	//--------------------------------------------------------------
	// insert a new edge in call graph from the synthetic procedure 
	// node that represents uses of the procedure variable name 
	// elt->formalName from the user procedure described by "node"
	// -- JMC 1/93
	//--------------------------------------------------------------

	// set up edge endpoints
	char *dummyCallerName = 
	  CallGraphNodeNameForCallerProcParPair(node->procName, cs->Name());
	CallGraphNode *dummyCaller = cg->LookupNode(dummyCallerName);
	CallGraphNode *callee = cg->LookupNode(elt->binding);
	assert(dummyCaller != 0);

	if (callee == 0){
	  errorMsgHandler.HandleMsg("Callgraph construction error: call through a procedure variable to unknown procedure %s from %s.\n",
				    elt->binding, node->procName);
	  returncode = -1;
	  
	} else {
	  CallGraphEdge *to_real_callee = 
	    new CallGraphEdge(cg, cs, dummyCaller, callee, elt->binding); 
	  
	  //--------------------------------------------------------------
	  // for each actual parameter passed at the callsite -- JMC 1/93
	  //--------------------------------------------------------------
	  for (ParamNameIterator ani(edge->paramBindings, ActualNameSet); 
	       ani.Current(); ++ani) {
	    const char *actual = ani.Current();
	    
	    //------------------------------------------------------------
	    // retrieve the set of bindings between the actual and the
	    // formals in the synthetic procedure node -- JMC 1/93
	    //------------------------------------------------------------
	    bindings = edge->paramBindings.GetForwardBindings(actual);
	    ParamBindingsSetIterator bi(bindings);
	    ParamBinding *syn_binding = bi.Current();
	    
	    //------------------------------------------------------------
	    // if the actual is a procedure constant, iterate through all
	    // of the formals it is bound to in the synthetic callee node
	    // -- JMC 1/93
	    //------------------------------------------------------------
	    if (syn_binding->a_class == APC_ProcConstant) {
	      for (; syn_binding = bi.Current(); ++bi) {
		
		//--------------------------------------------------------
		// get the single-valued forward binding of 
		// syn_binding->formal, a formal in the synthetic procedure 
		// node, to the corresponding formal in the real callee 
		// -- JMC 1/93
		//--------------------------------------------------------
		ParamBindingsSet *real_bindings = 
		  to_real_callee->paramBindings.
		  GetForwardBindings(syn_binding->formal);
		ParamBindingsSetIterator rbi(real_bindings);
		ParamBinding *real_binding = rbi.Current();
		
		//--------------------------------------------------------
		// add a triple to the worklist to consider the newly 
		// discovered binding of the procedure constant "actual"
		// to a formal in the real callee elt->binding -- JMC 1/93
		//--------------------------------------------------------
		worklist.Insert(actual, elt->binding, real_binding->formal);
	      }
	    }
	    
	    //-----------------------------------------------------------
	    // if actual is the procedure variable elt->formalName for
	    // which we have a newly discovered binding -- JMC 1/93
	    //-----------------------------------------------------------
	    else if (strcmp(actual, elt->formalName) == 0) {
	      for (; syn_binding = bi.Current(); ++bi) { 
		
		//--------------------------------------------------------
		// get the single-valued forward binding of 
		// syn_binding->formal, a formal in the synthetic procedure 
		// node, to the corresponding formal in the real callee 
		// -- JMC 1/93
		//--------------------------------------------------------
		ParamBindingsSet *real_bindings = 
		  to_real_callee->paramBindings.
		  GetForwardBindings(syn_binding->formal);
		ParamBindingsSetIterator rbi(real_bindings);
		ParamBinding *real_binding = rbi.Current();
		
		//--------------------------------------------------------
		// add a triple to the worklist to consider the propagation 
		// of the binding of procedure variable "actual" to 
		// elt->binding through the callsite to a formal in 
		// elt->binding which is also the real callee -- JMC 1/93
		//--------------------------------------------------------
		worklist.Insert (elt->binding, elt->binding, 
				 real_binding->formal);
	      }
	    } 
	    
	    //-----------------------------------------------------------
	    // if actual is a procedure variable, we have to propagate
	    // the its bindings to an invocation of elt->binding -- JMC 1/93
	    //-----------------------------------------------------------
	    else if (syn_binding->a_class == APC_ProcFormal) {
	      
	      //----------------------------------------------------------------
	      // get the bindings of the procedure variable passed at cg 
	      // callsite -- JMC 1/93
	      //----------------------------------------------------------------
	      ProcParBindingsSet *s = node->GetProcParBindings(actual);
	      ProcParBindingsIterator ppbi(s);
	      const char *procName;
	      
	      //----------------------------------------------------------------
	      // for each binding of the passed procedure variable -- JMC 1/93
	      //----------------------------------------------------------------
	      for (;  procName = ppbi.Current(); ++ppbi) {
		for (; syn_binding = bi.Current(); ++bi) {
		  
		  //--------------------------------------------------------
		  // get the single-valued forward binding of 
		  // syn_binding->formal, a formal in the synthetic procedure 
		  // node, to the corresponding formal in the real callee 
		  // -- JMC 1/93
		  //--------------------------------------------------------
		  ParamBindingsSet *real_bindings = 
		    to_real_callee->paramBindings.
		    GetForwardBindings(syn_binding->formal);
		  ParamBindingsSetIterator rbi(real_bindings);
		  ParamBinding *real_binding = rbi.Current();
		  
		  //--------------------------------------------------------
		  // add a triple to the worklist for each known binding of 
		  // the actual 
		  // -- JMC 1/93
		  //--------------------------------------------------------
		  worklist.Insert(procName, elt->binding, real_binding->formal);
		}
	      }
	    }
	  } 
	} 
      } // end else 
      
    } // end for all callsites of a user procedure 

    delete elt; // free each item from the worklist after use -- JMC 6/93

  } // end while worklist nonempty 
  
  return returncode;
}

