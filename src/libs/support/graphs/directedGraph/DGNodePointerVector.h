/* $Id: DGNodePointerVector.h,v 1.1 1997/03/11 14:36:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef DGNodePointerVector_h
#define DGNodePointerVector_h

//******************************************************************
// DGNodePointerVector.h: 
//
//  a vector of DGNodePointers to DirectedGraphNodes 
//
// Author: 
//   John Mellor-Crummey                              July 1994
//
// Copyright 1994, Rice University
//******************************************************************


#include <libs/support/vectors/PointerVector.h>


class DirectedGraphNode;


//******************************************************************
// class DGNodePointerVector
//******************************************************************
class DGNodePointerVector: private PointerVector {
public:
  // constructor/destructor
  DGNodePointerVector(unsigned int initialSlots = 16);
  ~DGNodePointerVector();
  
  void Append(const DirectedGraphNode *node);
  DirectedGraphNode *&Entry(unsigned int entryIndex) const; 
  DirectedGraphNode *&operator [] (unsigned int entryIndex) const;
  
  PointerVector::Reverse;
  PointerVector::ReInitialize;
  PointerVector::Shrink;
  PointerVector::NumberOfEntries;
  PointerVector::Sort;
  PointerVector::Search;
  PointerVector::PointerVectorDump;
};

#endif /* DGNodePointerVector_h */
