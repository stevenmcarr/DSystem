/* $Id: EqClassPairs.h,v 1.1 1997/03/11 14:34:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef EqClassPairs_h
#define EqClassPairs_h

//******************************************************************
//  EqClassPairs.h: 
//
//  a representation intervals of interest in a region of storage 
//  assigned to an equivalence class of variables
//  
// Author: 
//   John Mellor-Crummey                              January 1994
//
// Copyright 1994, Rice University
//******************************************************************


#include <include/ClassName.h>
#include <libs/support/tables/namedObject/NamedObject.h>
#include <libs/support/tables/namedObject/NamedObjectTable.h>

#include <libs/ipAnalysis/ipInfo/OffsetLength.h>

class FormattedFile; // minimal external declaration


//-----------------------------------------------------------------------
// class EqClassPairs
//-----------------------------------------------------------------------
class EqClassPairs: public NamedObjectIO, public OffsetLengthPairVector {
public:
  EqClassPairs(const char *name = 0);
  EqClassPairs(EqClassPairs &rhs);
  ~EqClassPairs();

  OffsetLengthPairVector::NumberOfEntries;

  OffsetLengthPairVector::GetPair;
  OffsetLengthPairVector::AddPair;

  OffsetLengthPairVector::operator ==;
  OffsetLengthPairVector::operator |=;

  void Dump();

private:
  int NamedObjectWriteUpCall(FormattedFile *file);
  int NamedObjectReadUpCall(FormattedFile *file);
  void NamedObjectDumpUpCall();

  CLASS_NAME_FDEF(EqClassPairs);
};


//-----------------------------------------------------------------------
// class EqClassPairSet
//-----------------------------------------------------------------------
class EqClassPairSet : private NamedObjectTableIO {
private:
  NamedObjectIO *NewEntry();
public:
  EqClassPairSet();
  EqClassPairSet(EqClassPairSet &rhs);
  ~EqClassPairSet();
  CLASS_NAME_FDEF(EqClassPairSet);

  EqClassPairs *GetEntry(const char *name);
  EqClassPairs *QueryEntry(const char *name);

  int operator == (EqClassPairSet &rhs);
  void operator |= (EqClassPairSet &rhs);

  int Write(FormattedFile *port);
  int Read(FormattedFile *port);
friend class EqClassPairSetIterator;
};

CLASS_NAME_EDEF(EqClassPairSet);


//-----------------------------------------------------------------------
// class EqClasPairSetIterator
//-----------------------------------------------------------------------
class  EqClassPairSetIterator : private NamedObjectTableIterator {
public:
   EqClassPairSetIterator(EqClassPairSet *set);
  ~EqClassPairSetIterator();
  
  EqClassPairs *Current();
  NamedObjectTableIterator::operator++;
  NamedObjectTableIterator::Reset;
};


#endif
