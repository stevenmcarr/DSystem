/* $Id: PointerStack.C,v 1.2 1997/03/27 20:52:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// 
// PointerStack.C: 
// 
// Author:  John Mellor-Crummey                               October 1993
//
// Copyright 1993, Rice University
//                                                                          
//***************************************************************************

#include <stdlib.h>
#include <assert.h>

#include <libs/support/misc/general.h>
#include <libs/support/stacks/PointerStack.h>
#include <libs/support/stacks/xstack.h>



PointerStack::PointerStack()
{
  hidden = new PointerStackS;
}


PointerStack::~PointerStack()
{
  Stack thestack = hidden->pstack;
  delete hidden;
}


void *PointerStack::Top()
{
  Stack thestack = hidden->pstack;
  void *top;

  if (stack_get(thestack, (Generic *) &top, STACK_TOP) == false)
      assert(0);

  return top;
}

void *PointerStack::Pop()
{
  void *top; 
  stack_pop(hidden->pstack, (Generic *) &top);
  return top;
}


void PointerStack::Push(void *item)
{
  stack_push(hidden->pstack, (Generic *) &item);
}

unsigned int PointerStack::Depth()
{
  return (unsigned int) stack_depth(hidden->pstack);
}





