/* $Id: PtrVector.H,v 1.1 1997/03/11 14:37:44 carr Exp $ */

#ifndef PtrVector_h
#define PtrVector_h

//******************************************************************
// PtrVector.h: 
//
//  a vector of pointers to template instantiation type
//
// Author: 
//   John Mellor-Crummey                              July 1994
//
// Copyright 1994, Rice University
//******************************************************************


#include <include/general.h>
#include <include/misc/PointerVector.h>



//******************************************************************
// template<class T> class PtrVector
//******************************************************************

template<class T> class PtrVector: private PointerVector {
  Boolean freeDiscardedPtrs;
public:
  // constructor/destructor
  PtrVector(unsigned int initialSlots = 16, 
	    Boolean _freeDiscardedPtrs = true);

  ~PtrVector();
  
  void Append(const T *);
  T *&Entry(unsigned int entryIndex) const; 
  T *&operator [] (unsigned int entryIndex) const;
  void Shrink(unsigned int newSize);

  PointerVector::Reverse;
  PointerVector::ReInitialize;
  PointerVector::NumberOfEntries;
  PointerVector::Sort;
  PointerVector::Search;
  PointerVector::PointerVectorDump;
};


#endif /* PtrVector_h */
