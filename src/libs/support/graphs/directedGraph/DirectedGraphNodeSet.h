/* $Id: DirectedGraphNodeSet.h,v 1.1 1997/03/11 14:36:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef DirectedGraphNodeSet_h
#define DirectedGraphNodeSet_h


//***************************************************************************
//  DirectedGraphNodeSet.h:
//
//  set to hold directed graph nodes
//
//  Author:
//    John Mellor-Crummey                                    July 1994
//
//  Copyright 1994, Rice University
//***************************************************************************

#include <libs/support/sets/WordSet.h>

class DirectedGraphNode;

class DirectedGraphNodeSet : private WordSet {
public:

  //--------------------------------------------------------------
  // constructor and destructor
  //--------------------------------------------------------------
  DirectedGraphNodeSet();
  ~DirectedGraphNodeSet();

  //--------------------------------------------------------------
  // add a member to the set 
  //--------------------------------------------------------------
  void Add(const DirectedGraphNode *node);
  void Delete(const DirectedGraphNode *node);

  //--------------------------------------------------------------
  // check to see if a member is in the set
  //--------------------------------------------------------------
  int IsMember(const DirectedGraphNode *node);
friend class DirectedGraphNodeSetIterator;
};


//-------------------------------------------------------------
// class DirectedGraphNodeSetIterator
//-------------------------------------------------------------
class DirectedGraphNodeSetIterator: private WordSetIterator {
public:
  DirectedGraphNodeSetIterator(const DirectedGraphNodeSet *theSet);
  DirectedGraphNode *Current() const;
  WordSetIterator::operator++;
  WordSetIterator::Reset;
};


#endif /* DirectedGraphNodeSet_h */
