/* $Id: WordObjectTable.h,v 1.5 1997/03/11 14:37:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef WordObjectTable_h
#define WordObjectTable_h

//*************************************************************************** 
//
// WordObjectTable
//                                                                          
//   can be used to store (by reference) anything derived from the 
//   WordObject base class.
//                                                                          
// WordObjectTableIterator: 
//
//   for enumerating entries in a WordObjectTable
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

class WordObject;             // minimal external declaration
class WordObjectIO;           // minimal external declaration
class FormattedFile;           // minimal external declaration


//-------------------------------------------------------------
// class WordObjectTable
//-------------------------------------------------------------
class WordObjectTable : private HashTable {
public:
  WordObjectTable(); 
  virtual ~WordObjectTable(); 

  void Create();

  //-------------------------------------------------------------
  // add an item to the table
  //-------------------------------------------------------------
  void AddEntry(WordObject *a);       

  //-------------------------------------------------------------
  // delete matching item from the table
  //-------------------------------------------------------------
  void DeleteEntry(const unsigned int id);        

  //-------------------------------------------------------------
  // get item matching id, if any
  //-------------------------------------------------------------
  WordObject *QueryEntry(const unsigned int id) const; 

  //-------------------------------------------------------------
  // access item at position "index" in table
  //-------------------------------------------------------------
  WordObject *GetEntryByIndex(const uint index) const;

  //-------------------------------------------------------------
  // returns the position in the table of the Word object;
  // -1 if not present
  //-------------------------------------------------------------
  int GetEntryIndex(const unsigned int id) const;

  //-------------------------------------------------------------
  // default cleanup function provided here deletes 
  // the WordObject referred to by entry
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
  void WordObjectTableDump();
  virtual void WordObjectTableDumpUpCall();

  CLASS_NAME_FDEF(WordObjectTable);

private:
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
friend class WordObjectTableIterator;
};


//-------------------------------------------------------------
// class WordObjectTableIO
//-------------------------------------------------------------
class WordObjectTableIO : public WordObjectTable {
public:
  WordObjectTableIO(); 
  virtual ~WordObjectTableIO(); 

  //-------------------------------------------------------------
  // support for I/O of derived classes
  //-------------------------------------------------------------
  int WordObjectTableRead(FormattedFile *file);
  virtual int WordObjectTableReadUpCall(FormattedFile *file);
  virtual WordObjectIO *NewWordObjectIO() = 0;

  int WordObjectTableWrite(FormattedFile *file);
  virtual int WordObjectTableWriteUpCall(FormattedFile *file);
  
  CLASS_NAME_FDEF(WordObjectTableIO);
};


//-------------------------------------------------------------
// class WordObjectTableIterator
//-------------------------------------------------------------
class WordObjectTableIterator : private HashTableIterator {
public:
  WordObjectTableIterator(const WordObjectTable *theTable);
  WordObject *Current() const;
  HashTableIterator::operator++;
  HashTableIterator::Reset;
};

#endif /* WordObjectTable_h */
