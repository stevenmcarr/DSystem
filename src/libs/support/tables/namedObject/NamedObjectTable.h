/* $Id: NamedObjectTable.h,v 1.5 1997/03/11 14:37:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef NamedObjectTable_h
#define NamedObjectTable_h

//*************************************************************************** 
//
// NamedObjectTable
//                                                                          
//   can be used to store (by reference) anything derived from the 
//   NamedObject base class.
//                                                                          
// NamedObjectTableIterator: 
//
//   for enumerating entries in a NamedObjectTable
//
//
// Author:  John Mellor-Crummey                            June 1993
//
// Copyright 1993, Rice University
//                                                                          
//***************************************************************************


#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef HashTable_h
#include <libs/support/tables/HashTable.h>
#endif

#ifndef ClassName_h
#include <include/ClassName.h>
#endif

class NamedObject;             // minimal external declaration
class NamedObjectIO;           // minimal external declaration
class FormattedFile;           // minimal external declaration


//-------------------------------------------------------------
// class NamedObjectTable
//-------------------------------------------------------------
class NamedObjectTable : private HashTable {
public:
  NamedObjectTable(); 
  virtual ~NamedObjectTable(); 

  void Create();

  //-------------------------------------------------------------
  // add an item to the table
  //-------------------------------------------------------------
  void AddEntry(NamedObject *a);       

  //-------------------------------------------------------------
  // delete matching item from the table
  //-------------------------------------------------------------
  void DeleteEntry(const char *const name);        

  //-------------------------------------------------------------
  // get item matching name, if any
  //-------------------------------------------------------------
  NamedObject *QueryEntry(const char *const name) const; 

  //-------------------------------------------------------------
  // access item at position "index" in table
  //-------------------------------------------------------------
  NamedObject *GetEntryByIndex(const uint index) const;

  //-------------------------------------------------------------
  // returns the position in the table of the named object;
  // -1 if not present
  //-------------------------------------------------------------
  int GetEntryIndex(const char *name) const;

  //-------------------------------------------------------------
  // default cleanup function provided here deletes 
  // the NamedObject referred to by entry
  //-------------------------------------------------------------
  virtual void EntryCleanup(void *entry); 

  //-------------------------------------------------------------
  // must be called before the destructor to clean up all objects
  // in the table using the EntryCleanup function. a call to destroy
  // is mandatory even if EntryCleanup is a no-op.
  //-------------------------------------------------------------
  HashTable::Destroy;

  HashTable::NumberOfEntries;

  //-------------------------------------------------------------
  // debugging output to stderr
  //-------------------------------------------------------------
  void NamedObjectTableDump();

  CLASS_NAME_FDEF(NamedObjectTable);

protected:
  void NamedObjectTableDumpContents();

private:
  virtual void NamedObjectTableDumpUpCall();
  //-------------------------------------------------------------
  // virtual functions for hashing and comparing
  // that override the defaults for HashTable
  //-------------------------------------------------------------
  uint HashFunct(const void *entry, const uint size);
  int EntryCompare(const void *entry1, const void *entry2); // 0 if equal

//-------------------------------------------------------------
// friend declaration required so HashTableIterator can be
// used with the private base class
//-------------------------------------------------------------
friend class NamedObjectTableIterator;
};


//-------------------------------------------------------------
// class NamedObjectTableIO
//-------------------------------------------------------------
class NamedObjectTableIO : public NamedObjectTable {
public:
  NamedObjectTableIO(); 
  virtual ~NamedObjectTableIO(); 

  //-------------------------------------------------------------
  // support for I/O of derived classes
  //-------------------------------------------------------------
  int NamedObjectTableRead(FormattedFile *file);
  virtual int NamedObjectTableReadUpCall(FormattedFile *file);
  virtual NamedObjectIO *NewEntry() = 0;

  int NamedObjectTableWrite(FormattedFile *file);
  virtual int NamedObjectTableWriteUpCall(FormattedFile *file);
  
  CLASS_NAME_FDEF(NamedObjectTableIO);
};


//-------------------------------------------------------------
// class NamedObjectTableIterator
//-------------------------------------------------------------
class NamedObjectTableIterator : private HashTableIterator {
public:
  NamedObjectTableIterator(const NamedObjectTable *theTable);
  NamedObject *Current() const;
  HashTableIterator::operator++;
  HashTableIterator::Reset;
};

#endif /* NamedObjectTable_h */
