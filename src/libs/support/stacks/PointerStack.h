/* $Id: PointerStack.h,v 1.2 1997/03/27 20:52:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// 
// PointerStack.h: 
// 
// Author:  John Mellor-Crummey                               October 1993
//
// Copyright 1993, Rice University
//                                                                          
//***************************************************************************

#ifndef PointerStack_h
#define PointerStack_h
#include <libs/support/stacks/xstack.h>


struct PointerStackS {
  Stack pstack;
  PointerStackS() { pstack = stack_create(sizeof(void *)); };
  ~PointerStackS() { stack_destroy(pstack); };
};

class PointerStack {
  struct PointerStackS *hidden;
public:
  PointerStack();
  ~PointerStack();

  void Push(void *item);
  void *Pop();

  void *Top();

  unsigned int Depth(); // 0 when empty
};

#endif
