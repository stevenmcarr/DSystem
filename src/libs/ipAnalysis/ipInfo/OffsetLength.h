/* $Id: OffsetLength.h,v 1.1 1997/03/11 14:34:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef OffsetLength_h
#define OffsetLength_h

//******************************************************************
//  OffsetLength.h: 
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


#include <include/ClassName.h>
#include <libs/support/vectors/PointerVector.h>

class FormattedFile; // minimal external declaration

//-----------------------------------------------------------------------
// class OffsetLengthPair
//-----------------------------------------------------------------------
class OffsetLengthPair {
public:
  int offset;
  unsigned int length;

  OffsetLengthPair(int offset = 0, unsigned int length = 0);
  static int Compare(OffsetLengthPair **first, OffsetLengthPair **second);
  static void Destroy(void *);

  OffsetLengthPair &operator =(OffsetLengthPair &);
  int operator==(OffsetLengthPair &);

  int Write(FormattedFile *file);
  int Read(FormattedFile *file);

  void Dump();
};

extern OffsetLengthPair infiniteInterval;


//-----------------------------------------------------------------------
// class OffsetLengthPairVector
//-----------------------------------------------------------------------
class OffsetLengthPairVector : private PointerVector {
  int normalized;
public:
  OffsetLengthPairVector();
  ~OffsetLengthPairVector();

  unsigned int NumberOfEntries();

  OffsetLengthPair *GetPair(unsigned int pairIndex);
  void AddPair(OffsetLengthPair *pair);

  int operator ==(OffsetLengthPairVector &rhs);
  void operator |=(OffsetLengthPairVector &rhs);

  // non-zero return code if any interval overlaps the offset
  // length pair
  int Overlaps(int offset, unsigned int length);

  int Read(FormattedFile *file);
  int Write(FormattedFile *file);
  void Dump();

private:
  void Normalize();
  void PointerVectorDumpUpCall(void *);
  void CoalescePairs();
  OffsetLengthPair *GetPairInternal(unsigned int pairIndex);
};

#endif

