/* $Id: AliasSets.C,v 1.2 1997/03/27 20:40:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//**********************************************************************
// AliasSets.C
//
// Quick and dirty implementation of alias pairs for scalar aliases
//
// Author: John Mellor-Crummey                          March 1994
//
// Copyright 1994, Rice University
//**********************************************************************

#include <string.h>
#include <stdio.h>

#include <libs/support/strings/StringIO.h>
#include <libs/ipAnalysis/problems/alias/AliasSets.h>
#include <libs/support/file/FormattedFile.h>
#include <libs/support/strings/rn_string.h>



//**********************************************************************
// class FormalAliases interface operations
//**********************************************************************

FormalAliases::FormalAliases(const char *_name) : NamedObjectIO(_name)
{
}


FormalAliases::FormalAliases(FormalAliases &rhs) : 
NamedObjectIO(rhs.name), StringSet(rhs), EqClassPairSet(rhs)
{
}


FormalAliases::~FormalAliases()
{
}


int FormalAliases::NamedObjectReadUpCall(FormattedFile *file)
{
  return StringSet::Read(file) || EqClassPairSet::Read(file);
}


int FormalAliases::NamedObjectWriteUpCall(FormattedFile *file)
{
  return StringSet::Write(file) || EqClassPairSet::Write(file);
}

void FormalAliases::AddGlobalAlias(const char *_name, int offset, int length)
{
  EqClassPairs *pairs = GetEntry(_name);
  pairs->AddPair(new OffsetLengthPair(offset, length));
}


//**********************************************************************
// class GlobalAliases interface operations
//**********************************************************************

GlobalAliases::GlobalAliases()
{
}

GlobalAliases::GlobalAliases(GlobalAliases &rhs) : 
name(rhs.name ? ssave(rhs.name) : 0), globalInfo(rhs.globalInfo)
{
}


GlobalAliases::GlobalAliases(const char *_name, int offset, int length) : 
name(_name ? ssave(_name) : 0), globalInfo(offset, length)
{
}


GlobalAliases::~GlobalAliases()
{
  if (name) sfree((char *) name);
}


int GlobalAliases::Read(FormattedFile *file)
{
  return ReadString((char **) &name, file) || file->Read(globalInfo.offset) || 
    file->Read(globalInfo.length) || StringSet::Read(file);
}


int GlobalAliases::Write(FormattedFile *file)
{
  return WriteString(name, file) || file->Write(globalInfo.offset) || 
    file->Write(globalInfo.length) || StringSet::Write(file);
}


int GlobalAliases::CompareKey(GlobalAliases *left, GlobalAliases *right)
{
  return strcmp(left->name, right->name) || 
    (left->globalInfo.offset - right->globalInfo.offset) || 
    (left->globalInfo.length - right->globalInfo.length);
}


unsigned int GlobalAliases::Hash(unsigned int size)
{
  return (hash_string((char *) name, size) + 
	  globalInfo.offset + globalInfo.length) % size;
}


//**********************************************************************
// class FormalAliasesSet interface operations
//**********************************************************************

FormalAliasesSet::FormalAliasesSet()
{
}


FormalAliasesSet::FormalAliasesSet(FormalAliasesSet &rhs) 
{
  FormalAliasesSetIterator entries(&rhs);
  FormalAliases *entry;
  for(; entry = entries.Current(); ++entries) {
    AddEntry(new FormalAliases(*entry));
  }
}


FormalAliasesSet::~FormalAliasesSet()
{
  Destroy();
}


FormalAliases *FormalAliasesSet::QueryEntry(const char *name) const
{
  return (FormalAliases *) NamedObjectTable::QueryEntry(name);
}


FormalAliases *FormalAliasesSet::GetEntry(const char *name)
{
  FormalAliases *fa = QueryEntry(name);
  if (fa == 0) {
    fa = new FormalAliases(name);
    AddEntry(fa);
  }
  return fa;
}


void FormalAliasesSet::operator|=(const FormalAliasesSet &rhs)
{
  FormalAliasesSetIterator fsi(&rhs);
  for (FormalAliases *rfs; rfs = fsi.Current(); ++fsi) {
    FormalAliases *lfs = GetEntry(rfs->name);
    lfs->StringSet::operator|=(*rfs);
    lfs->EqClassPairSet::operator|=(*rfs);
  }
}


int FormalAliasesSet::operator==(const FormalAliasesSet &rhs) const
{
  FormalAliasesSetIterator fsi(&rhs);
  for (FormalAliases *rfs; rfs = fsi.Current(); ++fsi) {
    FormalAliases *lfs = QueryEntry(rfs->name);
    if (lfs == 0 || !(lfs->EqClassPairSet::operator==(*rfs)) ||
	!(lfs->StringSet::operator==(*rfs)))
      return 0;
  }
  return 1;
}



int FormalAliasesSet::Read(FormattedFile *file)
{
  return NamedObjectTableRead(file);
}


int FormalAliasesSet::Write(FormattedFile *file)
{
  return NamedObjectTableWrite(file);
}

NamedObjectIO *FormalAliasesSet::NewEntry()
{
  return new FormalAliases();
}



//**********************************************************************
// class GlobalAliasesSet interface operations
//**********************************************************************

GlobalAliasesSet::GlobalAliasesSet()
{
  Create(sizeof(GlobalAliases *), 8);
}


GlobalAliasesSet::GlobalAliasesSet(GlobalAliasesSet &rhs)
{
  Create(sizeof(GlobalAliases *), 8);
  GlobalAliasesSetIterator it(&rhs);
  GlobalAliases* ra;
  for (; ra = it.Current(); ++it) AddEntry(new GlobalAliases(*ra));
}


GlobalAliasesSet::~GlobalAliasesSet()
{
  Destroy();
}


GlobalAliases *GlobalAliasesSet::GetEntry(const char *name, int offset, 
					  int length)
{
  GlobalAliases *ga = QueryEntry(name,offset,length);
  if (ga == 0)  {
    ga = new GlobalAliases(name, offset, length);
    AddEntry(ga);
  }
  return ga;
}


GlobalAliases *GlobalAliasesSet::QueryEntry(const char *name, int offset, 
					    int length) const
{
  GlobalAliases a(name, offset, length);
  GlobalAliases *p = &a; 
  GlobalAliases **found = (GlobalAliases **) HashTable::QueryEntry(&p);
  return (found ? *found : 0);
}


void GlobalAliasesSet::AddEntry(GlobalAliases *a) 
{
  GlobalAliases *p = a;
  HashTable::AddEntry(&p); // copy pointer into table
}


void GlobalAliasesSet::operator|=(const GlobalAliasesSet &rhs)
{
  GlobalAliasesSetIterator fsi(&rhs);
  GlobalAliases *rfs;
  for (; rfs = fsi.Current(); ++fsi) {
    GlobalAliases *lfs = 
      GetEntry(rfs->name, rfs->globalInfo.offset, rfs->globalInfo.length);
    *lfs |= *rfs;
  }
}


int GlobalAliasesSet::operator==(const GlobalAliasesSet &rhs) const
{
  GlobalAliasesSetIterator fsi(&rhs);
  GlobalAliases *rfs;
  for (; rfs = fsi.Current(); ++fsi) {
    GlobalAliases *lfs = 
      QueryEntry(rfs->name, rfs->globalInfo.offset, rfs->globalInfo.length);
    if (lfs == 0 || !(*lfs == *rfs)) return 0;
  }
  return 1;
}


int GlobalAliasesSet::Read(FormattedFile *file)
{
  uint n;
  if (file->Read(n)) return EOF;

  while (n-- != 0) {
    GlobalAliases *a = new GlobalAliases();
    if (a->Read(file)) {
      delete a;
      return EOF;
    }
    AddEntry(a);
  }
  return 0;
}


int GlobalAliasesSet::Write(FormattedFile *file)
{
  uint n = NumberOfEntries();
  if (file->Write(n)) return EOF;
  GlobalAliasesSetIterator it(this);
  for (GlobalAliases* a; a = it.Current(); ++it) {
    if (a->Write(file)) return EOF;
  }
}


uint GlobalAliasesSet::HashFunct(const void *entry, const uint size)
{
  return (*(GlobalAliases **) entry)->Hash(size); 
}


int GlobalAliasesSet::EntryCompare(const void *entry1, const void *entry2)
{
  return GlobalAliases::CompareKey(*(GlobalAliases **) entry1, 
				   *(GlobalAliases **) entry2);
}


void GlobalAliasesSet::EntryCleanup(void *entry)
{
  delete (*(GlobalAliases **) entry);
}


//**********************************************************************
// class FormalAliasesSetIterator interface operations
//**********************************************************************

FormalAliasesSetIterator::FormalAliasesSetIterator
(const FormalAliasesSet *set) :
NamedObjectTableIterator((NamedObjectTable *) set)
{
}


FormalAliasesSetIterator::~FormalAliasesSetIterator()
{
}


FormalAliases *FormalAliasesSetIterator::Current() const
{ 
  return (FormalAliases *) NamedObjectTableIterator::Current();
}


//**********************************************************************
// class GlobalAliasesSetIterator
//**********************************************************************

GlobalAliasesSetIterator::GlobalAliasesSetIterator
(const GlobalAliasesSet *theSet)
: HashTableIterator((const HashTable *) theSet)
{
}


GlobalAliasesSetIterator::~GlobalAliasesSetIterator()
{
}


GlobalAliases *GlobalAliasesSetIterator::Current() const
{
  void *entry =  HashTableIterator::Current();
  return (entry ? *(GlobalAliases **) entry : 0);
}
