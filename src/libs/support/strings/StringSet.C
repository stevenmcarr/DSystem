/* $Id: StringSet.C,v 1.1 1997/03/11 14:37:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*************************************************************************** 
// StringSet.C                                                January 1994  
//
// Author: John Mellor-Crummey                                             
//                                                                        
// Each entry in the set contains a pointer to a string.                  
//                                                                         
// If automatic cleanup of the strings is needed (i.e. they were allocated
// dynamically), then the StringSet can be passed a pointer to the        
// cleanup function when it is created.  This cleanup function will then  
// be used on each string in the StringSet.
//                                                  
// Copyright 1994, Rice University
//***************************************************************************


#include <libs/support/strings/StringSet.h>
#include <libs/support/tables/namedObject/NamedObject.h>
#include <libs/support/strings/rn_string.h>



//***************************************************************************
// class StringSet interface operations
//***************************************************************************


StringSet::StringSet()
{
}


StringSet::StringSet(StringSet &rhs)
{
  *this |= rhs;
}


StringSet::~StringSet()
{ 
  NamedObjectTable::Destroy();
}


void StringSet::Add(const char *string)
{ 
  if (IsMember(string) == 0) 
    NamedObjectTable::AddEntry(new NamedObjectIO(string));
}


void StringSet::Delete(const char *string)
{ 
  NamedObjectTable::DeleteEntry(string);
}



int StringSet::IsMember(const char *string)
{ 
  return (NamedObjectTable::QueryEntry(string) ? 1 : 0);
}


const char *StringSet::GetMember(unsigned int canonicalIndex)
{ 
  NamedObject *n = NamedObjectTable::GetEntryByIndex(canonicalIndex);
  return (n ? n->name : 0);
}


int StringSet::GetMemberIndex(const char *name)
{ 
  return NamedObjectTable::GetEntryIndex(name);
}

int StringSet::operator!=(StringSet &rhs)
{
	return !(*this == rhs);
}

int StringSet::operator==(StringSet &rhs)
{
  //-----------------------------------------
  // false if sets have different cardinality
  //-----------------------------------------
  if (NumberOfEntries() != rhs.NumberOfEntries()) return 0;

  //-----------------------------------------
  // false if some string in rhs is not in lhs
  //-----------------------------------------
  StringSetIterator strings(&rhs);
  const char *s;
  for (; s = strings.Current(); strings++) if (IsMember(s) == 0) return 0;

  return 1; // equal otherwise
}


void StringSet::operator|=(StringSet &rhs)
{
  StringSetIterator strings(&rhs);
  const char *string;
  for (; string = strings.Current(); strings++) Add(string);
}


int StringSet::Read(FormattedFile *file)
{ 
  return NamedObjectTableIO::NamedObjectTableRead(file);
}


int StringSet::Write(FormattedFile *file)
{ 
  return NamedObjectTableIO::NamedObjectTableWrite(file);
}


NamedObjectIO *StringSet::NewEntry()
{ 
  return new NamedObjectIO(0);
}


void StringSet::Dump()
{ 
  NamedObjectTableDump();
}


void StringSet::DumpContents()
{ 
  NamedObjectTableDumpContents();
}


//***************************************************************************
// class StringSetIterator interface operations
//***************************************************************************


StringSetIterator::StringSetIterator(const StringSet *set) :
NamedObjectTableIterator((NamedObjectTable *) set)
{
}


StringSetIterator::~StringSetIterator()
{ 
}


const char *StringSetIterator::Current() const
{ 
  NamedObject *current = NamedObjectTableIterator::Current();
  return (current ? current->name : 0);
}

