/* $Id: DependenceInfo.C,v 1.4 1997/03/11 14:35:58 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/moduleAnalysis/dependence/interface/DependenceInfo.h>

// *** details of components of the dependence graph interface ***
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

#include <libs/support/strings/rn_string.h>

#define INVALID_DEP_EDGE_INDEX  -1
#define	INITIAL_EDGE_COUNT	2000


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


//-----------------------------------------------------------------------
// DependenceInfo::DependenceInfo(FortTree ft, Context module, 
//                                  Context pgm_context, CallGraph *callgraph)
//
// constructor
//-----------------------------------------------------------------------
DependenceInfo::DependenceInfo(FortTree ft, Context module, 
				 Context pgm_context, CallGraph *callgraph)
{
  hidden = new DependenceInfoS;

  hidden->ft = ft;

  // create dependence graph structure components
  hidden->dg = dg_create_instance();
  hidden->el = el_create_instance(10);  /* num_edges */ 
  hidden->li = li_create_instance();
  hidden->si = create_side_info(ft);

  // create cfg information
  hidden->cfg_module = cfg_Open(ft);
  if (hidden->cfg_module) {
    cfgval_Open(hidden->cfg_module, false);
    ssa_Open(hidden->cfg_module,
	     /* ipInfo    */ (Generic) callgraph,
	     /* ipSmush   */ false,
	     /* doArrays  */ false, 
	     /* doDefKill */ false, /* this should be true for output deps */
	     /* doGated   */ false);
  }

  hidden->dt_info = dt_init(ft_Root(ft), hidden->si, hidden->cfg_module);
  dg_create_edge_structure(hidden->dg,INITIAL_EDGE_COUNT);

  // set dependence analysis control parameters 
  dg_set_external_analysis(hidden->dg, graph_local); 
  dg_set_local_analysis(hidden->dg, true);
  dg_set_set_interchange(hidden->dg, false); 
  dg_set_input_dependences(hidden->dg, false);	
  
  // perform the analysis and build the dependence graph
  dg_build(ft_Root(ft), hidden->ft, hidden->dg, hidden->si, 
	   hidden->dt_info, hidden->li, (Generic) callgraph, hidden->cfg_module);
  
  hidden->dg_edges = dg_get_edge_structure(hidden->dg);
  hidden->current_loop_node = AST_NIL;
  hidden->cur_edge_index = INVALID_DEP_EDGE_INDEX;
}


//-----------------------------------------------------------------------
// DependenceInfo::~DependenceInfo()
//
// destructor
//-----------------------------------------------------------------------
DependenceInfo::~DependenceInfo()
{
  // destroy cfg information
  if (hidden->cfg_module) {
	cfgval_Close(hidden->cfg_module);	
	ssa_Close(hidden->cfg_module);	
	cfg_Close(hidden->cfg_module);	
  }

  // destroy the dependence graph structures 
  dg_destroy(hidden->dg);
  el_destroy_instance(hidden->el);
  li_free(hidden->li);
  dt_free(hidden->dt_info);
  destroy_side_info(hidden->ft, hidden->si);

  delete hidden;
}

//-----------------------------------------------------------------------
// Boolean DependenceInfo::CurLoopSet(FortTreeNode node)
//
// sets the current loop to node; returns true if node is a loop
//-----------------------------------------------------------------------
Boolean DependenceInfo::CurLoopSet(FortTreeNode node)
{
  Boolean retcode = el_get_loop_info(hidden->li, node);
  if (retcode) hidden->current_loop_node = node;
  return retcode;
}


//-----------------------------------------------------------------------
// unsigned int DependenceInfo::CurLoopGatherDeps()
//
// returns the number of dependences edges gathered into an edge list 
// for the current loop
//-----------------------------------------------------------------------
unsigned int DependenceInfo::CurLoopGatherDeps()
{
  if (hidden->current_loop_node != AST_NIL) {
    int num_deps = 
      el_new_loop(hidden->el, hidden->li, hidden->si, hidden->dg, 
		  hidden->current_loop_node);	
    
    if (num_deps > 0) hidden->cur_edge_index = first_dependence(hidden->el);
    return (unsigned int) num_deps;
  } else return 0;
}


//-----------------------------------------------------------------------
// DG_Edge *DependenceInfo::GetCurrentEdge()
//
// returns a pointer to the current dependence edge in the edge list
// for the currently selected loop, or 0 if no such edge exists.
//-----------------------------------------------------------------------
DG_Edge *DependenceInfo::GetCurrentEdge()
{
  if (hidden->cur_edge_index != INVALID_DEP_EDGE_INDEX)
    return &hidden->dg_edges[hidden->cur_edge_index]; 
  else return 0;
}


//-----------------------------------------------------------------------
// void DependenceInfo::NextEdge()
//
// advance to the next edge in the edge list for the current loop
//-----------------------------------------------------------------------
void DependenceInfo::NextEdge()
{
  hidden->cur_edge_index = next_dependence(hidden->el);
}

//-----------------------------------------------------------------------
// void DependenceInfo::CurLoopGetPrivateVarList()
//
// return a string that is a comma separated list of the private variables
// for the currently selected loop
//-----------------------------------------------------------------------
char *DependenceInfo::CurLoopGetPrivateVarList()
{
  return el_get_private_list(hidden->li);
}


//-----------------------------------------------------------------------
// void DependenceInfo::CurLoopGetSharedVarList()
//
// return a string that is a comma separated list of the shared variables
// for the currently selected loop
//-----------------------------------------------------------------------
char *DependenceInfo::CurLoopGetSharedVarList()
{
  return el_get_shared_list(hidden->li);
}

//-----------------------------------------------------------------------
// void DependenceInfo::CurLoopUnprivatizeVar()
//
// remove a specified variable from the private variable list for the
// currently selected loop
//-----------------------------------------------------------------------
Boolean DependenceInfo::CurLoopUnprivatizeVar(char *vname)
{
  if (hidden->current_loop_node != AST_NIL) {
	el_remove_private_var(hidden->li, hidden->current_loop_node, vname);
	return true;
  } else return false;
}
