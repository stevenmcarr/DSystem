/* $Id: AliasAnnotMgrs.C,v 1.1 1997/03/11 14:34:53 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//****************************************************************************
// AliasAnnotMgrs.C
//
//
// Author: John Mellor-Crummey                                June 1994
//
// Copyright 1994, Rice University
//****************************************************************************



#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotMgrs.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotReg.h>

#include <libs/ipAnalysis/problems/alias/AliasAnnot.h>
#include <libs/ipAnalysis/problems/alias/AliasDFProblem.i>

//*****************************************************************************
// declarations
//*****************************************************************************

char *ALIAS_ANNOT = "ALIAS";

static void AnnotateCallGraph(CallGraph *cg);


//*****************************************************************************
// class AliasAnnotNodeMgr implementation 
//
// create and compute instances of class AliasAnnot for a CallGraphNode
//*****************************************************************************

class AliasAnnotNodeMgr: public CallGraphNodeAnnotMgr {
public:
  virtual Annotation *New();
  virtual Annotation *Compute(CallGraphNode *node);
};


Annotation *AliasAnnotNodeMgr::New()
{
  return new AliasAnnot();
}


Annotation *AliasAnnotNodeMgr::Compute(CallGraphNode *node)
{
  CallGraphNode *cgnode = (CallGraphNode *) node;
  AnnotateCallGraph(cgnode->GetCallGraph());
  return cgnode->GetAnnotation(ALIAS_ANNOT);
}


REGISTER_CG_ANNOT_MGR(ALIAS_ANNOT, AliasAnnotNodeMgr, 
		      CallGraphNode::annotMgrRegistry);



//*****************************************************************************
// class AliasAnnotNodeMgr implementation 
//
// create and compute instances of class AliasAnnot for a CallGraphEdge
//*****************************************************************************

class AliasAnnotEdgeMgr: public CallGraphEdgeAnnotMgr {
public:
  virtual Annotation *New();
  virtual Annotation *Compute(CallGraphEdge *edge);
};


Annotation *AliasAnnotEdgeMgr::New()
{
  return new AliasAnnot();
}


Annotation *AliasAnnotEdgeMgr::Compute(CallGraphEdge *edge) 
{
  CallGraphEdge *cgedge = (CallGraphEdge *) edge;
  AnnotateCallGraph(cgedge->GetCallGraph());
  return cgedge->GetAnnotation(ALIAS_ANNOT);
}


REGISTER_CG_ANNOT_MGR(ALIAS_ANNOT, AliasAnnotEdgeMgr, 
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
  AliasDFProblem dfp;
  dfp.Solve(cg);
}

