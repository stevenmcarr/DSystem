/* $Id: DoublyLinkedList.C,v 1.6 1998/02/19 15:25:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <libs/support/lists/DoublyLinkedList.h>

/******************************************************************
 * Doubly Linked List Abstraction              September 1991     *
 * Author: John Mellor-Crummey                                    *
 *                                                                *
 * this file contains a general purpose singly linked list        *
 * abstraction. the two components are the list itself,           *
 * and a template for entries in the list. this abstraction is    *
 * useless in its own right since the list and its entries        *
 * contain no information other than what is needed to describe   *
 * the structure. to make use of this abstraction, derive a       *
 * list entry class that contains some useful data, and           *
 * derive a corresponding list class composed of elements         *
 * of the derived list entry class. all of the structural         *
 * manipulation can be performed using the functions              *
 * provided in the base classes defined herein.                   *
 *                                                                *
 * Copyright 1991, Rice University, as part of the ParaScope      *
 * Programming Environment Project                                *
 *                                                                *
 ******************************************************************/

DoublyLinkedListEntry::DoublyLinkedListEntry()
{
  next = 0;
  previous = 0;
}

//----------------------------------------------------------------
// if a derived entry class contains any pointers to heap
// allocated storage, this virtual function should be
// defined for the derived class to free that storage
//----------------------------------------------------------------
DoublyLinkedListEntry::~DoublyLinkedListEntry()
{
  if (next) delete next;
  if (previous) delete previous;
}

DoublyLinkedListEntry *
DoublyLinkedListEntry::Next()
{
  return next;
}

DoublyLinkedListEntry *
DoublyLinkedListEntry::Previous()
{
  return previous;
}

void DoublyLinkedListEntry::LinkSuccessor(DoublyLinkedListEntry *e)
{
  next = e;
}

void DoublyLinkedListEntry::LinkPredecessor(DoublyLinkedListEntry *e)
{
  previous = e;
}

//----------------------------------------------------------------
// destructor invokes the destructor for the first element in 
// the list. the destructor for that element will delete any 
// successors to that element in the list.
//----------------------------------------------------------------
DoublyLinkedList::~DoublyLinkedList() 
{ 
  delete head;
}


//----------------------------------------------------------------
// add a list entry at the at the end of the current list
//
// pre-condition: the list entry must not already be part of a
//                list (i.e. it must have a null successor)
//----------------------------------------------------------------
void DoublyLinkedList::Append(DoublyLinkedListEntry *e) 
{ 
  assert(e->Next() == 0 && e->Previous() == 0); 
  
  if (head == 0) head = e;
  else {
    tail->LinkSuccessor(e); 
    e->LinkPredecessor(tail);
  }
  tail = e;
  count++;
}


//----------------------------------------------------------------
// add a list entry before the Old entry
//
// pre-condition: the list entry must not already be part of a
//                list (i.e. it must have a null successor)
//----------------------------------------------------------------
void DoublyLinkedList::InsertBefore(DoublyLinkedListEntry *Old,
				    DoublyLinkedListEntry *New) 
{ 
  assert(New->Next() == 0 && New->Previous() == 0); 
  
  if (Old == NULL) Append(New); 
  else
    {
      New->LinkPredecessor(Old->Previous());
      Old->LinkPredecessor(New);
      New->LinkSuccessor(Old);
      if (head = Old)
	head = New;
    }
  
  count++;
}


//----------------------------------------------------------------
// add a list entry after the Old entry
//
// pre-condition: the list entry must not already be part of a
//                list (i.e. it must have a null successor)
//----------------------------------------------------------------
void DoublyLinkedList::InsertAfter(DoublyLinkedListEntry *Old,
				   DoublyLinkedListEntry *New) 
{ 
  assert(New->Next() == 0 && New->Previous() == 0); 
  
  if (Old == NULL) Append(New); 
  else
    {
      New->LinkPredecessor(Old);
      New->LinkSuccessor(Old->Next());
      Old->LinkSuccessor(New);
      if (tail = Old)
	tail = New;
    }
  
  count++;
}


