/* $Id: WriteReachAnnot.C,v 1.5 1997/03/11 14:35:06 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//-------------------------------------------------------------------
// author : Seema Hiranandani
// content: This file contains routines to write the interprocedural
//          fortran D node annotation that contains reaching
//          decomposition information
// date   : July 1992
//-------------------------------------------------------------------

//--------------------
// include files
#include <libs/fortD/misc/FortD.h>
#include <libs/ipAnalysis/problems/fortD/ReachAnnot.h>
#include <libs/ipAnalysis/problems/fortD/FortD_Sets.h>
#include <libs/ipAnalysis/ipInfo/iptree.h>

#include <libs/ipAnalysis/problems/fortD/FortDNumProcsAnnot.h>

//-------------------------
// extern declarations
//-------------------------
EXTERN (void, map_set_to_sp, (SNODE *sp,
                           FortranDHashTableEntry *array_entry, 
                           FortranDHashTableEntry *decomp_entry, 
                           int a_index, int d_index));

EXTERN (void, map_decomp_to_sp, (FortranDHashTableEntry *decomp_entry,
                           SNODE *sp, int d_index));

void map_decomp(SNODE *sp, int numProcessors);

//------------------------------------------------------------------
// initialize class FD_EntryCount
//------------------------------------------------------------------
FD_EntryCount::FD_EntryCount()
{
    int i;

    decomp_count =  distrib_count = align_count = 0;
    
   for(i = 0; i < FD_MAX; ++i)
    {
      distrib_entries[i]    = NULL;
      align_entries[i]      = NULL;
      align_decomp_ht[i]    = NULL;
      align_ht_entries[i]   = NULL;
      decomp_entries[i]     = NULL;
      align_decomp_names[i] = NULL;
      param_names[i]        = NULL;
    }
}

//---------------------------------------------------------------------
// counts the number of decompositions and store pointers to the
// decomposition entries
//---------------------------------------------------------------------
void FD_EntryCount::process_decomp(FD_Set *set)
{
 char strbuf[FD_MAX];
 FD_SetItem *set_item;

 for(set_item = set->first_entry(); set_item != NULL; 
     set_item = set->next_entry())
 {
  if (!set_item->f->marked) 
  {
  set_item->f->marked = true;
  set_item->f->number++; 

  sprintf(strbuf, "%s%d",  set_item->f->name(), set_item->f->number);
  set_item->f->name_unique = ssave(strbuf);

  decomp_entries[decomp_count] = set_item->f; // store the decomp entries

  ++decomp_count;
  }
 }   
}

//---------------------------------------------------------------------
// at the end of writing information for a procedure, mark the 
//  decomposition as not seen
//---------------------------------------------------------------------
void FD_EntryCount::cleanup_decomp(FD_Set *set)
{
 FD_SetItem *set_item;

 for(set_item = set->first_entry(); set_item != NULL; 
     set_item = set->next_entry())
 {
   set_item->f->marked = false;
 }
}

//---------------------------------------------------------------------
// counts the number of distributions and store pointers to the
// distribution entries
//---------------------------------------------------------------------
void FD_EntryCount::process_distrib(FD_Set *set)
{ 

 FD_SetItem *set_item;

//-----------------------------------
// walk through the list of set items

 for(set_item = set->first_entry(); set_item != NULL; 
     set_item = set->next_entry())
 {

//--------------------------------------------------------------
// if this distribute entry has not been previously encountered

  if (!set_item->f->d->distrib_info[set_item->fd->d_index()]->marked)
   {

//-------------------------------------------------------------
// set number = the entry number of this distribution for
//              this decomposition, i.e. the output count

    set_item->f->d->distrib_info[set_item->fd->d_index()]->marked = true;
    ++distrib_count;
   }
 }
}

//---------------------------------------------------------------------
// after writing the distrib information for a procedure, clean up the
// symbol table by resetting marked
//---------------------------------------------------------------------
void FD_EntryCount::cleanup_distrib(FD_Set *set)
{
 FD_SetItem *set_item;
 for(set_item = set->first_entry(); set_item != NULL; 
     set_item = set->next_entry())
 {
  set_item->f->d->distrib_info[set_item->fd->d_index()]->marked = false;
  set_item->f->d->distrib_info[set_item->fd->d_index()]->number = 0;
  set_item->f->d->output_number = 0;
 }
}


//---------------------------------------------------------------------
// counts the number of alignments and store pointers to the
// align entries
// It traverses the list of reaching decompositions for a particular
// formal paramter. It obtains the alignment information, a pointer
// to the array and decomposition hash table entries, and the formal
// parameter name
//---------------------------------------------------------------------
 void FD_EntryCount::process_align(FD_Set *set, char *formal_name)
{
 FD_SetItem *set_item;

 for(set_item = set->first_entry(); set_item != NULL; 
     set_item = set->next_entry())
 {
 if (!set_item->array_ht->d->align_info[set_item->fd->a_index()]->marked) 
  { 
  set_item->array_ht->d->align_info[set_item->fd->a_index()]->marked = true;
//  set_item->array_ht->d->align_info[set_item->fd->a_index()]->number =
//                                     set_item->array_ht->d->output_number++;

  align_entries[align_count] =
           set_item->array_ht->d->align_info[set_item->fd->a_index()];

  align_ht_entries[align_count] = set_item->array_ht;

  align_decomp_ht[align_count] = set_item->f;

  param_names[align_count] = formal_name;
  ++align_count;
  }
 }

// since the same array entry in the hash table may be used by formals
// of some other procedure, it is necessary to set the marked variable as
// false. The marked variable is used to ensure that the same alignment
// is not written out more than once for the same variable. This may occur
// if there are multiple edges to a procedure

 for(set_item = set->first_entry(); set_item != NULL; 
     set_item = set->next_entry())
 {
  set_item->array_ht->d->align_info[set_item->fd->a_index()]->marked = false;
 }
}

//---------------------------------------------------------------------
// after writing the align information for a procedure, clean up the
// symbol table by resetting marked
//---------------------------------------------------------------------
void FD_EntryCount::cleanup_align(FD_Set *set)
{
 FD_SetItem *set_item;

  for(set_item = set->first_entry(); set_item != NULL; 
      set_item = set->next_entry())
 {
 set_item->array_ht->d->align_info[set_item->fd->a_index()]->marked = false;
 set_item->array_ht->d->align_info[set_item->fd->a_index()]->number = 0;
 set_item->array_ht->d->output_number = 0;
 }
}

//-----------------------------------------------------------------
// write all the decompositions, alignments and distributes that
// reach the procedure
//-----------------------------------------------------------------
void FD_EntryCount::write(FormattedFile &port)
{
 int i, j, k, l;

//-------------------------------------
// write the number of decompositions

 port.Write(decomp_count);

//------------------------------------------------------------------
// for each decomposition write the details 
// example decomposition d(100,100)
//------------------------------------------------------------------

 for (i = 0; i < decomp_count; ++ i)
 {
  port.Write(decomp_entries[i]->name_unique, NAME_LENGTH);
  port.Write(decomp_entries[i]->getdim());

  for (j = 0; j < decomp_entries[i]->getdim(); ++j)
  {
   write_typedescript(port, decomp_entries[i]->d->idtype[j]);
  }

//--------------------------------------------------------------------------
// write the number of distribute entries associated with this decomposition
// example distribute d(block, :)
//--------------------------------------------------------------------------

   port.Write(distrib_count);

  // printf("number of distribute entries = %d \n", distrib_count);
  for(k = 0; k < DCMAX; ++k)
   {
    if (decomp_entries[i]->d->distrib_info[k]->marked)
    {
     decomp_entries[i]->d->distrib_info[k]->number = 
                                         decomp_entries[i]->d->output_number++;
     for (l=0; l<decomp_entries[i]->getdim(); ++l)
     port.Write(decomp_entries[i]->d->distrib_info[k]->distinfo[l].distr_type);
    }
   }
 }

//-------------------------------------------------------------------------
// for each alignment write the details
// example align a(i, j) with d(j, i)
// the hash table entry is stored in : align_ht_entries[align_count];
//-------------------------------------------------------------------------
 port.Write(align_count);
 
 for(i=0;i<align_count;++i)
  {

//-----------------------------
// write name of decomposition
   port.Write(align_decomp_ht[i]->name_unique, NAME_LENGTH);

//-----------------------------
// write the name of the parameter
   port.Write(param_names[i], NAME_LENGTH);

//--------------------------------
// write the number of dimensions
   port.Write(align_ht_entries[i]->getdim());

//-----------------------------------------
// write the type i.e. real, integer, ....
  port.Write(align_ht_entries[i]->type);

  for (j = 0; j < align_ht_entries[i]->getdim(); ++j)
  {
   write_typedescript(port, align_ht_entries[i]->d->idtype[j]);
  }

//---------------------------------
// write the alignment information
   align_entries[i]->Write(port);
  }
}

//-------------------------------------------------------------
// 1. count number of decomposition entries to be written 
// 2. write the decomposition information
// 3. count number of distribute entries to be written
// 4. write the distribute information
// 5. count the number of alignment entries to be written 
// 6. write the alignment details
// 7. write the set information 
//-------------------------------------------------------------
void FD_Set::write(FormattedFile& port)
{
  FD_SetItem *set_item;
  char *decomp_name;
  int align_index, distrib_index;

  port.Write(this->Count());
  if(Count() != 0) {
   port.Write(name(), NAME_LENGTH); 
   }
  for (set_item = first_entry(); set_item != NULL; set_item = next_entry())
   {
    decomp_name = set_item->f->name_unique;

    align_index = 
        set_item->array_ht->d->align_info[set_item->fd->a_index()]->number;

    distrib_index = 
       set_item->f->d->distrib_info[set_item->fd->d_index()]->number;

    port.Write(decomp_name, NAME_LENGTH);
    port.Write(align_index);
    port.Write(distrib_index);
   }
}

//----------------------------------------------------------------
// Write the number of nodes i.e. procedures
// For each procedure  write the number of 
// formal parameters. For each parameter
// write the decomposition information
//----------------------------------------------------------------
void FD_Reach_Annot::write(FormattedFile& port)
{
 int i;
 port.Write("FD_NODE_ANNOTATION", NAME_LENGTH);

 FD_EntryCount d_entries = FD_EntryCount();

//--------------------------------
// write the number of parameters 
//--------------------------------
   port.Write(FD_MAX_PARAM);
//------------------------------------------------
// compute the counts  for the number of align, 
// distribute, decomposition entries
//------------------------------------------------
 for(i=0;i<FD_MAX_PARAM;++i)
  {
  d_entries.process_decomp(fortd_set[i]);
  d_entries.process_distrib(fortd_set[i]);
  d_entries.process_align(fortd_set[i], fortd_set[i]->name());
  }

//------------------------------------------
// write the decompositions/align/distribute
  d_entries.write(port);

//--------------------------------
// write the set information out

 port.Write("BEGIN_SET_ENTRY", NAME_LENGTH);
    for(i=0;i<FD_MAX_PARAM; ++i)
   {
    fortd_set[i]->write(port);
   }

 port.Write("END_FD_NODE_ANNOTATION" ,NAME_LENGTH);

//---------------------------
// clean up the temporaries
   for (i=0; i<FD_MAX_PARAM; ++i)
 { 
  d_entries.cleanup_decomp(fortd_set[i]);
  d_entries.cleanup_distrib(fortd_set[i]);
  d_entries.cleanup_align(fortd_set[i]);
 }
}

//-------------------------------------------------------
// map the set information to the hash table
//-------------------------------------------------------
void FD_Set::map_set_info(FortranDInfo *f)
{
 FD_SetItem *set_item;
 char* formal_name, *decomp_name;
 int i, align_index, distrib_index, dist_index;
 FortranDHashTableEntry *formal_entry, *decomp_entry;

 for(i=0;i<this->Count(); i++) {
  formal_name = name();
   
   for (set_item = first_entry(); set_item != NULL; set_item = next_entry())
    {
    decomp_name = set_item->f->name_unique;
    align_index = 
         set_item->array_ht->d->align_info[set_item->fd->a_index()]->number;
    distrib_index = 
              set_item->f->d->distrib_info[set_item->fd->d_index()]->number;

    formal_entry = f->GetEntry(formal_name);
    decomp_entry = f->GetEntry(decomp_name);

     if (formal_entry != 0) 
      {
      dist_index = formal_entry->d->dist_index;
      formal_entry->d->distrib_index[dist_index] = dist_index;
      formal_entry->d->dist_index++;
      map_set_to_sp(&formal_entry->d->sp[align_index][dist_index],formal_entry,
      decomp_entry, align_index, distrib_index);
      }
    }
  }
}

//--------------------------------------------------------------
//--------------------------------------------------------------
FortranDInfo* FD_EntryCount::map_reach_annot(CallGraph *cg)
{
  int i,j,k,l, d_index, a_index;
  FortranDInfo *f = new FortranDInfo();
  FortranDHashTableEntry *d1, *array_entry, *decomp_entry;

//----------------------------------------------
// map the decomposition information
//----------------------------------------------
  for(i=0; i < decomp_count; ++i)
  {
   d1 =  f->AddDecomp(decomp_entries[i]->name_unique, DECOMPTYPE,
                   decomp_entries[i]->getdim());
//------------------- map the decomp id number --------------------//
  d1->d->decomp_id_number =  decomp_entries[i]->d->decomp_id_number;

#if 0
//---------- map the context of the decomposition comment---------//
  d1->d->decomp_context = decomp_entries[i]->d->decomp_context;
#endif

   for(j = 0; j < decomp_entries[i]->getdim(); j++)

// -------- map the typedescript information ------------

copy_typedescript(d1->d->idtype[j], decomp_entries[i]->d->idtype[j]);

//--------- map the distribute information ------------


   FortDNumProcsAnnot *nProcsAnnot = (FortDNumProcsAnnot *) 
    cg->GetAnnotation(FORTD_NUM_PROCS_ANNOT, true);
   
   assert(nProcsAnnot);

  
  /* do this prior to closing ftt and ft */

 if(distrib_count > 1)
   printf("Cloning not supported in the Fortran D Compiler \n");

  for(k = 0; k < DCMAX; ++k)
   {
   if (decomp_entries[i]->d->distrib_info[k]->marked)
    {
     d_index = d1->d->d_index;
     decomp_entries[i]->d->distrib_info[k]->number =
                                 decomp_entries[i]->d->output_number++;

      d1->d->distrib_info[d_index]->id_number = 
                         decomp_entries[i]->d->distrib_info[k]->id_number;
      for (l = 0; l < decomp_entries[i]->getdim(); ++l)
       {
        Dist_type dist_type = decomp_entries[i]->d->distrib_info[k]->
                        distinfo[l].distr_type;

        if(dist_type == FD_DIST_LOCAL)
          put_distrib_info2(dist_type, 0, 
                           &(d1->d->distrib_info[d_index]->distinfo[l]));

        else if ((dist_type == FD_DIST_BLOCK) || 
                 (dist_type == FD_DIST_CYCLIC) || 
                 (dist_type == FD_DIST_BLOCK_CYCLIC)) 
          put_distrib_info(dist_type,
                          &(d1->d->distrib_info[d_index]->distinfo[l]));
      }
      map_decomp_to_sp(d1, &d1->d->sp[0][d1->d->d_index], d_index);
      map_decomp(&d1->d->sp[0][d_index], nProcsAnnot->numberProcs);
      d1->d->d_index++;
    }
   }
  }
 
