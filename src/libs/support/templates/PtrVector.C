/* $Id: PtrVector.C,v 1.1 1997/03/11 14:37:43 carr Exp $ */

//******************************************************************
// PtrVector.C: 
//
//  a vector of pointers that expands as required when elements are
//  appended. 
//
// Author: 
//   John Mellor-Crummey                              July 1993
//
// Copyright 1994, Rice University
//******************************************************************


#include <libs/utilities/Templates/PtrVector.H>


template<class T> PtrVector<T>::PtrVector
(unsigned int initialSlots, Boolean _freeDiscardedPtrs) :
PointerVector(initialSlots), freeDiscardedPtrs(_freeDiscardedPtrs)
{
}


template<class T> PtrVector<T>::~PtrVector()
{
  if (freeDiscardedPtrs) {
    unsigned int entries = PointerVector::NumberOfEntries();
    for (int i = 0; i < entries; i++) 
      delete (T *) PointerVector::Entry(i);
  }
}


template<class T> void PtrVector<T>::Append(const T *node)
{
  PointerVector::Append((T *) node);
}


template<class T> void PtrVector<T>::Shrink(unsigned int newSize)
{
  if (freeDiscardedPtrs) {
    unsigned int oldSize = NumberOfEntries();
    for (int i = newSize; i < oldSize; i++) 
      delete (T *) PointerVector::Entry(i);
  } 
  PointerVector::Shrink(newSize);
}


template<class T> T *&PtrVector<T>::operator[]
(unsigned int entryIndex) const
{
  return (T *&) PointerVector::Entry(entryIndex);
}


template<class T> T *&PtrVector<T>::Entry(unsigned int entryIndex) const
{
  return (T *&) PointerVector::Entry(entryIndex);
}
