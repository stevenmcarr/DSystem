/* $Id: SymValAnnot.C,v 1.3 1997/03/11 14:35:22 carr Exp $ */
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
#include <libs/ipAnalysis/problems/symbolic/SymValAnnot.h>
#include <libs/ipAnalysis/problems/symbolic/SymTrans.h>
// #include <SymValDFProblem.h>
#include <libs/ipAnalysis/ipInfo/iptree.h>

#include <libs/support/strings/OrderedSetOfStrings.h>
#include <libs/support/strings/StringBuffer.h>
#include <libs/moduleAnalysis/valNum/val_pass.h>
#include <libs/moduleAnalysis/valNum/val_ip.h>
#include <libs/moduleAnalysis/valNum/val.i>
#include <libs/ipAnalysis/problems/symbolic/globals_fwd_ht.h>
#include <libs/ipAnalysis/problems/symbolic/IpVarList.h>

char *SYMVAL_ANNOT = "SymVal";

static void mark_bogus(ValTable &V, ValNumber *fwd, ValNumber v);

//*****************************************************************************
// class  SymValAnnotMgr: a manager class for Return Symbolic info
//*****************************************************************************

class SymValAnnotMgr : public CallGraphAnnotMgr {
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


CallGraphAnnot *SymValAnnotMgr::New(CallGraphNode *node)
{
    return new SymValAnnot(node);
}


CallGraphAnnot *SymValAnnotMgr::DemandAnnotation(CallGraphNode *node)
{
    CallGraph *cg = node->GetCallGraph();

    //  SymValDFProblem dfp(cg);
    //  cg->SolveDFProblem(&dfp);

    //  Do my own dataflow iteration to avoid having to Clone etc.
    //
    CallGraphNodeIterator nodes(cg);
    nodes.Reset(PostOrder);

    for (;
         nodes.Current();
         nodes++)
    {
	nodes.Current()->PutAnnotation(SymExploitMod(nodes.Current()));
    }

    return node->GetAnnotation(SYMVAL_ANNOT);
}


REGISTER_CG_ANNOT_MGR(SYMVAL_ANNOT, SymValAnnotMgr);


//*****************************************************************************
// class  SymValAnnot: 
//      node annotations provide information about returned values of 
//	formal parameters and global variables
//*****************************************************************************


struct SymValAnnotS {
    ValIP info;
    GlobFwdMap glob;
    ValNumber *fwd;

    //  Implement a cache for translation
    //
    ValNumber *cache;
    struct ValTable *cacheFor;

    IpVarList refs;

