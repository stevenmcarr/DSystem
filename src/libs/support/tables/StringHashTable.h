/* $Id: StringHashTable.h,v 1.9 1997/03/11 14:37:37 carr Exp $ */
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
 * Copyright 1991, Rice University, as part of the Rn/ParaScope Programming *
 * Environment Project.                                                     *
 *                                                                          *
 ****************************************************************************/

#ifndef string_ht_h
#define string_ht_h

#include <string.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef rn_string_h
#include <libs/support/strings/rn_string.h>
#endif

#ifndef HashTable_h
#include <libs/support/tables/HashTable.h>
#endif

typedef
FUNCTION_POINTER(void, StringHashTableCleanupFunct, (char*));

class StringHashTable : private HashTable 
{
  public:
    StringHashTable(StringHashTableCleanupFunct aFunct = NULL);
    virtual ~StringHashTable();

	// number of entries in a string hash table
    uint count();

	// add a string to the table
    void add_entry(char* name);

	// delete a string from the table
    void delete_entry(char* name);

	// test for presence of the string in the table
    Boolean query_entry(char* name);

	// get the index of a string in the table
    int get_entry_index(char* name);

	// get a string corresponding to a given index
    char* get_entry_by_index(int index);

  private:
    StringHashTableCleanupFunct CleanupFunct;

	// virtual functions for hashing, comparing and cleaning-up 
        // strings.  These functions override the defaults for HashTable.
    virtual uint HashFunct(const void* entry, const uint size);
    virtual int  EntryCompare(const void* e1, const void* e2);
    virtual void EntryCleanup(void* entry);
friend class StringHashTableIterator;
};

class StringHashTableIterator : private HashTableIterator {
public:
  StringHashTableIterator(const StringHashTable *ht);
  ~StringHashTableIterator();
  char *Current();
  HashTableIterator::operator++;
  HashTableIterator::Reset;
};

#endif
