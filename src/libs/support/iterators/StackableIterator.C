/* $Id: StackableIterator.C,v 1.1 1997/03/11 14:36:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// StackableIterator.C
//
//   a base set of functionality for iterators that can be used with the
//   IteratorStack abstraction to traverse nested structures 
//
// Author: John Mellor-Crummey                                October 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#include <libs/support/iterators/StackableIterator.h>

Boolean StackableIterator::IsValid()
{
  return BOOL(this->CurrentUpCall() != 0);
}

StackableIterator::~StackableIterator()
{
}
