/* $Id: PointerVector.h,v 1.1 1997/03/11 14:37:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
// PointerVector.h: 
//
//  a vector of pointers that expands as required when elements are
//  appended. 
//
// Author: 
//   John Mellor-Crummey                              December 1993
//
// Copyright 1993, Rice University
//******************************************************************


#ifndef PointerVector_h
#define PointerVector_h

typedef void (*ShrinkDestructor)(void *entry);

class PointerVector {
public:
  // constructor/destructor
  PointerVector(unsigned int initialSlots = 16);
  virtual ~PointerVector();

  // discard contents 
  void ReInitialize(unsigned int initialSlots = 16);

  // discard contents 
  void Shrink(unsigned int newSize, ShrinkDestructor sd = 0);

  // add another entry 
  void Append(void *entry);

  // reverse the order of the entries
  void Reverse();

  // access to contents
  unsigned int NumberOfEntries() const;
  void *&Entry(unsigned int entryIndex) const; 
  void *&operator [] (unsigned int entryIndex) const;

  typedef int (*SortCompareFn)(const void **, const void **);
  typedef int (*SearchCompareFn)(const void **, const void **);

  void Sort(SortCompareFn compare);
  void *Search(const void **key, SearchCompareFn compare);

  void PointerVectorDump();
  virtual void PointerVectorDumpUpCall(void *);
private:
  unsigned int nextSlot;
  unsigned int numSlots;
  void **slots;
};

#endif /* PointerVector_h */
