/* $Id: IteratorStack.h,v 1.1 1997/03/11 14:36:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// IteratorStack.h
//
//   a stack of iterators that is itself an iterator. this abstraction is 
//   useful for traversing nested structures.
//
// Author: John Mellor-Crummey                                October 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef IteratorStack_h
#define IteratorStack_h

#ifndef StackableIterator_h
#include <libs/support/iterators/StackableIterator.h>
#endif

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

enum IterStackEnumType {
  ITER_STACK_ENUM_LEAVES_ONLY, ITER_STACK_ENUM_ALL_NODES
  };

class IteratorStack: public StackableIterator {
private:
  struct IteratorStackS *iteratorStackRepr;
  IterStackEnumType enumType;
  void FreeTop();
  void FreeStack(int minDepth);
protected:
  TraversalOrder traversalOrder;
  void Push(StackableIterator *);
  StackableIterator *Top(void) const;
public:
  IteratorStack(TraversalOrder torder, 
		IterStackEnumType enumType = ITER_STACK_ENUM_ALL_NODES);
  ~IteratorStack();
  
  void operator++();

  void Reset(); // pop all but one iterator off the stack; reset the one left

  // empty the stack and reset the state to that as if freshly constructed
  void ReConstruct(TraversalOrder torder, 
		   IterStackEnumType enumType = ITER_STACK_ENUM_ALL_NODES);

  Boolean IsValid();
  
  virtual StackableIterator *IteratorToPushIfAny(void *current) = 0;
  void *CurrentUpCall() const;
};

#endif
