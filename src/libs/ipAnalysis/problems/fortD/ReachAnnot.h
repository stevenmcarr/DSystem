/* $Id: ReachAnnot.h,v 1.8 2001/10/12 19:30:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef _FortDDFPROB_
#define _FortDDFPROB_

#include <stdio.h>

#include <libs/support/lists/IOSinglyLinkedList.h>
#include <libs/support/lists/SinglyLinkedList.h>
#include <libs/support/strings/OrderedSetOfStrings.h>
#include <libs/support/strings/StringBuffer.h>

#include <libs/ipAnalysis/callGraph/CallGraphIterators.h>
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>
#include <libs/ipAnalysis/callGraph/CallGraphDFProblem.h>

#include <libs/fortD/misc/FortD.h>
#undef is_open
#include <iostream.h>


// 9/25/93 RvH: Why all these constants instead of flexible data
// structures ?  GROMOS nonbal() has 40 parameters !
// 2/7/94 RvH: ... and GROMOS runmd() has 63 params.
//#define FD_MAX_PARAM 15
//#define FD_MAX 15
static const int FD_MAX_PARAM = 100;
static const int FD_MAX = 100;

// external declarations
class FormattedFile;

// forward declarations
class FD_Set;
class FDSetItem;

//------------------------------------------------------------------
//------------------------------------------------------------------
class FD_EntryCount
{
 public:
 int  decomp_count, distrib_count,  align_count;
 DistribEntryArray      *distrib_entries[FD_MAX];
 AlignList              *align_entries[FD_MAX];
 FortranDHashTableEntry *decomp_entries[FD_MAX];
 FortranDHashTableEntry *align_decomp_ht[FD_MAX];
 FortranDHashTableEntry *align_ht_entries[FD_MAX];
 char                   *align_decomp_names[FD_MAX];
 char                   *param_names[FD_MAX];
  

 FD_EntryCount(); 
 void process_distrib(FD_Set *set);
 void process_align  (FD_Set *set, char* formal_name);
 void process_decomp (FD_Set *set);
 void cleanup_distrib(FD_Set *set);
 void cleanup_decomp (FD_Set *set);
 void cleanup_align  (FD_Set *set);
 void write(FormattedFile &port);
 FortranDInfo* map_reach_annot(CallGraph *cg);
};

//---------------------------------------------------------------
// initialize fortranD annotation 
//---------------------------------------------------------------
class FD_Reach_Annot : public FlowGraphDFAnnot
{
 int count;
 char *name;
 public:
 FD_Set *fortd_set[FD_MAX_PARAM];
 FDSetEntryList *read_set[FD_MAX_PARAM];
 FortranDInfo *f;

 FD_Reach_Annot(); 
 FD_Reach_Annot(Boolean is_read);
 FD_Reach_Annot(FD_Set** fortd_set1, FortranDInfo *);
#if 0
 FD_Reach_Annot(IODictionary *diction);
#endif
 int ReadUpCall(FormattedFile*);
 int WriteUpCall(FormattedFile*);
#if 0
 Boolean Write(FormattedFile &);
#endif
 void write(FormattedFile &);
#if 0
 void read(FormattedFile &); 
#endif
 void Union(FD_Reach_Annot *f1, FD_Reach_Annot *f2);
 void Union(CallGraphNode *node1, CallGraphEdge *edge1, FD_Reach_Annot *f1);
 Boolean Diff(FD_Reach_Annot* out);
 int operator == (const DataFlowSet &) const;
 FlowGraphDFAnnot* Clone() const;
 void UnionG(CallGraphNode *node1, CallGraphEdge *edge1, FD_Reach_Annot *set1);
 int CreateCommonBlockAnnot(CallSite *cs_entry, FortranDInfo *fd, 
                            CallGraphNode*);
 void MapReachNodeAnnot(CallGraphNode*);

// methods to map the annotation to a set of
// strings

 OrderedSetOfStrings* CreateOrderedSetOfStrings();
};

 
extern char *FORTD_REACH_ANNOT;

#endif _FortDDFPROB_ 





