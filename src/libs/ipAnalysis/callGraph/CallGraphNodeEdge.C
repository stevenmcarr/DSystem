/* $Id: CallGraphNodeEdge.C,v 1.7 1997/03/11 14:34:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdio.h>
#include <assert.h>

#include <libs/ipAnalysis/ipInfo/ProcSummary.h>
#include <libs/ipAnalysis/ipInfo/ParameterList.h>
#include <libs/ipAnalysis/ipInfo/CallSite.h>

#include <libs/support/registry/ClassInstanceRegistry.h>
#include <libs/support/strings/StringIO.h>
#include <libs/support/file/FormattedFile.h>
#include <libs/support/tables/namedObject/NamedObject.h>
#include <libs/support/tables/namedObject/NamedObjectTable.h>
#include <include/ClassName.h>
#include <libs/support/strings/rn_string.h>
#include <libs/support/msgHandlers/DumpMsgHandler.h>

#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotMgrs.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>

#include <libs/support/strings/StringSet.h>


#define MAX_NAME_LENGTH 100


//*************************************************************************
// declarations
//*************************************************************************

//***********************
// forward declarations
//***********************

static CallGraphNodeType GetCallGraphNodeType(ProcSummary *ps);

char *CallGraphEdgeName(const char *callerProcName, int callsiteid);

char *CallGraphNodeNameForCallerProcParPair
(const char *callerName, const char *procVarName);

static char *CallGraphEdgeNameForProcParBinding
(const char *callerName, const char *procParBinding, int callsiteId);

static char *CallGraphSyntheticEdgeName
(const char *callerName, const char *calleeName);

//*************************************************************************
// class ProcParBindingsSet interface
//*************************************************************************

class ProcParBindingsSet : public NamedObjectIO, private StringSet {
public:
  ProcParBindingsSet(const char *_name) : NamedObjectIO(_name) { };
  ~ProcParBindingsSet() { };

  CLASS_NAME_FDEF(ProcParBindingsSet);

  StringSet::Add;
  StringSet::IsMember;

  int NamedObjectWriteUpCall(FormattedFile *file) { 
    return StringSet::Write(file);
  };

  int NamedObjectReadUpCall(FormattedFile *file) { 
    return StringSet::Read(file);
  };

  void NamedObjectDumpUpCall() { 
    dumpHandler.BeginScope();
    StringSet::DumpContents(); 
    dumpHandler.EndScope();
  };
friend class ProcParBindingSetIterator;
};


CLASS_NAME_IMPL(ProcParBindingsSet);


//*************************************************************************
// class ProcParBindingsSetIterator interface
//*************************************************************************

struct ProcParBindingsIteratorS {
  ProcParBindingsIteratorS(StringSet *t) : iterator(t) {};
  StringSetIterator iterator;
};


ProcParBindingsIterator::ProcParBindingsIterator(ProcParBindingsSet *s)
{
  hidden = new ProcParBindingsIteratorS((StringSet *) s);
}


ProcParBindingsIterator::~ProcParBindingsIterator()
{
  delete hidden; 
}


const char *ProcParBindingsIterator::Current()
{
  return  hidden->iterator.Current();
}


void ProcParBindingsIterator::Reset()
{
  hidden->iterator.Reset();
}


void ProcParBindingsIterator::operator++()
{
  (hidden->iterator)++;
}


//*************************************************************************
// class FormalParameterSet interface
//*************************************************************************

class FormalParameterSet : private NamedObjectTableIO {
public:
  ~FormalParameterSet() {
    NamedObjectTable::Destroy();
  };
  CLASS_NAME_FDEF(FormalParameterSet);

  Boolean IsMember(const char *theName) { 
    return QueryEntry(theName)?true:false; };
  
  void AddEntry(ProcParBindingsSet *set) {
    NamedObjectTable::AddEntry(set);
  };

  ProcParBindingsSet *QueryEntry(const char *formal) {
    return (ProcParBindingsSet *) NamedObjectTable::QueryEntry(formal);
  };

  int Write(FormattedFile *file) { return NamedObjectTableWrite(file); };
  int Read(FormattedFile *file) { return NamedObjectTableRead(file); };
  void Dump() { NamedObjectTable::NamedObjectTableDump(); };
private:
  NamedObjectIO *NewEntry() { return new FormalParameter(); };
};

CLASS_NAME_IMPL(FormalParameterSet);



//*************************************************************************
//                  CallGraphNode Abstraction
//*************************************************************************

ClassInstanceRegistry *CallGraphNode::annotMgrRegistry = 0;


CLASS_NAME_IMPL(CallGraphNode);


//------------------------------------------------------------------------
// call graph node constructors
//------------------------------------------------------------------------

// create a placeholder call graph node 
CallGraphNode::CallGraphNode
(CallGraph *cg, CallGraphNodeType _type, const char *_procName) : 
DirectedGraphNode(cg), formalParameterSet(new FormalParameterSet), 
entryPoints(new EntryPoints), procName(_procName ? ssave(_procName) : 0), 
type(_type)
{
  assert((type == CGNT_PlaceHolder && procName == 0) ||
	 (type != CGNT_PlaceHolder && procName != 0));
}


CallGraphNode::CallGraphNode(CallGraph *cg, ProcSummary *procSummary) : 
DirectedGraphNode(cg), type(GetCallGraphNodeType(procSummary)),
procName(ssave(procSummary->name)), formalParameterSet(new FormalParameterSet),
entryPoints(new EntryPoints)
{
  *entryPoints = procSummary->entryPoints;

  EntryPointsIterator entries(entryPoints);
  EntryPoint *entry;
  for (; entry = entries.Current(); entries++) {
    FormalParametersIterator formals(&entry->formals);
    FormalParameter *formal;
    for (; formal = formals.Current(); formals++) {
      if (formalParameterSet->QueryEntry(formal->name) == 0)
	formalParameterSet->AddEntry(new ProcParBindingsSet(formal->name));
    }
    cg->AddNodeNameMapEntry(this, entry->name);
  }
  cg->AddEntryExitEdgesForNode(this);
}


CallGraphNode::CallGraphNode
(CallGraph *cg, const char *syntheticNodeName, CallGraphNodeType _type, 
 int nparameters) : DirectedGraphNode(cg), type(_type), 
 procName(ssave(syntheticNodeName)),
 formalParameterSet(new FormalParameterSet), entryPoints(new EntryPoints)
{
  EntryPoint *entry = new EntryPoint(procName);

  // Initialize formal parameter names with synthetic names 
  for(int i = FIRST_FORMAL; i < (nparameters + FIRST_FORMAL); i++) {
    char temp[32]; // sufficient for our chosen synthetic names -- JMC 11/92
    sprintf(temp, "$p%i", i);
    entry->formals.Append(new FormalParameter(temp));
    formalParameterSet->AddEntry(new ProcParBindingsSet(temp));
  }

  entryPoints->AddEntry(entry);

  cg->AddNodeNameMapEntry(this, entry->name);
  cg->AddEntryExitEdgesForNode(this);
}


//------------------------------------------------------------------------
// call graph node destructor
//------------------------------------------------------------------------


CallGraphNode::~CallGraphNode() 
{
  delete formalParameterSet;
  delete entryPoints;

  delete procName;
}


//------------------------------------------------------------------------
// call graph node formal parameter <--> parameter position mapping
//------------------------------------------------------------------------


Boolean CallGraphNode::IsFormal(const char *theName) 
{
  return formalParameterSet->IsMember(theName);
}

const char *CallGraphNode::FormalPositionToName(const char *entryPoint,
						unsigned int pos) 
{
  EntryPoint *entry = entryPoints->QueryEntry(entryPoint);
  FormalParameter *formal = entry ? entry->formals.GetMember(pos) : 0;
  return (formal ? formal->name : 0);
}


int CallGraphNode::FormalNameToPosition(const char *entryPoint,
					const char *formal) 
{
  EntryPoint *entry = entryPoints->QueryEntry(entryPoint);
  return (entry ? entry->formals.GetMemberIndex(formal) : -1);
}


//------------------------------------------------------------------------
// access to containing call graph 
//------------------------------------------------------------------------

CallGraph *CallGraphNode::GetCallGraph()
{
  return (CallGraph *) Graph();
}
  

//------------------------------------------------------------------------
// call graph node formal parameter binding access
//------------------------------------------------------------------------


ProcParBindingsSet *CallGraphNode::GetProcParBindings(const char *formal) 
{
  return formalParameterSet->QueryEntry(formal);
}

Boolean CallGraphNode::ProcParHasBinding(const char *formal, 
					 const char *procName) 
{
  ProcParBindingsSet *p = formalParameterSet->QueryEntry(formal);
  return BOOL(p ? p->IsMember(procName) : 0);
}

void CallGraphNode::AddProcParBinding(const char *formal, const char *procName)
{
  ProcParBindingsSet *p = formalParameterSet->QueryEntry(formal);
  assert(p);
  p->Add(procName);
}

void CallGraphNode::DirectedGraphNodeDumpUpCall() 
{ 
  dumpHandler.Dump("procName = %s\n", procName);
  dumpHandler.BeginScope();
  formalParameterSet->Dump();
  dumpHandler.EndScope();
}

//------------------------------------------------------------------------
// annotation access
//------------------------------------------------------------------------
  
void CallGraphNode::PutAnnotation(Annotation *a) 
{ 
  AddAnnot(a);	
}
 

void CallGraphNode::DeleteAnnotation(const char *aname)
{ 
  DeleteAnnot(aname);	
}


Annotation *CallGraphNode::GetAnnotation(const char *name, Boolean demand) 
{
  // see if the annotation exists 
  Annotation *annot = QueryAnnot(name); 
  // if no such annotation exists and demand is true, construct it 
  if (annot == NULL && demand != false) {
    // look up the manager for the annotation
    CallGraphNodeAnnotMgr *mgr = 
      LOOKUP_STATIC_CLASS_INSTANCE(name, CallGraphNodeAnnotMgr,
				   GetAnnotMgrRegistry());
    
    assert(mgr != 0); // make sure we have a valid manager
    // have the manager construct the requested annotation 
    annot = mgr->Compute(this);
  }
  return annot;
}



//------------------------------------------------------------------------
// call graph node I/O
//------------------------------------------------------------------------


int CallGraphNode::DirectedGraphNodeReadUpCall(FormattedFile *file) 
{
  int ntype;
  int code;
  
  code = ReadString(&procName, file);
  if (code) return code;
  
  code = file->Read(ntype);
  if (code) return code;
  type = (CallGraphNodeType) ntype;

  code = formalParameterSet->Read(file);
  if (code) return code;

  code = entryPoints->Read(file);
  if (code) return code;

  GetCallGraph()->AddNodeNameMapEntry(this, this->procName);

  return 0; // success!
}

int CallGraphNode::DirectedGraphNodeWriteUpCall(FormattedFile *file)
{
  int code;

  code = WriteString(procName, file);
  if (code) return code;

  code = file->Write((int) type); 
  if (code) return code;

  code = formalParameterSet->Write(file);
  if (code) return code;

  code = entryPoints->Write(file);
  if (code) return code;

  return 0; // success!
}

ClassInstanceRegistry *CallGraphNode::GetAnnotMgrRegistry()
{
  return annotMgrRegistry;
}


//*************************************************************************
//                  CallGraphEdge Abstraction
//*************************************************************************

ClassInstanceRegistry *CallGraphEdge::annotMgrRegistry = 0;

CLASS_NAME_IMPL(CallGraphEdge);

CallGraphEdge::CallGraphEdge(CallGraph *cg) : DirectedGraphEdge(cg), 
type(CGET_PlaceHolder)
{
}


CallGraphEdge::CallGraphEdge
(CallGraph *cg, CallGraphNode *caller, CallGraphNode *callee, 
 CallGraphEdgeType _type) : DirectedGraphEdge(cg, caller, callee),
 type(_type)
{
  assert(type == CGET_ToExit || type == CGET_FromEntry || 
	 type == CGET_FromStart);

  callSiteName = 
    ssave(CallGraphSyntheticEdgeName(caller->procName, callee->procName));
  calleeEntryName = ssave(callee->procName);
  callSiteIndexInCaller = INVALID_CALLSITE_INDEX;
}


CallGraphEdge::CallGraphEdge(CallGraph *cg, CallGraphNode *caller,
CallGraphNode *callee) :  
DirectedGraphEdge(cg, caller, callee)
{
  callSiteName = 0;
  calleeEntryName = 0;
  callSiteIndexInCaller = INVALID_CALLSITE_INDEX;
}

  
CallGraphEdge::CallGraphEdge
(CallGraph *cg, CallGraphNode *caller, CallGraphNode *callee, 
const char *calleeName, CallSite *cs) : DirectedGraphEdge(cg, caller, callee),
type(CGET_CallSite)
{
  // set up edge name
  callSiteName = ssave(CallGraphEdgeName(Caller()->procName, cs->Id()));

  calleeEntryName = ssave(calleeName); 
  callSiteIndexInCaller = cs->Id(); // set callsite id
  
  ActualList *al = cs->GetActuals();
  EntryPoint *entry = callee->entryPoints->QueryEntry(calleeName);
  int numFormals = entry->formals.NumberOfEntries();

  assert(numFormals == al->Count());
  
  SinglyLinkedListIterator actuals(al);
  for (int pos = FIRST_FORMAL; pos < (numFormals + FIRST_FORMAL);
	 pos++, actuals++) {
    ActualListEntry *actual = (ActualListEntry *) actuals.Current();
    char *actualName = actual->Name();
    const char *formalName = callee->FormalPositionToName(calleeName, pos);
    
    // Add to binding if formal, global, local, or procedure 
    // (not expression or other constant) 
    int atype = actual->Type();
    if (atype & (VTYPE_FORMAL_PARAMETER | VTYPE_COMMON_DATA | 
		 VTYPE_LOCAL_DATA | VTYPE_PROCEDURE)) {
      
      ActualParamClass form = atype;
      
      paramBindings.Bind(actualName, actual->GetField(0) /* offset */, 
			  actual->GetField(1) /* length */, form, /* scope, */
			  formalName); 
    }
  }
  
  cg->AddEdgeNameMapEntry(this, this->callSiteName);
}


