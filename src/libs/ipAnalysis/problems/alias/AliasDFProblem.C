/* $Id: AliasDFProblem.C,v 1.2 1997/03/27 20:40:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
//    AliasDFProblem.C
//
//    definition of the alias interprocedural data flow problem. the 
//    side-effect of performing alias analysis is that each node and
//    edge in the callgraph is annotated with an AliasAnnot that
//    contains pairs of aliases.
//
//    Author: John Mellor-Crummey
//
//    Copyright 1994, Rice University
//***************************************************************************

#include <stdio.h>

#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphIterators.h>

#include <libs/ipAnalysis/problems/alias/AliasDFProblem.i>
#include <libs/ipAnalysis/problems/alias/AliasAnnot.h>



//***************************************************************************
// class AliasDFProblem interface operations
//***************************************************************************

AliasDFProblem::AliasDFProblem() : 
CallGraphFlowInsensitiveDFP(0, newAliasTop, DirectedEdgeOut) 
{
}


DataFlowSet *AliasDFProblem::newAliasTop()
{
  return new AliasAnnot();
}

//************************************
// initialization
//************************************

DataFlowSet *AliasDFProblem::InitializeEdge
(CallGraphEdge *edge, CallSiteLocalInfo *) 
{
  CallGraphNode *caller = edge->Caller();
  AliasAnnot *edgeAnnot = new AliasAnnot();
  
  //----------------------------------------------------------------
  // look at reverse bindings on edge and record an alias for every 
  // global that is bound to a formal
  //----------------------------------------------------------------
  for (ParamNameIterator fni(edge->paramBindings, FormalNameSet); 
       fni.Current(); ++fni) {
    const char *formal = fni.Current();
    ParamBinding *binding = edge->paramBindings.GetReverseBinding(formal);
    if (binding->a_class & APC_DataGlobal) {  // actual is a global
      edgeAnnot->Add(formal, binding->actual, binding->a_offset, 
		      binding->a_length);
    }
  }
  
  //----------------------------------------------------------------
  // for each actual in the bindings map, add aliases induced when
  // same actual (or a pair of potentially aliased actuals) are 
  // passed
  //----------------------------------------------------------------
  for (ParamNameIterator ani(edge->paramBindings, ActualNameSet); 
       ani.Current(); ++ani) {
    const char *actual = ani.Current();
    ParamBindingsSet *bindings = 
      edge->paramBindings.GetForwardBindings(actual);
    ParamBindingsSetIterator bi(bindings);
    
    //----------------------------------------------------------------
    // dont propagate aliases for procedure constants -- JMC 1/93
    // N.B. a_class is the same for all elements in the ParamBindingSet
    //----------------------------------------------------------------
    if (bi.Current()->a_class & APC_ProcConstant) continue;
    
    ParamBinding *formal, *formal2;
    
    // for bindings 2, ..., n 
    for(++bi; formal = bi.Current(); ++bi) {
      
      // for bindings 1, ..., formal
      for(ParamBindingsSetIterator bi2(bindings); 
	  (formal2 = bi2.Current()) != formal; ++bi2) {
	
	// formal if in formal dictionary and also in formal list 
	if (caller->IsFormal(actual)) {
	  // same actual (formal in caller) passed to 2 formals:
	  // formals are alias pair 
#if DEBUG
	  fprintf(stderr, "ALIAS: <%s, %s >\n", formal->formal, 
		  formal2->formal); 
#endif
	  edgeAnnot->Add(formal->formal, formal2->formal);
	} else { 
	  // compare a_offset and a_length each time actual passed to formals 
	  if( ((formal->a_offset <= formal2->a_offset) &&
	       (formal->a_offset + formal->a_length > formal2->a_offset)) ||
	      ((formal2->a_offset <= formal->a_offset) &&
	       (formal2->a_offset + formal2->a_length > formal->a_offset))){
	    //----------------------------------------------------------------
	    // overlappping globals or locals passed to 2 formals forming 
	    // an alias
	    //----------------------------------------------------------------
#if DEBUG
	    fprintf(stderr, "ALIAS: <%s, %s >\n", formal->formal, 
		    formal2->formal); 
#endif
	    edgeAnnot->Add(formal->formal, formal2->formal); 
	  }
	}
      }
    }
  }
  return edgeAnnot;
}


//************************************
// data-flow solver upcalls
//************************************

DataFlowSet *AliasDFProblem::Meet(CallGraphEdge *, CallGraphNode *, 
				  const DataFlowSet *nodeIn, 
				  DataFlowSet *meetPartialResult)
{
  *meetPartialResult |= *nodeIn;
  return meetPartialResult;
}


DataFlowSet *AliasDFProblem::NodeToEdge
(CallGraphNode *node, CallGraphEdge *edge, const DataFlowSet *nodeOut)
{
  AliasAnnot *edgeIn = new AliasAnnot();
  ParamBinding *formal, *formal2;
  
  //---------------------------------------------------------------------
  // Walk formal-formal pairs adding aliases to edge if both are passed 
  //---------------------------------------------------------------------
  FormalAliasesIterator formalAliasSets((AliasAnnot *) nodeOut);
  FormalAliases *fa;
  for( ; fa = formalAliasSets.Current(); ++formalAliasSets) {
    StringSetIterator strings(fa);
    const char *name2;
    for( ; name2 = strings.Current(); ++strings) {
      ParamBindingsSet *formal1_bindings = 
	edge->paramBindings.GetForwardBindings(fa->name);
      ParamBindingsSet *formal2_bindings = 
	edge->paramBindings.GetForwardBindings(name2);
      
      // add all pairs introduced by this alias 
      for(ParamBindingsSetIterator bi1(formal1_bindings); 
	  formal = bi1.Current(); ++bi1) {
	for(ParamBindingsSetIterator bi2(formal2_bindings); 
	    formal2 = bi2.Current(); ++bi2) {
	  edgeIn->Add(formal->formal, formal2->formal);
	}
      }
    }
  }
  
  //---------------------------------------------------------------------
  // Walk global-formal pairs adding aliases to edge as needed 
  //---------------------------------------------------------------------
  GlobalAliasesIterator globalAliasSets((AliasAnnot *) nodeOut);
  GlobalAliases *ga;
  for( ; ga = globalAliasSets.Current(); ++globalAliasSets) {
    StringSetIterator strings(ga);
    const char *name2;
    for( ; name2 = strings.Current(); ++strings) {
      ParamBindingsSet *formal_bindings = 
	edge->paramBindings.GetForwardBindings(name2);
      ParamBindingsSet *global_bindings = 
	edge->paramBindings.GetForwardBindings(ga->name);
      
      // add a global-formal alias pair to the edge for each binding
      // of the callers formal at the callsite
      
      for(ParamBindingsSetIterator fi(formal_bindings); 
	  formal = fi.Current(); ++fi) {
	edgeIn->Add(formal->formal, ga->name, ga->globalInfo.offset, 
		    ga->globalInfo.length);
	
	// add all pairs introduced by this alias 
	for(ParamBindingsSetIterator gi(global_bindings); 
	    formal2 = gi.Current(); ++gi) {
	  if ((formal2->a_offset == ga->globalInfo.offset))
	    edgeIn->Add(formal2->formal, formal->formal); 
	}
      }
    }
  }
  
  return edgeIn; // return new incoming information for this edge
}


DataFlowSet *AliasDFProblem::AtEdge
(CallGraphEdge *, const DataFlowSet *edgeInit, DataFlowSet *edgeIn)
{
  *edgeIn |= *edgeInit;
  return edgeIn;
}



DataFlowSet *AliasDFProblem::FinalizeNodeAnnotation
(CallGraphNode *, DataFlowSet *annot)
{
  return annot;
}


DataFlowSet *AliasDFProblem::FinalizeEdgeAnnotation
(CallGraphEdge *, DataFlowSet *annot)
{
  return annot;
}

