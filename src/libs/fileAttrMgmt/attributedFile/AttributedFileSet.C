/* $Id: AttributedFileSet.C,v 1.2 1997/03/27 20:31:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
// AttributedFileSet.C: 
//
//  set of FortranModules and FortranCompositions that are
//  the components of a composition
//
// Author: 
//   John Mellor-Crummey                              December 1993
//
// Copyright 1993, Rice University
//    
//******************************************************************

#include <stdlib.h>
#include <assert.h>

#include <libs/support/tables/HashTable.h>
#include <libs/support/tables/namedObject/NamedObject.h>
#include <libs/support/tables/namedObject/NamedObjectTable.h>

#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>
#include <libs/fileAttrMgmt/attributedFile/AttributedFileSet.h>
#include <libs/fileAttrMgmt/attributedFile/FileSuffixRegistry.h>


//****************************
// forward declarations
//****************************

static uint AttrFileEntryHashFunct(const void* entry, const uint size);
static int AttrFileEntryCompare(const void* entry1, const void* entry2);



//***************************************************************************
// interface operations for class AttrFileEntry
//***************************************************************************

class AttrFileEntry : public NamedObject {
public:
  uint refCnt;
  AttributedFile *attrFile;
  AttrFileEntry(AttributedFile *attrFile);
  ~AttrFileEntry();
};


AttrFileEntry::AttrFileEntry(AttributedFile *_attrFile) : 
NamedObject(ssave(_attrFile->ReferenceFilePathName()))
{
  refCnt = 0;
  attrFile = _attrFile;
}


AttrFileEntry::~AttrFileEntry()
{
  attrFile->Close();
  delete attrFile;
  sfree((char *) name);
}



AttributedFileSet::AttributedFileSet()  
{
  attrFileSetRepr = new AttributedFileSetS;
}


AttributedFileSet::~AttributedFileSet()
{
  attrFileSetRepr->ht.Destroy();
  delete attrFileSetRepr;
}


AttributedFile *AttributedFileSet::GetFile(const char *name)
{
  AttrFileEntry *ame = (AttrFileEntry *) attrFileSetRepr->ht.QueryEntry(name);
  return (ame ? ame->attrFile : 0);
}


AttributedFile *AttributedFileSet::OpenFile(const char *name)
{
  AttrFileEntry *ame = (AttrFileEntry *) attrFileSetRepr->ht.QueryEntry(name);
  if (ame == 0) {
    FileSuffixHandle *fsh = 
      GetFileSuffixRegistry()->Lookup(ExtractFileSuffix(name));

    if (fsh == 0) return 0;
    
    AttributedFile *attrFile = fsh->New(); 

    int code = attrFile->Open(name);
    if (code) {
      delete attrFile;
      return 0;
    } 
    ame = new AttrFileEntry(attrFile);
    attrFileSetRepr->ht.AddEntry(ame);
  }
  ame->refCnt++;
  return ame->attrFile;
}

void AttributedFileSet::CloseFile(AttributedFile *attrFile)
{
  AttrFileEntry *ame = (AttrFileEntry *) 
    attrFileSetRepr->ht.QueryEntry(attrFile->ReferenceFilePathName());
  assert(ame != 0);
  ame->refCnt--;
  assert(ame->refCnt == 0);
  if (ame->refCnt == 0) attrFileSetRepr->ht.DeleteEntry(ame->name);
}

AttributedFileSetIterator::AttributedFileSetIterator
(const AttributedFileSet *attrFileSet) 
{
  attrFileSetIterRepr = 
    new AttributedFileSetIteratorS(&attrFileSet->attrFileSetRepr->ht);
}


AttributedFileSetIterator::~AttributedFileSetIterator()
{
  delete attrFileSetIterRepr;
}


AttributedFile *AttributedFileSetIterator::Current() const
{
  AttrFileEntry *e =  (AttrFileEntry *) attrFileSetIterRepr->noti.Current();
  return (e ? e->attrFile : 0);
}


void *AttributedFileSetIterator::CurrentUpCall() const
{
  return Current();
}


void AttributedFileSetIterator::operator++() 
{
  ++attrFileSetIterRepr->noti;
}


void AttributedFileSetIterator::Reset() 
{
  attrFileSetIterRepr->noti.Reset();
}

