/* $Id: OverlapAnnotMgrs.C,v 1.1 1997/03/11 14:35:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotReg.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotMgrs.h>

#include <libs/ipAnalysis/problems/fortD/OverlapAnnot.h>
#include <libs/ipAnalysis/problems/fortD/OverlapForwardDFProblem.h>
#include <libs/ipAnalysis/problems/fortD/OverlapBackwardDFProblem.h>


char *FORTD_OVERLAP_ANNOT = "fortran D overlap";
char *IFORTD_OVERLAP_ANNOT = "I fortran D overlap";

class FortDOverlapNodeAnnotMgr : public CallGraphNodeAnnotMgr {
  Annotation *New(); 
  Annotation *Compute(CallGraphNode *node);
};


REGISTER_CG_ANNOT_MGR(FORTD_OVERLAP_ANNOT, FortDOverlapNodeAnnotMgr, 
		      CallGraphNode::annotMgrRegistry);

//------------------------------------------------------------------

Annotation* FortDOverlapNodeAnnotMgr::New()
{
 return new FD_Overlap_Annot(); 
}


//------------------------------------------------------------------
static void AnnotateCallGraph(CallGraph *cg)
{
 FortD_OverlapBackward_DFProblem dfp1;                                 
 dfp1.Solve(cg);

 FortD_OverlapForward_DFProblem dfp;
 dfp.Solve(cg);
}

//------------------------------------------------------------------
Annotation* FortDOverlapNodeAnnotMgr::Compute(CallGraphNode *node)
{
  CallGraphNode *cgnode = (CallGraphNode *) node;
  AnnotateCallGraph(cgnode->GetCallGraph());
  return(cgnode->GetAnnotation(FORTD_OVERLAP_ANNOT));
}

#if 0
//------------------------------------------------------------------
Annotation* FortDOverlapNodeAnnotMgr::DemandAnnotation(CallGraphEdge *edge)
{
  AnnotateCallGraph(edge->GetCallGraph());
  return(edge->GetAnnotation(FORTD_OVERLAP_ANNOT));
}
#endif

