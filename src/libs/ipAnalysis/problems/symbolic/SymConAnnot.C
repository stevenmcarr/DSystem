/* $Id: SymConAnnot.C,v 1.1 1997/03/11 14:35:19 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdarg.h>
#include <stdlib.h>

#include <libs/ipAnalysis/callGraph/CallGraphAnnotReg.h>
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphIterators.h>
// #include <ScalarModRefAnnot.h>
#include <libs/ipAnalysis/problems/symbolic/SymConAnnot.h>
#include <libs/ipAnalysis/problems/symbolic/SymValAnnot.h>
#include <libs/ipAnalysis/problems/symbolic/IpVarList.h>
#include <libs/ipAnalysis/problems/symbolic/SymTrans.h>
// #include <SymComDFProblem.h>
#include <libs/ipAnalysis/ipInfo/iptree.h>

#include <libs/support/strings/OrderedSetOfStrings.h>
#include <libs/support/strings/StringBuffer.h>
#include <libs/moduleAnalysis/valNum/val_pass.h>
#include <libs/moduleAnalysis/valNum/val_ip.h>
#include <libs/moduleAnalysis/valNum/val.i>
#include <libs/ipAnalysis/problems/symbolic/globals_fwd_ht.h>

char *SYMCON_ANNOT = "SymConstraints";

//*****************************************************************************
// class  SymConAnnotMgr: a manager class for Return Symbolic info
//*****************************************************************************

class SymConAnnotMgr : public CallGraphAnnotMgr {
  public:
    //-------------------------------------------------------------------
    // create an instance of the managed annotation 
    //-------------------------------------------------------------------
    virtual CallGraphAnnot *New(CallGraphNode *node); // for a node

    //-------------------------------------------------------------------
    // compute an instance of the managed annotation for (at least) the 
    // specified node 
    //-------------------------------------------------------------------
    virtual CallGraphAnnot *DemandAnnotation(CallGraphNode *node);
};


CallGraphAnnot *SymConAnnotMgr::New(CallGraphNode *node)
{
    return new SymConAnnot(node);
}


CallGraphAnnot *SymConAnnotMgr::DemandAnnotation(CallGraphNode *node)
{
    CallGraph *cg = node->GetCallGraph();

    //  SymComDFProblem dfp(cg);
    //  cg->SolveDFProblem(&dfp);

    //  Do my own dataflow iteration to avoid having to Clone etc.
    //
    CallGraphNodeIterator nodes(cg);

    //////////////////////
    //for (TraversalOrder ord = PreOrder;
    //     ord <= ReversePostOrder;
    //     ord++)
    //{
    //    nodes.Reset(ord);
    //    fprintf(stderr, "Order %d\n", (int)ord);
    //
    //    for (;
    //         nodes.Current();
    //         nodes++)
    //    {
    //        fprintf(stderr, "\t%s\n", nodes.Current()->procName);
    //    }
    //}
    ///////////////////////////
    nodes.Reset(ReversePostOrder);

    for (;
         nodes.Current();
         nodes++)
    {
	nodes.Current()->PutAnnotation(SymFwdMerge(nodes.Current()));
	SymFwdEdge(nodes.Current());
    }

    return node->GetAnnotation(SYMCON_ANNOT);
}


REGISTER_CG_ANNOT_MGR(SYMCON_ANNOT, SymConAnnotMgr);


//*****************************************************************************
// class  SymConAnnot: 
//      node and edge annotations provide information about passed values
//	(edge annots for each call site, node annots merged)
//	(global variables and formal parameters)
//*****************************************************************************


//  struct SymConAnnotS nonexistent, see ConstraintItem

SymConAnnot::SymConAnnot(CallGraphNode *node) : CallGraphAnnot(SYMCON_ANNOT)
{
    //  ScalarModRefAnnot *ref =
    //	    (ScalarModRefAnnot *) node->GetAnnotation(SCALAR_REF_ANNOT, true);
    //  ScalarModRefAnnotIterator refit(ref);

    int vCount = 0;
    char *name;
    int offset, length;

    //  for (refit.Reset();
    //	     name = refit.Current(vscope, offset, length);
    //	     refit++)

    SymValAnnot *valInfo = (SymValAnnot *) 
	node->GetAnnotation(SYMVAL_ANNOT, true);

    for (IpVarListIter scalars(valInfo->refScalars());
	 name = scalars.Current(offset, length);
	 scalars++)
    {
	vCount++;
    }
    if (vCount > 0) 
	hidden = new ConstraintItem[vCount];
    else
	hidden = (ConstraintItem *) 0;
}

SymConAnnot::SymConAnnot(CallGraphEdge *edge) : 
    CallGraphAnnot(SYMCON_ANNOT)
{
    CallGraphNode *node = edge->Callee();

    //  ScalarModRefAnnot *ref =
    //	    (ScalarModRefAnnot *) node->GetAnnotation(SCALAR_REF_ANNOT, true);
    //  ScalarModRefAnnotIterator refit(ref);

    int vCount = 0;
    // VarScope vscope;
    char *name;
    int offset, length;

    //  for (refit.Reset();
    //       name = refit.Current(vscope, offset, length);
    //       refit++)

    SymValAnnot *valInfo = (SymValAnnot *) 
	node->GetAnnotation(SYMVAL_ANNOT, true);

    for (IpVarListIter scalars(valInfo->refScalars());
	 name = scalars.Current(offset, length);
	 scalars++)
    {
	vCount++;
    }
    if (vCount > 0) 
	hidden = new ConstraintItem[vCount];
    else
	hidden = (ConstraintItem *) 0;
}

SymConAnnot::SymConAnnot() : CallGraphAnnot(SYMCON_ANNOT)
{
    hidden = (ConstraintItem *) 0;
}

SymConAnnot::~SymConAnnot()
{
    if (hidden) delete hidden; 
}

CallGraphAnnot *SymConAnnot::Clone()
{
    return NULL;
}

OrderedSetOfStrings *
SymConAnnot::CreateOrderedSetOfStrings(unsigned int width)
{
    OrderedSetOfStrings *oss = new OrderedSetOfStrings;
    StringBuffer string(width);
    // VarScope vscope;
  
    // string.Append("%s:", this->name);
  
    string.Append("SymConAnnot print not implemented yet");

    oss->Append(string.Finalize());
    return oss;
}

// upcall to read an annotation
int SymConAnnot::ReadUpCall(FormattedFile &port)
{
    return 0;
}

// upcall to write an annotation
int SymConAnnot::WriteUpCall(FormattedFile &port)
{
    return 0;
}

void SymConAnnot::Dump(CallGraphNode *node)
{
    //  ScalarModRefAnnot *ref =
    //      (ScalarModRefAnnot *) node->GetAnnotation(SCALAR_REF_ANNOT, true);
    //  ScalarModRefAnnotIterator refit(ref);

    int vCount = 0;
    // VarScope vscope;
    char *name;
    int offset, length;

    printf("Pairwise linear relations for %s\n", node->procName);

    //  for (refit.Reset();
    //       name = refit.Current(vscope, offset, length);
    //       refit++)

    SymValAnnot *valInfo = (SymValAnnot *) 
	node->GetAnnotation(SYMVAL_ANNOT, true);

    for (IpVarListIter scalars(valInfo->refScalars());
	 name = scalars.Current(offset, length);
	 scalars++)
    {
	if (hidden[vCount].parent == UNUSED)
	{
	    if (hidden[vCount].myCoeff == 1)
	    {
		printf("\t(%s @ %d) == %d\n", name, offset, 
		       hidden[vCount].addend);
	    }
	    else
	    {
		printf("\tVar #%d is %s @ %d\n", vCount, name, offset);
	    }
	}
	else
	{
	    printf("\t%d(%s @ %d) + %d(#%d) == %d\n",
		   hidden[vCount].myCoeff, name, offset,
		   hidden[vCount].pCoeff, hidden[vCount].parent,
		   hidden[vCount].addend);
	}
	vCount++;
    }
}