CallGraphEdge::CallGraphEdge
(CallGraph *cg, CallSite *cs, CallGraphNode *dummyCaller, 
 CallGraphNode *callee, const char *calleeName) : 
 DirectedGraphEdge(cg, dummyCaller, callee), type(CGET_ProcParBinding)
{
  // number of formals to the dummy node (representing the binding of a 
  // procedure parameter for a caller) must agree with the number of formals
  // of the called procedure -- JMC 11/92
  EntryPoint *dummyEntry = 
    dummyCaller->entryPoints->QueryEntry(dummyCaller->procName);

  EntryPoint *calleeEntry = 
    callee->entryPoints->QueryEntry(calleeName);

  int nCalleeFormals = calleeEntry->formals.NumberOfEntries();

  assert(dummyEntry->formals.NumberOfEntries() == nCalleeFormals);

  // set up bindings between formals to the dummy node and formals of the
  // real called procedure -- JMC 11/92
  for(int findex = FIRST_FORMAL; findex < (nCalleeFormals + FIRST_FORMAL);
	  findex++) {
    // need defined constants here once I figure out what the constants
    // stand for -- JMC   6 Jan 93
    paramBindings.Bind
      (Caller()->FormalPositionToName(dummyCaller->procName, findex), 0, -1, 
       APC_Untyped, /* LocalScope, */
       Callee()->FormalPositionToName(calleeName, findex));
  }
  
  // set up edge name
  callSiteName = 
    ssave(CallGraphEdgeNameForProcParBinding(Caller()->procName, calleeName,
					     cs->Id())); 
  
  callSiteIndexInCaller = cs->Id();
  
  cg->AddEdgeNameMapEntry(this, this->callSiteName);
}
  
  
CallGraphEdge::~CallGraphEdge() 
{  
  sfree(callSiteName); 
  sfree(calleeEntryName); 
}

