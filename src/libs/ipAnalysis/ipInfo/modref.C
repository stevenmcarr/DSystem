/* $Id: modref.C,v 1.14 1997/03/11 14:34:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************
 * Mod/Ref Information for a Scope             September 1991     *
 * Author: John Mellor-Crummey                                    *
 *                                                                *
 * this file contains code that supports an external              *
 * representation of summary of mod/ref for variables. all        *
 * information is represented in a form based on variable         *
 * equivalence classes.                                           *
 *                                                                *
 * Copyright 1991, Rice University, as part of the ParaScope      *
 * Programming Environment Project                                *
 *                                                                *
 ******************************************************************/

#include <stdio.h>
#include <assert.h>
#include <libs/support/file/FormattedFile.h>
#include <libs/ipAnalysis/ipInfo/modref.h>

#define PAIRLISTENTRY_STRING "PairListEntry: "


//-----------------------------------------------
// read an unambiguous representation of a 
// PairListEntry from a database port 
//-----------------------------------------------
int PairListEntry::ReadUpCall(FormattedFile& port)
{
  return port.Read(off) || port.Read(sz);
}


//-----------------------------------------------
// write an unambiguous representation of a 
// PairListEntry to a database port 
//-----------------------------------------------
int PairListEntry::WriteUpCall(FormattedFile& port)
{
  return port.Write(off) || port.Write(sz);
}


//-----------------------------------------------------
// derived function used by class 
// SinglyLinkedListEntryWithDBIO to allocate storage 
// for an PairList
//-----------------------------------------------------
SinglyLinkedListEntryWithDBIO *PairList::NewEntry()
{
  return (SinglyLinkedListEntryWithDBIO *) new PairListEntry;
}


//-----------------------------------------------------
// create an instance of a ModRefHashTableEntry for a
// specified equivalence class.
//-----------------------------------------------------
void
ModRefHashTableEntry::init(char *name, unsigned int vtype)
{
  modref[MODREFTYPE_REF] = new PairList();
  modref[MODREFTYPE_MOD] = new PairList();
  leader = (name ? ssave(name) : 0);
  type = vtype;
}


//-----------------------------------------------------
// read the external representation of a 
// ModRefHashTableEntry into an instance from a 
// database port.
//-----------------------------------------------------
int ModRefHashTableEntry::Read(FormattedFile& port)
{
#ifdef DEBUG
  (void) port.p_getc();
#endif DEBUG
  
  char buffer[IP_NAME_STRING_LENGTH];
  port.Read(buffer, IP_NAME_STRING_LENGTH);
  leader = ssave(buffer);
  port.Read(type);
  modref[MODREFTYPE_REF]->Read(port);
  modref[MODREFTYPE_MOD]->Read(port);
  
#ifdef DEBUG
  (void) port.p_getc();
#endif DEBUG
  return 0; //success!?!
}


//-----------------------------------------------------
// write the external representation of a 
// ModRefHashTableEntry of an instance to a 
// database port.
//-----------------------------------------------------
int ModRefHashTableEntry::Write(FormattedFile& port)
{
#ifdef DEBUG
  port.p_putc('(');
#endif DEBUG
  
  port.Write(leader, IP_NAME_STRING_LENGTH);
  port.Write(type);
  modref[MODREFTYPE_REF]->Write(port);
  modref[MODREFTYPE_MOD]->Write(port);
  
#ifdef DEBUG
  port.p_putc(')');
#endif DEBUG
  return 0; //success!?!
}

//-----------------------------------------------------
// free storage allocated on the behalf of fields
// in a ModRefHashTableEntry instance.
//-----------------------------------------------------
void 
ModRefHashTableEntry::DeleteEntry()
{
  sfree(leader);
  delete modref[MODREFTYPE_REF];
  delete modref[MODREFTYPE_MOD];
}

ModRefInfo::ModRefInfo() : HashTable () 
{
  current_index = 0; 

  HashTable::Create(sizeof(ModRefHashTableEntry), 8);
}


//-----------------------------------------------------
// derived function for class HashTable that
// invokes the hash member function for a 
// ModRefHashTableEntry instance.
//-----------------------------------------------------
unsigned int 
ModRefInfo::entry_hash(void *entry, unsigned int size)
{
  return ((ModRefHashTableEntry *)entry)->hash(size);
}

//-----------------------------------------------------
// derived function for class HashTable that
// invokes the DeleteEntry member function for a 
// ModRefHashTableEntry instance.
//-----------------------------------------------------
void 
ModRefInfo::entry_delete_callback(void *entry, void *)
{
  ((ModRefHashTableEntry *)entry)->DeleteEntry();
}


