/* $Id: val_ht.h,v 1.7 1997/03/11 14:36:19 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/**************************************************************************** 
 * ValEntry Hash Table Utility                               February 1993  *
 * Author: Paul Havlak                                                      *
 *                                                                          *
 * This val hash table is derived from the generic HashTable class.         *
 * Each entry in the table contains a pointer to a val. Saving the          *
 * ValEntry's is the responsibility of the clients of this abstraction.     *
 *                                                                          *
 * Copyright 1991, 1993 Rice University, as part of the Rn/ParaScope        *
 * Programming Environment Project.                                         *
 *                                                                          *
 ****************************************************************************/

#ifndef val_ht_h
#define val_ht_h

#ifndef rn_varargs_h
#include <include/rn_varargs.h>
#endif

#ifndef HashTable_h
#include <libs/support/tables/HashTable.h>
#endif

struct ValEntry;

class FormattedFile;  // minimal external declaration

typedef FUNCTION_POINTER(void, ValTableForAllFunct, (ValEntry* ve, va_list argList));

class ValTable : private HashTable 
{
  public:
    ValTable();
   ~ValTable();

        // number of entries in a val hash table
    int count();

    void ForAll(ValTableForAllFunct aFunct, ...);

        // add a ValEntry to the table
    void add_entry(ValEntry *ve);

        // get index of ValEntry in table
    int operator[](ValEntry *ve);

        // get ValEntry for given index
    ValEntry &operator[](int index);

        //  ASCII database I/O routines
    Boolean Write(FormattedFile& port);

    Boolean Read(FormattedFile& port);

  private:
        // virtual functions for hashing and comparing ValEntrys that override 
        // the defaults for HashTable
    virtual uint HashFunct(const void *entry, const unsigned int size);
    virtual int  EntryCompare(const void *e1, const void *e2); /* 0 if equal */

       //  delete a ValEntry from the table only when destructing table...
       //  garbage collection?  what is that?
    virtual void EntryCleanup(void *entry);
};

#endif