//----------------------------------------------
// map the align information
//----------------------------------------------

  for(i = 0; i< align_count; ++i) {
  array_entry = f->GetEntry(param_names[i]);
   if (!array_entry)
    {
     array_entry = f->AddDecomp(param_names[i], ARRAYTYPE,
                                align_ht_entries[i]->getdim());
     array_entry->d->dec_name = ssave(align_decomp_ht[i]->name_unique);
     array_entry->type = align_ht_entries[i]->type;

//----------store the arrays aligned to the decomposition in a list----------
  
    decomp_entry = f->GetEntry(align_decomp_ht[i]->name_unique);
    decomp_entry->d->name_info->append_entry(param_names[i]);
    a_index = array_entry->d->a_index;
  
   for(j =0; j< align_ht_entries[i]->getdim(); ++ j)
    {
     copy_typedescript
             (array_entry->d->idtype[j],align_ht_entries[i]->d->idtype[j]);
    }
      copy_align_list
             (array_entry->d->align_info[a_index], align_entries[i]);
   ++array_entry->d->a_index;
  }
 }
 return f;
}


//--------------------------------------------------------------
// after the dataflow problem has been solved map the final
// reaching annotation to a form usable by the code generator
//--------------------------------------------------------------
void FD_Reach_Annot::MapReachNodeAnnot(CallGraphNode *node)
{
  FD_EntryCount d_entries = FD_EntryCount();
  int i;

  for (i=0; i<FD_MAX_PARAM; ++i)
  {
   d_entries.process_decomp(fortd_set[i]);
   d_entries.process_distrib(fortd_set[i]);
   d_entries.process_align(fortd_set[i], 
                           fortd_set[i]->name());
  }

  f =  d_entries.map_reach_annot(node->GetCallGraph());

//------------ store the set entry information--------------
  for(i=0;i<FD_MAX_PARAM;++i)
   fortd_set[i]->map_set_info(f);

  for (i=0; i<FD_MAX_PARAM; ++i)
  { 
  d_entries.cleanup_decomp(fortd_set[i]);
  d_entries.cleanup_distrib(fortd_set[i]);
  d_entries.cleanup_align(fortd_set[i]);
  }
}