//-----------------------------------------------------
// derived function for class HashTable that
// invokes the compare member function for a 
// ModRefHashTableEntry instance.
//-----------------------------------------------------
int 
ModRefInfo::entry_compare(void *e1, void *e2)
{
  return ((ModRefHashTableEntry *)e1)->compare((ModRefHashTableEntry *)e2);
}


//-----------------------------------------------------
// record mod or ref information for a variable by
// adding it to an instance of a ModRefHashTableEntry 
// that represents the specified equivalence class
// (if no entry for that equivalence class exists,
// create a new one)
//-----------------------------------------------------
void 
ModRefInfo::AddModRef(ModRefType mut, char *leader, int offset, 
	unsigned int size, unsigned int vtype)
{
  ModRefHashTableEntry e;
  e.leader = leader;
  ModRefHashTableEntry *found = (ModRefHashTableEntry *) query_entry(&e);
  if (found == 0) {
    e.init(leader, vtype);
    e.modref[mut]->Append(offset,size);
    add_entry(&e);
  } else {
    assert( vtype == found->type);
    found->modref[mut]->Append(offset,size);
  }
}


//-----------------------------------------------------
// function for enumerating equivalence classes in 
// a ModRefInfo hashtable
//-----------------------------------------------------
char *
ModRefInfo::GetNextLeader(unsigned int &vtype)
{
  ModRefHashTableEntry *e = 
    (ModRefHashTableEntry *) get_entry_by_index(current_index++);
  if (e == 0) {
    vtype = 0;
    return 0;
  } else { 
    vtype = e->type;
    return e->leader;
  }
}

char *ModRefInfo::GetNextLeader()
{
	unsigned int vtype;
	return GetNextLeader(vtype);
}

//-----------------------------------------------------
// get first equivalence class in a ModRefInfo
// hashtable
//-----------------------------------------------------
char *ModRefInfo::GetFirstLeader(unsigned int &vtype) 
{ 
  current_index = 0; 
  return GetNextLeader(vtype); 
}

char *ModRefInfo::GetFirstLeader() 
{
	unsigned int vtype;
	return GetFirstLeader(vtype);
}


//-----------------------------------------------------
// report a count of distinct mod or ref entries 
// recorded for a specified equivalence class
//-----------------------------------------------------
unsigned int 
ModRefInfo::ModRefCount(ModRefType mut, char *leader)
{
  ModRefHashTableEntry e;
  e.leader = leader;
  ModRefHashTableEntry *found = (ModRefHashTableEntry *) query_entry(&e);
  if (found != 0) return found->modref[mut]->Count();
  else return 0;
}


//-----------------------------------------------------
// return the first mod or ref entry for a specified 
// equivalence class
//-----------------------------------------------------
Boolean
ModRefInfo::GetFirstModRef(ModRefType mut, char *leader, int *offset, 
	unsigned int *size, unsigned int *vtype)
{
  ModRefHashTableEntry e;
  e.leader = leader;
  ModRefHashTableEntry *found = (ModRefHashTableEntry *) query_entry(&e);
  if (found == 0) return false;
  PairListEntry *le = found->modref[mut]->First();
  if (le == 0) return false; 
  else { *offset = le->Offset(); *size = le->Size(); *vtype = found->type; }
  return true;
}


//-----------------------------------------------------
// return the next mod or ref entry in an enumeration 
// of the information for a specified equivalence class 
//-----------------------------------------------------
Boolean
ModRefInfo::GetNextModRef(ModRefType mut, char *leader, int *offset, 
	unsigned int *size, unsigned int *vtype)
{
  ModRefHashTableEntry e;
  e.leader = leader;
  ModRefHashTableEntry *found = (ModRefHashTableEntry *) query_entry(&e);
  if (found == 0) return false;
  PairListEntry *le = found->modref[mut]->Next();
  if (le == 0) return false; 
  else { *offset = le->Offset(); *size = le->Size(); *vtype = found->type; }
  return true;
}


//-----------------------------------------------------
// write an external representation of an instance of
// a ModRefInfo HashTable to a database port.
//-----------------------------------------------------
int ModRefInfo::Write(FormattedFile& port)
{
  int i;
  int n = count();
  port.Write(n); // write a count of the entries in the table
  for(i=0; i < n; i++) 
    ((ModRefHashTableEntry *)get_entry_by_index(i))->Write(port);
  return 0; //success!?!
}


//-----------------------------------------------------
// read an external representation of an instance of
// a ModRefInfo HashTable from a database port.
//-----------------------------------------------------
int ModRefInfo::Read(FormattedFile& port)
{
  int i;
  port.Read(i); // read a count of the entries in the table
  while(i-- > 0) {
    ModRefHashTableEntry m;
    m.init();
    m.Read(port);
    add_entry(&m);
  }
  return 0; //success!?!
}
