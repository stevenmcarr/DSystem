/* $Id: AFormalAnnotMgr.C,v 1.1 1997/03/11 14:35:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*****************************************************************************
// AFormalAnnotMgr.C
//
//   a manager class for AFormal annotations 
//
// Author: John Mellor-Crummey                                 September 1994
//
// Copyright 1994, Rice University. All rights reserved.
//*****************************************************************************

#include <stdarg.h>
#include <stdlib.h>

#include <libs/ipAnalysis/callGraph/CallGraphAnnotReg.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotMgrs.h>
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>

#include <libs/ipAnalysis/problems/rsd/AFormalAnnot.h>
#include <libs/ipAnalysis/problems/rsd/AFormalDFProblem.i>


//*****************************************************************************
// class  AFormalAnnotMgr interface operations
//*****************************************************************************

class AFormalAnnotMgr : public CallGraphNodeAnnotMgr {
public:
  Annotation *New();
  Annotation *Compute(CallGraphNode *node);
};


REGISTER_CG_ANNOT_MGR(AFORMAL_ANNOT, AFormalAnnotMgr, 
		      CallGraphNode::annotMgrRegistry);


Annotation *AFormalAnnotMgr::New()
{
  return new AFormalAnnot;
}


Annotation *AFormalAnnotMgr::Compute(CallGraphNode *node)
{
  CallGraphNode *cgnode = (CallGraphNode *) node;
  CallGraph *cg = cgnode->GetCallGraph();

  AFormalDFProblem dfp;
  dfp.Solve(cg);

  return cgnode->GetAnnotation(AFORMAL_ANNOT);
}

