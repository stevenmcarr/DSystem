/* $Id: WordSet.h,v 1.4 1997/03/11 14:37:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef WordSet_h
#define WordSet_h

//*************************************************************************** 
//
// WordSet.h
//                                                                          
// Author:  John Mellor-Crummey                             January 1994
//
// Copyright 1994, Rice University
//***************************************************************************


#ifndef HashTable_h
#include <libs/support/tables/HashTable.h>
#endif

class FormattedFile; // minimal external declaration


//-------------------------------------------------------------
// class WordSet
//-------------------------------------------------------------
class WordSet : private HashTable {
public:
  WordSet(); 
  WordSet(WordSet &rhs); 
  ~WordSet(); 
  
  void Add(unsigned long entry);       
  void Delete(unsigned long entry);
  int IsMember(unsigned long entry);

  unsigned long GetEntryByIndex(unsigned int indx);

  int operator==(WordSet &rhs);
  void operator|=(WordSet &rhs);
  
  HashTable::NumberOfEntries;
  
  int Read(FormattedFile *file);
  int Write(FormattedFile *file);
  
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
friend class WordSetIterator;
};


//-------------------------------------------------------------
// class WordSetIterator
//-------------------------------------------------------------
class WordSetIterator : private HashTableIterator {
public:
  WordSetIterator(const WordSet *theTable);
  unsigned long *Current() const;
  HashTableIterator::operator++;
  HashTableIterator::Reset;
};

#endif /* WordSet_h */
