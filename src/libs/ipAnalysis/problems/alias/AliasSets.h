/* $Id: AliasSets.h,v 1.1 1997/03/11 14:34:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef AliasSets_h
#define AliasSets_h


//**********************************************************************
// AliasSets.h
//
// Quick and dirty implementation of alias pairs for scalar aliases
//
// Author: John Mellor-Crummey                          March 1994
//
// Copyright 1994, Rice University
//**********************************************************************

#include <libs/support/tables/namedObject/NamedObject.h>
#include <libs/support/tables/namedObject/NamedObjectTable.h>
#include <libs/support/strings/StringSet.h>

#include <libs/ipAnalysis/ipInfo/OffsetLength.h>
#include <libs/ipAnalysis/ipInfo/EqClassPairs.h>



//---------------------------------------------------------------------
// class FormalAliases:  set of formal parameters aliased to 
//                       formal parameter "name"
//---------------------------------------------------------------------
class FormalAliases: public NamedObjectIO, public StringSet, 
public EqClassPairSet {
  int NamedObjectReadUpCall(FormattedFile *file);
  int NamedObjectWriteUpCall(FormattedFile *file);
public:
  FormalAliases(FormalAliases &);
  FormalAliases(const char *_name = 0);
  ~FormalAliases();
  void AddGlobalAlias(const char *_name, int offset, int length);
};


//---------------------------------------------------------------------
// class GlobalAliases:  set of formal parameters aliased to 
//                       global (name, offset, length) triple
//---------------------------------------------------------------------
class GlobalAliases: public StringSet {
private:
  GlobalAliases();
public:
  const char *name;
  OffsetLengthPair globalInfo;

  GlobalAliases(const char *name, int offset, int length);
  GlobalAliases(GlobalAliases &);
  ~GlobalAliases();

  static int CompareKey(GlobalAliases *left, GlobalAliases *right);
  unsigned int Hash(unsigned int size);

  int Read(FormattedFile *file);
  int Write(FormattedFile *file);
friend class GlobalAliasesSet;
};


//---------------------------------------------------------------------
// class FormalAliasesSet:  set of sets of aliases for each formal 
//                          parameter 
//---------------------------------------------------------------------
class FormalAliasesSet : private NamedObjectTableIO {
public:
  FormalAliasesSet();
  FormalAliasesSet(FormalAliasesSet &);

  ~FormalAliasesSet();

  FormalAliases *QueryEntry(const char *_name) const;
  FormalAliases *GetEntry(const char *_name);

  void operator|=(const FormalAliasesSet &rhs);
  int operator==(const FormalAliasesSet &rhs) const;

  int Read(FormattedFile *);
  int Write(FormattedFile *);
private:
  NamedObjectIO *NewEntry();
friend class FormalAliasesSetIterator;
};


//---------------------------------------------------------------------
// class GlobalAliasesSet:  set of sets of aliases for each global 
//---------------------------------------------------------------------
class GlobalAliasesSet : private HashTable {
public:
  GlobalAliasesSet();
  GlobalAliasesSet(GlobalAliasesSet&);

  ~GlobalAliasesSet();

  GlobalAliases *QueryEntry(const char *name, int offset, int length) const;
  GlobalAliases *GetEntry(const char *name, int offset, int length);
  void AddEntry(GlobalAliases *newAlias);

  void operator|=(const GlobalAliasesSet &rhs);
  int operator==(const GlobalAliasesSet &rhs) const;

  int Read(FormattedFile *);
  int Write(FormattedFile *);
private:
  uint HashFunct(const void *entry, const uint size);
  int EntryCompare(const void *entry1, const void *entry2); // 0 if equal
  void EntryCleanup(void *entry); 
};


//---------------------------------------------------------------------
// class FormalAliasesSetIterator: enumerate FormalAliasesSet members 
//---------------------------------------------------------------------
class FormalAliasesSetIterator : private NamedObjectTableIterator {
public:
  FormalAliasesSetIterator(const FormalAliasesSet *annot);
  ~FormalAliasesSetIterator();
  FormalAliases *Current() const;
  NamedObjectTableIterator::operator++;
  NamedObjectTableIterator::Reset;
};


//---------------------------------------------------------------------
// class GlobalAliasesSetIterator:  enumerate GlobalAliasesSet members
//---------------------------------------------------------------------
class GlobalAliasesSetIterator : private HashTableIterator {
public:
  GlobalAliasesSetIterator(const GlobalAliasesSet *annot);
  ~GlobalAliasesSetIterator();
  GlobalAliases *Current() const;
  HashTableIterator::operator++;
  HashTableIterator::Reset;
};



#endif /* AliasSets_h */