    SymValAnnotS(CallGraphNode *node);
    ~SymValAnnotS() 
    {
	if (fwd)   delete fwd;
	if (cache) delete cache;
    }
};


SymValAnnotS::SymValAnnotS(CallGraphNode *node) 
	: fwd(NULL), cache(NULL), cacheFor(NULL)
{
    /////////////////////////////////////////////////////////////////
    //
    //  Create structures for improved symbolic information
    //
    info.values = new ValTable;

    //		map from actuals and globals at call sites to their values
    //		map from formals and globals on return to their values
    //
    info.pass = new ValPassMap;

    //		map values in oldSym to values in newSym
    //
    int count, i;

    ValIP *oldSym = node->local_info->val_ip;

    count = oldSym->values->count();
    fwd = new int[count];

    for (i = 0; i < count; i++) fwd[i] = VAL_NIL;

    //////////////////////////////////////////////////////////////////////////
    //
    //  Now mark all old ValNumbers reachable from the return value of 
    //  DUMMY_GLOBAL as forwarding to VAL_BOGUS_GLOBAL -- this will indicate
    //  that the forwarding value must be computed differently
    //
    mark_bogus(*(oldSym->values), fwd,
	       oldSym->pass->query_entry("DUMMY_GLOBAL", 0));
}

SymValAnnot::SymValAnnot(CallGraphNode *node) : CallGraphAnnot(SYMVAL_ANNOT)
{
    hidden = new SymValAnnotS(node);
}

SymValAnnot::~SymValAnnot()
{
    delete hidden; 
}

CallGraphAnnot *SymValAnnot::Clone()
{
    return NULL;
}

OrderedSetOfStrings *
SymValAnnot::CreateOrderedSetOfStrings(unsigned int width)
{
    OrderedSetOfStrings *oss = new OrderedSetOfStrings;
    StringBuffer string(width);
    VarScope vscope;
  
    // string.Append("%s:", name);
  
    string.Append(" ");

    oss->Append(string.Finalize());
    return oss;
}

// upcall to read an annotation
int SymValAnnot::ReadUpCall(FormattedFile &port)
{
    return 0;
}

// upcall to write an annotation
int SymValAnnot::WriteUpCall(FormattedFile &port)
{
    return 0;
}


void SymValAnnot::encache(ValTable *newTab, ValNumber ov, ValNumber nv)
{
    if (hidden->cache && (hidden->cacheFor != newTab))
    {
	delete hidden->cache;
	hidden->cache    = NULL;
	hidden->cacheFor = NULL;
    }
    if (!(hidden->cache))
    {
	int count = hidden->info.values->count();

	hidden->cache = new ValNumber[count];
	for (int i = 0; i < count; i++) hidden->cache[i] = VAL_NIL;
	hidden->cacheFor = newTab;
    }
    hidden->cache[ov] = nv;
}

ValNumber SymValAnnot::cached(ValTable *newTab, ValNumber ov)
{
    if (hidden->cache && (hidden->cacheFor == newTab)) return hidden->cache[ov];
    return VAL_NIL;
}

ValIP *SymValAnnot::valIp()
{
    return &(hidden->info);
}

IpVarList *SymValAnnot::refScalars()
{
    return &(hidden->refs);
}

ValNumber SymValAnnot::getGlobal(ValNumber vn, 
				 ValNumber nameId,
				 int varOffset)
{
    return hidden->glob.query_entry(vn, nameId, varOffset);
}
void SymValAnnot::addGlobal(ValNumber vn, 
			    ValNumber nameId,
			    int varOffset,
			    ValNumber nv)
{
    hidden->glob.add_entry(vn, nameId, varOffset, nv);
}
ValNumber &SymValAnnot::fwd(ValNumber ov)
{
    return hidden->fwd[ov];
}

static void mark_bogus(ValTable &V, ValNumber *fwd, ValNumber v)
{
    if ((v == VAL_NIL) || (fwd[v] == VAL_BOGUS) ||
	(v == VAL_TOP) || (v == VAL_BOTTOM))
    {
	return;
    }
    int i, arity;
    fwd[v] = VAL_BOGUS;

    switch(ve_type(V[v]))
    {
      case VAL_ENTRY:
	break;
      case VAL_RETURN:
	mark_bogus(V, fwd, ve_input(V[v]));
	break;
      case VAL_GAMMA:
	//  don't mark test as bogus -- it doesn't involve DUMMY_GLOBAL
	//
	//  intentional fallthrough to VAL_PHI case
	//
      case VAL_PHI:
	arity = ve_arity(V[v]);
	for (i = 0; i < arity; i++) mark_bogus(V, fwd, ve_kid(V[v], i));
	break;
      case VAL_ETA:
	//  don't  mark test as bogus -- it doesn't involve DUMMY_GLOBAL
	//
	mark_bogus(V, fwd, ve_final(V[v]));
	break;
      case VAL_VAR_MU:
	mark_bogus(V, fwd, ve_init(V[v]));
	mark_bogus(V, fwd, ve_varIter(V[v]));
	break;
      case VAL_VARIANT:
	// die_with_message("mark_bogus: disallowed type\n");
	fwd[v] = VAL_BOTTOM;	//  is this safe?
	break;
      default:
	die_with_message("mark_bogus: disallowed type\n");
	// fwd[v] = VAL_BOTTOM;	//  dangerous hack!  (should abort?)
    }
}

void SymValAnnot::Dump()
{
    IpVarListEntry *e;

    hidden->info.Dump();
    for (IpVarListIter it(hidden->refs);
	 e = it.Current();
	 it++)
    {
	printf("%s * %d @ %d\n", e->name, e->length, e->offset);
    }
}
