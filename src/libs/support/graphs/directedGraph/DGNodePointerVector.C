/* $Id: DGNodePointerVector.C,v 1.1 1997/03/11 14:36:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
// DGNodePointerVector.C: 
//
//  a vector of pointers that expands as required when elements are
//  appended. 
//
// Author: 
//   John Mellor-Crummey                              July 1993
//
// Copyright 1994, Rice University
//******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <libs/support/graphs/directedGraph/DGNodePointerVector.h>


DGNodePointerVector::DGNodePointerVector(unsigned int initialSlots) : 
PointerVector(initialSlots)
{
}


DGNodePointerVector::~DGNodePointerVector()
{
}


void DGNodePointerVector::Append(const DirectedGraphNode *node)
{
  PointerVector::Append((DirectedGraphNode *) node);
}


DirectedGraphNode *&DGNodePointerVector::operator[]
(unsigned int entryIndex) const
{
  return (DirectedGraphNode *&) PointerVector::Entry(entryIndex);
}


DirectedGraphNode *&DGNodePointerVector::Entry(unsigned int entryIndex) const
{
  return (DirectedGraphNode *&) PointerVector::Entry(entryIndex);
}
