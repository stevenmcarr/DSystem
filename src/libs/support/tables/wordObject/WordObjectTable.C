/* $Id: WordObjectTable.C,v 1.6 1997/03/11 14:37:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// 
// WordObjectTable: 
// 
//   This table can be used to store (by reference) anything derived from 
//   the WordObject base class. 
//                                                                          
// WordObjectTableIterator: 
//
//   for enumerating entries in a WordObjectTable
//
//
// Author:  John Mellor-Crummey                       June 1993
//
// Copyright 1993, Rice University
//                                                                          
//***************************************************************************


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <libs/support/strings/rn_string.h>
#include <libs/support/file/FormattedFile.h>
#include <libs/support/tables/wordObject/WordObject.h>
#include <libs/support/tables/wordObject/WordObjectTable.h>


//**********************************************************************
// implementation of class WordObjectTable
//**********************************************************************

CLASS_NAME_IMPL(WordObjectTable)


WordObjectTable::WordObjectTable()
{
  Create();
}


void WordObjectTable::Create()
{
  HashTable::Create(sizeof(WordObject *), 8);
}


WordObjectTable::~WordObjectTable()
{
  Destroy();
}


//----------------------------------------------------------------------------
// definition of virtual function for hashing entries in the 
// table (this definition overrides the virtual function of the base class)
//----------------------------------------------------------------------------
uint WordObjectTable::HashFunct(const void *entry, const uint size)
{
  return IntegerHashFunct((*(WordObject **) entry)->id, size);
}


//----------------------------------------------------------------------------
// definition of virtual function for comparing entries in the 
// table (this definition overrides the virtual function of the base class)
//----------------------------------------------------------------------------
int WordObjectTable::EntryCompare(const void *e1, const void *e2)
{
  return (*(WordObject **) e1)->id - (*(WordObject **) e2)->id;
}
  

// add an annotation to the table
void WordObjectTable::AddEntry(WordObject *a) 
{
  WordObject *p = a;
  HashTable::AddEntry(&p); // copy pointer into table
}

  
// delete an annotation from the table
void WordObjectTable::DeleteEntry(const unsigned int id)
{
  WordObject a(id);
  WordObject *p = &a;
  HashTable::DeleteEntry(&p); 
}
  

void WordObjectTable::EntryCleanup(void *entry)
{
  delete (*(WordObject **) entry);
}


void WordObjectTable::WordObjectTableDumpUpCall()
{
}

void WordObjectTable::WordObjectTableDump()
{
  fprintf(stderr, "%s\n", ClassName());
  WordObjectTableDumpUpCall();
  WordObjectTableIterator noti(this);
  WordObject *entry;
  for (; entry = noti.Current(); noti++) {
    entry->WordObjectDump();
    fprintf(stderr,"\n");
  }
}



// test for presence of an annotation in the table
WordObject *WordObjectTable::QueryEntry(const unsigned int id) const 
{
  WordObject a(id);
  WordObject *p = &a; 
  WordObject **found = (WordObject **) HashTable::QueryEntry(&p);
  return (found ? *found : 0);
}

// find index of an item
int WordObjectTable::GetEntryIndex(const unsigned int id) const
{
  WordObject a(id);
  WordObject *p = &a; 
  return HashTable::GetEntryIndex(&p);
}

// access item at position "index" in table
WordObject *WordObjectTable::GetEntryByIndex(const uint index) const
{
  WordObject **found = (WordObject **) HashTable::GetEntryByIndex(index);
  return (found ? *found : 0);
}


//**********************************************************************
// implementation of class WordObjectTableIO
//**********************************************************************

CLASS_NAME_IMPL(WordObjectTableIO)


WordObjectTableIO::WordObjectTableIO()
{
}


WordObjectTableIO::~WordObjectTableIO()
{
}


int WordObjectTableIO::WordObjectTableWriteUpCall(FormattedFile *)
{
  return 0;
}


int WordObjectTableIO::WordObjectTableReadUpCall(FormattedFile *)
{
  return 0;
}


int WordObjectTableIO::WordObjectTableRead(FormattedFile *file)
{ 
  uint n;
  int code = file->Read(n);
  if (code) return EOF;

  while (n-- != 0) {
    WordObjectIO* no = NewWordObjectIO();

    code = no->WordObjectRead(file);
    if (code) {
      delete no;
      return EOF;
    }

    AddEntry(no);
  }

  return WordObjectTableReadUpCall(file);
}

int WordObjectTableIO::WordObjectTableWrite(FormattedFile *file)
{ 
  uint n = NumberOfEntries();
  int code = file->Write(n);
  if (code) return EOF;

  WordObjectTableIterator it(this);
  WordObjectIO* no;

  for (; no = (WordObjectIO *) it.Current(); it++) {
    code = no->WordObjectWrite(file);
    if (code) return EOF;
  }

  return WordObjectTableWriteUpCall(file);
}




//**********************************************************************
// implementation of class WordObjectTableIterator
//**********************************************************************

WordObjectTableIterator::WordObjectTableIterator(const WordObjectTable *theTable)
: HashTableIterator((const HashTable *) theTable)
{
}


WordObject *WordObjectTableIterator::Current() const
{
  void *entry =  HashTableIterator::Current();
  return (entry ? *(WordObject **) entry : 0);
}

