/* $Id: FortDHash.C,v 1.24 1997/03/27 20:33:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <assert.h>
#include <iostream.h>
#include <stdlib.h>
#include <string.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#include <libs/fortD/misc/FortD.h>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/fortD/localInfo/fd_symtable.h>

#include <libs/ipAnalysis/ipInfo/CallSite.h>

//////////////////////////////////////////////////////////////////
//
//  FortranDHashTableEntry Class Members
//
//////////////////////////////////////////////////////////////////

//---------------------------------------------------
// FortranDHashTableEntry constructor
//---------------------------------------------------
FortranDHashTableEntry::FortranDHashTableEntry()
{
   nameD = (char*)0;
   numdim = 0;
   fform = UNKNOWN;
   type = UNKNOWN;
   name_unique = (char*)0;
   d = (DecEntry*)0;
   marked = false;
   number = 0;
}

//---------------------------------------------------
// FortranDHashTableEntry destructor
//---------------------------------------------------
FortranDHashTableEntry::~FortranDHashTableEntry()
{
//   if (nameD) sfree(nameD);
//   delete d;
}

//---------------------------------------------------
// Initialize FortranDHashTableEntry
//---------------------------------------------------
void
FortranDHashTableEntry::init(enum FORM fm, char *nme, int dim)
{
  type = UNKNOWN;
  number = 0;
  marked = false;
  name_unique = (char*)0;

  fform = fm;
  nameD = ssave(nme);
  numdim = dim;
  d = new DecEntry(fm);    
}

//-----------------------------------------------------
// hash the name of the entry
//-----------------------------------------------------
unsigned int
FortranDHashTableEntry::hash(unsigned int size)
{
  return hash_string(nameD, size);
}

//-----------------------------------------------------
// compare names 
//-----------------------------------------------------
int
FortranDHashTableEntry::compare(FortranDHashTableEntry* e1)
{
  return strcmp(this->nameD, e1->nameD);
}

//-----------------------------------------------------
// get the name
//-----------------------------------------------------
char*
FortranDHashTableEntry::name()
{
   return nameD;
}

//-----------------------------------------------------
// change the name
//-----------------------------------------------------
void
FortranDHashTableEntry::add_name(char* name)
{
//   if (nameD) sfree(nameD);

   nameD = ssave(name); 
}

//-----------------------------------------------------
// get the dimension
//-----------------------------------------------------
int
FortranDHashTableEntry::getdim()
{
   return numdim;
}

//-----------------------------------------------------
// previously not commented
//-----------------------------------------------------
SNODE* 
FortranDHashTableEntry::sp(int align_index, int dist_index)
{ 
   return &d->sp[align_index][dist_index];
}

//-----------------------------------------------------
// previously not commented
//-----------------------------------------------------
void
FortranDHashTableEntry::put_form(enum FORM fform1) 
{ 
   type = fform1; 
}


//////////////////////////////////////////////////////////////////
//
//  FortranDInfo Class Members
//
//////////////////////////////////////////////////////////////////

//-----------------------------------------------------
// derived function for class HashTable that
// invokes the hash member function for a 
// FortranDHashTableEntry instance.
//-----------------------------------------------------
uint
FortranDInfo::HashFunct(const void *entryV, const uint size)
{
  FortranDHashTableEntry* entry = (FortranDHashTableEntry*)entryV;

  return entry->hash(size);
}

//-----------------------------------------------------
//-----------------------------------------------------
int 
FortranDInfo::EntryCompare(const void *entryV1, const void *entryV2)
{
  FortranDHashTableEntry* entry1 = (FortranDHashTableEntry*)entryV1;
  FortranDHashTableEntry* entry2 = (FortranDHashTableEntry*)entryV2;

  return entry1->compare(entry2);
}

//-----------------------------------------------------
// Store the decomposition info in the hash table                
//-----------------------------------------------------
FortranDHashTableEntry*
FortranDInfo::AddDecomp(char *name, enum FORM fform, int numdim)
{
  FortranDHashTableEntry  e; 
  FortranDHashTableEntry* found = (FortranDHashTableEntry*)0; 

  e.add_name(name);
  found = (FortranDHashTableEntry*)HashTable::QueryEntry((const void*)&e);
  if (found == 0) 
  {
     e.init(fform, name, numdim);
     HashTable::AddEntry((void*)&e);
     found = (FortranDHashTableEntry*)HashTable::QueryEntry((const void*)&e);
     return found;
  }
  else 
  {
     return found;
  }
}

//-----------------------------------------------------
// Find a particular entry in the hash table
//-----------------------------------------------------
FortranDHashTableEntry*
FortranDInfo::GetEntry(char* name)
{
  FortranDHashTableEntry  e;
  FortranDHashTableEntry* found = (FortranDHashTableEntry*)0; 

  e.add_name(name);

  found = (FortranDHashTableEntry*)HashTable::QueryEntry((const void*)&e);

  return found;
}

//------------------------------------------------------------------------
// if the entry is for a decomposition then store a list of names
// of arrays aligned to the decomposition and information about
// the distribution
// if the entry is for an array, store the alignment information of
// the array, the index of the distrib_info that contains distribution
// information and the name of the decomposition the array is aligned with
//-------------------------------------------------------------------------
DecEntry::DecEntry(enum FORM formm)
{
  int i, j;
  current_index = d_index = a_index = n_index = dist_index = 0;

  for(i=0;i<DCMAX;++i) distrib_index[i] = 0;

  if (formm == DECOMPTYPE)
    {
       name_info = new NameList();
       for(i = 0; i < DCMAX; ++i) distrib_info[i] = new DistribEntryArray();
    }

  if (formm == ARRAYTYPE)
    {
       for(i = 0; i < DCMAX; i++) align_info[i] = new AlignList();
    }

  dec_name = NULL;

     // used during the interprocedural phase
  unique_name = NULL;
  output_number = 0;

  for(i = 0; i < DC_MAXDIM; ++i) memset(&idtype[i], 0, sizeof(TYPEDESCRIPT));

  for(i = 0; i < DCMAX; ++i)
    {
       for(j = 0; j < DCMAX; ++j) memset(&sp[i][j], 0, sizeof(SNODE));
    }
}

//-------------------------------------------------------------
//-------------------------------------------------------------
void DecEntry::AddBounds(int dim, int up, int lb, Expr_type expr_lo, Expr_type expr_up )
{
 TYPEDESCRIPT *id_type;

  id_type = &(idtype[dim]);

  if (expr_lo == Expr_constant)
    {
      expr_lower(&id_type->lo, expr_lo, lb);
    }
  else 
    {
      cout << "DecEntry::AddBounds, Non constant Bounds \n";
    }
 
  if (expr_up == Expr_constant)
    {
      expr_upper(&id_type->up, expr_up, up);
    }
  else
    {
      cout << "DecEntry::AddBounds, Non constant Bounds \n";
    }

  return;
}

//----------------------------------------------------------
// store decomposition information           
//----------------------------------------------------------
void DecEntry::AddDecEntry(struct dc_id id, int numdim)
{
  int i;
  TYPEDESCRIPT *id_type;
 
  for (i = 0; i < numdim; i++)
    {
      id_type = &(idtype[i]);  

      if (id.subs.expr[i].type == value)
        {
          expr_lower(&id_type->lo, Expr_constant, 1);
          expr_upper(&id_type->up, Expr_constant, id.subs.expr[i].val);
        }
      else if (id.subs.expr[i].type == variable)
        {
          expr_lower  (&id_type->lo,   Expr_constant, 1);
          expr_upper_2(&id_type->up,   Expr_simple_sym, 100, id.subs.expr[i].str);
        }
      else 
        { 
          expr_lower(&id_type->lo, Expr_complex, 1);
          expr_upper(&id_type->up, Expr_complex, 100);
        }
    }   
}

//----------------------------------------------------------
//----------------------------------------------------------
void DecEntry::AddAlignInfo (int dim_num, int index, int offset, 
                             int coeff, enum ALIGNTYPE stype)
{
  align_info[a_index]->append_entry(index,offset,coeff, stype);
}

//----------------------------------------------------------
//----------------------------------------------------------
void DecEntry::InitAlignInfo (int numdim, AST_INDEX node)
{
  align_info[a_index]->ndim = numdim;
  align_info[a_index]->perfect_align = false;
  align_info[a_index]->node = node;
}

//----------------------------------------------------------------
// Add the id number for the align directive
//----------------------------------------------------------------
void DecEntry::AddAlignIdNumber(AST_INDEX node, FortTree ft)
{
  int i;

  i = ft_NodeToNumber(ft, node);

  if (i)
    {
      align_info[a_index]->id_number = i;
    }
  else
    {
      cout << "Error: Could not obtain unique id number \n";
    }

  return;
} 

//----------------------------------------------------------------
// Add the id number for the distrib directive
//----------------------------------------------------------------
void DecEntry::AddDistribIdNumber(AST_INDEX node, FortTree ft)
{
  int i;
 
  i = ft_NodeToNumber(ft, node);

  if (i)
    {
      distrib_info[d_index]->id_number = i;
    }
  else
    {
      cout << "Error: Could not obtain unique id number \n";
    }
 
  return;
}

#if 0
//----------------------------------------------------------------
// Add the context for the decomposition
// Note: current implementation requires that decomposition, align,
//       and distribute be defined in the same module
//----------------------------------------------------------------
void DecEntry::AddDecompContext(Context  c)
{
  decomp_context = c;

  return;
}
#endif

//----------------------------------------------------------------
// Add the id number for the decomp directive
//----------------------------------------------------------------
void DecEntry::AddDecompIdNumber(AST_INDEX node, FortTree ft)
{
  int i;

  i = ft_NodeToNumber(ft, node);
  if(i)
   decomp_id_number = i;
  else
   cout << "Error: Could not obtain unique id number \n";
}

//----------------------------------------------------------
// Add alignment information for each dimension of the array
//----------------------------------------------------------
void DecEntry::AddAlignInfo
(int numdim, int dim_num, int index, int offset, int coeff, enum ALIGNTYPE stype, AST_INDEX node)
{
 int i;
 for(i=0;i<numdim;i++)
 {
 align_info[a_index]->append_entry(i+1, 0, 1, ALIGN_PERFECT); 
 }

 align_info[a_index]->append_entry(index, offset, coeff, stype);
 align_info[a_index]->ndim = numdim;
 align_info[a_index]->perfect_align = false;
 align_info[a_index]->node = node;
 ++a_index;
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
void DecEntry::AddAlignInfo(int numdim, AST_INDEX node)
{
  align_info[a_index]->ndim = numdim;
  align_info[a_index]->perfect_align = true;
  align_info[a_index]->node = node;
 ++a_index;
}

//-----------------------------------------------------
// Part of the analysis of the align statement
// Store the list of arrays aligned to a decomposition 
//-----------------------------------------------------
void DecEntry::AddDecompNameList(struct dc_list *arrays)
{
 int j;
 Boolean entry;
 NameEntry *n_entry;

 for (j = 0; j < arrays->num; j++){
  entry = false;
   for (n_entry = name_info->first_entry();
        n_entry != 0; n_entry = name_info->next_entry()){
     if (!strcmp(n_entry->array_name, arrays->id[j].str))
     entry = true;
    }
  if(!entry)
  name_info->append_entry(arrays->id[j].str);
  }
}

//-----------------------------------------------------
// Part of the analysis of the distribute statement 
// Store d_index in the distrib_index structure for
// each array mapped to the decomposition
//-----------------------------------------------------
void DecEntry::AddDistribIndex(FortranDInfo *fd)
{
 FortranDHashTableEntry *sp = NULL;
 char *name;
 NameEntry *n;
 
  for(n = name_info->first_entry(); n != 0;
      n = name_info->next_entry())
  {
    name = n->array_name;
    if(!(sp = fd->GetEntry(name))){
      cout << "DECOMPOSITION " << name << " not declared";
      exit(0); }
   sp->d->distrib_index[dist_index] = d_index-1;
   sp->d->dist_index++;
  }
 }


//-----------------------------------------------------
// derived function used by class 
// SinglyLinkedListEntryIO to allocate storage 
// for an AlignList
//-----------------------------------------------------
SinglyLinkedListEntryIO* AlignList::NewEntry()
{
	return (SinglyLinkedListEntryIO *) new AlignEntry;
}

//-----------------------------------------------------
// derived function used by class 
// SinglyLinkedListEntryIO to allocate storage 
// for a NameList
//-----------------------------------------------------
SinglyLinkedListEntryIO* NameList::NewEntry()
{
	return (SinglyLinkedListEntryIO *) new NameEntry;
}

//-----------------------------------------------------
// derived function used by class 
// SinglyLinkedListEntryIO to allocate storage 
// for a DecList
//-----------------------------------------------------
SinglyLinkedListEntryIO* DecList::NewEntry()
{
	return (SinglyLinkedListEntryIO *) new DecEntry;
}


//-----------------------------------------------------
// For each parameter in a callsite store 
// 1) the number of decompositions that reach, 
// 2) name(s) of the decomposition
// 3) set of alignment and distribute statements
//-----------------------------------------------------

//-----------------------------------------------------
// derived function used by class 
// SinglyLinkedListEntryIO to allocate storage 
// for a FDSetList
//-----------------------------------------------------
SinglyLinkedListEntryIO* FDSetEntryList::NewEntry()
{
	return (SinglyLinkedListEntryIO *) new FDSetEntry;
}


//-----------------------------------------------------
//-----------------------------------------------------
FDSetEntry* FDSetEntryList::first_entry() 
{ 
	return current = (FDSetEntry *) SinglyLinkedList::First(); 
}
 
//-----------------------------------------------------
//-----------------------------------------------------
FDSetEntry* FDSetEntryList::next_entry() 
{ 
  if(current == 0) return(0);
  else {
	return current = 
	(current ? (FDSetEntry *) current->Next() : 0);
  }
}

//-----------------------------------------------------
//-----------------------------------------------------
void NameList::delete_entry(NameEntry* n)
{
 if (current==n){
  current = (current ? (NameEntry*)current->Next() : 0); 
  }
  SinglyLinkedList::Delete((SinglyLinkedListEntry*)n);
}

//-------------------------------------------------------------------
// initialize Typedescript structure
//-------------------------------------------------------------------
void Typedescript(TYPEDESCRIPT& idtype)
{
    expr_init(&idtype.lo);
    expr_init(&idtype.up);
    expr_init(&idtype.step);

}

//-------------------------------------------------------------------
// return a string containing upper and lower
//-------------------------------------------------------------------
void typedescript_str(StringBuffer *str, TYPEDESCRIPT& idtype)
{
  char *upper, *lower;
  
  lower =  expr_string(&idtype.lo);
  upper =  expr_string(&idtype.up);

  if ((lower != 0) && (upper != 0))
   {
   str->Append("%s:%s", lower, upper);
   return ;
   }
  
  if(lower == 0)
   {
   str->Append("%s,", upper);
   return;
   }
}

void typedescript_string(StringBuffer *str, TYPEDESCRIPT* idtype)
{
  char *upper, *lower;
  
  lower =  expr_string(&idtype->lo);
  upper =  expr_string(&idtype->up);

  if ((lower != 0) && (upper != 0))
   {
   str->Append("%s:%s", lower, upper);
   return ;
   }
  
  if(lower == 0)
   {
   str->Append("%s,", upper);
   return;
   }
}

//--------------------------------------------------------------
// copy the typedescript structure
//--------------------------------------------------------------
void copy_typedescript(TYPEDESCRIPT& copy_into, TYPEDESCRIPT& copy_from)
{
  expr_copy(&copy_into.lo, &copy_from.lo);
  expr_copy(&copy_into.up, &copy_from.up);
  expr_copy(&copy_into.step, &copy_from.step);
}

//-------------------------------------------------------------------
// initialize Dist_info structure
//-------------------------------------------------------------------
void Distinfo(DIST_INFO &distinfo)
{
    distinfo.ddim = 0;
    distinfo.distr_type = FD_DIST_DC_NONE;
    distinfo.size = 0;
    distinfo.bksize = 0; 
    distinfo.blocksize1 = 0;
    distinfo.blocksize2 = 0;
    distinfo.min_access = 0;
    distinfo.max_access = 0;
    distinfo.irreg_id = "";
    distinfo.arg_position = 0;
}

//----------------------------------------------------------------
// AlignEntry writes itself out to the database
//----------------------------------------------------------------
int AlignEntry::WriteUpCall(FormattedFile& port)
{
 port.Write(FD_AlignTypeString, strlen(FD_AlignTypeString));
 port.Write(s.stype);
 port.Write(s.index);
 port.Write(s.offset);
 port.Write(s.coeff);
 return 0;
}

//----------------------------------------------------------------
// AlignList writes itself out to the database
//----------------------------------------------------------------
int AlignList::Write(FormattedFile& port)
{
      // not used
  //AlignEntry *align_entry;

// write the align information for each dimension
   port.Write("ALIGN ID = ", 20);
   port.Write(id_number);

   if(perfect_align) {
    port.Write(true);   
  }
   else {
    port.Write(false);
    SinglyLinkedListIO::Write(port);
  }
   return 0; // success ?!
}

//---------------------------------------------------------------
// compares two alignment entries. Returns true if they are equal
// else return false
//---------------------------------------------------------------
Boolean AlignList::isEqual(AlignList *a)
{
 AlignEntry *a2_entry;

// if differing number of entries, return false
 if(a->Count() != Count())
  return false;

// if perfect alignment return true
 if (perfect_align && a->perfect_align)
  return true;

 a2_entry = first_entry();  

// for each dimension check  the alignment specifications for
// both entries. If they are the same return true else return false

 for(AlignEntry *a_entry = a->first_entry(); a_entry != 0; a_entry =
     a->next_entry())
  {
  if (!(a2_entry->isEqual(a_entry)))
   return(false);
  a2_entry = next_entry();
  } 
 return true;
}

//----------------------------------------------------------------
// DistribEntry writes itself out to the database
//----------------------------------------------------------------
void write_typedescript(FormattedFile& port, TYPEDESCRIPT& idtype)
{
  expr_write(&idtype.lo,   port);
  expr_write(&idtype.up,   port);
}

//----------------------------------------------------------------
// check to see if two entries have the same distribution
//----------------------------------------------------------------
Boolean DistribEntryArray::isEqual(DistribEntryArray *d, int numdim)
{
 for(int i=0; i< numdim; ++i)
  {
  if(distinfo[i].distr_type != d->distinfo[i].distr_type)
   return false;
  }
 return true;
}
 
//----------------------------------------------------------------
// DecEntry writes itself out to the database
// write the DistribEntry Expression for DECOMPOSITION
// write AlignList for ARRAYS
//----------------------------------------------------------------
void DecEntry::write(FormattedFile& port,
                     enum FORM formm, int numdim) 
{
 int i, j;

 switch(formm){

  case DECOMPTYPE:
#if 0
  port.Write(decomp_context);  
#endif
  port.Write(decomp_id_number);
  for(i=0;i<numdim;++i)
  {
   write_typedescript(port, idtype[i]);
  }

  port.Write(d_index);  
  for(i=0; i<d_index; ++i)
  {
   port.Write(distrib_info[i]->id_number);
   for(j=0;j<numdim;++j)
   {
    port.Write(distrib_info[i]->distinfo[j].distr_type);
    port.Write(i);
   }
  }
  break;
 case ARRAYTYPE:
  port.Write(FD_AlignIndexString, strlen(FD_AlignIndexString));
  for(i=0;i<numdim;++i)
   {
    write_typedescript(port, idtype[i]);
   }

  port.Write(a_index);
  for(i=0;i<a_index;++i)
  {
  port.Write(i);
  align_info[i]->Write(port);
  }
  break;
 }
}

int FDSetEntry::ReadUpCall(FormattedFile&)
 {
 return 0;
 }

//----------------------------------------------------------------
// write the set out to the database
//----------------------------------------------------------------
int FDSetEntry::WriteUpCall(FormattedFile& port)
{
  port.Write(decomp_name, strlen(decomp_name)); 
  port.Write(align_index);
  port.Write(distrib_index);
  return 0; // success ?!
}

//----------------------------------------------------------------
// write the sets at every callsite out to the database
//----------------------------------------------------------------
void FortranDInfo::WriteCallInfo(IPinfoTreeNode* tree, FormattedFile& port)
{
 int count1 = 0; 
 ActualList *alist;
 ActualListEntry *a_entry;
  
 port.Write(FD_CallSiteInfoString, strlen(FD_CallSiteInfoString));


   for(NonUniformDegreeTreeIterator it0(tree, PREORDER);
                                    it0.Current() != 0; ++it0)
   {
    count1 = count1 + ((IPinfoTreeNode*)it0.Current())->calls->Count();
   }
 port.Write(count1);

//----------------------
// for each callsite   
   for(NonUniformDegreeTreeIterator it(tree, PREORDER);
                                    it.Current() != 0; ++it)
   {
      CallSitesIterator callsites(((IPinfoTreeNode*)it.Current())->calls);
      for(CallSite *c_entry; c_entry = callsites.Current(); ++callsites) {
         port.Write((Generic)c_entry->Id());
         alist = c_entry->GetActuals();

         port.Write(alist->Count());
         for(a_entry = alist->First(); a_entry != 0;
	     a_entry = alist->Next()) {

            port.Write(a_entry->Name(), 100); // temporary::::: to be removed   
            if(a_entry->fortd_set == 0)
               port.Write(0);
            else {
  	       ((FDSetEntryList*) a_entry->fortd_set)->Write(port);
            }
         }

         WriteGlobalsCallInfo(c_entry, port);

      }
   }
}

//----------------------------------------------------------------
// read the sets at every callsite
//----------------------------------------------------------------
void FortranDInfo::ReadCallInfo(IPinfoTreeNode *tree, FormattedFile& port)
{
 ActualList *alist;
 ActualListEntry *a_entry;
 int entries_in_list, callsite_id, align_indx, distrib_indx;
 int fd_a_entry_count, alist_count;
 char decomp_name[100], call_string[30], actual[100];
  
 port.Read(call_string, strlen(FD_CallSiteInfoString));

// read each callsite entry

 port.Read(entries_in_list);
 while(entries_in_list-- > 0){
  port.Read(callsite_id);
//----------------------
// for each callsite   
   for(NonUniformDegreeTreeIterator it(tree, PREORDER);
                                    it.Current() != 0; ++it)
   {
      CallSitesIterator callsites(((IPinfoTreeNode*)it.Current())->calls);
      for(CallSite *c_entry; c_entry = callsites.Current(); ++callsites) {
         if(c_entry->Id() == callsite_id) {
   	    alist = c_entry->GetActuals();
   	    port.Read(alist_count);

// for each actual parameter at the call site read the 
// local reaching decomposition sets

   	    for(a_entry = alist->First(); a_entry != 0;
       	        a_entry = alist->Next()) {
    	       port.Read(actual, 100); // to be removed, just a test:::
    	       port.Read(fd_a_entry_count);
    	       a_entry->fortd_set=(SinglyLinkedListIO*)(new FDSetEntryList());
     	       while(fd_a_entry_count--){
     	             port.Read(decomp_name, 100);
     	          port.Read(align_indx);
     	          port.Read(distrib_indx);
     	          ((FDSetEntryList*)(a_entry->fortd_set))->append_entry
                               (decomp_name, align_indx,distrib_indx);
    	       }
            }
            ReadGlobalsCallInfo(c_entry, port);
         }
      }
  }
 }
}



//----------------------------------------------------------------
// FDSetEntryList reads itself from a database
//----------------------------------------------------------------
int FDSetEntryList::Read(FormattedFile& port)
{
  int entry_count, i;
  int align_index, distrib_index;
  char decomp_name[NAME_LENGTH];

 port.Read(entry_count);
 for(i=0;i<entry_count;i++)
 {
  port.Read(decomp_name, NAME_LENGTH);
  port.Read(align_index);
  port.Read(distrib_index);

  append_entry(decomp_name, align_index, distrib_index);
 }
 return 0; // success ?!
}

//----------------------------------------------------------------
// AlignEntry reads itself from a database
//----------------------------------------------------------------
int AlignEntry::ReadUpCall(FormattedFile& port)
{
 int s_type;
 char a[30];
 port.Read(a, strlen(FD_AlignTypeString));
 port.Read(s_type);
 s.stype = (enum ALIGNTYPE)s_type;
 port.Read(s.index);
 port.Read(s.offset);
 port.Read(s.coeff);
 return 0; //! success ?!
}

//----------------------------------------------------------------
// AlignList reads itself from a database
//----------------------------------------------------------------
int AlignList::Read(FormattedFile& port, int numdim)
{
 AlignEntry *a_entry;
 int entry, i, count;
 char a[20];

 port.Read(a, 20);
 port.Read(id_number);
 port.Read(entry);
 if (entry == 1)
 {
  perfect_align = true;
  for( i=0; i< numdim;++i)
  {
   a_entry  = new AlignEntry();
   append_entry(a_entry);
  }
 }
 else
 {
  port.Read(count);
  for( i= 0; i<count;++i)
   {
   a_entry = new AlignEntry();
   a_entry->ReadUpCall(port);
   append_entry(a_entry);
   }
  }
 return 0; // success ?!
}


//----------------------------------------------------------------
// DistribEntry reads itself from the database
//----------------------------------------------------------------
void read_typedescript(FormattedFile& port, TYPEDESCRIPT& idtype)
{
  expr_read(&idtype.lo, port);
  expr_read(&idtype.up, port);
//  expr_read(&idtype.step, port);
}

//----------------------------------------------------------------
// DecEntry reads itself from the database
// read the DistribEntry Expression
//----------------------------------------------------------------
void DecEntry::read(FormattedFile& port,
                     enum FORM formm, int numdim) 
{
 int i, j, d_type;
 char AlignIndexStr[100];

 switch(formm){
  case DECOMPTYPE:
#if 0
  port.Read(decomp_context);
#endif
  port.Read(decomp_id_number);
  for(i=0;i<numdim;++i)
   {
   read_typedescript(port, idtype[i]);
   }
  
  port.Read(d_index);
  for(i=0; i<d_index; ++i)
  {
   port.Read(distrib_info[i]->id_number);
   for(j=0;j<numdim;++j)
   {
    port.Read(d_type);
    distrib_info[i]->distinfo[j].distr_type = (Dist_type)d_type;
    port.Read(i);
   }
  }
  break;

  case ARRAYTYPE:
  port.Read(AlignIndexStr, strlen(FD_AlignIndexString));
  for(i=0;i<numdim;++i)
   {
    read_typedescript(port, idtype[i]);
   }

  port.Read(a_index);
  for(i=0;i<a_index;++i)
  {
  port.Read(i);
  align_info[i]->Read(port, numdim);
  }
  break;
 }
}

//-------------------------------------------------------------------
// convert the type information obtained from the global
// symbol table into a form that will fit the Fortran D sym table
//-------------------------------------------------------------------
enum FORM form_type(int type)
{
 if(type ==  TYPE_INTEGER)
  return (INTTYPE);
 if(type == TYPE_REAL)
  return (REAL);
 if(type == TYPE_DOUBLE_PRECISION)
  return (DOUBLE_P);
 if(type == TYPE_COMPLEX)
  return(COMPLEX);
 if(type == TYPE_CHARACTER)
  return(CHARTYPE);
 else return(UNKNOWN);
}


//---------------------------------------------------------------
// get the align entry
//---------------------------------------------------------------
SUBSCR_INFO * get_align_entry(AlignList *alist, int dim_num, int* ddim)
{
 Boolean done = true;
 *ddim = 0;

 for(AlignEntry *a = alist->first_entry(); a != 0; 
                 a = alist->next_entry())
  {
   
   if(a->sub_info()->index == dim_num+1)
    return (a->sub_info());      
    ++(*ddim);
  }
 return (0);
}

//---------------------------------------------------------------
// this function returns the distribution type given the name of
// the array and the dimension number
//---------------------------------------------------------------
Dist_type
FortranDInfo::GetDistributeType(char *array_name, int dim_num)
{
 int i,j, ddim;
 FortranDHashTableEntry *dec_entry;
 FortranDHashTableEntry *array_entry;
 SUBSCR_INFO *s;

  array_entry = GetEntry(array_name);

  if(array_entry != 0)
  {
//------------------------------------------------------------
// go through the distrib_index structure, dist_index times 
// get the distrib structure 
// check the state field, if ACTIVE, for each dimension, print
// dist_type 
//------------------------------------------------------------

 if (!(dec_entry = GetEntry(array_entry->d->dec_name))) 
 {
   cout << "DECOMPOSITION " << dec_entry->d->dec_name << " not declared";
   exit(0); 
  }

  for(j=0;j<array_entry->d->a_index;++j){
   if(array_entry->d->align_info[j]->state == ACTIVE){
    for(i=0; i<array_entry->d->dist_index; ++i){
     if (dec_entry->d->distrib_info[array_entry->d->distrib_index[i]]->state
          == ACTIVE)
    {
//--------------------------
// if perfect alignment
//--------------------------
    if(array_entry->d->align_info[j]->perfect_align)
     return (dec_entry->d->distrib_info[array_entry->d->distrib_index[i]]->
             distinfo[dim_num].distr_type);

//-------------------------
// if not perfect alignment
//------------------------- 
     s = get_align_entry(array_entry->d->align_info[j], dim_num, &ddim);
     if (s == 0)
      return (FD_DIST_LOCAL);
     else
     return (dec_entry->d->distrib_info[array_entry->d->distrib_index[i]]->
        distinfo[ddim].distr_type);
     }
    }
   }
  }
 }
 return(FD_DIST_BLOCK);
}

void copy_align_list(AlignList *copy_into, AlignList *copy_from)
{
 AlignEntry *a;

  copy_into->id_number = copy_from->id_number;
  if(copy_from->perfect_align)
   {
    copy_into->perfect_align = true;
    for (AlignEntry *a_entry = copy_from->first_entry(); a_entry != 0;  
                     a_entry = copy_from->next_entry())
    {
    a = new AlignEntry();
    copy_into->append_entry(a);
    }
   }

  else
   {
    copy_into->perfect_align = false;
    for(AlignEntry *a2_entry = copy_from->first_entry(); a2_entry != 0;  
                    a2_entry = copy_from->next_entry())
    {
    a = new AlignEntry();
    copy_align_entry(a, a2_entry);
    copy_into->append_entry(a);
    }
   }

}

void copy_align_entry(AlignEntry *copy_into, AlignEntry *copy_from)
{
 copy_into->s.stype  = copy_from->s.stype;
 copy_into->s.index  = copy_from->s.index;
 copy_into->s.offset = copy_from->s.offset;
 copy_into->s.coeff  = copy_from->s.coeff;
}   

//-----------------------------------------------------------------
// string functions for decomposition, align, and distribute
//-----------------------------------------------------------------
void AlignList::align_str(StringBuffer *s, int numdim)
{
 if (perfect_align)
  {
  s->Append("%c", ' ');  
  return;
  }
 else
  {
  for(int i = 0; i < numdim; ++i)
   { 
     if(i==0)
     s->Append("%s", "(");

     s->Append("%c", 'i'+ i);
     if(i != numdim - 1)
     s->Append("%s", ",");
   }
   s->Append("%s", ") ");
  }
}

StringBuffer* FortranDHashTableEntry::typedescript_string(int dim)
{
  StringBuffer *s;
  s = new StringBuffer(100);
  typedescript_str(s, d->idtype[dim]); 
  return s;
}


StringBuffer* FortranDHashTableEntry::align_string(int align_index, 
                                                   char* formal_name)
{
  StringBuffer *s;
  s = new StringBuffer(100);
  s->Append(" %s", formal_name);
  AlignList *a =  d->align_info[align_index];
  a->align_str(s, numdim);
  return s;
}

StringBuffer* FortranDHashTableEntry::align_with_string(int align_index, FortranDHashTableEntry *dec_entry)
{
 StringBuffer *s;
 int i, index, value;
 AlignEntry *align_entry;
 char c;

 s = new StringBuffer(100);
 s->Append("  %s", dec_entry->name());
 AlignList *a = d->align_info[align_index];

 if (a->perfect_align) 
   return s;
 else
 { 
  s->Append("%s", "(");
  align_entry = a->first_entry();

  for(i=0;i< dec_entry->numdim;++i)   
  {
   switch(align_entry->stype())
   {
    case ALIGN_OFFSET:
     index = align_entry->index();
     value = align_entry->val();
     c = 'i' + index - 1;

     if (value < 0)
      s->Append( "%c %s %d", c, "-", abs(value));
     else
      s->Append("%c %s %d",  c, "+", value);
    break;

    case ALIGN_PERFECT:
     index = align_entry->index();
     c = 'i' + index - 1;
     s->Append("%c", c);
    break;

    case ALIGN_CONST:
     value = align_entry->val();
     s->Append("%d", value);
    break;
   
    default: 
     cout << "Alignment type not supported \n";
     exit(0);
    break;
   }
   if (i != numdim-1)
    s->Append("%s", ",");
    align_entry = a->next_entry();
  }
 }
 s->Append("%s", ")");
 return s;
}

StringBuffer* FortranDHashTableEntry::distrib_string(int distrib_index)
{
 int i;
 StringBuffer *s;
 s = new StringBuffer(100);
  for (i=0; i < numdim; ++i)
   {
    if(d->distrib_info[distrib_index]->distinfo[i].distr_type == FD_DIST_BLOCK)
       s->Append("%s", "BLOCK");
    else
     if(d->distrib_info[distrib_index]->distinfo[i].distr_type == FD_DIST_CYCLIC)
       s->Append("%s", "CYCLIC");
    else
     if(d->distrib_info[distrib_index]->distinfo[i].distr_type == FD_DIST_LOCAL)
       s->Append("%s", ":");
    else
       cout << "distrib_string::distribution type not supported \n";

   if(i != numdim-1)
    s->Append("%s", ",");
   }
 return s;
}

//------------------------------------------------------------------
// record the context and the ft in the fortran d info
// called from the code generation phase
//------------------------------------------------------------------
void FortranDInfo::ContextFt(Context cc, FortTree fttree)
{
 contxt = cc;
 ft = fttree;
}
