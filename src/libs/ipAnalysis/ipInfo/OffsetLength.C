/* $Id: OffsetLength.C,v 1.2 1999/06/11 20:34:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
//  OffsetLength.C: 
//
//  a representation offset-length pairs which are useful for 
//  intervals of interest in a region of storage assigned to
//  an equivalence class of variables 
//  
// Author: 
//   John Mellor-Crummey                              December 1993
//
// Copyright 1993, Rice University
//******************************************************************


#include <stdio.h>
#include <stdlib.h>
#include <values.h>

#include <libs/support/strings/rn_string.h>
#include <libs/ipAnalysis/ipInfo/OffsetLength.h>
#include <libs/support/file/FormattedFile.h>



//***********************************************************************
// declarations
//***********************************************************************


//-----------------------------------------------------------------------
// infiniteInterval describes an interval that spans from the most 
// negative integer to the most positive integer
//-----------------------------------------------------------------------
#ifndef LINUX
OffsetLengthPair infiniteInterval(HIBITI, ~0);
#else
OffsetLengthPair infiniteInterval(MININT, ~0);
#endif



//***********************************************************************
// class OffsetLengthPair interface operations
//***********************************************************************


OffsetLengthPair::OffsetLengthPair(int _offset, unsigned int _length) :
offset(_offset), length(_length)
{
}


void OffsetLengthPair::Destroy(void *entry)
{
  delete (OffsetLengthPair *) entry;
}


int OffsetLengthPair::Compare(OffsetLengthPair **first, 
			      OffsetLengthPair **second)
{
  return ((*first)->offset - (*second)->offset); 
}


OffsetLengthPair &OffsetLengthPair::operator =(OffsetLengthPair &rhs)
{
  offset = rhs.offset; 
  length = rhs.length;
  return *this;
}


int OffsetLengthPair::operator ==(OffsetLengthPair &rhs)
{
  return (offset == rhs.offset) && (length == rhs.length);
}


//========================================================================
// file I/O operations
//========================================================================


int OffsetLengthPair::Read(FormattedFile *file)
{
  return file->Read(offset) || file->Read(length);
}


int OffsetLengthPair::Write(FormattedFile *file)
{
  return file->Write(offset) || file->Write(length);
}
 

//========================================================================
// output support for debugging 
//========================================================================


void OffsetLengthPair::Dump()
{
  fprintf(stderr, "offset = %d, length = %d\n", offset, length); 
}


//***********************************************************************
// class OffsetLengthPairVector interface operations
//***********************************************************************

OffsetLengthPairVector::OffsetLengthPairVector() : normalized(1) 
{
}


OffsetLengthPairVector::~OffsetLengthPairVector()
{
  Shrink(0, OffsetLengthPair::Destroy);
}


// can't allow anyone using NumberOfEntries for iteration to see
// unnormalized vector
unsigned int OffsetLengthPairVector::NumberOfEntries()
{
  Normalize();
  return PointerVector::NumberOfEntries();
}


OffsetLengthPair *OffsetLengthPairVector::GetPair
(unsigned int entryIndex) 
{
  Normalize();
  return GetPairInternal(entryIndex);
}


void OffsetLengthPairVector::AddPair(OffsetLengthPair *pair)
{
  PointerVector::Append(pair);
  unsigned int nentries = PointerVector::NumberOfEntries();
  if (nentries > 1) {
    OffsetLengthPair *prev = GetPairInternal(nentries - 2);
    if ((prev->offset + prev->length) > pair->offset) normalized = 0;
  }
}


int OffsetLengthPairVector::operator ==(OffsetLengthPairVector &rhs)
{
  unsigned int npairs = rhs.NumberOfEntries();
  for(unsigned int i = 0; i < npairs; i++)
    if (!(*GetPair(i) == *rhs.GetPair(i))) return 0;
  return 1;
}


void OffsetLengthPairVector::operator |=(OffsetLengthPairVector &rhs)
{
  unsigned int npairs = rhs.NumberOfEntries();
  for(unsigned int i = 0; i < npairs; i++)
    // AddPair incrementally maintains normalized flag
    AddPair(new OffsetLengthPair(*rhs.GetPair(i)));
}


static int overlaps(const void **p1, const void **p2)
{
  OffsetLengthPair *op1 = *(OffsetLengthPair **)p1;
  OffsetLengthPair *op2 = *(OffsetLengthPair **)p2;

  if (op1->offset > (op2->offset + op2->length)) return 1; 
  if ((op1->offset + op1->length) < op2->offset) return -1;

  return 0; // overlaps
}

//=====================================================================
// int OffsetLengthPairVector::Overlaps(int offset, unsigned int len)
//   
//=====================================================================
int OffsetLengthPairVector::Overlaps(int offset, unsigned int length)
{
  Normalize();

  OffsetLengthPair tmp(offset, length);
  OffsetLengthPair *tmpptr = &tmp;
  OffsetLengthPair *overlapper =
       (OffsetLengthPair *) PointerVector::Search((const void **) &tmpptr, overlaps);
  return (overlapper != 0);
}


//=====================================================================
// void OffsetLengthPairVector::Normalize()
//   
//   sort offset-length pairs by offset and coalese intervals that meet
//   or overlap
//=====================================================================
void OffsetLengthPairVector::Normalize()
{
  if (normalized == 0) {
    PointerVector::Sort((SortCompareFn) OffsetLengthPair::Compare);
    CoalescePairs();
    normalized = 1;
  }
}


//=====================================================================
// void OffsetLengthPairVector::CoalescePairs()
//
//   pre-condition:
//     offset-length pairs are sorted by offset
//   post-conditions:
//     offset-length pairs are sorted by offset
//     no two intervals meet or overlap
//=====================================================================
void OffsetLengthPairVector::CoalescePairs()
{
  unsigned int npairs = PointerVector::NumberOfEntries();
  if (npairs > 1) {
    unsigned int j = 0;
    unsigned int i = 1;

    OffsetLengthPair *entryj = GetPairInternal(j);
    OffsetLengthPair *entryi;
    
    for(; i < npairs;) {
      int curend = entryj->offset + entryj->length; 
      
      for(; i < npairs ; i++) {
	entryi = GetPairInternal(i);

	//-----------------------------------
	// if intervals overlap --> coalesce
	//-----------------------------------
	if (entryi->offset <= curend) {  
	  int newend = entryi->offset + entryi->length; 
	  if (newend > curend) {
	    curend = newend;
	    entryj->length = newend - entryj->offset;
	  }
	} else { 
	  //----------------------------------------
	  // advance to next target interval and 
	  // splice out coalesced intervals, if any
	  //----------------------------------------
	  entryj = entryi;
	  if (i++ != ++j) *GetPairInternal(j) = *entryj;
	  break;
	}
      }
    }
    j++; // convert j from an index, where  0 <= j < npairs, to a length
    if (j < npairs) Shrink(j, OffsetLengthPair::Destroy);
  }
}


OffsetLengthPair *OffsetLengthPairVector::GetPairInternal
(unsigned int entryIndex) 
{
  return (OffsetLengthPair *) PointerVector::Entry(entryIndex);
}



//========================================================================
// file I/O operations
//========================================================================


// invariant: external representation is normalized
int OffsetLengthPairVector::Write(FormattedFile *file)
{
  unsigned int size = NumberOfEntries();
  int code = file->Write(size);
  if (code) return code;
  for (unsigned i = 0; i < size; i++) {
    OffsetLengthPair *e = GetPair(i);
    if (code = e->Write(file)) return code;
  }
  return 0;
}


// invariant: external representation is normalized
int OffsetLengthPairVector::Read(FormattedFile *file)
{
  unsigned int size;
  int code = file->Read(size);
  if (code) return code;

  for (unsigned i = 0; i < size; i++) {
    OffsetLengthPair *e = new OffsetLengthPair;
    if (code = e->Read(file)) return code;
    AddPair(e);
  }
  return 0;
}


//========================================================================
// output support for debugging 
//========================================================================
 

void  OffsetLengthPairVector::PointerVectorDumpUpCall(void *entry)
{
  ((OffsetLengthPair *) entry)->Dump();
}


void OffsetLengthPairVector::Dump()
{
  PointerVectorDump();
}

