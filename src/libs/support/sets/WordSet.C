/* $Id: WordSet.C,v 1.2 1997/03/27 20:51:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// 
// WordSet: 
// 
//   This table can be used to store a set of words
//                                                                          
// WordSetIterator: 
//
//   for enumerating entries in a WordSet
//
// Author:  John Mellor-Crummey                       January 1994 
//
// Copyright 1994, Rice University
//***************************************************************************


#include <stdio.h>
#include <libs/support/sets/WordSet.h>
#include <libs/support/file/FormattedFile.h>


//**********************************************************************
// implementation of class WordSet
//**********************************************************************


WordSet::WordSet()
{
  HashTable::Create(sizeof(unsigned long), 8);
}


WordSet::WordSet(WordSet &rhs)
{
   *this |= rhs;
}


WordSet::~WordSet()
{
  HashTable::Destroy();
}


uint WordSet::HashFunct(const void *entry, const uint size)
{
  return *((unsigned long *) entry) % size;
}


int WordSet::EntryCompare(const void *e1, const void *e2)
{
  return  *((unsigned long *) e1) - *((unsigned long *) e2);
}
  

void WordSet::Add(unsigned long entry) 
{
  HashTable::AddEntry(&entry); 
}

  
void WordSet::Delete(unsigned long entry)
{
  HashTable::DeleteEntry(&entry); 
}
  

int WordSet::IsMember(unsigned long entry)
{
  void *found  = HashTable::QueryEntry(&entry);
  return (found != 0);
}


unsigned long WordSet::GetEntryByIndex(unsigned int indx) 
{
  return *((unsigned long *) HashTable::GetEntryByIndex(indx)); 
}


int WordSet::operator==(WordSet &rhs)
{
  //-----------------------------------------
  // false if sets have different cardinality
  //-----------------------------------------
  if (rhs.NumberOfEntries() != NumberOfEntries()) return 0;

  //-----------------------------------------
  // false if some word in rhs is not in lhs
  //-----------------------------------------
  WordSetIterator words(&rhs);
  unsigned long *word;
  for (; word = words.Current(); ++words) if (IsMember(*word) == 0) return 0;

  return 1; // equal otherwise
}


void WordSet::operator|=(WordSet &rhs)
{
  WordSetIterator words(&rhs);
  unsigned long *word;
  for (; word = words.Current(); ++words) Add(*word);
}


int WordSet::Read(FormattedFile *file)
{ 
  unsigned int nentries;
  if (file->Read(nentries)) return EOF;
  while (nentries-- > 0) {
    unsigned int tmp;
    if (file->Read(tmp)) return EOF; 
    Add(tmp);
  }
  return 0;
}


int WordSet::Write(FormattedFile *file)
{ 
  unsigned int nentries = NumberOfEntries();
  if (file->Write(nentries)) return EOF;

  WordSetIterator words(this);
  unsigned long *word;
  for (; word = words.Current(); ++words) 
    if (file->Write(*word)) return EOF;

  return 0;
}



//**********************************************************************
// implementation of class WordSetIterator
//**********************************************************************

WordSetIterator::WordSetIterator(const WordSet *theTable)
: HashTableIterator((const HashTable *) theTable)
{
}


unsigned long *WordSetIterator::Current() const
{
  return (unsigned long *) HashTableIterator::Current();
}

