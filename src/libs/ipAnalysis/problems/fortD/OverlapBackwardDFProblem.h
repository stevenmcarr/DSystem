/* $Id: OverlapBackwardDFProblem.h,v 1.4 2001/10/12 19:30:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef _FD_OverlapBackward_
#define _FD_OverlapBackward_

#include <stdio.h>
#include <libs/support/misc/general.h>

#include <libs/fortD/misc/FortD.h>
#undef is_open
#include <iostream.h>
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphDFProblem.h>
#include <libs/support/lists/IOSinglyLinkedList.h>
#include <libs/support/lists/SinglyLinkedList.h>
#include <libs/fortD/localInfo/fd_symtable.h>
#include <libs/fortD/misc/ReadInterInfo.h>
#include <libs/fortD/localInfo/LocalOverlap.h>
#include <libs/ipAnalysis/problems/fortD/OverlapAnnot.h>


class FortD_OverlapBackward_DFProblem : public CallGraphDFProblem {
 public:
  FortD_OverlapBackward_DFProblem() : CallGraphDFProblem(0)
  {
  top = new FD_Overlap_Annot();
  bottom = new FD_Overlap_Annot();
  direction = Backward; 
  };
  virtual void  *DFtrans(void *new_in_annot_v, void *old_out_annot_v,
                         void *self_node_v, unsigned char &changed);
  virtual void *DFmeet(void *partial_result_annot_v, void *pred_node_annot_v,
                       void *self_node_v, void *edge_v);
   void InitializeNode (CallGraphNode *nodei, ProcLocalInfo *);
   void InitializeEdge (CallGraphEdge *){};
   void read(Context context, char *name);
   void UpdateOverlapsReach(CallGraphAnnot *an, OverlapList *ov, CallGraphNode *n);
};

#endif _FD_OverlapBackward_
