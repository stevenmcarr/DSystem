/* $Id: DependenceInfo.h,v 1.2 1997/03/27 20:46:29 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: DependenceInfo.h,v 1.2 1997/03/27 20:46:29 carr Exp $ 

//****************************************************************************
// DependenceInfo.h
//
// an encapsulation of all of the dependence graph construction, traversal,
// and manipulation code.
//
// NOTES:
// the interface is woefully incomplete. this encapsulation contains only
// sufficient interface to support a single client: the data race 
// instrumentation system. other clients should 
//****************************************************************************


#ifndef context_h
#include <libs/support/database/context.h>
#endif

#ifndef FortTree_h
#include <libs/frontEnd/fortTree/FortTree.h>
#endif

#ifndef dg_instance_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#endif

#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/dep_dt.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/el.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/moduleAnalysis/cfg/cfg.h>
#include <libs/moduleAnalysis/cfgValNum/cfgval.h>
#include <libs/moduleAnalysis/ssa/ssa.h>

class CallGraph;            // minimal external definition


//-----------------------------------------------------------------------
// DependenceInfo hidden representation
//-----------------------------------------------------------------------
struct DependenceInfoS {
  FortTree      ft;
  DG_Instance	*dg;
  EL_Instance	*el;
  LI_Instance	*li;
  SideInfo	*si;
  DT_info	*dt_info;
  DG_Edge       *dg_edges;
  AST_INDEX     current_loop_node;
  int           cur_edge_index;
  CfgInfo       cfg_module;	
};


class DependenceInfo {
  struct DependenceInfoS *hidden;
public:
  DependenceInfo(FortTree ft,  Context module, Context pgm_context, 
		  CallGraph *callgraph);
  ~DependenceInfo();
  Boolean CurLoopSet(FortTreeNode node);
  unsigned int CurLoopGatherDeps();
  char *CurLoopGetPrivateVarList();
  char *CurLoopGetSharedVarList();
  Boolean CurLoopUnprivatizeVar(char *vname);
  DG_Edge *GetCurrentEdge();
  void NextEdge();
};

