/* $Id: OverlapAnnot.h,v 1.5 1997/03/11 14:35:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef _OverlapAnnot_
#define _OverlapAnnot_

#include <stdio.h>
#include <iostream>


#include <libs/ipAnalysis/callGraph/CallGraphDFProblem.h>
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphIterators.h>

#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>

#include <libs/fortD/localInfo/fd_symtable.h>
#include <libs/fortD/misc/FortD.h>
#include <libs/fortD/localInfo/LocalOverlap.h>

#include <libs/support/lists/IOSinglyLinkedList.h>
#include <libs/support/lists/SinglyLinkedList.h>

extern char *FORTD_OVERLAP_ANNOT;
extern char *IFORTD_OVERLAP_ANNOT;


OverlapList* GetOverlapInfo(CallGraphNode *n);
//---------------------------------------------------------------
// Overlap annotation attached at the nodes
//---------------------------------------------------------------
class FD_Overlap_Annot : public FlowGraphDFAnnot
{
 OverlapList *overlap_info; 

 public:
 FD_Overlap_Annot(OverlapList *overlap);
 FD_Overlap_Annot(): FlowGraphDFAnnot(FORTD_OVERLAP_ANNOT)
 {
  overlap_info = new OverlapList();
 };
 
 int OverlapCount() 
 {
  return this->overlap_info->Count();
 }
 
 FD_Overlap_Annot(FD_Overlap_Annot *copy);
 FD_Overlap_Annot(OverlapList *overlap, char* name);
 int ReadUpCall(FormattedFile *port);
 int WriteUpCall(FormattedFile *port);
 
 void Union(CallGraphNode *node1, CallGraphEdge *edge1, FD_Overlap_Annot *f1);
 void UnionF(CallGraphNode *node1, CallGraphEdge *edge1, FD_Overlap_Annot *f1);
 void Union(FD_Overlap_Annot *node_info, CallGraphNode *n);
 int operator ==(const DataFlowSet &rhs) const;
 Boolean Diff(FD_Overlap_Annot* out);
 FlowGraphDFAnnot* Clone() const;
 int write(FormattedFile &port) 
 {
 overlap_info->Write(port);
 return 0; // hide deficiency of underlying implementation -- JMC 6/93
 };

 int read(FormattedFile &port)
 {
  overlap_info->Read(port);
  return 0; // hide deficiency of underlying implementation -- JMC 6/93
 };
 OverlapList* GetOverlapInf() { return overlap_info; };
};


#endif _OverlapAnnot_ 
