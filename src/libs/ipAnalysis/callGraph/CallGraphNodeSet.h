/* $Id: CallGraphNodeSet.h,v 1.1 1997/03/11 14:34:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef CallGraphNodeSet_h
#define CallGraphNodeSet_h


//***************************************************************************
//  CallGraphNodeSet.h:
//
//  set to hold call graph nodes
//
//  Author:
//    John Mellor-Crummey                                    May 1994
//
//  Copyright 1994, Rice University
//***************************************************************************

#include <libs/support/sets/WordSet.h>

class CallGraphNode;

class CallGraphNodeSet : private WordSet {
public:

  //--------------------------------------------------------------
  // constructor and destructor
  //--------------------------------------------------------------
  CallGraphNodeSet();
  ~CallGraphNodeSet();

  //--------------------------------------------------------------
  // add a member to the set 
  //--------------------------------------------------------------
  void Add(CallGraphNode *node);

  //--------------------------------------------------------------
  // check to see if a member is in the set
  //--------------------------------------------------------------
  int IsMember(CallGraphNode *node);
};

#endif /* CallGraphNodeSet_h */
