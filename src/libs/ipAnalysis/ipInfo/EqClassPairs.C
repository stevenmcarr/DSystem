/* $Id: EqClassPairs.C,v 1.2 1997/03/27 20:40:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
//  EqClassPairs.C: 
//
//  a representation intervals of interest in a region of storage 
//  assigned to an equivalence class of variables
//  
// Author: 
//   John Mellor-Crummey                              January 1994
//
// Copyright 1994, Rice University
//******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <values.h>

#include <libs/support/strings/rn_string.h>
#include <libs/ipAnalysis/ipInfo/EqClassPairs.h>
#include <libs/support/file/FormattedFile.h>



//***********************************************************************
// class EqClassScalarModRefInfo interface operations
//***********************************************************************


CLASS_NAME_IMPL(EqClassPairs);


EqClassPairs::EqClassPairs(const char *_name) :
NamedObjectIO(_name)
{
}


EqClassPairs::EqClassPairs(EqClassPairs &rhs) :
NamedObjectIO(rhs.name) 
{
  unsigned int npairs = rhs.NumberOfEntries();
  for(unsigned int i = 0; i < npairs; i++)
    AddPair(new OffsetLengthPair(*rhs.GetPair(i)));
}


EqClassPairs::~EqClassPairs()
{
}


//========================================================================
// file I/O operations
//========================================================================


int EqClassPairs::NamedObjectReadUpCall(FormattedFile *file)
{
  return OffsetLengthPairVector::Read(file);
}


int EqClassPairs::NamedObjectWriteUpCall(FormattedFile *file)
{
  return OffsetLengthPairVector::Write(file);
}
 

//========================================================================
// output support for debugging 
//========================================================================


void EqClassPairs::NamedObjectDumpUpCall()
{
  OffsetLengthPairVector::Dump();
}


void EqClassPairs::Dump()
{
  NamedObjectDump();
}


//***********************************************************************
// class EqClassPairSet interface operations
//***********************************************************************

CLASS_NAME_IMPL(EqClassPairSet);

EqClassPairSet::EqClassPairSet()
{
}


EqClassPairSet::EqClassPairSet(EqClassPairSet &rhs)
{
  EqClassPairSetIterator entries(&rhs);
  EqClassPairs *entry;
  for(; entry = entries.Current(); ++entries) {
    AddEntry(new EqClassPairs(*entry));
  }
}


EqClassPairSet::~EqClassPairSet()
{
  Destroy();
}


EqClassPairs *EqClassPairSet::GetEntry(const char *name)
{
  EqClassPairs *entry = QueryEntry(name);
  if (entry == 0) {
    entry = new EqClassPairs(name);
    NamedObjectTable::AddEntry(entry);
  }
  return entry;
}


#if 0
void EqClassPairSet::CleanupEntry(void *entry)
{
  delete (EqClassPairs *) entry;
}
#endif


EqClassPairs *EqClassPairSet::QueryEntry(const char *name)
{
  return (EqClassPairs *) NamedObjectTable::QueryEntry(name);
}


int EqClassPairSet::operator ==(EqClassPairSet &rhs)
{
  //-----------------------------------------
  // false if sets have different cardinality
  //-----------------------------------------
  if (NumberOfEntries() != rhs.NumberOfEntries()) return 0;
  
  //-----------------------------------------
  // false if some entry in rhs differs from 
  // its corresponding lhs entry
  //-----------------------------------------
  EqClassPairSetIterator entries(this);
  EqClassPairs *lhsentry;
  for (; lhsentry = entries.Current(); ++entries) {
    EqClassPairs *rhsentry = rhs.GetEntry(lhsentry->name);
    if (rhsentry == 0 || !(*lhsentry == *rhsentry)) return 0;
  }

  return 1; // equal otherwise
}


void EqClassPairSet::operator |=(EqClassPairSet &rhs)
{
  EqClassPairSetIterator entries(&rhs);
  EqClassPairs *rhsentry;
  for (; rhsentry = entries.Current(); ++entries) {
    EqClassPairs *lhsentry = GetEntry(rhsentry->name);
    *lhsentry |= *rhsentry;
  }
}


//========================================================================
// file I/O operations
//========================================================================


int EqClassPairSet::Read(FormattedFile *file)
{
  return NamedObjectTableRead(file);
}


int EqClassPairSet::Write(FormattedFile *file)
{
  return NamedObjectTableWrite(file);
}


NamedObjectIO *EqClassPairSet::NewEntry()
{
  return new EqClassPairs();
}
 

//***********************************************************************
// class EqClassPairSetIterator interface operations
//***********************************************************************

EqClassPairSetIterator::EqClassPairSetIterator(EqClassPairSet *set) : 
NamedObjectTableIterator((NamedObjectTable *) set)
{
}


EqClassPairSetIterator::~EqClassPairSetIterator()
{
}


EqClassPairs *EqClassPairSetIterator::Current()
{
  return (EqClassPairs *) NamedObjectTableIterator::Current();
}
