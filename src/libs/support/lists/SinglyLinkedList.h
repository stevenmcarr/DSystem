/* $Id: SinglyLinkedList.h,v 1.2 1997/06/25 15:16:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef SinglyLinkedList_h
#define SinglyLinkedList_h

/******************************************************************
 * Singly Linked List Abstraction              September 1991     *
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

class SinglyLinkedListEntry { 			// a list entry 
  SinglyLinkedListEntry *next;
public:
  // constructor to initialize successor pointer 
  SinglyLinkedListEntry();
  
  // if a derived entry class contains any pointers to heap
  // allocated storage, this virtual function should be
  // defined for the derived class to free that storage
  virtual ~SinglyLinkedListEntry();
  
  // access successor information
  SinglyLinkedListEntry *Next();
  void LinkSuccessor(SinglyLinkedListEntry *e);
};

class SinglyLinkedList {               // the list class itself
  SinglyLinkedListEntry *head, *tail; 
  int count;
public:
  // constructor
  SinglyLinkedList();
  
  // destructor: if a derived list class contains any pointers 
  // to heap allocated storage, this virtual function should 
  // be defined for the derived class to free that storage
  virtual ~SinglyLinkedList();
  
  // inspect list state
  SinglyLinkedListEntry *First();
  SinglyLinkedListEntry *Last();
  unsigned int Count();
  
  // add/remove items from list
  void Append(SinglyLinkedListEntry *e);
  void Push(SinglyLinkedListEntry *e);
  void Delete(SinglyLinkedListEntry *e);
  SinglyLinkedListEntry *Pop();
};


class SinglyLinkedListIterator {
  SinglyLinkedListEntry *current_entry;
  SinglyLinkedListEntry *first;
public:
  SinglyLinkedListIterator(SinglyLinkedList *l);
  SinglyLinkedListIterator(SinglyLinkedList &l);
  
  void operator ++();
  SinglyLinkedListEntry *Current();
  void Reset();
};

#endif
