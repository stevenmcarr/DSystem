/* $Id: ReachAnnot.C,v 1.5 1997/03/27 20:41:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//-------------------
// include files

#include <stdio.h>
#include <libs/fortD/misc/FortD.h>
#include <libs/ipAnalysis/problems/fortD/ReachAnnot.h>
#include <libs/ipAnalysis/problems/fortD/ReachDFProblem.h>
#include <libs/ipAnalysis/ipInfo/iptree.h>

#include <libs/fortD/localInfo/CommBlock.h>
#include <libs/ipAnalysis/problems/fortD/CommonBlockAnnot.h>
#include <libs/ipAnalysis/problems/fortD/FortD_Sets.h>
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphIterators.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotReg.h>
#include <libs/ipAnalysis/callGraph/CallSiteParamBindings.h>

#include <libs/support/strings/OrderedSetOfStrings.h>
#include <libs/support/strings/StringBuffer.h>
#include <libs/ipAnalysis/problems/fortD/ReachAnnotString.h>

extern char *FORTD_COMMON_BLOCK_ANNOT;

//--------------------------------------------------------------------
// forward declaration

Boolean FDSetEntry_isEqual(FDSetEntry* f, FDSetEntry* f2, 
FortranDHashTableEntry *decomp_ht, FortranDHashTableEntry *array_ht1,
FortranDHashTableEntry *array_ht2);

//------------------------------------------------------------------
// constructor for FD_Reach_Annot class
//------------------------------------------------------------------
FD_Reach_Annot::FD_Reach_Annot(): 
   FlowGraphDFAnnot(FORTD_REACH_ANNOT) // initialize parent class 
{
     for(int i=0;i<FD_MAX_PARAM;++i)
      fortd_set[i] = new FD_Set; 
    f = new FortranDInfo();
}

//------------------------------------------------------------------
// constructor for FD_Reach_Annot class
//------------------------------------------------------------------
FD_Reach_Annot::FD_Reach_Annot(Boolean is_read): 
   FlowGraphDFAnnot(FORTD_REACH_ANNOT) // initialize parent class 
{
    int i;
    count = 0;
    if (!is_read) 
    {
     for(i=0;i<FD_MAX_PARAM;++i)
      fortd_set[i] = new FD_Set; 
    }
    else 
    {
     for(i=0;i<FD_MAX_PARAM;++i)
      read_set[i] = new FDSetEntryList; 
    }
    f = 0;

}

//------------------------------------------------------------------
// constructor for FD_Reach_Annot class
//------------------------------------------------------------------
FD_Reach_Annot::FD_Reach_Annot(FD_Set** fortd_set1, FortranDInfo *ff) :
   FlowGraphDFAnnot(FORTD_REACH_ANNOT) // initialize parent class
		{
     int i;
     for(i=0; i<FD_MAX_PARAM;++i)
     fortd_set[i] = fortd_set1[i];
     f = ff;
    }

//------------------------------------------------------------------------
// extract all the common block information collected during the local
// phase and attached to the call site entry. It contains decomposition set
// information for each array  and details on the common block it resides
// in
//------------------------------------------------------------------------
int FD_Reach_Annot::CreateCommonBlockAnnot(CallSite *c_entry, FortranDInfo *f, 
CallGraphNode *callee)
{
 int i = 0;
 int count1;
 FDSetEntryList *fdset_list;
 FDSetEntry *l_entry;
 FortranDHashTableEntry *hash_table_decomp, *hash_table_array;
 FD_CommBlkNodeAnnotation *c = 
  (FD_CommBlkNodeAnnotation *)callee->GetAnnotation(FORTD_COMMON_BLOCK_ANNOT);

 for(common_block_ent *e = f->common_block_info->first_entry(); 
               e != 0; e = f->common_block_info->next_entry())
  {
//---------------------------------------------------       
// for each decomposition reaching the array
// append the fortd_set_entry information and the 
// hash table entry for the array and decomposition


  fdset_list = (FDSetEntryList*)(c_entry->fortd_set[i]);
  if (fdset_list != 0)
  {
  count1 = fdset_list->Count();
  for(l_entry =  fdset_list->first_entry(); l_entry != 0;
      l_entry= fdset_list->next_entry())
  {
   hash_table_decomp = f->GetEntry(l_entry->name());
   hash_table_array  = f->GetEntry(e->name);
   fortd_set[i]->append_entry
                      (hash_table_decomp, hash_table_array,l_entry, e);
  }
  common_block_ent *common_entry = c->translate_global(e);
  if(common_entry != 0)
  fortd_set[i]->save_name(common_entry->name);

  else
  {
  fortd_set[i]->save_name(e->name);
  }
 
 ++i;
  }
 }

 return i; // start at next entry 
}

//---------------------------------------------------------------------
// walk the list of decomposition set entries belonging to the
// caller (set1). For actuals, find the corresponding formal entry
// and union the set of the actual parameter with the set of the
// formal parameter
// for global variables, find the corresponding global variable
// in the callee. If it exists, then union its information, else
// create a new entry. At this point also check to see if the
// common block layout of the caller and callee are consistent
//---------------------------------------------------------------------
void FD_Reach_Annot::Union(CallGraphNode *node1, CallGraphEdge *edge1, FD_Reach_Annot *set1)
{
 int i,j;
 const char *formal, *actual;
 ParamBinding *formal_bi;
 FD_Set *fortd_set1;
 j =0;
 CallGraphNode *caller = edge1->Caller();
 FD_CommBlkNodeAnnotation *caller_annot = 
  (FD_CommBlkNodeAnnotation *)caller->GetAnnotation(FORTD_COMMON_BLOCK_ANNOT);

 //--------------------------------------------------------------
 // for each actual parameter passed at the callsite 
 //--------------------------------------------------------------
 for (ParamNameIterator ani(edge1->paramBindings, ActualNameSet);
      ani.Current(); ++ani) {
      actual = ani.Current();

 //-----------------------------------------------------------------
 // retrieve the set of bindings between the actual and the formals
 //-----------------------------------------------------------------
     ParamBindingsSet *bindings = 
         edge1->paramBindings.GetForwardBindings(actual);
       int formal_count = 0;
       for (ParamBindingsSetIterator bi(bindings); 
            formal_bi = bi.Current(); ++bi)
          {
          if(formal_count > 0) {
           // cout<<form(" The Fortran D compiler does not handle aliases \n");
          }
          formal = formal_bi->formal;
          ++formal_count;

	  // the cast ops to char * are a hack. these funcs should all
	  // take const char * as arg. -- JMC 94
  if (caller_annot->fort_d_is_common((char *) actual))
   {
   actual = caller_annot->fort_d_get_common_entry_nme
            ((char *) actual, formal_bi->a_offset, formal_bi->a_length);
   }

//-------------------------------------------------------------------
// for each actual parameter, find corresponding formal if it exists
// Union the decomposition set of the formal with the actual

  for(i=0;i<FD_MAX_PARAM;i++) {
   fortd_set1 = set1->fortd_set[i];

   //---------------------------------------------------------
   // find the fortd_set corresponding to the actual parameter
 
   if(fortd_set1->name() != NULL){
     if (!strcmp(actual, fortd_set1->name())){

   //---------------------------------------------------------
   // find the set entry corresponding to the formal parameter

     for (j = 0; j < FD_MAX_PARAM; ++j)
       if (!strcmp(formal, fortd_set[j]->name())){
       fortd_set[j]->Union(fortd_set1);
       break;
      }
     }
    }
   }
 }
}
  UnionG(node1, edge1, set1);
}

//---------------------------------------------------------------------
// union fortd_set1 (caller's annot ) with 'this' (callee's annot)
//---------------------------------------------------------------------
void FD_Set::Union(FD_Set *f2, common_block_ent *callee_global_entry)
{
FD_SetItem *set_item, *f_set_item, *f_setcopy;
Boolean exists;

//---------------------------------------------------------------
// go through all the items in the set and check if they already
// exist in the first set. If an item does not exist append it 

 for(f_set_item =  f2->first_entry(); f_set_item != NULL; 
     f_set_item = f2->next_entry()) 
  {

   exists = false;

//------------------------------
// check if the set item exists

   for(set_item = first_entry(); set_item != NULL; set_item = next_entry())
    {
     if ((set_item->f == f_set_item->f) && 
      (FDSetEntry_isEqual(set_item->fd,f_set_item->fd, set_item->f,
                           set_item->array_ht, f_set_item->array_ht))) 
       exists = true; 
    }
    if (!exists) 
    {
     f_setcopy = f_set_item->copy(callee_global_entry);
     append_entry(f_setcopy);
    }
  }
}
 
//----------------------------------------------------------------------
// set1 belongs to the caller
// for global variables, find the corresponding global variable
// in the callee. If it exists, then union its information, else
// create a new entry. At this point also check to see if the
// common block layout of the caller and callee are consistent
//-----------------------------------------------------------------------
void FD_Reach_Annot::UnionG(CallGraphNode *node1, CallGraphEdge *edge1, FD_Reach_Annot *set1)
{
 char *nme;
 int index = 0;
 CallGraphNode *callee = node1;
 CallGraphNode *caller = edge1->Caller();
 common_block_ent* callee_global_entry = 0;
 common_block_ent* common_block_entry = 0;
 Boolean add_entry = true;
 Boolean found = false;
 FD_SetItem *set_item;

 FD_CommBlkNodeAnnotation *callee_annot = 
 (FD_CommBlkNodeAnnotation *)callee->GetAnnotation(FORTD_COMMON_BLOCK_ANNOT);
 FD_CommBlkNodeAnnotation *caller_annot = 
 (FD_CommBlkNodeAnnotation *)caller->GetAnnotation(FORTD_COMMON_BLOCK_ANNOT);

//----------------------------------------------------------------------
// go through the list of set entries in the caller annot, check to see
// if it's a global variable. If true, then search the callee_annot for
// the entry. If it exists then search the callee set for the entry. if
// it exists, union the information of the caller else, create a new
// entry.


  for(int i = 0; i < FD_MAX_PARAM; ++i) {
   FD_Set *fortd_set1 = set1->fortd_set[i];
   callee_global_entry = 0;

//----------------------------------------------------------------
// check to see if the entry is a global entry
// it is a global entry if the common_block_ent (c) is not null

   if (fortd_set1 != 0) {
     set_item = fortd_set1->first_entry();
     if (set_item != 0)
      {
       if(set_item->c != 0)
       {
        callee_global_entry = callee_annot->translate_global(set_item->c);

        if (callee_global_entry == 0) 
        {
        nme = fortd_set1->name();
        common_block_entry = set_item->c;
        }
        else
        {
        nme = callee_global_entry->name;
        common_block_entry = callee_global_entry;
        }
//----------------------------------------------------------------
// check to make sure the entry is not zero. if its not 0,
// search the fortd_set entries of the callee for e->name

      add_entry = true;
      found = false;
      for(int j=0,found = false; j < FD_MAX_PARAM && !found; ++j)
      {
//----------------------------------------------------------------
// if it exists then perform a union of the fortd_set entries, else
// create a new entry

	  // 15 Sept 1994 -- it appears that the name() == NULL is
	  // an implicit communication between 
	  // FD_Reach_Annot::Union(FD_Set *, ...) and this function.
	  // HIGHLY SUSPICIOUS. it would appear that 
	  // FD_Reach_Annot::Union(FD_Set *, ...) and 
	  // FD_Reach_Annot::UnionG(..) have to be called in that
	  // order.
       if (fortd_set[j]->name() == 0)
        {
         found = true;
         index = j;
        }
       else if (strcmp(fortd_set[j]->name(), nme) == 0)
        {
        fortd_set[j]->Union(fortd_set1, common_block_entry);
        found =  true;
        add_entry = false;
        }     
       }
     if (add_entry) {
       fortd_set[index]->save_name(nme);
       fortd_set[index]->Union(fortd_set1, common_block_entry);
     }
    }
   }
  }
 }
}

//------------------------------------------------------------------
// union 2 Fortran D set entries
//------------------------------------------------------------------
void FD_Set::Union(FD_Set *f2)
{
FD_SetItem *set_item, *f_set_item, *f_setcopy;
Boolean exists;

//---------------------------------------------------------------
// go through all the items in the set and check if they already
// exist in the first set. If an item does not exist append it 

 for(f_set_item =  f2->first_entry(); f_set_item != NULL; 
     f_set_item = f2->next_entry()) 
  {

   exists = false;

//-------------------------------
// check if the set item  exists

   for(set_item = first_entry(); set_item != NULL; set_item = next_entry())
    {
     if ((set_item->f == f_set_item->f) && 
      (FDSetEntry_isEqual(set_item->fd,f_set_item->fd, set_item->f,
                           set_item->array_ht, f_set_item->array_ht)))
       exists = true; 
    }
    if (!exists) 
    {
     f_setcopy = f_set_item->copy();
     append_entry(f_setcopy);
    }
  }
}

//------------------------------------------------------------------
// f1 = edge set, f2 =  callee annotation
//------------------------------------------------------------------
void FD_Set::Union(FD_Set *f1, FD_Set *f2)
{
FD_SetItem *f_set1, *f_set2, *f_setcopy;
Boolean done;

// FD_SetItem *s;
// for (s = this->first_entry(); s != NULL; s = this->next_entry())
//  cout << s->f << s->fd->a_index() << s->fd->d_index() 
//       << s->fd->name() << flush;

//----------------------------------------
// copy all the entries in f1 into (this)

 for(f_set1 = f1->first_entry(); f_set1 != NULL; 
     f_set1 = f1->next_entry()) 
 {
  f_setcopy =  f_set1->copy();
  append_entry(f_setcopy);
 }
  done = true;

//--------------------------------------------------------------
// look for entries in f2 that do not exist in f1. Append those
// entries to (this)

 for(f_set2 = f2->first_entry(); f_set2 != NULL;
     f_set2 = f2->next_entry()) 
 {
   done = false;
  
   for(f_set1 = f1->first_entry(); f_set1 != NULL; 
       f_set1 = f1->next_entry()) 
    {
     if ((f_set1->f == f_set2->f) &&
         FDSetEntry_isEqual(f_set1->fd, f_set2->fd, f_set1->f, 
                          f_set1->array_ht, f_set2->array_ht))
      done = true;
    }
    if (!done)
     { 
//    cout << " num entries  = " << f2->Count() << flush;
      f_setcopy = f_set2->copy();
      append_entry(f_setcopy);
//    cout << f_set2->f << f_set2->fd->a_index() 
//    << f_set2->fd->d_index() << f_set2->fd->name() << '\n' << flush;
//    cout<<form(" appended %s \n", f_set2->f->name());
     }
  }
}
 
//------------------------------------------------------------------
// n1 = callee's annotation, union, the edge annotation (f)
//------------------------------------------------------------------
void FD_Reach_Annot::Union(FD_Reach_Annot *n1, FD_Reach_Annot *edgeAnnot)
{
  // the assert below was added since the logic is wrong for the case when
  // either n1 or edgeAnnot is NULL but the other is non-NULL since 
  // the union should contain at least the entries from the non-NULL set
  // 6 Sept 1994 -- John Mellor-Crummey
  assert(n1 && edgeAnnot); 

  if (!n1 || !edgeAnnot) return;

  for(int i = 0; i < FD_MAX_PARAM; ++i) 
  {
     if (edgeAnnot->fortd_set[i]->name() == NULL)
     { 
	  // 15 Sept 1994 -- it appears that the nme = NULL below
	  // is an implicit communication between  this function and
	  // FD_Reach_Annot::UnionG(..).
	  // HIGHLY SUSPICIOUS. it would appear that 
	  // FD_Reach_Annot::Union(FD_Set *, ...) and 
	  // FD_Reach_Annot::UnionG(..) have to be called in that order.
       fortd_set[i]->nme = NULL;
     } else {
        fortd_set[i]->nme = ssave(edgeAnnot->fortd_set[i]->name());
     }

     fortd_set[i]->Union(edgeAnnot->fortd_set[i], n1->fortd_set[i]);
  }
}


//------------------------------------------------------------------
// Compare 2 fortran D sets. If they differ return true else return
// false
//------------------------------------------------------------------
Boolean FD_Set::Diff(FD_Set *f_set)
{
FD_SetItem *f1, *f2;
 
 if (f_set->Count() != Count())
 {
  return(true);
 }

 f2 = first_entry();
 for (f1 = f_set->first_entry(); f1 != NULL; f1 = f_set->next_entry())
  { 
  if ((f1->f != f2->f) || 
      (!FDSetEntry_isEqual(f1->fd,f2->fd, f1->f, f1->array_ht, f2->array_ht)))
   return(true);
   f2 = next_entry();
  }
 return(false);
}


int  FD_Reach_Annot::operator == (const DataFlowSet &rhs) const
{
  return ((FD_Reach_Annot*) this)->Diff((FD_Reach_Annot*) &rhs) ? 0 : 1;
}

//----------------------------------------------------------------------
// Compare two fortran d reach annotations. If they differ return true
// else return false
//----------------------------------------------------------------------
Boolean FD_Reach_Annot::Diff(FD_Reach_Annot* out)
{
 int i;
 int  entry_count = fortd_set[0]->Count();
 int  entry_c = out->fortd_set[0]->Count();
 
 for(i=0;i<FD_MAX_PARAM;++i)
  {
     if(fortd_set[i]->Diff(out->fortd_set[i]))
    return(true);
  }
 return(false);
}

//------------------------------------------------------------------
// create a copy/clone of the annotation set_item passed as void *
//------------------------------------------------------------------
FlowGraphDFAnnot* FD_Reach_Annot::Clone() const
{
 FD_Reach_Annot *return_item;
 return_item = new FD_Reach_Annot((FD_Set**)fortd_set, (FortranDInfo *) f);

 return return_item;
}

int FD_Reach_Annot::ReadUpCall(FormattedFile *port)
{
#if 0
 read(*port);
 return true;
#endif
 return 1; // graceful failure; this should be fixed MUCH later
}

int FD_Reach_Annot::WriteUpCall(FormattedFile *port)
{
#if 0
 write(*port);
 return true;
#endif
 return 1; // graceful failure; this should be fixed MUCH later
}

#if 0
Boolean FD_Reach_Annot::Write(FormattedFile &port)
{
 FlowGraphDFAnnot::Write(&port);
 write(port);
 return true;
}
#endif

//-----------------------------------------------------------------------
// this function checks to see if two arrays are aligned and distributed
// in the same way
//-----------------------------------------------------------------------
Boolean FDSetEntry_isEqual(FDSetEntry* f, FDSetEntry* f2, 
FortranDHashTableEntry *decomp_ht, FortranDHashTableEntry *array_ht1,
FortranDHashTableEntry *array_ht2)
{
//----------------------------------------------------------------------
// if the align indexes are not the same, check to see if the arrays are 
// aligned in the same way

  if (f->a_index() != f2->a_index())
   if (!(array_ht1->d->align_info[f->a_index()]->
         isEqual(array_ht2->d->align_info[f->a_index()])))
     return false;

//----------------------------------------------------------------------    
// if the distribute indexes are not the same, check to see if the
// arrays are distributed in the same way

  if (f->d_index() != f2->d_index())
    if(!(decomp_ht->d->distrib_info[f->d_index()]->
       isEqual(decomp_ht->d->distrib_info[f2->d_index()],decomp_ht->numdim)))
      return false;

   if (strcmp(f->name(), f2->name()) != 0)
    return false;

  return true;
}

// things to do:
// 1. structure the string for 1 decomposition
// 2. get the list of formals
// 3. get the list of globals

OrderedSetOfStrings* 
FD_Reach_Annot::CreateOrderedSetOfStrings()
{
 int i;
 FD_Reach_Annot_String fd_string(this);
 StringBuffer string(80);
 OrderedSetOfStrings *s = new OrderedSetOfStrings;
 Boolean done = false;

 string.Append("%s: \n", FORTD_REACH_ANNOT);
 
// walk the list of formals 
  for(i=0; done == false; ++i)
  {
   if(fortd_set[i] == 0)
    done = true;

   else if(fortd_set[i]->name() == 0)
    done = true;

   else
   {
    fd_string.CreateDecompositionString(i, &string);   
    fd_string.CreateAlignString(i, &string);
    fd_string.CreateDistributeString(i,&string);
   }
  }
 s->Append(string.Finalize());
 return s;
}


