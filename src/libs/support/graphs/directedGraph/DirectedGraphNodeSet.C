/* $Id: DirectedGraphNodeSet.C,v 1.1 1997/03/11 14:36:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/support/graphs/directedGraph/DirectedGraphNodeSet.h>

//*************************************************************************
//                  DirectedGraphNodeSet Abstraction
//*************************************************************************

//------------------------------------------------------------------------
// call graph node set constructor
//------------------------------------------------------------------------

DirectedGraphNodeSet::DirectedGraphNodeSet()
{
}

//------------------------------------------------------------------------
// call graph node set destructor
//------------------------------------------------------------------------

DirectedGraphNodeSet::~DirectedGraphNodeSet()
{
}

//------------------------------------------------------------------------
// call graph node set add a member
//------------------------------------------------------------------------

void DirectedGraphNodeSet::Add(const DirectedGraphNode *node) 
{ 
   WordSet::Add((unsigned long) node); 
}

void DirectedGraphNodeSet::Delete(const DirectedGraphNode *node) 
{ 
   WordSet::Delete((unsigned long) node); 
}

//------------------------------------------------------------------------
// call graph node set check for membership
//------------------------------------------------------------------------

int DirectedGraphNodeSet::IsMember(const DirectedGraphNode *node) 
{
   return WordSet::IsMember((unsigned long) node);
}



//**********************************************************************
// implementation of class DirectedGraphNodeSetIterator
//**********************************************************************

DirectedGraphNodeSetIterator::DirectedGraphNodeSetIterator
(const DirectedGraphNodeSet *theSet) : 
WordSetIterator((const WordSet *) theSet)
{
}


DirectedGraphNode *DirectedGraphNodeSetIterator::Current() const
{
  unsigned long *ptr = WordSetIterator::Current();

  return (ptr ?  *(DirectedGraphNode **) ptr : 0);
}


