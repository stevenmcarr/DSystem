/* $Id: DependenceInfo.h,v 1.1 1997/03/11 14:35:59 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: DependenceInfo.h,v 1.1 1997/03/11 14:35:59 carr Exp $ 

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

class CallGraph;            // minimal external definition

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

