/* $Id: CommonBlockMgrsAnnot.C,v 1.1 1997/03/11 14:34:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//****************************************************************************
// CommonBlockAnnotMgrs.C
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

#include <libs/ipAnalysis/problems/fortD/CommonBlockAnnot.h>

#if 0
#include <libs/ipanalysis/include/CommonBlockDFProblem.h>
#endif

//*****************************************************************************
// declarations
//*****************************************************************************

char *FORTD_COMMON_BLOCK_ANNOT = "fortran D common block";

#if 0
static void AnnotateCallGraph(CallGraph *cg);
#endif


//*****************************************************************************
// class CommonBlockAnnotNodeMgr implementation 
//
// create and compute instances of class CommonBlockAnnot for a CallGraphNode
//*****************************************************************************

class CommonBlockAnnotNodeMgr: public CallGraphNodeAnnotMgr {
public:
  virtual Annotation *New();
  virtual Annotation *Compute(CallGraphNode *node);
};


Annotation *CommonBlockAnnotNodeMgr::New()
{
  return new FD_CommBlkNodeAnnotation();
}


Annotation *CommonBlockAnnotNodeMgr::Compute(CallGraphNode *node)
{
  CallGraphNode *cgnode = (CallGraphNode *) node;
#if 0
  AnnotateCallGraph(cgnode->GetCallGraph());
#endif
  return cgnode->GetAnnotation(FORTD_COMMON_BLOCK_ANNOT);
}


REGISTER_CG_ANNOT_MGR(FORTD_COMMON_BLOCK_ANNOT, CommonBlockAnnotNodeMgr, 
		      CallGraphNode::annotMgrRegistry);



#if 0
//*****************************************************************************
// class CommonBlockAnnotEdgeMgr implementation 
//
// create and compute instances of class CommonBlockAnnot for a CallGraphEdge
//*****************************************************************************

class CommonBlockAnnotEdgeMgr: public CallGraphEdgeAnnotMgr {
public:
  virtual Annotation *New();
  virtual Annotation *Compute(CallGraphEdge *edge);
};


Annotation *CommonBlockAnnotEdgeMgr::New()
{
  return new FD_CommBlkNodeAnnotation();
}


Annotation *CommonBlockAnnotEdgeMgr::Compute(CallGraphEdge *edge) 
{
  CallGraphEdge *cgedge = (CallGraphEdge *) edge;
#if 0
  AnnotateCallGraph(cgedge->GetCallGraph());
#endif
  return cgedge->GetAnnotation(FORTD_COMMON_BLOCK_ANNOT);
}


REGISTER_CG_ANNOT_MGR(FORTD_COMMON_BLOCK_ANNOT, CommonBlockAnnotEdgeMgr, 
		      CallGraphEdge::annotMgrRegistry);

#endif

#if 0


//****************************************************************************
// private operations
//****************************************************************************


//----------------------------------------------------------------------------
// currently support batch demand driven computation of annotations
// for nodes and edges in a callgraph
//----------------------------------------------------------------------------
static void AnnotateCallGraph(CallGraph *cg)
{   
  FortD_CommonBlock_DFProblem dfp;
  cg->SolveDFProblem(&dfp);
}

#endif
