/* $Id: IntegerHashTable.h,v 1.5 1997/03/11 14:37:34 carr Exp $ */
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

#ifndef int_ht_h
#define int_ht_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef HashTable_h
#include <libs/support/tables/HashTable.h>
#endif

class IntHashEntry 
{
  public:
    IntHashEntry(uint k, int v);

    uint hash(uint size);
    int compare(IntHashEntry* e);

    friend class IntHashTable;

  private:
    uint key; 
    int val;
    const uint prime;
    const uint rshift;
    const uint lshift;
};


class IntHashTable : private HashTable 
{

  public:
    IntHashTable(int def = -1);
    virtual ~IntHashTable();

        // number of entries in hash table
    unsigned int count();

        // add a pair to the table
    void add_entry(uint k, int v);

        // delete any pair with key k from the table
    void delete_entry(uint k);

        // test for presence of the key in the table
    int query_entry(uint k);

  private:
    const int deft;

        // virtual functions for hashing and comparing integer pairs that 
        // override the defaults for HashTable
    virtual uint HashFunct(const void* entry, const uint size);
    virtual int EntryCompare(const void* e1, const void* e2);
};

#endif
