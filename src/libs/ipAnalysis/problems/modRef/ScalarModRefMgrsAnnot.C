/* $Id: ScalarModRefMgrsAnnot.C,v 1.6 1997/03/11 14:35:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/ipAnalysis/callGraph/CallGraphAnnotMgrs.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>

#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotReg.h>

#include <libs/ipAnalysis/problems/alias/AliasAnnot.h>

#include <libs/ipAnalysis/problems/modRef/ScalarModRefDFProblem.i>
#include <libs/ipAnalysis/problems/modRef/ScalarModRefAnnot.h>

//*************************************************************************
// forward declarations
//*************************************************************************

static void AnnotateCallGraph(CallGraph *cg, ModRefType atype, 
			      char *problem_name);


static Annotation *ModRefCompute (CallGraphEdge *edge, 
				      const char *const nodeAnnotName, 
				      const char *const edgeAnnotName);



//*************************************************************************
// class ScalarGMODAnnotMgr
//
// manage creation and computation of instances of GMOD annotations 
// for CallGraphNodes
//*************************************************************************

char *SCALAR_GMOD_ANNOT = "GMOD";

class ScalarGMODAnnotMgr : public CallGraphNodeAnnotMgr {
public:
  virtual Annotation *New();
  virtual Annotation *Compute(CallGraphNode *node);
};


REGISTER_CG_ANNOT_MGR(SCALAR_GMOD_ANNOT, ScalarGMODAnnotMgr, 
		      CallGraphNode::annotMgrRegistry);


Annotation *ScalarGMODAnnotMgr::New()
{
  return new ScalarModRefAnnot(SCALAR_GMOD_ANNOT);
}


Annotation *ScalarGMODAnnotMgr::Compute(CallGraphNode *node) 
{
  AnnotateCallGraph(node->GetCallGraph(), 
		    MODREFTYPE_MOD, SCALAR_GMOD_ANNOT);
  return node->GetAnnotation(SCALAR_GMOD_ANNOT);
}


//*************************************************************************
// class ScalarGREFAnnotMgr
//
// manage creation and computation of instances of GREF annotations 
// for CallGraphNodes
//*************************************************************************

char *SCALAR_GREF_ANNOT = "GREF";

class ScalarGREFAnnotMgr : public CallGraphNodeAnnotMgr {
public:
  virtual Annotation *New();
  virtual Annotation *Compute(CallGraphNode *node);
};


REGISTER_CG_ANNOT_MGR(SCALAR_GREF_ANNOT, ScalarGREFAnnotMgr, 
		      CallGraphNode::annotMgrRegistry);


Annotation *ScalarGREFAnnotMgr::New()
{
  return new ScalarModRefAnnot(SCALAR_GREF_ANNOT);
}


Annotation *ScalarGREFAnnotMgr::Compute(CallGraphNode *node) 
{
  AnnotateCallGraph(node->GetCallGraph(), 
		    MODREFTYPE_REF, SCALAR_GREF_ANNOT);
  return node->GetAnnotation(SCALAR_GREF_ANNOT);
}



//*************************************************************************
// class ScalarREFAnnotMgr
//
// manage creation and computation of instances of GREF annotations 
// for CallGraphEdges
//*************************************************************************

char *SCALAR_REF_ANNOT = "REF";

class ScalarREFAnnotMgr : public CallGraphEdgeAnnotMgr {
public:
  virtual Annotation *New();
  virtual Annotation *Compute(CallGraphEdge *node);
};


REGISTER_CG_ANNOT_MGR(SCALAR_REF_ANNOT, ScalarREFAnnotMgr, 
		      CallGraphEdge::annotMgrRegistry);


Annotation *ScalarREFAnnotMgr::New()
{
  return new ScalarModRefAnnot(SCALAR_REF_ANNOT);
}


Annotation *ScalarREFAnnotMgr::Compute(CallGraphEdge *edge) 
{
  return ModRefCompute(edge, SCALAR_GREF_ANNOT, SCALAR_REF_ANNOT);
}



//*************************************************************************
// class ScalarMODAnnotMgr
//
// manage creation and computation of instances of GREF annotations 
// for CallGraphEdges
//*************************************************************************

char *SCALAR_MOD_ANNOT = "MOD";

class ScalarMODAnnotMgr : public CallGraphEdgeAnnotMgr {
public:
  virtual Annotation *New();
  virtual Annotation *Compute(CallGraphEdge *node);
};


REGISTER_CG_ANNOT_MGR(SCALAR_MOD_ANNOT, ScalarMODAnnotMgr, 
		      CallGraphEdge::annotMgrRegistry);


Annotation *ScalarMODAnnotMgr::New()
{
  return new ScalarModRefAnnot(SCALAR_MOD_ANNOT);
}


Annotation *ScalarMODAnnotMgr::Compute(CallGraphEdge *edge) 
{
  return ModRefCompute(edge, SCALAR_GMOD_ANNOT, SCALAR_MOD_ANNOT);
}



//************************************************************************
// private operations
//************************************************************************


//-------------------------------------------------------------------------
// currently support batch demand driven computation of annotations
// for nodes and edges in a callgraph
//-------------------------------------------------------------------------
static void AnnotateCallGraph(CallGraph *cg, ModRefType atype, 
			      char *problem_name)
{   
  ScalarModRefDFProblem dfp(cg, atype, problem_name);
  dfp.Solve(cg);
}


static Annotation *ModRefCompute (CallGraphEdge *edge, 
				      const char *const nodeAnnotName, 
				      const char *const edgeAnnotName)
{
  // get the GMOD/GREF info for the callee
  ScalarModRefAnnot *nodeAnnot = (ScalarModRefAnnot *)
    (edge->Callee())->GetAnnotation(nodeAnnotName, true);

  // create a temporary copy of GMOD/GREF info
  ScalarModRefAnnot mrtmp(*nodeAnnot, edgeAnnotName);

  // get the ALIAS info for edge
  AliasAnnot *aliases = 
    (AliasAnnot *) edge->GetAnnotation(ALIAS_ANNOT, true);

  // augment the GMOD/GREF info with aliases for edge 
  mrtmp.AugmentWithAliases(aliases);
  
  // translate the annotation from the callee to the caller name space
  ScalarModRefAnnot *edgeAnnot = new ScalarModRefAnnot(edgeAnnotName);
  edgeAnnot->AugmentCallerAnnotFromCalleeAnnot(edge, &mrtmp);

  // add the resulting annotation to the edge in the call graph
  edge->PutAnnotation(edgeAnnot);

  // return the MOD/REF annotation computed for the edge
  return edgeAnnot;
}
