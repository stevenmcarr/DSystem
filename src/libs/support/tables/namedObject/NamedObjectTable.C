/* $Id: NamedObjectTable.C,v 1.6 1997/03/11 14:37:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// 
// NamedObjectTable: 
// 
//   This table can be used to store (by reference) anything derived from 
//   the NamedObject base class. 
//                                                                          
// NamedObjectTableIterator: 
//
//   for enumerating entries in a NamedObjectTable
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
#include <libs/support/tables/namedObject/NamedObject.h>
#include <libs/support/tables/namedObject/NamedObjectTable.h>
#include <libs/support/msgHandlers/DumpMsgHandler.h>


//**********************************************************************
// implementation of class NamedObjectTable
//**********************************************************************

CLASS_NAME_IMPL(NamedObjectTable)


NamedObjectTable::NamedObjectTable()
{
  Create();
}


void NamedObjectTable::Create()
{
  HashTable::Create(sizeof(NamedObject *), 8);
}


NamedObjectTable::~NamedObjectTable()
{
  Destroy();
}


//----------------------------------------------------------------------------
// definition of virtual function for hashing entries in the 
// table (this definition overrides the virtual function of the base class)
//----------------------------------------------------------------------------
uint NamedObjectTable::HashFunct(const void *entry, const uint size)
{
  return hash_string((char *)(*(NamedObject **) entry)->name, size);
}


//----------------------------------------------------------------------------
// definition of virtual function for comparing entries in the 
// table (this definition overrides the virtual function of the base class)
//----------------------------------------------------------------------------
int NamedObjectTable::EntryCompare(const void *e1, const void *e2)
{
  return strcmp((*(NamedObject **) e1)->name, (*(NamedObject **) e2)->name);
}
  

// add an annotation to the table
void NamedObjectTable::AddEntry(NamedObject *a) 
{
  NamedObject *p = a;
  HashTable::AddEntry(&p); // copy pointer into table
}

  
// delete an annotation from the table
void NamedObjectTable::DeleteEntry(const char *const name)
{
  NamedObject a(name);
  NamedObject *p = &a;
  HashTable::DeleteEntry(&p); 
}
  

void NamedObjectTable::EntryCleanup(void *entry)
{
  delete (*(NamedObject **) entry);
}


void NamedObjectTable::NamedObjectTableDumpUpCall()
{
}


void NamedObjectTable::NamedObjectTableDump()
{
  dumpHandler.Dump("%s\n", ClassName());
  dumpHandler.BeginScope();
  NamedObjectTableDumpUpCall();
  NamedObjectTableDumpContents();
  dumpHandler.EndScope();
}


void NamedObjectTable::NamedObjectTableDumpContents()
{
  NamedObjectTableIterator noti(this);
  NamedObject *entry;
  for (; entry = noti.Current(); noti++) {
    entry->NamedObjectDump();
  }
}



// test for presence of an annotation in the table
NamedObject *NamedObjectTable::QueryEntry(const char *const name) const 
{
  NamedObject a(name);
  NamedObject *p = &a; 
  NamedObject **found = (NamedObject **) HashTable::QueryEntry(&p);
  return (found ? *found : 0);
}

// find index of an item
int NamedObjectTable::GetEntryIndex(const char *name) const
{
  NamedObject a(name);
  NamedObject *p = &a; 
  return HashTable::GetEntryIndex(&p);
}

// access item at position "index" in table
NamedObject *NamedObjectTable::GetEntryByIndex(const uint index) const
{
  NamedObject **found = (NamedObject **) HashTable::GetEntryByIndex(index);
  return (found ? *found : 0);
}


//**********************************************************************
// implementation of class NamedObjectTableIO
//**********************************************************************

CLASS_NAME_IMPL(NamedObjectTableIO)


NamedObjectTableIO::NamedObjectTableIO()
{
}


NamedObjectTableIO::~NamedObjectTableIO()
{
}


int NamedObjectTableIO::NamedObjectTableWriteUpCall(FormattedFile *)
{
  return 0;
}


int NamedObjectTableIO::NamedObjectTableReadUpCall(FormattedFile *)
{
  return 0;
}


int NamedObjectTableIO::NamedObjectTableRead(FormattedFile *file)
{ 
  uint n;
  int code = file->Read(n);
  if (code) return EOF;

  while (n-- != 0) {
    NamedObjectIO* no = NewEntry();

    code = no->NamedObjectRead(file);
    if (code) {
      delete no;
      return EOF;
    }

    AddEntry(no);
  }

  return NamedObjectTableReadUpCall(file);
}

int NamedObjectTableIO::NamedObjectTableWrite(FormattedFile *file)
{ 
  uint n = NumberOfEntries();
  int code = file->Write(n);
  if (code) return EOF;

  NamedObjectTableIterator it(this);
  NamedObjectIO* no;

  for (; no = (NamedObjectIO *) it.Current(); it++) {
    code = no->NamedObjectWrite(file);
    if (code) return EOF;
  }

  return NamedObjectTableWriteUpCall(file);
}




//**********************************************************************
// implementation of class NamedObjectTableIterator
//**********************************************************************

NamedObjectTableIterator::NamedObjectTableIterator(const NamedObjectTable *theTable)
: HashTableIterator((const HashTable *) theTable)
{
}


NamedObject *NamedObjectTableIterator::Current() const
{
  void *entry =  HashTableIterator::Current();
  return (entry ? *(NamedObject **) entry : 0);
}