//----------------------------------------------------------------
// add a list entry at the front of the current list
//
// pre-condition: the list entry must not already be part of a
//                list (i.e. it must have a null successor)
//----------------------------------------------------------------
void DoublyLinkedList::Push(DoublyLinkedListEntry *e) 
{
  assert(e->Next() == 0 && e->Previous() == 0); 
  
  if (tail == 0) tail = e;
  else
    {
      e->LinkSuccessor(head);
      head->LinkPredecessor(e);
    }
  head = e;
  count++;
}


//----------------------------------------------------------------
// unlink and return the first entry in the list. if the list is
// empty, return null entry.
//
// post-condition: the list entry returned (if any) will have a
//                 null successor
//----------------------------------------------------------------
DoublyLinkedListEntry *DoublyLinkedList::Pop() 
{
  DoublyLinkedListEntry *first = head;
  if (head) {
    count--;
    head = head->Next();
    head->LinkPredecessor(0);
  }
  if (first) {
    if (tail == first) tail = 0;
    first->LinkSuccessor(0);
    first->LinkPredecessor(0);
  }
  return first;
}

//----------------------------------------------------------------
// delete a particular entry from the DoublyLinkedList
//
// note: make sure to zero the 'next' field for the list
// prior to deleting it, otherwise the destructor for the
// list entry will get rid of the rest of the list...
//
//----------------------------------------------------------------
void DoublyLinkedList::Delete(DoublyLinkedListEntry *e)
{
  DoublyLinkedListEntry *e1;
  count--;
  if (e == head && e == tail){
    head = tail = 0;
    e->LinkSuccessor(0);
    e->LinkPredecessor(0);
    delete e;
  }
  
  else if (e == head) {
    head = e->Next();
    head->LinkPredecessor(0);
    e->LinkSuccessor(0);
    e->LinkPredecessor(0);
    delete e;
  }
  else if (e == tail) {
    tail = tail->Previous();
    tail->LinkSuccessor(0);
    e->LinkSuccessor(0);
    e->LinkPredecessor(0);
    delete e;
  }
  else {
    e->Previous()->LinkSuccessor(e->Next());
    e->Next()->LinkPredecessor(e->Previous());
    e->LinkSuccessor(0);
    e->LinkPredecessor(0);
    delete e;
  }
}


DoublyLinkedList::DoublyLinkedList()
{
  count = 0;
  head = 0;
  tail = 0;
}


DoublyLinkedListEntry *
DoublyLinkedList::First()
{
  return head;
}


DoublyLinkedListEntry *
DoublyLinkedList::Last()
{
  return tail;
}


unsigned int
DoublyLinkedList::Count()
{
  return count;
}


DoublyLinkedListIterator::DoublyLinkedListIterator(DoublyLinkedList *l,
						   Boolean Rev)
{ 
  // extend to handle degenerate case (NULL list) to increase convenience 
  // of use -- JMC 1/93
  Reverse = Rev;
  if (Reverse)
    first = current_entry = (l ? l->Last() : 0);
  else
    first = current_entry = (l ? l->First() : 0);
}


DoublyLinkedListIterator::DoublyLinkedListIterator(DoublyLinkedList &l,
						   Boolean Rev)
{ 
  Reverse = Rev;
  if (Reverse)
    first = current_entry = l.Last();
  else
    first = current_entry = l.First(); 
}


void DoublyLinkedListIterator::operator ++()
{
  current_entry = (current_entry ? current_entry->Next() : 0);
}

void DoublyLinkedListIterator::operator --()
{
  current_entry = (current_entry ? current_entry->Previous() : 0);
}

DoublyLinkedListEntry *DoublyLinkedListIterator::Current()
{
  return current_entry;
}

void DoublyLinkedListIterator::Reset()
{
  current_entry = first;
}

