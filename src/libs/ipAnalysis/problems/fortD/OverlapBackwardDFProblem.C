/* $Id: OverlapBackwardDFProblem.C,v 1.5 1997/03/11 14:35:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//-------------------------------------------------------------------------
// author   : Seema Hiranandani
// content  : solution to the overlap backward problem
// date    : July 1992
//-------------------------------------------------------------------------

//-------------------
// include files

#include <stdio.h>
#include <libs/fortD/misc/FortD.h>
#include <libs/ipAnalysis/problems/fortD/ReachAnnot.h>
#include <libs/ipAnalysis/problems/fortD/FortD_Sets.h>
#include <libs/ipAnalysis/problems/fortD/CommonBlockAnnot.h>
#include <libs/ipAnalysis/problems/fortD/OverlapAnnot.h>
#include <libs/ipAnalysis/problems/fortD/OverlapForwardDFProblem.h>
#include <libs/ipAnalysis/problems/fortD/OverlapBackwardDFProblem.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/ipInfo/iptree.h>

extern char* FORTD_COMMON_BLOCK_ANNOT;
extern char* FORTD_OVERLAP_ANNOT;
extern char* FORTD_REACH_ANNOT;
extern char* IFORTD_OVERLAP_ANNOT;
//-----------------------------------------------------------------------------
// this function updates the overlap estimates using the reaching decomposition
// information 
//-----------------------------------------------------------------------------
void FortD_OverlapBackward_DFProblem::UpdateOverlapsReach(CallGraphAnnot *an_t, OverlapList *ov, CallGraphNode *n)
{
 
 char *formal;
 FD_SetItem *set_item;
 DIST_INFO *distribute_info = NULL;
 common_block_ent *common_entry;

 FD_Reach_Annot *an = (FD_Reach_Annot *)an_t;
 FD_CommBlkNodeAnnotation *common_block_annot = 
 (FD_CommBlkNodeAnnotation *) n->GetAnnotation(FORTD_COMMON_BLOCK_ANNOT);
  
//----------------------------------------------------------
// attach the common_block_information to the overlap entry 

   for(overlap_ent *e = ov->first_entry(); e != 0; e = ov->next_entry())
    {
     if (common_entry = common_block_annot->fortd_is_global(e->name()))
      { e->put_commonb_entry(common_entry); }
     else
     e->common_entry = 0;
    }

 for(int i=0;i<FD_MAX_PARAM;++i) 
  {
     formal = an->fortd_set[i]->name();
 
   if(an->fortd_set[i]->Count() > 1)
     cout<<form("Currently unable to handle multiple reaching decompositions \n");

//---------------------------------------------------------------------
// get the distribute info data structure for the formal parameter
// get the globals information for the entry

    if(an->fortd_set[i]->Count() != 0)
    {
    for(set_item = an->fortd_set[i]->first_entry(); set_item != NULL; 
        set_item = an->fortd_set[i]->next_entry())
     {
       distribute_info =
        &(set_item->f->d->distrib_info[set_item->fd->d_index()]->distinfo[0]);
//       common_entry =  set_item->c;
      }

//--------------------------------------------------------------------------
// search for the overlap entry for the formal or global in the Overlap List
   
      for(overlap_ent *e = ov->first_entry(); e != 0; e = ov->next_entry())
        {
         if(strcmp(e->name(), formal) == 0) {

//---------------------------------------------------------------------
//  if the distribution is local, reset range to 0
//  if the distribution is block, do nothing to the range
//  if the distribution is cyclic, reset range to 0 
//---------------------------------------------------------------------

          for(int k=0;k<e->getdim();++k)
           {
            switch(distribute_info->distr_type)
            {
             case FD_DIST_LOCAL:
             e->upper[k] = 0;
             e->lower[k] = 0;
             break;

             case FD_DIST_BLOCK:
             break;
              
             default:
             e->upper[k] = 0;
             e->lower[k] = 0;
             break;
            }
           ++distribute_info;
           }
        }
    }
  }
 }
}

//------------------------------------------------------------------
// when initializing the CallGraphNode information, look at the reaching
// decomposition information to initialize overlap values
//------------------------------------------------------------------
void FortD_OverlapBackward_DFProblem::InitializeNode(CallGraphNode *n, ProcLocalInfo *)
{
FD_Overlap_Annot *overlap_node_annot;
 
 FD_Reach_Annot *an = (FD_Reach_Annot*) n->GetAnnotation(FORTD_REACH_ANNOT);
 OverlapList *overlap_info  =  GetOverlapInfo(n); 

 UpdateOverlapsReach((CallGraphAnnot*)an, overlap_info, n);
 overlap_node_annot = new FD_Overlap_Annot(overlap_info, IFORTD_OVERLAP_ANNOT);
 n->PutAnnotation(overlap_node_annot);
}

//------------------------------------------------------------------ 
//  meet  function. finds the Union of accumulator (in1)
//------------------------------------------------------------------ 
void *FortD_OverlapBackward_DFProblem::DFmeet(void *in1, void *in2, void *vrtx, void *edg)
{
 CallGraphEdge *edge1 = (CallGraphEdge*)edg;
 CallGraphNode *CallGraphNode1 = (CallGraphNode*)vrtx;
 char name1[20];

 FD_Overlap_Annot *set_in1 = (FD_Overlap_Annot*)in1;
 FD_Overlap_Annot *set_in2 = (FD_Overlap_Annot*)in2;
 
 // if there is edge information, union callee annot with caller annot
 // placing the result in the caller
  
  FD_Overlap_Annot *result = 
                   new FD_Overlap_Annot(set_in1);

// set_in2 = callee's CallGraphAnnot

  if (edge1 != 0) 
  {
  result->Union(CallGraphNode1, edge1, set_in2);
//  delete set_in1;
  }
  return((void*)result);
//  else
//    return (in1);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void* FortD_OverlapBackward_DFProblem::DFtrans(void *in, void *out, void *user_i,
unsigned char &change)
{
 CallGraphNode *n = (CallGraphNode *) user_i;
 char aname[20];
 FD_Overlap_Annot *set_in, *set_out;
 FD_Overlap_Annot *CallGraphNode_annot;

  set_in =  ((FD_Overlap_Annot*)in);
  set_out = ((FD_Overlap_Annot*)out);

//  cout<<form(" node name, overlap_backward = %s \n", n->proc_name);

// get the CallGraphNode CallGraphAnnot
  CallGraphNode_annot = 
                     (FD_Overlap_Annot*) n->GetAnnotation(IFORTD_OVERLAP_ANNOT);

//--------------------------------------------------
// union the in edge with CallGraphNode Annotation
 
//  cout<<form(" Node Annot Count = %d , %s \n", CallGraphNode_annot->OverlapCount(), n->Procname);

  set_in->Union(CallGraphNode_annot, n);

  if(set_in->Diff(set_out))
   {
    change = 1;

//--------------------------------------------------    
// union the CallGraphNode_annot with the in edge

     CallGraphNode_annot->Union(set_in, n);
   }
  return (void *) in;
}

