/* $Id: IteratorStack.C,v 1.2 1997/03/11 14:36:45 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// IteratorStack.C
//
//   a stack of iterators that is itself an iterator. this abstraction is 
//   useful for traversing nested structures.
//
// Author: John Mellor-Crummey                                October 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#include <assert.h>
#include <stdlib.h>

#include <libs/support/iterators/IteratorStack.h>
#include <libs/support/stacks/PointerStack.h>

struct IteratorStackS {
  PointerStack pstack;
};


IteratorStack::IteratorStack(TraversalOrder torder, 
			     IterStackEnumType _enumType) 
{
  iteratorStackRepr = new IteratorStackS;
  ReConstruct(torder, _enumType);
}


IteratorStack::~IteratorStack()
{
  FreeStack(0);
  delete iteratorStackRepr;
}


StackableIterator *IteratorStack::Top() const
{
  return (StackableIterator *) iteratorStackRepr->pstack.Top();
}


void IteratorStack::Push(StackableIterator *iterator)
{
  StackableIterator *newtop = iterator;
  while (newtop != 0) {
    iteratorStackRepr->pstack.Push(newtop);
    if (traversalOrder != PostOrder) break;
    newtop = IteratorToPushIfAny(Top()->CurrentUpCall());
  }
}


void IteratorStack::operator++()
{
  for(;;) {
    StackableIterator *top = Top();
    
    if ((traversalOrder == PreOrder) || (traversalOrder == PreAndPostOrder)) {
      void *current = top->CurrentUpCall();
      (*top)++; // advance iterator at the top of stack
      StackableIterator *newtop = IteratorToPushIfAny(current);
      if (newtop) { 
	Push(newtop);
	top = Top();
      }
    } 
    else (*top)++; // advance iterator at the top of stack
    
    if (top->IsValid() == false) {
      Boolean popped = false;
      while ((Top()->IsValid() == false) &&
	     (iteratorStackRepr->pstack.Depth() > 1)) {
	FreeTop();
	popped = true;
      } 
      if (popped && (enumType == ITER_STACK_ENUM_LEAVES_ONLY)) continue;
    } else if (traversalOrder == PostOrder) {
      void *current = top->CurrentUpCall();
      StackableIterator *newtop = IteratorToPushIfAny(current);
      if (newtop) Push(newtop);
    }
    break;
  }
}


void IteratorStack::ReConstruct (TraversalOrder torder, 
				 IterStackEnumType _enumType) 
{
  enumType = _enumType;
  if (enumType == ITER_STACK_ENUM_LEAVES_ONLY) traversalOrder = PostOrder;
  else {
    assert((torder == PreOrder) || (torder == PostOrder) || 
	   (torder == PreAndPostOrder));
    traversalOrder = torder;
  }

  FreeStack(0);
}


void IteratorStack::Reset()
{
  FreeStack(1); // leave top element on stack
  Top()->Reset();
  if (traversalOrder == PostOrder) 
    Push(IteratorToPushIfAny(Top()->CurrentUpCall()));
}


void *IteratorStack::CurrentUpCall() const
{
  return Top()->CurrentUpCall();
}


Boolean IteratorStack::IsValid()
{
  return Top()->IsValid();
}


void IteratorStack::FreeTop()
{
  delete (StackableIterator *) iteratorStackRepr->pstack.Pop();
}


// free the top (depth - mindepth) elements on the stack, leaving minDepth
// elements on the stack: FreeStack(1) leaves only one element on the stack
void IteratorStack::FreeStack(int minDepth)
{
  int depth = iteratorStackRepr->pstack.Depth();
  for (; depth-- > minDepth;) FreeTop();
}



