/* $Id: AstIterators.h,v 1.2 1997/03/11 14:29:19 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef AstIterators_h
#define AstIterators_h


//***************************************************************************
// AstIterators.h: 
//
//   A set of iterators that support convenient traversal of abstract syntax 
//   trees
//
// Author: 
//   John Mellor-Crummey                              February 1994
//
// Copyright 1994, Rice University
//***************************************************************************


#ifndef StackableIterator_h
#include <libs/support/iterators/StackableIterator.h>
#endif


#ifndef IteratorStack_h
#include <libs/support/iterators/IteratorStack.h>
#endif


#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif


#ifndef ClassName_h
#include <include/ClassName.h>
#endif


//===========================================================================
// GENERAL INFORMATION
//
//   each of the following iterators is intended to be used with the 
//   following idiom:
//
//   <iterator type> nodes(rootOfWalk, <walk control parameters>);
//   AST_INDEX node;
//   for (; node = nodes.Current(); nodes++) {
//      [ your code goes here ... ] 
//   }
//
//   the ReplaceCurrent() method can be used to update the iterator state 
//   as necessary if your code replaces the current node and you want to 
//   continue the iteration from the current point.
// 
//   the iterator can be advanced either with the ++ operator or the
//   Advance method. 
//
//   an iterator can be reset using either the Reset or ReConstruct methods 
//   for repeated use 
//  
//===========================================================================



//===========================================================================
// class AstNodeIterator: 
//
//  an abstract base class for AST traversal iterators this is not intended
//  for direct use outside this package
//
//===========================================================================

enum AstIterDirection {AST_ITER_FORWARD, AST_ITER_BACKWARD};
enum TraversalVisitType {PreVisit, PostVisit};

class AstNodeIterator : public StackableIterator {
protected:
  AST_INDEX node;
  AstIterDirection direction;
  void *CurrentUpCall() const;
  Boolean IsValid(); 
public:
  AstNodeIterator(AST_INDEX startNode, 
		  AstIterDirection _direction = AST_ITER_FORWARD);
  ~AstNodeIterator();

  virtual AST_INDEX Current() const = 0;
  virtual void ReplaceCurrent(AST_INDEX newCurrent);

  virtual void Advance() = 0;
  void operator++();

  virtual void Reset() = 0;
  void ReConstruct(AST_INDEX startNode,
		   AstIterDirection _direction = AST_ITER_FORWARD);
  CLASS_NAME_FDEF(AstNodeIterator);
};




//***************************************************************************
// class AstIterator:
//
//   an iterator that enumerates the subtree of the AST rooted at rootOfWalk,
//   including the rootOfWalk.
// 
//   walks can be either PreOrder, PostOrder, or PreAndPostOrder. 
// 
//   for traversals of type PreAndPostOrder, each node is enumerated twice
//   and the VisitType() method can be used to distinguish between a PreVisit
//   and a PostVisit.
//
//   an enumeration can either be of type FULL_WALK, which enumerates each 
//   node of the subtree, or of type STMTS_ONLY which enumerates only the 
//   nodes in the subtree that satisfy the is_statement() predicate. for 
//   walks of type STMTS_ONLY, the rootOfWalk must either be a statement or 
//   a list node containing statements.
//
//   walks can be FORWARD, or BACKWARD. this affects whether sons and list 
//   elements are enumerated left-to-right or right-to-left.
//
//   the ++ opperator will enter subtrees as encountered. the Advance 
//   method can be invoked with either ITER_CONTINUE or SKIP_CHILDREN, 
//   enabling explicit control over whether subtrees or lists 
//   rooted at the current node should be visited.
//
//***************************************************************************

enum AstIterAdvanceDirective {AST_ITER_SKIP_CHILDREN, AST_ITER_CONTINUE};
enum AstIterWalkType {AST_ITER_FULL_WALK, AST_ITER_STMTS_ONLY};


class AstIterator : public IteratorStack {
  AstIterWalkType walkType;
  AstIterAdvanceDirective advanceAction;
  TraversalVisitType visitType;
  AstIterDirection direction;
  StackableIterator *IteratorToPushIfAny(void *current);
  void ResetHelper(AST_INDEX rootOfWalk);
public:
  AstIterator(AST_INDEX rootOfWalk, TraversalOrder order = PreOrder, 
	      AstIterWalkType _walkType = AST_ITER_FULL_WALK,
	      AstIterDirection _direction = AST_ITER_FORWARD);
  ~AstIterator();

  AST_INDEX Current() const;

  // VisitType() is used to distinguish between pre and post order visits
  // for traversals of type PreAndPostOrder
  TraversalVisitType VisitType(); 

  void ReplaceCurrent(AST_INDEX newCurrent);

  IteratorStack::operator++;
  void Advance(AstIterAdvanceDirective advanceAction = AST_ITER_CONTINUE);

  void Reset();
  void ReConstruct(AST_INDEX newRootOfWalk, TraversalOrder order = PreOrder, 
		   AstIterWalkType _walkType = AST_ITER_FULL_WALK,
		   AstIterDirection _direction = AST_ITER_FORWARD);
};




//***************************************************************************
// class AstSonsIterator:
//
//   an iterator that enumerates all of the immediate children of an AST node
//
//   enumerations can be FORWARD, or BACKWARD. this affects whether sons 
//   are enumerated left-to-right or right-to-left.
//
//***************************************************************************

class AstSonsIterator : public AstNodeIterator {
  int nSons;    // number of sons
  int curSon;   // current son in the enumeration
public:
  AstSonsIterator(AST_INDEX parent, 
		  AstIterDirection _direction = AST_ITER_FORWARD);
  ~AstSonsIterator();

  AST_INDEX Current() const;
  void ReplaceCurrent(AST_INDEX newCurrent);

  void Advance();
  AstNodeIterator::operator++;

  void Reset();
  void ReConstruct(AST_INDEX newParent,
		   AstIterDirection _direction = AST_ITER_FORWARD);
  CLASS_NAME_FDEF(AstSonsIterator);
};




//***************************************************************************
// class AstListElementsIterator:
//
//   an iterator that enumerates all of the imediate elements in an AST list
//
//   enumerations can be FORWARD, or BACKWARD. this affects whether list 
//   elements are enumerated left-to-right or right-to-left.
//
//***************************************************************************

class AstListElementsIterator : public AstNodeIterator {
  AST_INDEX current;
public:
  AstListElementsIterator(AST_INDEX listNode,	
			  AstIterDirection _direction = AST_ITER_FORWARD);
  ~AstListElementsIterator();	

  AST_INDEX Current() const;
  void ReplaceCurrent(AST_INDEX newCurrent);

  void Advance();
  AstNodeIterator::operator++;

  void Reset();
  void ReConstruct(AST_INDEX newListNode, 
		   AstIterDirection _direction = AST_ITER_FORWARD);
  CLASS_NAME_FDEF(AstListElementsIterator);
};




//***************************************************************************
// class AstAncestorsIterator:
//
//   an enumeration of all of the ancestors of an AST node. the self node
//   is not an element of the enumeration.
//
//   N.B.: the enumeration includes nodes of type LIST_OF_NODES which would
//   be skipped if tree_out() were used to advance the iterator. 
//
//***************************************************************************

class AstAncestorsIterator : private AstNodeIterator {
  AST_INDEX current;
public:
  AstAncestorsIterator(AST_INDEX self);
  ~AstAncestorsIterator();

  AST_INDEX Current() const;

  void Advance();
  AstNodeIterator::operator++;

  void Reset();
  CLASS_NAME_FDEF(AstAncestorsIterator);
};


#endif