//------------------------------------------------------------------------
// access to edge endpoints
//------------------------------------------------------------------------

CallGraphNode *CallGraphEdge::Caller()
{
  return (CallGraphNode *) Src();
}

CallGraphNode *CallGraphEdge::Callee()
{
  return (CallGraphNode *) Sink();
}

//------------------------------------------------------------------------
// access to containing call graph 
//------------------------------------------------------------------------

CallGraph *CallGraphEdge::GetCallGraph()
{
  return (CallGraph *) Graph();
}
  

//------------------------------------------------------------------------
// annotation access
//------------------------------------------------------------------------
  
void CallGraphEdge::PutAnnotation(Annotation *a) 
{ 
  AddAnnot(a);	
}
 

void CallGraphEdge::DeleteAnnotation(const char *aname)
{ 
  DeleteAnnot(aname);	
}


Annotation *CallGraphEdge::GetAnnotation(const char *name, Boolean demand) 
{
  // see if the annotation exists 
  Annotation *annot = QueryAnnot(name); 
  // if no such annotation exists and demand is true, construct it 
  if (annot == NULL && demand != false) {
    // look up the manager for the annotation
    CallGraphEdgeAnnotMgr *mgr = 
      LOOKUP_STATIC_CLASS_INSTANCE(name, CallGraphEdgeAnnotMgr,
				   GetAnnotMgrRegistry());
    
    assert(mgr != 0); // make sure we have a valid manager
    // have the manager construct the requested annotation 
    annot = mgr->Compute(this);
  }
  return annot;
}



