/* $Id: PerfAnnot.C,v 1.3 1997/03/11 14:35:10 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * 
 * Implemention of methods for class PerfEstAnnot. See the header file for
 * this class for more infomation.
 *
 * Author: N. McIntosh
 *
 * Copyright 1992, Rice University, as part of the ParaScope
 * Programming Environment Project
 *
 */

#include <assert.h>
#include <libs/support/strings/rn_string.h>
#include <libs/support/strings/OrderedSetOfStrings.h>
#include <libs/support/strings/StringBuffer.h>
#include <libs/support/file/FormattedFile.h>

#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotReg.h>
#include <libs/ipAnalysis/problems/perfEst/PerfAnnot.h>
#include <libs/ipAnalysis/problems/perfEst/PerfEstDFProblem.h>

#define PERF_ANNOT_STR "Performance Estimation"
char *PERF_ANNOT = PERF_ANNOT_STR;

class PerfEstAnnotMgr : public CallGraphAnnotMgr {
  //-------------------------------------------------------------------
  // create an instance of the perf annotation 
  //-------------------------------------------------------------------
  CallGraphAnnot *New(CallGraphEdge *) { return 0; }; // for an edge
  CallGraphAnnot *New(CallGraphNode *node);

  //-------------------------------------------------------------------
  // compute an instance of the alias annotation for (at least) the 
  // specified node or edge
  //-------------------------------------------------------------------
  CallGraphAnnot *DemandAnnotation(CallGraphNode *node);
  CallGraphAnnot *DemandAnnotation(CallGraphEdge *) { return 0; };
};

REGISTER_CG_ANNOT_MGR(PERF_ANNOT, PerfEstAnnotMgr);

CallGraphAnnot *PerfEstAnnotMgr::New(CallGraphNode *node)
{
  return new PerfEstAnnot(node);
}

static void PerfEstAnnotateCallGraph(CallGraph *cg)
{   
  PerfEstDFProblem dfp(cg);
  cg->SolveDFProblem(&dfp);
}

CallGraphAnnot *PerfEstAnnotMgr::DemandAnnotation(CallGraphNode *node)
{
  PerfEstAnnotateCallGraph(node->GetCallGraph());
  return node->GetAnnotation(PERF_ANNOT_STR);
}

PerfEstAnnot::~PerfEstAnnot()
{
  if (pname)
    sfree(pname);
  if (e) 
    delete e;
}

PerfEstAnnot::PerfEstAnnot(CallGraphNode *node)
  : CallGraphAnnot(PERFANNOT_NAME)
{
  Construct(PerfEstAnnot_Function, node->procName, 0);
}

PerfEstAnnot::PerfEstAnnot(PerfEstAnnotType at,
			   char *pn, PerfEstExpr *ne) :
			   CallGraphAnnot(PERFANNOT_NAME)
{
  Construct(at, pn, ne);
}

void PerfEstAnnot::Construct(PerfEstAnnotType at,
			     char *pn, PerfEstExpr *ne)
{
  atype = at;
  total_time = 0.0;
  local_time = 0.0;
  exec_count = 0.0;
  isroot = false;
  e = 0;
  switch(atype) {
    case PerfEstAnnot_Top:
    case PerfEstAnnot_Bottom:
      pname = 0;
      break;
    case PerfEstAnnot_Function: {
      assert(pn);
      pname = ssave(pn);
      if (ne)
	e = ne->clone();
      else 
	e = new PerfEstExpr;
      break;
    }
    default:
      assert(1 != 0);
  }
}

void PerfEstAnnot::Print()
{
  switch(atype) {
    case PerfEstAnnot_Top:
      printf("TOP\n");
      break;
    case PerfEstAnnot_Bottom:
      printf("BOTTOM\n");
      break;
    case PerfEstAnnot_Function:
      e->print(stdout);
      break;
    }
}

int PerfEstAnnot::WriteUpCall(FormattedFile& port)
{
  int code = port.Write(atype);
  if (code) return code;
  if (atype == PerfEstAnnot_Function) {
    code = port.Write(pname) ||
      e->write(port) ||
      port.Write(total_time) ||
      port.Write(local_time) ||
      port.Write(exec_count) ||
      port.Write((int) isroot);
    if (code) return code;
  }
  return 0;
}

int PerfEstAnnot::ReadUpCall(FormattedFile& port)
{
  int anintvar;
  
  int code = port.Read(anintvar);
  if (code) return code;
  atype = (PerfEstAnnotType) anintvar;
  
  if (atype == PerfEstAnnot_Function) {
    char buf[1024];
    code = port.Read(buf);
    if (code) return code;
    pname = ssave(buf);
    
    code = e->read(port) ||
      port.Read(total_time) ||
      port.Read(local_time) ||
      port.Read(exec_count) ||
      port.Read(anintvar);
    if (code) return code;
    
    isroot = (Boolean) anintvar;
  }
  return 0;
}

CallGraphAnnot *PerfEstAnnot::Clone()
{
  PerfEstAnnot *n = new PerfEstAnnot(atype, pname, e);
  return (CallGraphAnnot *) n;
}

Boolean PerfEstAnnot::IsTop()
{
  return (Boolean) (atype == PerfEstAnnot_Top);
}

Boolean PerfEstAnnot::IsBottom()
{
  return (Boolean) (atype == PerfEstAnnot_Bottom);
}

// generate printable version of the annotation
OrderedSetOfStrings *
PerfEstAnnot::CreateOrderedSetOfStrings(unsigned int width)
{
  StringBuffer string(width);
  OrderedSetOfStrings *oss = new OrderedSetOfStrings;

  string.Append("PerfEstAnnot::CreateOrderedSetOfStrings not implemented yet.");
  oss->Append(string.Finalize());

  return oss;
}
