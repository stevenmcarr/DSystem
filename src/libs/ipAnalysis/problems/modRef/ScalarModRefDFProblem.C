/* $Id: ScalarModRefDFProblem.C,v 1.4 1997/03/11 14:35:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*****************************************************************************
//
//  ScalarModRefDFProblem.C
//
//    Scalar Mod/Ref summary data flow problem (MOD and REF) definitions.
//       creates MOD (REF) annotations for nodes that represent GMOD (GREF)
//       and MOD (REF) annotations for edges
//
//  Author:
//    John Mellor-Crummey                                      January 1993
//    (based on an earlier implementation by Mary Hall and Rene Rodriquez)
//  
//    Copyright 1993, Rice University, as part of the ParaScope Programming
//    Environment Project
//*****************************************************************************

#include <stdlib.h>
#include <assert.h>

#include <libs/ipAnalysis/problems/alias/AliasAnnot.h>

// call graph driver external interface
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphIterators.h>
#include <libs/ipAnalysis/callGraph/CallSiteParamBindings.h>

// local information external interface
#include <libs/fileAttrMgmt/fortranModule/ScalarModRefModAttr.h>
#include <libs/ipAnalysis/ipInfo/ProcScalarModRefInfo.h>
#include <libs/ipAnalysis/ipInfo/OffsetLength.h>

#include <libs/ipAnalysis/problems/modRef/ScalarModRefDFProblem.i>
#include <libs/ipAnalysis/problems/modRef/ScalarModRefAnnot.h>


// forward declarations

static void AugmentAnnotWithModRefInfo(ModRefType which, 
				       ScalarModRefAnnot *annot, 
				       ProcScalarModRefInfo *mrinfo);

static void MapNodeAnnotToInEdges(CallGraphNode *node,
				  ScalarModRefAnnot *annot);

static void AugmentIncomingEdgeAnnot(ScalarModRefAnnot *annot, 
				     CallGraphEdge *edge);

//=============================================================================
// ScalarModRefDFProblem::ScalarModRefDFProblem
//    
//    constructor for an instance of the Scalar MOD/REF dataflow problem
//=============================================================================
ScalarModRefDFProblem::ScalarModRefDFProblem
(CallGraph *callgraph, ModRefType whichType, char *aname) :
CallGraphDFProblem(CLASS_NAME(ScalarModRefModAttr))
{  
  cg = callgraph;

  // problem dependent initialization
  this->which = whichType;
  annot_name = aname;

  top = new ScalarModRefAnnot(annot_name);
  bottom = new ScalarModRefAnnot(annot_name);
  direction = Backward;
}


//=============================================================================
// ScalarModRefDFProblem::InitializeNode
//
//    compute the initial IMOD/IREF information for a node using initial 
//    information collected in a local analysis phase 
//=============================================================================
void ScalarModRefDFProblem::InitializeNode(CallGraphNode *node, 
					   ProcLocalInfo *mrinfo) 
{
  // initialize node annotation GMOD/GREF 
  ScalarModRefAnnot *initA = new ScalarModRefAnnot(annot_name);	
  node->PutAnnotation(initA);
  
  // augment with local information, if any 
  if (mrinfo) {
    AugmentAnnotWithModRefInfo(which, initA, (ProcScalarModRefInfo *) mrinfo);
  }
}


//=============================================================================
// ScalarModRefDFProblem::InitializeEdge
//
//    initialize MOD/REF annotation at each callsite to an empty set; this will
//    be augmented as appropriate during dataflow analysis
//=============================================================================
void ScalarModRefDFProblem::InitializeEdge(CallGraphEdge *edge) 
{
  edge->PutAnnotation(new ScalarModRefAnnot(annot_name));
}


//=============================================================================
// ScalarModRefDFProblem::FinalizeAnnotation
//
//     add alias information to a GMOD/GREF annotation on a node and the 
//     MOD/REF information on incoming edges
//=============================================================================
void ScalarModRefDFProblem::FinalizeAnnotation(CallGraphNode *node, 
					       FlowGraphDFAnnot *annot)
{
  ScalarModRefAnnot *smr_annot = (ScalarModRefAnnot *) annot;
  
  //----------------------------------------------------------------------
  // store the GMOD/GREF annotation for the current node
  //----------------------------------------------------------------------
  node->PutAnnotation(smr_annot);

  //----------------------------------------------------------------------
  // delete GMOD/GREF annotations temporarily on edges 
  //----------------------------------------------------------------------
  CallGraphEdgeIterator edges(node, DirectedEdgeIn);
  for (CallGraphEdge *edge; edge = edges.Current(); edges++) {
    edge->DeleteAnnotation(smr_annot->name);
  }
}


//********************************************************************
// DATA FLOW ANALYSIS FUNCTIONS                                       
//********************************************************************


//=============================================================================
// void *ScalarModRefDFProblem::DFtrans
//   
//    augment the union of incoming MOD/REF information from callees with 
//    initial information to compute GMOD/GREF for the current node. if the 
//    newly computed GMOD/GREF differs from the previously computed value, use
//    the new GMOD/GREF information to update the MOD/REF annotation on each 
//    each incoming callsite edge by translating the annotation using through 
//    the parameter bindings at the callsite
//=============================================================================
void *ScalarModRefDFProblem::DFtrans(void *new_in_annot_v, 
				      void *old_out_annot_v, 
				     void *self_node_v, 
				      unsigned char &changed)
{
  CallGraphNode *self_node = (CallGraphNode *) self_node_v;
  ScalarModRefAnnot *new_in_annot = (ScalarModRefAnnot *) new_in_annot_v;
  ScalarModRefAnnot *old_out_annot = (ScalarModRefAnnot *) old_out_annot_v;

  //-----------------------------------------------------------------------
  // union initial information with result of the meet to compute the new
  // GMOD/GREF annotation for the node
  //-----------------------------------------------------------------------
  *new_in_annot |= *((ScalarModRefAnnot *) 
		     self_node->GetAnnotation(annot_name)); 
  
  if(*old_out_annot != *new_in_annot) {
    //-----------------------------------------------------------------------
    // set changed flag so the dataflow solver continues to iterate
    //-----------------------------------------------------------------------
    changed = 1; 

    //-----------------------------------------------------------------------
    // map the annotation to the incoming edges
    //-----------------------------------------------------------------------
    MapNodeAnnotToInEdges(self_node, new_in_annot);
  }
  
  return new_in_annot;
}


//=============================================================================
// void *ScalarModRefDFProblem::DFmeet
// 
//      for a node, compute union of incoming MOD/REF information from all 
//      callees
//=============================================================================
void *ScalarModRefDFProblem::DFmeet(void *partial_result_annot_v, 
				     void *pred_node_annot_v, 
				    void *self_node_v, 
				     void *edge_v)
{
  CallGraphEdge *edge = (CallGraphEdge *) edge_v;
  CallGraphNode *node = (CallGraphNode *) self_node_v;
  
  ScalarModRefAnnot *partial_result_annot = 
    (ScalarModRefAnnot *) partial_result_annot_v;  
  ScalarModRefAnnot *pred_node_annot = (ScalarModRefAnnot *) pred_node_annot_v;
  
  //--------------------------------------------------------------------
  // if the meet involves information from a callsite edge
  //--------------------------------------------------------------------
  if (edge) {
    ScalarModRefAnnot *edge_annot = 
      (ScalarModRefAnnot *) edge->GetAnnotation(annot_name);

    //----------------------------------------------------------------------
    // augment node GMOD/GREF with MOD/REF for the callsite edge -- JMC 1/93
    //----------------------------------------------------------------------
    partial_result_annot->formals |= edge_annot->formals; 
    partial_result_annot->globals |= edge_annot->globals; 
  }
  
  return partial_result_annot;  
}


//********************************************************************
// local functions
//********************************************************************


static void AugmentAnnotWithModRefInfo
(ModRefType which, ScalarModRefAnnot *annot, ProcScalarModRefInfo *mrinfo)
{
  ProcScalarModRefInfoIterator eqclasses(mrinfo);
  
  //----------------------------------------------------------------
  // for each equivalence class
  //----------------------------------------------------------------
  EqClassScalarModRefInfo *eqclass;
  for (; eqclass = eqclasses.Current(); eqclasses++) {
    OffsetLengthPairVector *olpairs = &eqclass->pairs[which];
    unsigned int npairs = olpairs->NumberOfEntries();
    
    if (npairs == 0) continue;
    
    if (eqclass->type  & VTYPE_FORMAL_PARAMETER) {
      annot->formals.Add(eqclass->name);
    } else if (eqclass->type & VTYPE_COMMON_DATA) {  
      EqClassPairs *mypairs = annot->globals.GetEntry(eqclass->name);
      
      for (unsigned int i = 0; i < npairs; i++) {
	mypairs->AddPair(new OffsetLengthPair(*olpairs->GetPair(i)));
      }
    } else {
      // ignore locals; they do not matter -- JMC 2/94
    }
  }
}


static void MapNodeAnnotToInEdges(CallGraphNode *node,
				  ScalarModRefAnnot *annot)
{
  //-----------------------------------------------------------------------
  // for each incoming callsite edge, push the GMOD/GREF info at this 
  // node through the callsite by augmenting the MOD/REF edge annotation 
  //-----------------------------------------------------------------------
  CallGraphEdgeIterator edges(node, DirectedEdgeIn);
  CallGraphEdge *edge;
  for(; edge = edges.Current(); edges++) AugmentIncomingEdgeAnnot(annot, edge);
}


static void AugmentIncomingEdgeAnnot(ScalarModRefAnnot *annot, 
				     CallGraphEdge *edge)
{
  ScalarModRefAnnot *edgeAnnot = 
    (ScalarModRefAnnot *) edge->GetAnnotation((char *) annot->name);
  edgeAnnot->AugmentCallerAnnotFromCalleeAnnot(edge, annot);
}

