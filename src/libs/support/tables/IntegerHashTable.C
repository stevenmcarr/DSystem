/* $Id: IntegerHashTable.C,v 1.2 1997/03/11 14:37:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/**************************************************************************** 
 * Integer Hash Table Utility	                         October 1991	    *
 * Author: Paul Havlak, adapted from string_ht.h                            *
 *                                                                          *
 * This integer hash table is derived from the generic HashTable class.     *
 *                                                                          *
 * Copyright 1991, 1993 Rice University, as part of the Rn/ParaScope        *
 *			Programming Environment Project.                    *
 *                                                                          *
 ****************************************************************************/

#include <libs/support/tables/IntegerHashTable.h>

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//
//
IntHashEntry::IntHashEntry(uint k, int v) 
  : key(k), val(v), prime(7), rshift(3), lshift(8*sizeof(int) - rshift)
{
  return;
}

//
//
uint IntHashEntry::hash(uint size)
{
  return ((key >> rshift)*prime ^ (key << lshift)) % size;
}

//
//
int IntHashEntry::compare(IntHashEntry *e)
{ 
  return (this->key != e->key);
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//
//
IntHashTable::IntHashTable(int def) 
  : deft(def)
{
  HashTable::Create(sizeof(IntHashEntry), 8);
}

//
//
IntHashTable::~IntHashTable() 
{
  HashTable::Destroy();
}

//
//
uint IntHashTable::count() 
{ 
  return HashTable::NumberOfEntries(); 
}

//
//
void IntHashTable::add_entry(uint k, int v)
{
  IntHashEntry e(k, v);

  HashTable::AddEntry((void*)&e);
}

//
//
void IntHashTable::delete_entry(uint k) 
{
  IntHashEntry e(k, 0);

  HashTable::DeleteEntry((void*)&e);
}

//
//
int IntHashTable::query_entry(uint k) 
{
  IntHashEntry e(k, 0);

  IntHashEntry* found = (IntHashEntry*)HashTable::QueryEntry((void*)&e);

  if (found) return found->val;
  else       return deft;
}

//
//
uint IntHashTable::HashFunct(const void* entry, const uint size)
{
  return ((IntHashEntry*)entry)->hash(size);
}

//
//
int IntHashTable::EntryCompare(const void* e1, const void* e2)
{
  return ((IntHashEntry*)e1)->compare((IntHashEntry*)e2);
}
