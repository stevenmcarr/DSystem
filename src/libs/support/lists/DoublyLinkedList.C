/* $Id: DoublyLinkedList.C,v 1.5 1997/03/11 14:36:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// DList.cc - Member function definitions for the doubly linked list class
//
// Nat McIntosh - added shift functions, iterator, and incorporated into Rn
// Cliff Click  - Hacked 10/12/91
// Cliff Click  - Made functionality and coding style match rest of class lib
//
// Author: Chris Vick, Copyright (C) 1991
// This material is released for public use, but is not placed in the public
// domain.  You are free to use/mangle/copy this code, but since I retain
// ownership you cannot copyright or patent this code.	Please leave this
// copyright notice in copies of this code, so that future programmers cannot
// be denied using it.
//

#include <libs/support/lists/DoublyLinkedList.h>

//------------------------------~DList-----------------------------------------
// Empty the list, ignoring contents.
DList::~DList()
{
  register DList *temp;
  while( x.cnt ) {              // While list not empty
    temp = h;                   // Item to nuke
    h = h->h;                   // Get next item
    temp->x.cnt = 0;            // Nothing here (list is empty)
    delete temp;                // Nuke this item
    x.cnt--;                    // Lower list size
  }
}

void DList::iterate_func(voidfunc_ptr_voidstar func)
{
  DList *tmp;
  int i;

  i = (int)x.cnt;
  tmp = h;
  while (i--) {
    (*func)(tmp->x.user);
    tmp = tmp->h;
  }
  return;
}

// Cycle shift head to tail
DList& DList::operator >>(int nshift)
{
  DList *p;

  while (nshift--) {
    /* remove from head */
     p = h;
     h = h->h;
     h->t = this;
     /* add to tail */
     p->h = this;
     p->t = t;
     t->h = p;
     t = p;
   }
  return *this;
}

// Cycle shift tail to head
DList& DList::operator <<(int nshift)
{
  DList *p;

  while (nshift--) {
    /* remove from tail */
    p = t;
    t = t->t;
    t->h = this;
    /* add to head */
    p->h = h;
    p->t = this;
    h->t = p;
    h = p;
  }
  return *this;
}

#if 0
void print_list_elem(void *u)
{
  printf("%d ", u);
}

void print_dlist(DList *l)
{
  printf("0x%x: ", l);
  (*l).iterate_func(print_list_elem);
  printf("\n");
}

main(int argc, char **argv)
{
  DList x;

  x >>= (void *) 3;
  x >>= (void *) 2;
  x >>= (void *) 1;
  print_dlist(&x);
  x >> 1;
  print_dlist(&x);
  x >> 1;
  print_dlist(&x);
  x >> 1;
  print_dlist(&x);
  x >> 3;
  print_dlist(&x);
}
#endif
