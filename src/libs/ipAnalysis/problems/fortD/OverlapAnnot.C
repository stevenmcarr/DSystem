/* $Id: OverlapAnnot.C,v 1.4 1997/03/27 20:41:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//-------------------------------------------------------------------------
// author   : Seema Hiranandani
// content  :
// date     : July 1992
//-------------------------------------------------------------------------

#include <libs/ipAnalysis/callGraph/CallGraphAnnotReg.h>

#include <libs/ipAnalysis/problems/fortD/CommonBlockAnnot.h>
#include <libs/ipAnalysis/problems/fortD/ReachAnnot.h>
#include <libs/ipAnalysis/problems/fortD/OverlapAnnot.h>
#include <libs/ipAnalysis/problems/fortD/OverlapBackwardDFProblem.h>
#include <libs/ipAnalysis/problems/fortD/OverlapForwardDFProblem.h>

#include <libs/ipAnalysis/problems/fortD/FortDLocalAnnot.h>

#include <libs/ipAnalysis/ipInfo/iptree.h>

extern Boolean globals_match(common_block_ent* , common_block_ent *e); 
//------------------------------------------------------------------
// return the class structure that contains the overlap information
// collected during the local phase
//------------------------------------------------------------------
OverlapList* GetOverlapInfo(CallGraphNode *n)
{
 const IPinfoTree *iptree =
	((FortDLocalAnnot *)n->GetAnnotation(FORTD_LOCAL_ANNOT, true))->tree;
 FortranDInfo *fd = (FortranDInfo*)(iptree->tree->fd);
 return fd->overlap_info;
}

//------------------------------------------------------------------
// constructors for the overlap annotation class
//------------------------------------------------------------------
FD_Overlap_Annot::FD_Overlap_Annot(OverlapList *overlap): 
FlowGraphDFAnnot(FORTD_OVERLAP_ANNOT) // initialize parent class 
{
 overlap_info = new OverlapList();
 for(overlap_ent* i= overlap->first_entry(); i != 0; 
                  i= overlap->next_entry())
 {
  overlap_ent *a = new overlap_ent(i);
  overlap_info->append_entry(a);
 } 
}

FD_Overlap_Annot::FD_Overlap_Annot(OverlapList *overlap, char* name): 
FlowGraphDFAnnot(name) // initialize parent class 
{
 overlap_info = new OverlapList();
 for(overlap_ent* i= overlap->first_entry(); i != 0; 
                  i= overlap->next_entry())
 {
  overlap_ent *a = new overlap_ent(i);
  overlap_info->append_entry(a);
 } 
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------
FD_Overlap_Annot::FD_Overlap_Annot(FD_Overlap_Annot *overlap_copy):
FlowGraphDFAnnot(FORTD_OVERLAP_ANNOT)
{
 overlap_info = new OverlapList();
 for(overlap_ent* i= overlap_copy->overlap_info->first_entry(); i != 0; 
                  i =overlap_copy->overlap_info->next_entry())
 {
  overlap_ent *a = new overlap_ent(i);
  overlap_info->append_entry(a);
 }
}


//------------------------------------------------------------------------
// walk the list of overlap entries belonging to  the CallGraphNode (set1)
// if an entry is found in the "in" annotation, union the entry
// with the CallGraphNode's entry.
// if an entry is not found, then create a copy of the entry 
//------------------------------------------------------------------------
void FD_Overlap_Annot::Union(FD_Overlap_Annot *set1, CallGraphNode *n)
{ 
 Boolean unioned = false;
 OverlapList *overlap_info_temp = new OverlapList();
 common_block_ent *comm_entry, *comm;
 char *nme;

 FD_CommBlkNodeAnnotation *CallGraphNode_annot =
      (FD_CommBlkNodeAnnotation *)n->GetAnnotation(FORTD_COMMON_BLOCK_ANNOT);
//---------------------------------------------
// perform the union operation for globals
  
 

// if(set1->overlap_info->entry_count() >= 8) 
  {
// cout<<form("OverlapEntry Count = %d   Proc Name = %s\n", set1->overlap_info->entry_count(), n->proc_name);
  }

 for(overlap_ent* itm = set1->overlap_info->first_entry();
                  itm != 0; itm = set1->overlap_info->next_entry())
  {
//------------------------------------
// check if it is a global variable

   if (itm->common_entry != 0)
   { 
    comm_entry = CallGraphNode_annot->translate_global(itm->common_entry);

//--------------------------------------------------------------------------
// if there is a global entry, then get the name

   if (comm_entry != 0) {
    nme = comm_entry->name;
    comm = comm_entry;
//-------------------------------------
// find the global entry in this set
    for(overlap_ent* in_item = this->overlap_info->first_entry();
        in_item != 0; in_item = this->overlap_info->next_entry())
    {
     if(strcmp(nme, in_item->name()) == 0)
     {
      in_item->union_t(itm);
      unioned = true;
     }
   } 
  }
  
  else 
  {  // comm_entry == 0
    nme = itm->name();
    comm = itm->common_entry;
    for(overlap_ent* in_item2 = this->overlap_info->first_entry();
        in_item2 != 0; in_item2 = this->overlap_info->next_entry())
      { if (in_item2->common_entry != 0) {
        if (globals_match(itm->common_entry, in_item2->common_entry)) 
        {
         in_item2->union_t(itm);
         unioned = true;
        }
       }
      }
    }
//-------------------------------------------------------
// if the global entry is not found, add it to the list
  if (!unioned) 
    {
     overlap_ent *e = new overlap_ent(itm, nme,
                                      comm);
     overlap_info_temp->append_entry(e);
    }
   unioned = false;
  }
 }

//--------------------------------------------------------------------
// for the rest of the entries 

 for(overlap_ent* CallGraphNode_item = set1->overlap_info->first_entry();
                  CallGraphNode_item != 0; CallGraphNode_item = set1->overlap_info->next_entry())
  {
   if(CallGraphNode_item->common_entry == 0)
   {
    for(overlap_ent* in_item = this->overlap_info->first_entry();
                     in_item != 0; in_item = this->overlap_info->next_entry())
    {
    if(strcmp(CallGraphNode_item->name(), in_item->name()) == 0)
     {
      in_item->union_t(CallGraphNode_item);
      unioned = true;
      break;
     }
    }
    if (!unioned) 
    {
     overlap_ent *e = new overlap_ent(CallGraphNode_item, 
     CallGraphNode_item->name(), CallGraphNode_item->common_entry);
     overlap_info_temp->append_entry(e);
    }
   unioned = false;
  }
 }
  for(overlap_ent *i= overlap_info_temp->first_entry(); i != 0; 
          i= overlap_info_temp->next_entry())
     {
      overlap_ent *entry = new overlap_ent(i, i->name(), i->common_entry);
      this->overlap_info->append_entry(entry);
     }
}

//----------------------------------------------------------------
// return true if globals entries c and c2 refer to the same 
// common block entry
//----------------------------------------------------------------
Boolean globals_match(common_block_ent *c, common_block_ent *c2)
{
 if ((c == 0) || (c2 == 0))
  return(false);
 if ((strcmp(c->leader, c2->leader) == 0) && (c->offset == c2->offset)
     && (c->size == c2->size)) {
     return (true);
    }
 else
   return(false);
}

//---------------------------------------------------------------------
// walk the list of overlap entries belonging to  the callee
// The names of the arrays for the callee are in terms of formals
// if the name matches formal, look for the entry in caller. The
// caller's entry name is represented as actuals
// if an entry exists perform a merge else, create a new entry for the
// caller, merge the corresponding entry from the callee and attach
// it to the list
//---------------------------------------------------------------------
void FD_Overlap_Annot::Union(CallGraphNode *, CallGraphEdge *edge1, FD_Overlap_Annot *set1)
{
 const char *formal, *actual;
 Boolean unioned = false, found = false, found2 = false;
 OverlapList *overlap_info_temp = new OverlapList();
 CallGraphNode *caller_CallGraphNode = edge1->Caller();

 FD_CommBlkNodeAnnotation *caller_annot =
 (FD_CommBlkNodeAnnotation *)caller_CallGraphNode->
                             GetAnnotation(FORTD_COMMON_BLOCK_ANNOT);

//---------------------------------------------
// perform the union operation for globals
 
 for(overlap_ent* callee = set1->overlap_info->first_entry();
                  callee != 0; callee = set1->overlap_info->next_entry())
  {
   found = false;
//------------------------------------
// check if it is a global variable

   if (callee->common_entry != 0)
    { 
     //-----------------------------------------------------------
     // look for the overlap entry in the caller
     // If it exists, then perform a union, else create a new entry

      found = false;
      for(overlap_ent* caller = this->overlap_info->first_entry();
       caller != 0 && !found; caller = this->overlap_info->next_entry())
        {
          //----------------------------------------------------- 
          // check to see if the global in the caller is the same
          // as the global in the callee
        if(caller->common_entry != 0)
         {
         if (globals_match(caller->common_entry, callee->common_entry))
          found = true;
          break;
         }
        }
//---------------------------------------------------------------
// if the global entry has been found, union callee's information

     if (found) {
      caller->union_t(callee);
      found = false;
      }  
//---------------------------------------------------------------
// if the global entry is not found, create a new entry 

     else {
     overlap_ent *e = new overlap_ent(callee, callee->name(), 
                                      callee->common_entry);
     overlap_info_temp->append_entry(e);
     }
    }
  }
//---------------------------------------------------------------
//  set1 = callee's annotation, in terms of formal parameters

//---------------------------------------------------------------------
// map a formal to an actual
//---------------------------------------------------------------------
 for (ParamNameIterator fni(edge1->paramBindings, FormalNameSet);
             fni.Current(); ++fni) {
     formal = fni.Current();

    ParamBinding *binding = 
      edge1->paramBindings.GetReverseBinding(formal);
      actual = binding->actual;

// for ( DictI t(&(edge1->bind)); t.k; t++) {
//  actual = (char*) t.k;
//  formal_info  = (struct bind_info *) t.v;
//  formal = formal_info->formal;

  found = found2 = false;


  for(overlap_ent* callee = set1->overlap_info->first_entry(); callee != 0 && !found;
                   callee = set1->overlap_info->next_entry())
  {
   if (strcmp(callee->name(), formal) == 0)
   {
   found = true;
   for(overlap_ent* caller = this->overlap_info->first_entry();
       caller != 0 && !found2; caller = this->overlap_info->next_entry())
    {
    if(strcmp(caller->name(), actual) == 0)
      {
      found2 = true;
      caller->union_t(callee);
      unioned = true;
      }
    }
    if (!unioned) 
     {
     overlap_ent *e = new overlap_ent(callee, (char *) actual);
     overlap_info_temp->append_entry(e);
     unioned = false;
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

int FD_Overlap_Annot::operator ==(const DataFlowSet &rhs) const
{
	return ((FD_Overlap_Annot *) this)->
		Diff((FD_Overlap_Annot *) &rhs);
}

//------------------------------------------------------------------
// compare each of the overlap entries in the list and check to see
// the entry names and their lower and upper ranges are the same
//------------------------------------------------------------------
Boolean FD_Overlap_Annot::Diff(FD_Overlap_Annot* out)
{
 overlap_ent *i, *j;

 //cout<<form(" count1 =   %d,   count2 = %d \n", this->overlap_info->entry_count(),out->overlap_info->entry_count());


// test1:  check to see if the count is the same
 if(this->overlap_info->Count() != out->overlap_info->Count())
   return(true);

 for(i = overlap_info->first_entry(),
     j = out->overlap_info->first_entry() ; i != 0; 
     i = overlap_info->next_entry(), 
     j = out->overlap_info->next_entry())
   {
     if (strcmp(i->name(), j->name()) == 0)
      {
       for(int k = 0; k < i->numdim; ++k)
        {
         if ((i->upper[k] != j->upper[k]) || ( i->lower[k] != j->lower[k]))
          return true;
        }
      }
   }
// if it reaches this point in the program return false. i.e. there is no
// difference between this and out annotations
  return false;
}



//------------------------------------------------------------------
// create a copy/clone of the annotation set_item passed as void *
//------------------------------------------------------------------
FlowGraphDFAnnot* FD_Overlap_Annot::Clone() const
{
 FD_Overlap_Annot *return_item;
 return_item = new FD_Overlap_Annot(overlap_info);

 return return_item;
}

//------------------------------------------------------------------
int FD_Overlap_Annot::ReadUpCall(FormattedFile *port)
{
 return read(*port);
}

//------------------------------------------------------------------
int FD_Overlap_Annot::WriteUpCall(FormattedFile *port)
{
 return write(*port);
}
