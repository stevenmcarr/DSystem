/* $Id: FortDLocalAnnot.C,v 1.2 1997/03/27 20:41:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Id */
#include <stdarg.h>
#include <stdlib.h>


#include <libs/fortD/misc/FortD.h>

#include <libs/ipAnalysis/ipInfo/ProcFortDInfo.h>

#include <libs/support/strings/OrderedSetOfStrings.h>

#include <libs/fileAttrMgmt/fortranModule/FortDModAttr.h>

#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotReg.h>
#include <libs/ipAnalysis/callGraph/CallGraphIterators.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeSet.h>
#include <libs/ipAnalysis/callGraph/ModuleInfoIterator.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotMgrs.h>

#include <libs/ipAnalysis/problems/fortD/FortDLocalAnnot.h>

class HashTable;

char *FORTD_LOCAL_ANNOT = "FortDLocalAnnot";

//*****************************************************************************
// class  FortDLocalAnnotMgr
//*****************************************************************************

class FortDLocalAnnotMgr : public CallGraphNodeAnnotMgr {
public:
  Annotation *New();
  Annotation *Compute(CallGraphNode *node);
};


Annotation *FortDLocalAnnotMgr::New()
{
  return new FortDLocalAnnot();
}


Annotation *FortDLocalAnnotMgr::Compute(CallGraphNode *node)
{
  CallGraph *cg = ((CallGraphNode *) node)->GetCallGraph();
  CallGraphNodeIterator nodeSet(cg, Unordered);
  CallGraphNodeSet hasFortDLocalInfo;

  ModuleInfoIterator modules(cg->program, CLASS_NAME(FortDModAttr));
  for (; modules.module; modules.Advance(false)) {
    FortDModAttr *fdAttr = (FortDModAttr *) modules.moduleInfo;

    //-----------------------------------------------------------------------
    // for each procedure defined in the module ...
    //-----------------------------------------------------------------------
    ModuleLocalInfoIterator summaries(fdAttr);
    for(ProcFortDInfo *summary; summary = (ProcFortDInfo *)summaries.Current(); 
        ++summaries) {
      CallGraphNode *aNode = cg->LookupNode(summary->name);
      FortDLocalAnnot *fda = new FortDLocalAnnot(summary->tree);
#if 0
      // we leave the date intact in summary->tree as well; thus we need
      // to make sure we don not both delete it. we delete things here,
      // so we skip the destruct in ProcFortDInfo. hack hack ...
      // THIS ALL NEEDS TO BE REWRITTEN -- JMC 6/94
      summary->tree = 0;	
#endif
      aNode->PutAnnotation(fda);
      hasFortDLocalInfo.Add(aNode);
    }
  }

  //--------------------------------------------------------------------
  // perform node initialization for nodes that have no fortD local info
  //--------------------------------------------------------------------
  for (CallGraphNode* aNode; aNode = nodeSet.Current(); ++nodeSet) {
    if (hasFortDLocalInfo.IsMember(aNode)) {
      continue;
    }
    else {
      IPinfoTreeNode *nullNode;
      IPinfoTree *nullTree;
      FortDLocalAnnot *nullAnnot;

      nullNode = new IPinfoTreeNode();
      nullNode->fd = (HashTable*) new FortranDInfo();
      nullTree = new IPinfoTree(nullNode);
      nullAnnot = new FortDLocalAnnot(nullTree);
      aNode->PutAnnotation(nullAnnot);
    }
  }
  nodeSet.Reset(Unordered);

  return node->GetAnnotation(FORTD_LOCAL_ANNOT);
}


REGISTER_CG_ANNOT_MGR(FORTD_LOCAL_ANNOT, FortDLocalAnnotMgr, 
		      CallGraphNode::annotMgrRegistry);


//*****************************************************************************
// class  FortDLocalAnnot: 
//*****************************************************************************



FortDLocalAnnot::FortDLocalAnnot(IPinfoTree *_tree) : 
	Annotation(FORTD_LOCAL_ANNOT), tree(_tree)
{
}


FortDLocalAnnot::~FortDLocalAnnot()
{
  delete (IPinfoTree *) tree;
}


int FortDLocalAnnot::ReadUpCall(FormattedFile *)
{
  return 1;
}


int FortDLocalAnnot::WriteUpCall(FormattedFile *)
{
  return 1;
}


OrderedSetOfStrings *
FortDLocalAnnot::CreateOrderedSetOfStrings()
{
  // no printable representation
  return new OrderedSetOfStrings;
}
