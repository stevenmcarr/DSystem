/* $Id: globals_fwd_ht.h,v 1.5 1997/03/11 14:35:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef globals_fwd_ht_h
#define globals_fwd_ht_h

#include <libs/support/misc/port.h>
#include <libs/support/tables/HashTable.h>

#include <libs/moduleAnalysis/valNum/val.h>

/**************************************************************************** 
 *  Hash table for Forwarding Value Numbers	                  June 1993 *
 *  Author: Paul Havlak                                                     *
 *                                                                          *
 *  This hash table is derived from the generic HashTable class...	    *
 *                                                                          *
 *  Copyright 1993 Rice University, as part of the Rn/ParaScope       	    *
 *  Programming Environment Project.                                        *
 *                                                                          *
 ****************************************************************************/

class GlobFwdEntry 
{
  public:
    friend class GlobFwdMap;

  private:
    ValNumber oVal;
    ValNumber varId;
    int offset;
    ValNumber nVal;

      // constructor for an entry
    GlobFwdEntry(ValNumber ov, ValNumber var, int o, ValNumber nv):
	oVal(ov), varId(var), offset(o), nVal(nv) {};

    unsigned int hash(unsigned int size)
    {
	return (oVal ^ (varId << 8) ^ (offset << 16)) % size;
    };

    int compare(GlobFwdEntry *e)
    {
        return ((this->oVal != e->oVal) ||
		(this->varId != e->varId) ||
		(this->offset != e->offset));
        /* if different, any non-zero value suffices */
    };
};

class GlobFwdMap : private HashTable 
{
  public:
    GlobFwdMap() : HashTable () 
    {
      HashTable::Create (sizeof(GlobFwdEntry), 8);
    };

      // number of entries in hash table
    unsigned int count() 
    { 
      return HashTable::NumberOfEntries(); 
    };

      // add to the table
    void add_entry(ValNumber ov, ValNumber var, int off, ValNumber nv)
    {
        GlobFwdEntry e(ov, var, off, nv);
        HashTable::AddEntry(&e);
    };

      // delete any entry with given key from the table
    void delete_entry(ValNumber ov, ValNumber var, int off) 
    {
        GlobFwdEntry e(ov, var, off, VAL_NIL);
        HashTable::DeleteEntry(&e);
    };

      // test for presence of the key in the table
    int query_entry(ValNumber ov, ValNumber var, int off) 
    {
        GlobFwdEntry e(ov, var, off, VAL_NIL);
        GlobFwdEntry *found = (GlobFwdEntry *) HashTable::QueryEntry(&e);
        if (found)
            return found->nVal;
        else
            return VAL_NIL;
    };

  private:
    virtual unsigned int HashFunct(const void *entry, const unsigned int size)
    {
      return ((GlobFwdEntry *)entry)->hash(size);
    }

    virtual int EntryCompare(const void *e1, const void *e2)
    {
      return ((GlobFwdEntry *)e1)->compare((GlobFwdEntry *)e2);
    }

    virtual void EntryCleanup(void *entry)
    {
      return;
    };

    GlobFwdEntry &operator[](int index)     // get entry for given index
    {
      return *((GlobFwdEntry *) HashTable::GetEntryByIndex(index));
    }
};

#endif globals_fwd_ht_h
