/* $Id: DoublyLinkedList.h,v 1.8 1998/02/19 15:25:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef DoublyLinkedList_h
#define DoublyLinkedList_h

#include <libs/support/misc/general.h>

/******************************************************************
 * Doubly Linked List Abstraction              September 1991     *
 * Author: John Mellor-Crummey                                    *
 *                                                                *
 * this file contains a general purpose singly linked list        *
 * abstraction. the three components are the list itself,         *
 * a template for entries in the list, and an iterator for        *
 * entries in the list. this abstraction is useless in its own    *
 * right since the list and its entries contain no information    *
 * other than what is needed to describe  the structure. to make  *
 * use of this abstraction, derive a list entry class that        *
 * contains some useful data, and derive a corresponding list     *
 * class composed of elements of the derived list entry class.    *
 * all of the structural manipulation can be performed using the  *
 * functions provided in the base classes defined herein.         *
 *                                                                *
 * Copyright 1991, 1992  Rice University, as part of the          *
 * ParaScope Programming Environment Project                      *
 *                                                                *
 ******************************************************************/

class DoublyLinkedListEntry { 			// a list entry 
  DoublyLinkedListEntry *next;
  DoublyLinkedListEntry *previous;
public:
  // constructor to initialize successor pointer 
  DoublyLinkedListEntry();
  
  // if a derived entry class contains any pointers to heap
  // allocated storage, this virtual function should be
  // defined for the derived class to free that storage
  virtual ~DoublyLinkedListEntry();
  
  // access successor information
  DoublyLinkedListEntry *Next();
  DoublyLinkedListEntry *Previous();
  void LinkSuccessor(DoublyLinkedListEntry *e);
  void LinkPredecessor(DoublyLinkedListEntry *e);
};

class DoublyLinkedList {               // the list class itself
  DoublyLinkedListEntry *head, *tail; 
  int count;
public:
  // constructor
  DoublyLinkedList();
  
  // destructor: if a derived list class contains any pointers 
  // to heap allocated storage, this virtual function should 
  // be defined for the derived class to free that storage
  virtual ~DoublyLinkedList();
  
  // inspect list state
  DoublyLinkedListEntry *First();
  DoublyLinkedListEntry *Last();
  unsigned int Count();
  
  // add/remove items from list
  void Append(DoublyLinkedListEntry *e);
  void InsertBefore(DoublyLinkedListEntry *Old,
		    DoublyLinkedListEntry *New);
  void InsertAfter(DoublyLinkedListEntry *Old,
		   DoublyLinkedListEntry *New);
  void Push(DoublyLinkedListEntry *e);
  void Delete(DoublyLinkedListEntry *e);
  DoublyLinkedListEntry *Pop();
};


class DoublyLinkedListIterator {
  DoublyLinkedListEntry *current_entry;
  DoublyLinkedListEntry *first;
  Boolean Reverse;
public:
  DoublyLinkedListIterator(DoublyLinkedList *l,
			   Boolean reverse = false);
  DoublyLinkedListIterator(DoublyLinkedList &l,
			   Boolean reverse = false);
  
  void operator ++();
  void operator --();
  DoublyLinkedListEntry *Current();
  void Reset();
};

#endif
