/* $Id: CommBlock.C,v 1.8 1997/03/11 14:28:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//--------------------------------------------------------------------------
// author : Seema Hiranandani
//
// content:   The functions in this file are used to parse common block 
//            statements, extract
//            relevant information such as name of common block, position 
//            of variables in the common block. This information is 
//            stored in a linked list which
//            may be used later on to determine which variables are within 
//            common blocks
//
// date   : September 1992
//-------------------------------------------------------------------------
#undef is_open
#include <stream.h>

#include <libs/frontEnd/ast/ast_include_all.h>
#include <libs/frontEnd/fortTree/fortsym.h>
#include <libs/frontEnd/ast/treeutil.h>

#include <libs/fortD/localInfo/CommBlock.h>
#include <libs/fortD/misc/fd_types.h>
#include <libs/fortD/misc/FortD.h>

#include <libs/ipAnalysis/ipInfo/iptypes.h>
#include <libs/ipAnalysis/ipInfo/CallSite.h>

#include <libs/support/tables/symtable.h>

SinglyLinkedListEntryIO *CommonBlockList::NewEntry()
{
   return new common_block_ent;
}

//------------------------------------------------------------------------
// read a common block entry
//------------------------------------------------------------------------
int common_block_ent::ReadUpCall(FormattedFile& port)
{
   char nme[NAME_LENGTH], leadre[NAME_LENGTH];
   
   port.Read(nme, NAME_LENGTH);
   port.Read(leadre, NAME_LENGTH);
   name = ssave(nme);
   leader = ssave(leadre);
   port.Read(offset);
   port.Read(size);
   return 0; // success ?!
}

//------------------------------------------------------------------------
// write a common block entry
//------------------------------------------------------------------------
int common_block_ent::WriteUpCall(FormattedFile& port)
{ 
   port.Write(name, strlen(name));
   port.Write(leader, strlen(leader));
   port.Write(offset);
   port.Write(size);
   return 0; // success ?!
}

//------------------------------------------------------------------------
// store in a list all globals that have been distributed 
//------------------------------------------------------------------------
void FortranDInfo::CheckAndStoreCommonBlock(char *name, SymDescriptor d, char *entry_name)
{
   char *leader;
   int offset;
   int  size;
   unsigned int vtype;
   
   if (entry_name_symdesc_name_To_leader_offset_size_vtype(entry_name, d, name,
					       &leader, &offset, &size, &vtype))
   {
      if (vtype & VTYPE_COMMON_DATA)
      {
	 //cout << name << " is declared in common block " << leader << "\n";
	 common_block_info->append_entry(name, leader, offset, size);
      }
   }
}


//------------------------------------------------------------------------
// store a list of reaching decompositions for globals at a callsite
//------------------------------------------------------------------------
void FortranDInfo::StoreReachDecompGlobals(CallSite *c)
{
   int i = 0;
   FortranDHashTableEntry *f;

   for(common_block_ent *e = common_block_info->first_entry(); e != 0;
       e = common_block_info->next_entry())
   {
      c->fortd_set[i] = (SinglyLinkedListIO*)(new  FDSetEntryList());
      f = GetEntry(e->name);

      if ( f == 0) 
      {
	 // cout << "Global Variable " << e->name << " has decomposition = TOP \n";
      } else {
	 AddSetInfo(f,(FDSetEntryList*)(c->fortd_set[i]));
      }

      ++i;
   }
}

//------------------------------------------------------------------------
// write the set information of globals
//------------------------------------------------------------------------
void FortranDInfo::WriteGlobalsCallInfo(CallSite *c, FormattedFile &port)
{

   int i =0;

   port.Write(FD_CallSiteGlobalsString, strlen(FD_CallSiteGlobalsString));
   port.Write(common_block_info->Count());

   for (common_block_ent *e = common_block_info->first_entry(); e != 0;
	e = common_block_info->next_entry())
   {
      e->WriteUpCall(port);
      ((FDSetEntryList *)c->fortd_set[i])->Write(port);
      ++i;
   }

   port.Write(FD_CallSiteGlobalsStringEnd, strlen(FD_CallSiteGlobalsStringEnd));
}

//------------------------------------------------------------------------
// read the set information of globals
//------------------------------------------------------------------------
void FortranDInfo::ReadGlobalsCallInfo(CallSite *c, FormattedFile &port)
{
   int i =0, entries = 0;;
   char CallSiteGlobalsString[30], CallSiteGlobalsStringEnd[30];
   common_block_ent *e;

   int  count = common_block_info->Count();

   port.Read(CallSiteGlobalsString, strlen(FD_CallSiteGlobalsString));

   port.Read(entries);
   while(entries-- > 0)
   {
      c->fortd_set[i] =(SinglyLinkedListIO*)(new FDSetEntryList());

      //-------------------------------------
      // read common block information

      e = new common_block_ent();
      e->ReadUpCall(port);
      if (count == 0)  // read the common_block_info only once for all callsites
	// in procedure this
      {
	 common_block_info->Append(e);
      }

      //--------------------------------------
      // read global set information 

      ((FDSetEntryList *)c->fortd_set[i])->Read(port);
      ++i;
   }
   port.Read(CallSiteGlobalsStringEnd, strlen(FD_CallSiteGlobalsStringEnd));
}

//---------------------------------------------------------------------------
// additional information required to be stored as annotations on the nodes
// is details on the common block, i.e. name of the variables and arrays,
// positions, size
// 1. parse the common block statement, for each variable, get it's details
//    from the symbol table
// 2. store that information in FortranD Info
// 3. write it out to be read during the interprocedural propagation phase
// 4. it will be used to map caller's globals with callee's globals
//---------------------------------------------------------------------------
void FortranDInfo::ParseAndStoreCommon(AST_INDEX node, char* entry_name, 
                                       SymDescriptor d)
{
   AST_INDEX common_elt_list, name_list;
   char *common_name, *name, *leader;
   common_block_entry_list *entry;
   int offset;
   int  size;
   unsigned int vtype;
   
   common_elt_list =  list_first(gen_COMMON_get_common_elt_LIST(node));
   common_name     =  gen_get_text(gen_COMMON_ELT_get_name(common_elt_list));

   //----------------------------------------------------- 
   // get the list of names in the common block
   // for each name, get the details from the symbol table
   
   entry =  common_blks_decl->append_entry(common_name);

   //----------------------------------------------------
   // walk the list of names and mark them in the list 

   name_list = list_first(gen_COMMON_ELT_get_common_vars_LIST(common_elt_list));

   while(name_list != AST_NIL)
   {
      name = gen_get_text(gen_ARRAY_DECL_LEN_get_name(name_list));
      if (entry_name_symdesc_name_To_leader_offset_size_vtype(entry_name, d,
					 name, &leader, &offset, &size, &vtype))
	entry->common_list->append_entry(name, leader, offset, size);
      name_list =  list_next(name_list);
   }
}


SinglyLinkedListEntryIO *common_block_list::NewEntry()
{
   return new common_block_entry_list;
}


//---------------------------------------------------------------------------
// read the common block declaration entry information 
//---------------------------------------------------------------------------
int common_block_entry_list::ReadUpCall(FormattedFile& port)
{
   char temp[NAME_LENGTH];
   int code = port.Read(temp, MAX_NAME) || common_list->Read(port);
   name = ssave(temp);
   return code; 
}

//---------------------------------------------------------------------------
// write the common block declaration entry information 
//---------------------------------------------------------------------------
int common_block_entry_list::WriteUpCall(FormattedFile& port)
{
   return port.Write(name, strlen(name)) || common_list->Write(port);
}