//------------------------------------------------------------------------
// call graph edge I/O
//------------------------------------------------------------------------

int CallGraphEdge::DirectedGraphEdgeReadUpCall(FormattedFile *file) 
{
  int ntype;
  int code = file->Read(ntype);
  if (code) return code;
  type = (CallGraphEdgeType) ntype;

  code = ReadString(&callSiteName, file) || 
    file->Read(callSiteIndexInCaller) || ReadString(&calleeEntryName, file);

  if (code) return code;

  GetCallGraph()->AddEdgeNameMapEntry(this, callSiteName);
 
  // read parameter binding map and return final result code
  return paramBindings.Read(file);
}


int CallGraphEdge::DirectedGraphEdgeWriteUpCall(FormattedFile *file) 
{
  return file->Write((int) type) || WriteString(callSiteName, file) || 
    file->Write(callSiteIndexInCaller) ||
    WriteString(calleeEntryName, file) || paramBindings.Write(file);
}

void CallGraphEdge::DirectedGraphEdgeDumpUpCall() 
{
  dumpHandler.Dump("callsiteName = %s\n", callSiteName);
  dumpHandler.Dump("calleeEntryName = %s\n", calleeEntryName);
  dumpHandler.Dump("callSiteIndexInCaller = %d\n", callSiteIndexInCaller);
  paramBindings.Dump();
}

