/* $Id: OrderedSetOfStrings.h,v 1.3 1997/03/11 14:37:29 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
// OrderedSetOfStrings.h: 
//
//  a vector of strings that can be read and written to a file
//
// Author: 
//   John Mellor-Crummey                              
//
// Copyright 1993, Rice University
//******************************************************************


#ifndef OrderedSetOfStrings_h
#define OrderedSetOfStrings_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef PointerVector_h
#include <libs/support/vectors/PointerVector.h>
#endif

class FormattedFile;   // minimal external declaration

class OrderedSetOfStrings : private PointerVector {
  Boolean deallocateStringsAtDestruction;
public:
  // constructor
  OrderedSetOfStrings(Boolean destructorDeallocateStrings = false);

  // destructor
  ~OrderedSetOfStrings();

  // free each of the strings in the set using sfree, emptying the set
  // NOTE: since this set can also be used to contain strings not allocated
  // with ssave, this function is not called by the destructor by default.
  // one can arrange to have it called by the destructor either by arrangement
  // with the constructor or through DestructorDeallocateStrings
  void DeallocateStrings();

  // specify whether destructor should deallocate strings in set or not 
  void DestructorDeallocateStrings(Boolean doit);

  // add another string to the Set
  void Append(char *string);

  // access to state
  unsigned int Size() const;
  PointerVector::NumberOfEntries;
  char *operator [] (int sindex) const;

  void Print() const;
  void Dump() const;

  // I/O
  int Write(FormattedFile *file) const;
  int Read(FormattedFile *file);
};

#endif /* OrderedSetOfStrings_h */
