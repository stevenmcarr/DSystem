/* $Id: PointerVector.C,v 1.1 1997/03/11 14:37:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
// PointerVector.C: 
//
//  a vector of pointers that expands as required when elements are
//  appended. 
//
// Author: 
//   John Mellor-Crummey                              December 1993
//
// Copyright 1993, Rice University
//******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <libs/support/vectors/PointerVector.h>


PointerVector::PointerVector(unsigned int initialSlots) : slots(0)
{
  ReInitialize(initialSlots);
}


PointerVector::~PointerVector()
{
  if (slots) free((char*)slots);
}


void PointerVector::ReInitialize(unsigned int initialSlots)
{
  if (slots) free((char*)slots);
  nextSlot = 0;
  numSlots = initialSlots;
  slots = (void **) malloc(numSlots * sizeof(void *));
}


void PointerVector::Append(void *entry)
{
  if (nextSlot == numSlots) {
    slots = (void **) realloc((char *) slots, (numSlots <<= 1) * sizeof(void *));
  }
  slots[nextSlot++] = entry;
}


void PointerVector::Reverse()
{ 
  int lastIndex = nextSlot - 1;
  int midIndex =  nextSlot >> 1;
  for(unsigned int i = 0; i < midIndex; i++) {
    void *tmp = Entry(i);
    Entry(i) = Entry(lastIndex - i);
    Entry(lastIndex - i) = tmp;
  }
}


void PointerVector::Shrink(unsigned int newSize, ShrinkDestructor sd)
{
  if (sd != 0) {
    unsigned int oldSize = NumberOfEntries();
    for (int i = newSize; i < oldSize; i++) sd(Entry(i));
  }

  slots = (newSize != 0) ? 
    (void **) realloc((char *)slots, newSize * sizeof(void *)) : 
  (void **) (free((char*)slots), 0);

  nextSlot = newSize;
  numSlots = newSize;
}


unsigned int PointerVector::NumberOfEntries() const
{
  return nextSlot;
}


void *&PointerVector::Entry(unsigned int entryIndex) const
{
  assert(entryIndex < nextSlot);
  return slots[entryIndex];
}


void *&PointerVector::operator[](unsigned int entryIndex) const
{
  return Entry(entryIndex);
}


typedef int (*QsortCompareFn)(const void *, const void *);

void PointerVector::Sort(SortCompareFn compare)
{
  qsort(slots, NumberOfEntries(), sizeof(void *), (QsortCompareFn) compare);
}


typedef int (*BsearchCompareFn)(const void *, const void *);

void *PointerVector::Search(const void **key, SearchCompareFn compare)
{
  return bsearch((const char*)key, (const char*)slots, NumberOfEntries(), sizeof(void *), 
		 (BsearchCompareFn) compare);
}


void PointerVector::PointerVectorDump()
{
  unsigned int size = NumberOfEntries();
  for (unsigned int i = 0; i < size; i++) PointerVectorDumpUpCall(Entry(i));
}

void PointerVector::PointerVectorDumpUpCall(void *)
{
  assert(0);
}


