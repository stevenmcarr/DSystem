/* $Id: AstIterators.C,v 1.4 1997/03/11 14:29:18 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

//***************************************************************************
// AstIterators.C: 
//
//   A set of iterators that support convenient traversal of abstract syntax 
//   trees
//
// Author: 
//   John Mellor-Crummey                              February 1994
//
// Copyright 1994, Rice University
//***************************************************************************

#include <assert.h>

#include <libs/support/iterators/StackableIterator.h>

#include <libs/frontEnd/ast/AstIterators.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/astlist.h>
#include <libs/frontEnd/ast/gen.h>


//***************************************************************************
// private declarations
//***************************************************************************


const int AST_FIRST_SON_INDEX  = 1; 

inline int AST_LAST_SON_INDEX(int nSons) 
{ 
  return nSons + AST_FIRST_SON_INDEX - 1; 
}



//***************************************************************************
// class AstSimpleNodeIterator
//***************************************************************************

class AstSimpleNodeIterator : public AstNodeIterator {
  int first;
public:
  const TraversalVisitType visitType;

  AstSimpleNodeIterator(AST_INDEX n, TraversalVisitType _visitType) : 
  AstNodeIterator(n), first(AST_FIRST_SON_INDEX), visitType(_visitType) {};
  AST_INDEX Current() const { 
    return ((first == AST_FIRST_SON_INDEX) ? node : AST_NIL); };
  void ReplaceCurrent(AST_INDEX newCurrent) { node = newCurrent; };
  void Advance() { first =  AST_FIRST_SON_INDEX - 1; };
  void Reset() { first = AST_FIRST_SON_INDEX; };
  CLASS_NAME_FDEF(AstSimpleNodeIterator);
};

CLASS_NAME_IMPL(AstSimpleNodeIterator);



//***************************************************************************
// interface operations for class AstNodeIterator
//***************************************************************************

CLASS_NAME_IMPL(AstNodeIterator);

AstNodeIterator::AstNodeIterator
(AST_INDEX startNode, AstIterDirection _direction) :
node(startNode), direction(_direction)
{
}


AstNodeIterator::~AstNodeIterator() 
{
}


void *AstNodeIterator::CurrentUpCall() const 
{ 
  return (void *) Current(); 
}


void AstNodeIterator::ReplaceCurrent(AST_INDEX)
{ 
  assert(0);
}


void AstNodeIterator::operator++() 
{ 
  Advance(); 
}


Boolean AstNodeIterator::IsValid() 
{ 
  return BOOL(Current() != AST_NIL); 
}


void AstNodeIterator::ReConstruct(AST_INDEX startNode, 
				  AstIterDirection _direction)
{
  node = startNode;
  direction = _direction; 
}


//***************************************************************************
// interface operations for class AstSonsIterator
//***************************************************************************

CLASS_NAME_IMPL(AstSonsIterator);


AstSonsIterator::AstSonsIterator
(AST_INDEX parent, AstIterDirection _direction) : 
AstNodeIterator(parent, _direction)
{
  Reset();
}


AstSonsIterator::~AstSonsIterator()
{
}


AST_INDEX AstSonsIterator::Current() const 
{
  return ((node && (curSon >= AST_FIRST_SON_INDEX) && 
	   (curSon <= AST_LAST_SON_INDEX(nSons))) ? 
	  ast_get_son_n(node, curSon) : AST_NIL);
}


void AstSonsIterator::ReplaceCurrent(AST_INDEX)
{
  // iterator finds current as son of parent; therefore this is a no-op
}


void AstSonsIterator::Advance() 
{ 
  // advance to next son, skipping sons with value AST_NIL
  while (node && ((direction == AST_ITER_FORWARD) ? 
		  (++curSon <= AST_LAST_SON_INDEX(nSons)) :
		  (--curSon >= AST_FIRST_SON_INDEX)) && Current() == AST_NIL); 
}


void AstSonsIterator::Reset() 
{ 
  nSons = (node ? ast_get_son_count(node) : 0);
  curSon = ((direction == AST_ITER_FORWARD) ? AST_FIRST_SON_INDEX : 
	    AST_LAST_SON_INDEX(nSons));
  
  if (Current() == AST_NIL) Advance(); // skip sons with value AST_NIL
}


void AstSonsIterator::ReConstruct(AST_INDEX newParent, 
				  AstIterDirection _direction) 
{ 
  AstNodeIterator::ReConstruct(newParent, _direction);
  Reset();
}



//***************************************************************************
// interface operations for class AstListElementsIterator
//***************************************************************************

CLASS_NAME_IMPL(AstListElementsIterator);

AstListElementsIterator::AstListElementsIterator
(AST_INDEX listNode, AstIterDirection _direction) : 
AstNodeIterator(listNode, _direction) 
{ 
  Reset(); 
}


AstListElementsIterator::~AstListElementsIterator()
{ 
}


AST_INDEX AstListElementsIterator::Current() const 
{ 
  return current; 
}


void AstListElementsIterator::ReplaceCurrent(AST_INDEX newCurrent)
{
  current = newCurrent;
}


void AstListElementsIterator::Advance() 
{ 
  if (current != AST_NIL) 
    current = ((direction == AST_ITER_FORWARD) ? 
	       list_next(current) : list_prev(current));
}


void AstListElementsIterator::Reset() 
{ 
  current = ((direction == AST_ITER_FORWARD) ? 
	     list_first(node) : list_last(node));
}


void AstListElementsIterator::ReConstruct(AST_INDEX newListNode,
					  AstIterDirection _direction) 
{ 
  AstNodeIterator::ReConstruct(newListNode, _direction);
  Reset();
}



//***************************************************************************
// interface operations for class AstIterator
//***************************************************************************


AstIterator::AstIterator
(AST_INDEX rootOfWalk, TraversalOrder order, AstIterWalkType _walkType, 
AstIterDirection _direction) : 
IteratorStack(order, ITER_STACK_ENUM_ALL_NODES), walkType(_walkType), 
advanceAction(AST_ITER_CONTINUE), direction(_direction)
{
  ResetHelper(rootOfWalk);
}


AstIterator::~AstIterator()
{
}


AST_INDEX AstIterator::Current() const
{
  return (AST_INDEX) CurrentUpCall();
}


void AstIterator::ReplaceCurrent(AST_INDEX newCurrent)
{
  ((AstNodeIterator *) Top())->ReplaceCurrent(newCurrent);
}


void AstIterator::Advance(AstIterAdvanceDirective _advanceAction)
{
  advanceAction = _advanceAction;
  IteratorStack::operator++();
}


void AstIterator::Reset()
{  
  IteratorStack::Reset(); 
}


void AstIterator::ReConstruct(AST_INDEX newRootOfWalk, TraversalOrder order, 
			      AstIterWalkType _walkType,
			      AstIterDirection _direction)
{  
  IteratorStack::ReConstruct(order, ITER_STACK_ENUM_ALL_NODES);
  walkType = _walkType;
  direction = _direction;
  ResetHelper(newRootOfWalk);
}


TraversalVisitType AstIterator::VisitType()
{  
  switch(traversalOrder) {
  case PreOrder: return PreVisit;
  case PostOrder: return PostVisit;
  case PreAndPostOrder: {
    AstNodeIterator *top = (AstNodeIterator *) Top();
    if (top->ClassName() != CLASS_NAME(AstSimpleNodeIterator))
      return PreVisit;
    else return ((AstSimpleNodeIterator *) top)->visitType;
  }
  default:
    assert(0);
    return PostVisit;
  }
}


//===========================================================================
// private operations for class AstIterator
//===========================================================================

StackableIterator *AstIterator::IteratorToPushIfAny(void *current)
{
  AST_INDEX node = (AST_INDEX) current;
  if (node != AST_NIL) {
    if (traversalOrder == PreAndPostOrder) {
      AstNodeIterator *top = (AstNodeIterator *) Top();
      if (top->ClassName() != CLASS_NAME(AstSimpleNodeIterator)) {
	Push(new AstSimpleNodeIterator(node, PostVisit));
      } else {
	if (((AstSimpleNodeIterator *) top)->visitType == PreVisit) 
	  Push(new AstSimpleNodeIterator(node, PostVisit));
	else goto done;
      }
    }
    if (advanceAction == AST_ITER_CONTINUE) {
      if (walkType == AST_ITER_FULL_WALK) {
	if (is_list(node)) return new AstListElementsIterator(node, direction);
	else if (ast_get_son_count(node) > 0) {
	  AstNodeIterator *ni = new AstSonsIterator(node, direction);
	  if (ni->Current()) return ni;
	  else {
	    delete ni;
	    return 0;
	  }
        }
      } else {
	if (is_compound(node)) {
	  AST_INDEX stmtList = ast_get_son_n(node, in_son(node));
	  if (stmtList != AST_NIL) { 
	    AstNodeIterator *ni = 
		new AstListElementsIterator(stmtList, direction);
	    if (ni->Current()) return ni;
	    else {
		delete ni;
		return 0;
	    }
	  }
	}
      }
    } 
  }
 done:
  advanceAction = AST_ITER_CONTINUE;
  return 0;
}


void AstIterator::ResetHelper(AST_INDEX rootOfWalk)
{
  //----------------------------------------------------------
  // if walkType == AST_ITER_STMTS_ONLY, there are only two 
  // non-trivial cases
  //   1. rootOfWalk is a LIST_OF_NODES containing statements, walk 
  //      subtrees of statements rooted at list elements
  //   2. rootOfWalk is a statement: walk subtree rooted at rootOfWalk
  // otherwise, walk subtree rooted at rootOfWalk
  //----------------------------------------------------------

  if ((walkType == AST_ITER_STMTS_ONLY) && is_list(rootOfWalk) && 
      is_statement(list_first(rootOfWalk))) 
    Push(new AstListElementsIterator(rootOfWalk, direction));
  else {
    int walkThisNode = 
      ((walkType == AST_ITER_FULL_WALK) || is_statement(rootOfWalk));
    Push(new AstSimpleNodeIterator((walkThisNode ? rootOfWalk : 0),
				   ((traversalOrder == PostOrder) ? 
				    PostVisit : PreVisit)));
  }
}




//**************************************************************************
// interface operations for class AstAncestorsIterator
//***************************************************************************

CLASS_NAME_IMPL(AstAncestorsIterator);

AstAncestorsIterator::AstAncestorsIterator(AST_INDEX self) : 
AstNodeIterator(self) 
{ 
  Reset(); 
}


AstAncestorsIterator::~AstAncestorsIterator()
{ 
}


AST_INDEX AstAncestorsIterator::Current() const 
{ 
  return current; 
}


void AstAncestorsIterator::Advance() 
{ 
  if (current != AST_NIL) {
    if (in_list(current)) current = list_head(current);
    else current = tree_out(current);
  }
}


void AstAncestorsIterator::Reset() 
{ 
  current = node; 
  Advance();
}