ClassInstanceRegistry *CallGraphEdge::GetAnnotMgrRegistry()
{
  return annotMgrRegistry;
}



//*************************************************************************
// miscellaneous interface functions
//*************************************************************************

char *CallGraphEdgeName(const char *callerProcName, int callsiteid)
{
  static char temp[80];   // long enough for a procname in Fortran -- JMC 11/92
  sprintf(temp, "%s@%d", callerProcName, callsiteid);
  return temp;
}

char *CallGraphNodeNameForCallerProcParPair(const char *callerName, 
					    const char *procVarName)
{
  // synthesize name to represent (caller, procedure parameter) pair 
  static char temp[80];   // long enough for a procname in Fortran -- JMC 11/92
  sprintf(temp, "%s.%s", callerName, procVarName);
  return temp;
}


//*************************************************************************
//                  private operations
//*************************************************************************

static CallGraphNodeType GetCallGraphNodeType(ProcSummary *ps)
{
  switch(ps->procType) {
  case ProcType_PGM: return CGNT_Program;
  case ProcType_BDATA: return CGNT_BlockData;
  case ProcType_FUN: return CGNT_Function;
  case ProcType_SUB: return CGNT_Subroutine;
  case ProcType_ILLEGAL: assert(0); return CGNT_PlaceHolder;
  }
}


static char *CallGraphSyntheticEdgeName(const char *callerName, 
				 const char *calleeName)
{
  static char temp[80];   // long enough for a procname in Fortran
  sprintf(temp, "%s->%s", callerName, calleeName);
  return temp;
}


static char *CallGraphEdgeNameForProcParBinding(const char *callerName,
						const char *procParBinding,
						int callsiteId)
{
  // synthesize name to represent edge from node representing 
  // (caller, procedure parameter) pair to the real callee for a particular
  // binding of the procedure parameter
  static char temp[80];   // long enough for a procname in Fortran -- JMC 11/92
  sprintf(temp, "%s(%s)@%d", callerName, procParBinding, callsiteId);
  return temp;
}
  
