/* $Id: StringHashTable.C,v 1.7 1997/03/11 14:37:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/**************************************************************************** 
 * String Hash Table Utility                                 October 1991   *
 * Author: John Mellor-Crummey                                              *
 *                                                                          *
 * This string hash table is derived from the generic HashTable class.      *
 * Each entry in the table contains a pointer to a string.                  *
 *                                                                          *
 * If automatic cleanup of the strings is needed (i.e. they were allocated  *
 * dynamically,) then the StringHashTable can be passed a pointer to the    *
 * cleanup function when it is created.  This cleanup function will then    *
 * be used on each string in the StringHashTable.                           *
 *                                                                          *
 *                                                                          *
 * Copyright 1991, Rice University, as part of the Rn/ParaScope Programming *
 * Environment Project.                                                     *
 *                                                                          *
 ****************************************************************************/

#include <libs/support/tables/StringHashTable.h>

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

class StringHashTableEntry 
{
  public:
    StringHashTableEntry(char* n);

    uint Hash(uint size);
    int  Compare(StringHashTableEntry* e);

    friend class StringHashTable;

    char* name;
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//
//
StringHashTableEntry::StringHashTableEntry(char* n) 
{ 
  name = n; 
}

//
//
uint StringHashTableEntry::Hash(uint size) 
{ 
  return hash_string(name, size); 
}

//
//
int StringHashTableEntry::Compare(StringHashTableEntry* e) 
{ 
  if (*this->name != *e->name) return -1; 
  else                         return strcmp(this->name, e->name); 
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//
//
StringHashTable::StringHashTable(StringHashTableCleanupFunct aFunct)
  : HashTable () 
{
  HashTable::Create(sizeof(StringHashTableEntry), 8);

  CleanupFunct = aFunct;
}

//
//
StringHashTable::~StringHashTable()
{
  HashTable::Destroy();
}

//
//
unsigned int StringHashTable::count() 
{ 
  return HashTable::NumberOfEntries();
}

//
//
void StringHashTable::add_entry(char* name) 
{
  StringHashTableEntry e(name);

  HashTable::AddEntry((void*)&e);
}

//
//
void StringHashTable::delete_entry(char* name) 
{
  StringHashTableEntry e(name);

  HashTable::DeleteEntry((void*)&e);
}

//
//
Boolean StringHashTable::query_entry(char* name) 
{
  StringHashTableEntry  e(name);
  StringHashTableEntry* found;

  found = (StringHashTableEntry*)HashTable::QueryEntry((void*)&e);

  return (Boolean)(found != 0);
}

//
//
int StringHashTable::get_entry_index(char* name) 
{
  StringHashTableEntry e(name);

  return HashTable::GetEntryIndex((void*)&e);
}

//
//
char* StringHashTable::get_entry_by_index(int index) 
{
  StringHashTableEntry* e;

  e = (StringHashTableEntry*)HashTable::GetEntryByIndex(index);

  if (e) return  e->name;
  else   return (char*)0;
}

//----------------------------------------------------------------------------
// StringHashTable definition of the virtual function for cleaning up entries
// in the table (this function overrides the vfunct of the base class)
//----------------------------------------------------------------------------
void StringHashTable::EntryCleanup(void* entry)
{
  if (CleanupFunct)
    {
      CleanupFunct(((StringHashTableEntry*)entry)->name);
    }

  return;
}

//----------------------------------------------------------------------------
// StringHashTable definition of virtual function for hashing entries in the 
// table (this function overrides the vfunct of the base class)
//----------------------------------------------------------------------------
uint StringHashTable::HashFunct(const void* entry, const uint size) 
{
  return ((StringHashTableEntry*)entry)->Hash(size);
}

//----------------------------------------------------------------------------
// StringHashTable definition of virtual function for comparing entries in the 
// table (this function overrides the vfunct of the base class)
//----------------------------------------------------------------------------
int StringHashTable::EntryCompare(const void* e1, const void* e2)
{
  return ((StringHashTableEntry*)e1)->Compare((StringHashTableEntry*)e2);
}


//***************************************************************************
// class StringHashTableIterator interface operations
//***************************************************************************


StringHashTableIterator::StringHashTableIterator(const StringHashTable *ht) :
HashTableIterator((const HashTable *) ht)
{
}


StringHashTableIterator::~StringHashTableIterator() 
{
}


char *StringHashTableIterator::Current() 
{
  StringHashTableEntry *entry = 
    (StringHashTableEntry *) HashTableIterator::Current();

  return (entry ? entry->name : 0);
}
