/* $Id: DoublyLinkedList.h,v 1.7 1997/03/11 14:36:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef _DLIST_
#define _DLIST_

// DList - Class definitions for a doubly linked list object.
//
// Nat McIntosh - incorporated into Rn
// Cliff Click  - Hacked 10/12/91
// Cliff Click  - Made functionality and coding style match rest of class lib
//
// Author: Chris Vick, Copyright (C) 1991, 1992
// This material is released for public use, but is not placed in the public
// domain.  You are free to use/mangle/copy this code, but since I retain
// ownership you cannot copyright or patent this code.	Please leave this
// copyright notice in copies of this code, so that future programmers cannot
// be denied using it.
//

#ifndef _PORT_
#include <libs/support/misc/port.h>
#endif	_PORT_

typedef void (*voidfunc_ptr_voidstar)(void *);

//------------------------------Dlist------------------------------------------
// Dlist is a generic circular doubly linked list class which can be used to
// store any type of data by creating classes and casting class pointers to
// void pointers to store the data for each list element.

// Forward pointer layout:    ... -> tail -> DList* -> head -> ...
// operator >>= :: Insert at head of list
// operator <<= :: Insert at tail of list
// operator ++	:: Remove from head of list
// operator --	:: Remove from tail of list
// operator >>	:: Cycle shift head to tail
// operator <<	:: Cycle shift tail to head

class DList {
private:
  DList *h, *t; 		// Head & tail of list.
  union _DList_union_tag {
    void *user; 		// User's info (or list size for list-header)
    uint32 cnt; 		// Size of linked list
  } x;
  DList(DList *hh, DList *tt, void *u) : h(hh), t(tt) {x.user=u;}

public:
  // Empty list constructor.  Also sets list size to 0 by default.
  DList() { h = t = this; x.cnt = 0; }
  ~DList();			// destructor

  // Zero a list - assume that the fields are garbage and reinitialize
  // them to an empty list.
  void zero() { h = t = this; x.cnt = 0; };

  uint32 size() { return x.cnt; }	 // Returns the size of the list
  void *head(void) { return h->x.user; } // Convient access
  void *tail(void) { return t->x.user; } // Convient access

  // Adds an element to the head of the list
  DList &operator >>=(void *u) { DList *p = new DList(h, this, u); x.cnt++;
				 h->t = p; h = p; return *this; }
  // Adds an element to the tail of the list
  DList &operator <<=(void *u) { DList *p = new DList(this, t, u); x.cnt++;
				 t->h = p; t = p; return *this; }
  // Removes the head element from the list
  void *operator ++(void) { DList *p = h; h = h->h; h->t = this;
    void *u = p->x.user; p->x.cnt = 0; delete p; x.cnt--; return u; }
  // Removes the tail element from the list
  void *operator --(void) { DList *p = t; t = t->t; t->h = this;
    void *u = p->x.user; p->x.cnt = 0; delete p; x.cnt--; return u; }
  // Call a function once for each element in a list, passing 
  // it the 'user' field for each list element.
  void iterate_func(voidfunc_ptr_voidstar func);

  DList &operator >>(int nshift);	// Cycle shift head to tail
  DList &operator <<(int nshift);	// Cycle shift tail to head

};

#endif	_DLIST_
