/* $Id: val_pass.h,v 1.8 1997/03/11 14:36:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/**************************************************************************** 
 *  Hash table for Passed Values of Parameters                    June 1993 *
 *  Author: Paul Havlak                                                     *
 *                                                                          *
 *  This hash table is derived from the generic HashTable class;	    *
 *  maps from (call site, parm #) pairs to value numbers for the passed     *
 *  value.  This is necessary, despite the annotation of return value       *
 *  numbers with the passed value, because of the existence of nonreference *
 *  actual parameters (e.g., expressions).				    *
 *                                                                          *
 *  Copyright 1991, 1993 Rice University, as part of the Rn/ParaScope       *
 *  Programming Environment Project.                                        *
 *                                                                          *
 ****************************************************************************/

#ifndef val_pass_h
#define val_pass_h

#include <libs/support/misc/general.h>

#include <libs/support/tables/HashTable.h>
#include <libs/support/misc/port.h>
#include <libs/support/strings/rn_string.h>

#include <libs/moduleAnalysis/valNum/val_enum.h>

class FormattedFile;  // minimal external declaration

class ValPassEntry 
{
  public:
    friend class ValPassMap;

  private:
    int site;
    char *name;
    uint offset;
    ValNumber val;

    ValPassEntry(int s, char *n, uint o, ValNumber v);
    ~ValPassEntry();

    unsigned int hash(unsigned int size);
    int compare(ValPassEntry *e);

    void ValPassEntry::Dump(void);
    void ValPassEntry::Write(FormattedFile& port);
    void ValPassEntry::Read(FormattedFile& port);
};

class ValPassMap : private HashTable 
{
  public:
    ValPassMap();

      // number of entries in hash table
    unsigned int count(void);

      // add to the table
    void add_entry(int s, char *n, uint o, ValNumber v);

    void add_entry(ValPassEntry *ep);

      //  add passed value
    void add_entry(int s, uint p, ValNumber v);

      //  add returned value
    void add_entry(char * n, uint o, ValNumber v);

      // delete any entry with given key from the table
    void delete_entry(int s, char *n, uint o);

      // test for presence of the key in the table
    int query_entry(int s, char *n, uint o = 0);

      // test for presence of passed value
    int query_entry(int s, uint p);

      // test for presence of returned value
    int query_entry(char *n, uint o = 0);

    void Dump(void);

      //  ASCII database I/O routines
    Boolean Write(FormattedFile& port);
    Boolean Read(FormattedFile& port);

  private:
    virtual unsigned int HashFunct(const void *entry, const unsigned int size);
    virtual int EntryCompare(const void *e1, const void *e2);
    virtual void EntryCleanup(void *entry);

      // get entry for given index
    ValPassEntry &operator[](int index);
};

#endif val_pass_h
