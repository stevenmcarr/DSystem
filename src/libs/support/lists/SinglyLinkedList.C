/* $Id: SinglyLinkedList.C,v 1.10 1997/03/11 14:36:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <libs/support/lists/SinglyLinkedList.h>

/******************************************************************
 * Singly Linked List Abstraction              September 1991     *
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

SinglyLinkedListEntry::SinglyLinkedListEntry()
{
  next = 0;
}

//----------------------------------------------------------------
// if a derived entry class contains any pointers to heap
// allocated storage, this virtual function should be
// defined for the derived class to free that storage
//----------------------------------------------------------------
SinglyLinkedListEntry::~SinglyLinkedListEntry()
{
  if (next) delete next;
}

SinglyLinkedListEntry *
SinglyLinkedListEntry::Next()
{
  return next;
}

void SinglyLinkedListEntry::LinkSuccessor(SinglyLinkedListEntry *e)
{
  next = e;
}

//----------------------------------------------------------------
// destructor invokes the destructor for the first element in 
// the list. the destructor for that element will delete any 
// successors to that element in the list.
//----------------------------------------------------------------
SinglyLinkedList::~SinglyLinkedList() 
{ 
  delete head;
}


//----------------------------------------------------------------
// add a list entry at the at the end of the current list
//
// pre-condition: the list entry must not already be part of a
//                list (i.e. it must have a null successor)
//----------------------------------------------------------------
void SinglyLinkedList::Append(SinglyLinkedListEntry *e) 
{ 
  assert(e->Next() == 0); 
  
  if (head == 0) head = e;
  else tail->LinkSuccessor(e); 
  tail = e;
  count++;
}


//----------------------------------------------------------------
// add a list entry at the front of the current list
//
// pre-condition: the list entry must not already be part of a
//                list (i.e. it must have a null successor)
//----------------------------------------------------------------
void SinglyLinkedList::Push(SinglyLinkedListEntry *e) 
{
  assert(e->Next() == 0); 
  
  if (tail == 0) tail = e;
  e->LinkSuccessor(head);
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
SinglyLinkedListEntry *SinglyLinkedList::Pop() 
{
  SinglyLinkedListEntry *first = head;
  if (head) {
    count--;
    head = head->Next();
  }
  if (first) {
    if (tail == first) tail = 0;
    first->LinkSuccessor(0);
  }
  return first;
}

//----------------------------------------------------------------
// delete a particular entry from the SinglyLinkedList
//
// note: make sure to zero the 'next' field for the list
// prior to deleting it, otherwise the destructor for the
// list entry will get rid of the rest of the list...
//
//----------------------------------------------------------------
void SinglyLinkedList::Delete(SinglyLinkedListEntry *e)
{
  SinglyLinkedListEntry *e1;
  count--;
  if (e == head && e == tail){
    head = tail = 0;
    e->LinkSuccessor(0);
    delete e;
  }
  
  else if (e == head) {
    head = e->Next();
    e->LinkSuccessor(0);
    delete e;
  }
  
  /* get the entry that points to e */
  else {
    e1 = head;
    while ((e1->Next() != e) && (e1->Next() != 0))
      e1 = e1->Next(); 
    
    if (e1->Next() != 0){
      e1->LinkSuccessor(e->Next());
      if (e == tail)
	tail = e1;
      
      e->LinkSuccessor(0);
      delete e;
    }
    else ++count; /* no such entry */
  }
}


SinglyLinkedList::SinglyLinkedList()
{
  count = 0;
  head = 0;
  tail = 0;
}


SinglyLinkedListEntry *
SinglyLinkedList::First()
{
  return head;
}


SinglyLinkedListEntry *
SinglyLinkedList::Last()
{
  return tail;
}


unsigned int
SinglyLinkedList::Count()
{
  return count;
}


SinglyLinkedListIterator::SinglyLinkedListIterator(SinglyLinkedList *l)
{ 
  // extend to handle degenerate case (NULL list) to increase convenience 
  // of use -- JMC 1/93
  first = current_entry = (l ? l->First() : 0);
}


SinglyLinkedListIterator::SinglyLinkedListIterator(SinglyLinkedList &l)
{ 
  first = current_entry = l.First(); 
}


void SinglyLinkedListIterator::operator ++()
{
  current_entry = (current_entry ? current_entry->Next() : 0);
}

SinglyLinkedListEntry *SinglyLinkedListIterator::Current()
{
  return current_entry;
}

void SinglyLinkedListIterator::Reset()
{
  current_entry = first;
}

