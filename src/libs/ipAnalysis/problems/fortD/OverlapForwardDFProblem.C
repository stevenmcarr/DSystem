/* $Id: OverlapForwardDFProblem.C,v 1.5 2001/09/17 00:53:52 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//-------------------------------------------------------------------------
// author   : Seema Hiranandani
// content  :
// date    : July 1992
//-------------------------------------------------------------------------

//-------------------
// include files

#include <stdio.h>
#include <libs/fortD/misc/FortD.h>
#include <libs/ipAnalysis/problems/fortD/ReachAnnot.h>
#include <libs/ipAnalysis/problems/fortD/OverlapAnnot.h>
#include <libs/ipAnalysis/problems/fortD/OverlapForwardDFProblem.h>
#include <libs/ipAnalysis/problems/fortD/OverlapBackwardDFProblem.h>
#include <libs/ipAnalysis/problems/fortD/CommonBlockAnnot.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/ipInfo/iptree.h>

extern char* FORTD_OVERLAP_ANNOT;
extern char* FORTD_COMMON_BLOCK_ANNOT;
extern char* IFORTD_OVERLAP_ANNOT;
extern Boolean globals_match(common_block_ent* , common_block_ent *e);

void FortD_OverlapForward_DFProblem::InitializeNode(CallGraphNode *, ProcLocalInfo *)
{
}

//------------------------------------------------------------------ 
//  meet  function. finds the Union of accumulator (in1)
//------------------------------------------------------------------ 
void *FortD_OverlapForward_DFProblem::DFmeet(void *in1, void *in2, void *vrtx, void *edg)
{
 CallGraphEdge *CallGraphEdge1 = (CallGraphEdge*)edg;
 CallGraphNode *CallGraphNode1 = (CallGraphNode*)vrtx;
 char name1[20];

 FD_Overlap_Annot *set_in1 = (FD_Overlap_Annot*)in1;
 FD_Overlap_Annot *set_in2 = (FD_Overlap_Annot*)in2;
 
 // if there is CallGraphEdge information, union caller annot with callee annot
 // placing the result in the callee

  FD_Overlap_Annot *result = 
                   new FD_Overlap_Annot(set_in1);

// set_in2 = caller's annotation

  if (CallGraphEdge1 != 0) 
  {
  result->UnionF(CallGraphNode1, CallGraphEdge1, set_in2);
  }
  return((void*)result);
//  else
//    return (in1);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void* FortD_OverlapForward_DFProblem::DFtrans(void *in, void *out,
void *user_i, unsigned char &change)
{
 CallGraphNode *n = (CallGraphNode *) user_i;
 char aname[20];
 FD_Overlap_Annot *set_in, *set_out;
 FD_Overlap_Annot *CallGraphNode_annot;

  set_in =  ((FD_Overlap_Annot*)in);
  set_out = ((FD_Overlap_Annot*)out);

// get the CallGraphNode annotation
  CallGraphNode_annot = (FD_Overlap_Annot*)
                         n->GetAnnotation(IFORTD_OVERLAP_ANNOT);


// union the in edge with node annotation
  set_in->Union(CallGraphNode_annot, n);

  if(set_in->Diff(set_out))
   {
    change = 1;
  
// union the CallGraphNode_annot with the in CallGraphEdge
    CallGraphNode_annot->Union(set_in, n);

   }
  return (void *) in;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void FD_Overlap_Annot::UnionF(CallGraphNode *, CallGraphEdge *CallGraphEdge1, FD_Overlap_Annot *set1)
{

 const char *formal, *actual;
 ParamBinding *formal_bi;
 Boolean unioned = false, found = false, found2 = false;
 OverlapList *overlap_info_temp = new OverlapList();

 CallGraphNode *callee_CallGraphNode = CallGraphEdge1->Callee();

 FD_CommBlkNodeAnnotation *callee_annot =
   (FD_CommBlkNodeAnnotation *)callee_CallGraphNode->GetAnnotation(FORTD_COMMON_BLOCK_ANNOT);

//  set1 = caller's annotation

//---------------------------------------------
// perform the union operation for globals
 
 for(overlap_ent* caller = set1->overlap_info->first_entry();
                  caller != 0; caller = set1->overlap_info->next_entry())
  {
   found = false;
//------------------------------------
// check if it is a global variable

   if (caller->common_entry != 0)
    {
     //-----------------------------------------------------------
     // look for the overlap entry in the callee
     // If it exists, then perform a union, else create a new entry

      found = false;
      overlap_ent *callee;
      for(callee = this->overlap_info->first_entry();
       callee != 0 && !found; callee = this->overlap_info->next_entry())
        {
          //----------------------------------------------------- 
          // check to see if the global in the callee is the same
          // as the global in the caller

         if (globals_match(caller->common_entry, callee->common_entry))
          {
          found = true;
          break;
          }
        }
//---------------------------------------------------------------
// if the global entry has been found, union caller information

     if (found) {
      callee->union_t(caller);
      }  
//---------------------------------------------------------------
// if the global entry is not found, create a new entry 

     else {
     common_block_ent *comm_entry = 
                       callee_annot->translate_global(caller->common_entry);
     overlap_ent *e;
      if (comm_entry != 0)
        e = new overlap_ent(caller, comm_entry->name, comm_entry);
      else
        e = new overlap_ent(caller, caller->name(), caller->common_entry);
     overlap_info_temp->append_entry(e);
     }
    }
  }

//  set1 = caller's annotation, in terms of actual parameters

  //--------------------------------------------------------------
 // for each actual parameter passed at the callsite
 //--------------------------------------------------------------
 for (ParamNameIterator ani(CallGraphEdge1->paramBindings, ActualNameSet);
      ani.Current(); ++ani) 
     {
      actual = ani.Current();

 //-----------------------------------------------------------------
 // retrieve the set of bindings between the actual and the formals
 //-----------------------------------------------------------------
     ParamBindingsSet *bindings =
       CallGraphEdge1->paramBindings.GetForwardBindings(actual);
       for (ParamBindingsSetIterator bi(bindings);
            formal_bi = bi.Current(); ++bi)
       {
       formal = formal_bi->formal;
       found = found2 = false;
  
// walk the list of overlap entries belonging to  the caller
// The names of the arrays for the caller are in terms of actuals
// if the name matches formal, look for the entry in callee. The
// callee's entry name is represented as formals
// if an entry exists perform a merge else, create a new entry for the
// callee, merge the corresponding entry from the caller and attach
// it to the list

  for(overlap_ent* caller = set1->overlap_info->first_entry(); (caller != 0 &&
   !found);
                   caller = set1->overlap_info->next_entry())
  {
   if (strcmp(caller->name(), actual) == 0)
   {
   found = true;
   for(overlap_ent* callee = this->overlap_info->first_entry();
       (callee != 0 && !found2); callee = this->overlap_info->next_entry())
    {
    if(strcmp(callee->name(), formal) == 0)
      {
      found2 = true;
      callee->union_t(caller);
      unioned = true;
      break;
      }
    }
    if (!unioned) 
    {
     overlap_ent *e = new overlap_ent(caller, (char *) formal);
     overlap_info_temp->append_entry(e);
     unioned = false;
    }
   }
  }
 }
}
  for(overlap_ent *i= overlap_info_temp->first_entry(); i != 0; 
                   i= overlap_info_temp->next_entry())
     {
      overlap_ent *entry = new overlap_ent(i, i->name(), i->common_entry);
      this->overlap_info->append_entry(entry);
     }
}


//=============================================================================
// FortD_OverlapForward_DFProblem::FinalizeAnnotation
//  put the annotation on the node and delete the annotation containing
//  initial information that is currently hanging from the node
//=============================================================================
void FortD_OverlapForward_DFProblem::FinalizeAnnotation(CallGraphNode *node, 
					       FlowGraphDFAnnot *annot)
{
  FD_Overlap_Annot *oannot = (FD_Overlap_Annot *) annot;
  
  //----------------------------------------------------------------------
  // store the newly computed overlap annotation for the current node
  //----------------------------------------------------------------------
  node->PutAnnotation(oannot);
  
  //----------------------------------------------------------------------
  // delete the initial overlap information annotation; it is no longer 
  // needed
  //----------------------------------------------------------------------
  node->DeleteAnnotation(IFORTD_OVERLAP_ANNOT);
}



