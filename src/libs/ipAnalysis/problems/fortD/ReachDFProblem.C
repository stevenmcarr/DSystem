/* $Id: ReachDFProblem.C,v 1.4 1997/03/27 20:41:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//-------------------------------------------------------------------------
// author   : Seema Hiranandani
// content  : 
// The functions in this file solve a forward data flow problem specific
// to compiling Fortran D. They solve the reaching decomposition
// interprocedural problem
//  date    : July 1992
//-------------------------------------------------------------------------

//-------------------
// include files

#include <stdio.h>
#include <libs/fortD/misc/FortD.h>
#include <libs/ipAnalysis/problems/fortD/ReachDFProblem.h>
#include <libs/ipAnalysis/problems/fortD/ReachAnnot.h>
#include <libs/ipAnalysis/ipInfo/iptree.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/fortD/localInfo/CommBlock.h>
#include <libs/ipAnalysis/problems/fortD/CommonBlockAnnot.h>
#include <libs/ipAnalysis/problems/fortD/FortD_Sets.h>
#include <libs/ipAnalysis/problems/fortD/FortDLocalAnnot.h>

extern char *FORTD_REACH_ANNOT;
extern char *FORTD_COMMON_BLOCK_ANNOT;

//------------------------------------------------------------------
// attach common block information at the node
//------------------------------------------------------------------
void FortD_Reach_DFProblem::InitializeNode(CallGraphNode *node, ProcLocalInfo *)
{
     // This needs to be checked by John and/or Gil for correctness about
     // what annotation is being added to the call graph in the case where
     // a fortd local annotation is not present.  curetonk 5/25/94
  FortDLocalAnnot *fortDLocalAnnot;

  fortDLocalAnnot = (FortDLocalAnnot*)node->GetAnnotation(FORTD_LOCAL_ANNOT, true);

  if (!fortDLocalAnnot) 
  {
     assert(0);
//     fortDLocalAnnot = new FortDLocalAnnot(node);
//     node->PutAnnotation(fortDLocalAnnot);
//     fortDLocalAnnot = (FortDLocalAnnot*)node->GetAnnotation(FORTD_LOCAL_ANNOT, true);
  }
  else
  {
     FD_CommBlkNodeAnnotation *c, *comm_e;
     const IPinfoTree *iptree = fortDLocalAnnot->tree;
     FortranDInfo *fd = (FortranDInfo*)(iptree->tree->fd);

     c =  new FD_CommBlkNodeAnnotation(fd->common_blks_decl);
     node->PutAnnotation(c);
     comm_e = 
        (FD_CommBlkNodeAnnotation*)node->GetAnnotation(FORTD_COMMON_BLOCK_ANNOT, true); 
  }
}

//------------------------------------------------------------------
// get the callsite information stored in the IPinfoTree structure
// It contains a list of decomposition, align, distribute statements
// that reach each parameter at a callsite
// Attach the FortranD Hash Table entry to the edge annotation
//--------------------------------------------------------------------
void FortD_Reach_DFProblem::InitializeEdge(CallGraphEdge *edge)
{
CallSites *cs;
ActualList *alist;
ActualListEntry *a_entry;
FDSetEntry *l_entry;
int callsite_id;
FD_Reach_Annot *fd_edge;
FortranDHashTableEntry *hash_table_decomp, *hash_table_array;
FDSetEntryList *fdset_list;
int count1 = 0, i = 0;
unsigned int param_position = 0;  
CallGraphNode *callee, *caller;

 caller = edge->Caller();

//--------------------------------------------------
// get the iptree structure from the procedure node

const IPinfoTree *iptree = 
	((FortDLocalAnnot *)caller->GetAnnotation(FORTD_LOCAL_ANNOT, true))->tree;

//-------------------------------------------------
// get the FortranDInfo from the iptree structure

  FortranDInfo *fd = (FortranDInfo*)(iptree->tree->fd);

//-----------------------------------------------------------
// get the list of callsites, the callsite_id and the callee

  cs = iptree->tree->calls;
  callsite_id =  edge->callSiteIndexInCaller;
  callee = edge->Callee();

//----------------------
// for each callsite do

   for(NonUniformDegreeTreeIterator it(iptree->tree, PREORDER);
                                    it.Current() != 0; ++it)
   {
    CallSitesIterator callsites(((IPinfoTreeNode*)it.Current())->calls);
    for(CallSite *c; c = callsites.Current(); ++callsites)

//-------------------------------------------------------------
// if the callsite id matches the callsite id of the edge then

     if (c->Id() == edge->callSiteIndexInCaller) {

//---------------------------------------------------  
// create an annotation to be attached to the edge

   fd_edge = new FD_Reach_Annot(false);

//----------------------------------------------------------------
// create an annotation that contains a list of globals and their
// decompositions that reach the call site
 
   i = fd_edge->CreateCommonBlockAnnot(c, fd, edge->Callee());
  
//-----------------------------------------
// for each parameter at the callsite

 alist = c->GetActuals();
 // param_position = 1;
 for(a_entry=alist->First(); a_entry != 0; a_entry = alist->Next()) {
  
//---------------------------------------------------       
// for each decomposition reaching the parameter
// append the fortd_set_entry information and the 
// hash table entry for the array and decomposition

     fdset_list = (FDSetEntryList*)(a_entry->fortd_set);
     count1 = fdset_list->Count();
      for(l_entry = fdset_list->first_entry(); l_entry != 0;
          l_entry=fdset_list->next_entry())
       {
      hash_table_decomp = fd->GetEntry(l_entry->name());
      hash_table_array  = fd->GetEntry(a_entry->Name());
      
      fd_edge->fortd_set[i]->
       append_entry(hash_table_decomp, hash_table_array,l_entry);
      }

    fd_edge->fortd_set[i]->save_name((char *)callee->
           FormalPositionToName(edge->calleeEntryName, param_position));
    ++param_position;
     ++i;
   }
   edge->PutAnnotation(fd_edge);
  }
 }
}

//------------------------------------------------------------------ 
//  meet  function. finds the Union of accumulator (in1)
//------------------------------------------------------------------ 
void *FortD_Reach_DFProblem::DFmeet(void *in1, void *in2, void *vrtx, void *edg)
{
  CallGraphEdge *edge1 = (CallGraphEdge*)edg;
  CallGraphNode *node1 = (CallGraphNode*)vrtx;
  FD_Reach_Annot *set_in1 = (FD_Reach_Annot*)in1;
  FD_Reach_Annot *set_in2 = (FD_Reach_Annot*)in2;

  //---------------------------------------------------------------------
  // if there is edge information get the edge annotation
  // union caller annot with callee annot placing the result in callee
  //---------------------------------------------------------------------

  if (edg) 
  { 
    //---------------------------------------------------------------------
    // the case immediately below was added to patch the solution so it will 
    // not unnecessarily fail for edges from ENTRY or START or for edges to
    // EXIT. edge annotations for edges other than real callsites are not 
    // initialized and are thus NULL. the first union operation fails for NULL 
    // annotations. in the case of edges from ENTRY and START or edges to 
    // EXIT, the edges carry no information, so it is safe to say that the 
    // meet of the incoming partial result and NULL is equivalent to the 
    // incoming partial result. 
    // NOTE: synthetic edges representing procedure parameter bindings are
    //       not initialized and they will cause the first union operation 
    //       below to fail. this is a flaw of this data flow problem 
    //       implementation. before the assert was added in the union 
    //       operation, it just silently gave incorrect results.
    // 6 Sept 1994 -- John Mellor-Crummey
    //---------------------------------------------------------------------
    if ((edge1->Caller()->type == CGNT_Entry) ||
	(edge1->Caller()->type == CGNT_Start) ||
	(node1->type == CGNT_Exit)) return in1;

     FD_Reach_Annot *result = new FD_Reach_Annot();
     FD_Reach_Annot *edgeAnnot = (FD_Reach_Annot*)edge1->GetAnnotation(FORTD_REACH_ANNOT);

     //cout<<form("node name = %s \n", node1->proc_name);

     //------------------------------
     // set_in1 = callee's annotation
     //------------------------------
 
     result->Union(set_in1, edgeAnnot);

     //------------------------------
     // set_in2 = caller's annotation
     //------------------------------

     // add the decompositions in set_in2 that reach result
     result->Union(node1, edge1, set_in2);

     delete set_in1;

     return((void*)result);
  }
  else
  {
     return (in1);
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void* FortD_Reach_DFProblem::DFtrans(void *in, void *out, void *user_i, 
                            unsigned char &change)
{
 CallGraphNode *n = (CallGraphNode *) user_i;
 char aname[20];
 FD_Reach_Annot *set_in, *set_out, *result;
 
  set_in =  ((FD_Reach_Annot*)in);
  set_out = ((FD_Reach_Annot*)out);

  if  (!set_in->Diff(set_out))
 {
   if (n->GetAnnotation(FORTD_REACH_ANNOT) == NULL)
  {
   result = new FD_Reach_Annot(set_in->fortd_set, 0);
   n->PutAnnotation(result);   
  }
 }
 else
  change = 1;

  return (void *) in;
}

