/* $Id: StackableIterator.h,v 1.1 1997/03/11 14:36:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// StackableIterator.h
//
//   a base set of functionality for iterators that can be used with the
//   IteratorStack abstraction to traverse nested structures 
//
// Author: John Mellor-Crummey                                October 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef StackableIterator_h
#define StackableIterator_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

class StackableIterator {
public:
  virtual ~StackableIterator();

  virtual void operator++() = 0;
  virtual void Reset() = 0;

  virtual Boolean IsValid();

  virtual void *CurrentUpCall() const = 0;
};

#endif
