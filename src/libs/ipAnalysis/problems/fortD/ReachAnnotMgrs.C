/* $Id: ReachAnnotMgrs.C,v 1.1 1997/03/11 14:35:03 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//****************************************************************************
// ReachAnnotMgrs.C
//
//
// Author: John Mellor-Crummey                                June 1994
//
// Copyright 1994, Rice University
//****************************************************************************


#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotReg.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotMgrs.h>

#include <libs/ipAnalysis/problems/fortD/ReachAnnot.h>
#include <libs/ipAnalysis/problems/fortD/ReachDFProblem.h>

//*****************************************************************************
// declarations
//*****************************************************************************

char *FORTD_REACH_ANNOT = "Fort D Reach Decomp";

static void AnnotateCallGraph(CallGraph *cg);


//*****************************************************************************
// class ReachAnnotNodeMgr implementation 
//
// create and compute instances of class ReachAnnot for a CallGraphNode
//*****************************************************************************

class ReachAnnotNodeMgr: public CallGraphNodeAnnotMgr {
public:
  virtual Annotation *New();
  virtual Annotation *Compute(CallGraphNode *node);
};


Annotation *ReachAnnotNodeMgr::New()
{
  return new FD_Reach_Annot();
}


Annotation *ReachAnnotNodeMgr::Compute(CallGraphNode *node)
{
  CallGraphNode *cgnode = (CallGraphNode *) node;
  AnnotateCallGraph(cgnode->GetCallGraph());
  return cgnode->GetAnnotation(FORTD_REACH_ANNOT);
}


REGISTER_CG_ANNOT_MGR(FORTD_REACH_ANNOT, ReachAnnotNodeMgr, 
		      CallGraphNode::annotMgrRegistry);



//*****************************************************************************
// class ReachAnnotNodeMgr implementation 
//
// create and compute instances of class ReachAnnot for a CallGraphEdge
//*****************************************************************************

class ReachAnnotEdgeMgr: public CallGraphEdgeAnnotMgr {
public:
  virtual Annotation *New();
  virtual Annotation *Compute(CallGraphEdge *edge);
};


Annotation *ReachAnnotEdgeMgr::New()
{
  return new FD_Reach_Annot();
}


Annotation *ReachAnnotEdgeMgr::Compute(CallGraphEdge *edge) 
{
  CallGraphEdge *cgedge = (CallGraphEdge *) edge;
  AnnotateCallGraph(cgedge->GetCallGraph());
  return cgedge->GetAnnotation(FORTD_REACH_ANNOT);
}


REGISTER_CG_ANNOT_MGR(FORTD_REACH_ANNOT, ReachAnnotEdgeMgr, 
		      CallGraphEdge::annotMgrRegistry);




//****************************************************************************
// private operations
//****************************************************************************


//----------------------------------------------------------------------------
// currently support batch demand driven computation of annotations
// for nodes and edges in a callgraph
//----------------------------------------------------------------------------
static void AnnotateCallGraph(CallGraph *cg)
{   
  FortD_Reach_DFProblem dfp;
  dfp.Solve(cg);
}
