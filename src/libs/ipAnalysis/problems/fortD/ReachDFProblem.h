/* $Id: ReachDFProblem.h,v 1.2 1997/03/11 14:35:05 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef _ReachDFProblem_
#define _ReachDFProblem_

#include <libs/support/misc/general.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphDFProblem.h>
#include <libs/ipAnalysis/problems/fortD/ReachAnnot.h>

class ProcLocalInfo;

//---------------------------------------------------------------
//---------------------------------------------------------------

class FortD_Reach_DFProblem : public CallGraphDFProblem {
public:

// constructor for the Fortran D Reaching Decomposition DataFlow Problem

  FortD_Reach_DFProblem() : CallGraphDFProblem(0) {
    top =    new FD_Reach_Annot(false);
    bottom = new FD_Reach_Annot(false);
    direction = Forward; // forward
  };
  
  virtual void  *DFtrans(void *new_in_annot_v, void *old_out_annot_v, 
			 void *self_node_v, unsigned char &changed);
  virtual void *DFmeet(void *partial_result_annot_v, void *pred_node_annot_v,
		       void *self_node_v, void *edge_v);
  void InitializeNode (CallGraphNode *node, ProcLocalInfo *);
  void InitializeEdge (CallGraphEdge *edge);
};

#endif
