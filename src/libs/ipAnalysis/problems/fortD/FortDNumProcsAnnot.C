/* $Id: FortDNumProcsAnnot.C,v 1.2 1997/03/27 20:41:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*****************************************************************************
// include files
//*****************************************************************************

#include <iostream.h>


#include <libs/fortD/misc/FortD.h>

#include <libs/ipAnalysis/ipInfo/ProcFortDInfo.h>

#include <libs/fileAttrMgmt/fortranModule/FortDModAttr.h>

#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotMgrs.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotReg.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/ModuleInfoIterator.h>

#include <libs/ipAnalysis/problems/fortD/FortDLocalAnnot.h>

#include <libs/ipAnalysis/problems/fortD/FortDNumProcsAnnot.h>

//*****************************************************************************
// external data
//*****************************************************************************

char* FORTD_NUM_PROCS_ANNOT = "FortDNumProcsAnnot";

//*****************************************************************************
// class  FortDNumProcsAnnotMgr
//*****************************************************************************

class FortDNumProcsAnnotMgr : public CallGraphAnnotMgr 
{
  public:
    Annotation* New();
    Annotation* Compute(CallGraph* graph);
};

Annotation* FortDNumProcsAnnotMgr::New()
{
  return new FortDNumProcsAnnot();
}

Annotation* FortDNumProcsAnnotMgr::Compute(CallGraph* graph)
{
  CallGraph* cg = (CallGraph*)graph;
  FortDNumProcsAnnot* numProcsAnnot;
  int numProcs = -1;

  //--------------------------------------------------------------------
  // check all of the procedure nodes of the call graph to see if they
  // define the n$proc parameter.
  //--------------------------------------------------------------------
  ModuleInfoIterator modules(cg->program, CLASS_NAME(FortDModAttr));
  for (; modules.module; modules.Advance(false)) 
  {
     FortDModAttr *fdAttr = (FortDModAttr *) modules.moduleInfo;

     //-----------------------------------------------------------------------
     // for each procedure defined in the module ...
     //-----------------------------------------------------------------------
     ModuleLocalInfoIterator summaries(fdAttr);
     for(ProcFortDInfo *summary; summary = (ProcFortDInfo *)summaries.Current();
         ++summaries) 
     {
        CallGraphNode *aNode = cg->LookupNode(summary->name);
        FortDLocalAnnot *fdla = (FortDLocalAnnot*)aNode->GetAnnotation(FORTD_LOCAL_ANNOT,
                                                                       true);
        FortranDInfo* fdi = (FortranDInfo*)fdla->tree->tree->fd;

        if (fdi->def_numprocs)
        {
           if (numProcs == -1)
           {
              numProcs = fdi->numprocs; 
           }

              // if two procedures have a different value for the n$procs 
              // parameter, emit a warning but keep the previous value.
           if (numProcs != fdi->numprocs)
           {
              cerr << "Warning: Value of parameter n$procs changes in procedure "
                   << summary->name 
                   << endl;
           } 
        }
     }
  }

  numProcsAnnot = new FortDNumProcsAnnot(numProcs);
  cg->PutAnnotation(numProcsAnnot);

  return (Annotation*)numProcsAnnot;
}

REGISTER_CG_ANNOT_MGR(FORTD_NUM_PROCS_ANNOT, FortDNumProcsAnnotMgr, 
                      CallGraph::annotMgrRegistry);

//*****************************************************************************
// class  FortDNumProcsAnnot
//*****************************************************************************

FortDNumProcsAnnot::FortDNumProcsAnnot(int _numberProcs)
  : Annotation(FORTD_NUM_PROCS_ANNOT), numberProcs(_numberProcs)
{
}

FortDNumProcsAnnot::~FortDNumProcsAnnot()
{
}

int FortDNumProcsAnnot::ReadUpCall(FormattedFile*)
{
   return 1;
}

int FortDNumProcsAnnot::WriteUpCall(FormattedFile*)
{
   return 1;
}

OrderedSetOfStrings* FortDNumProcsAnnot::CreateOrderedSetOfStrings()
{
  // no printable representation
  return new OrderedSetOfStrings;
}
