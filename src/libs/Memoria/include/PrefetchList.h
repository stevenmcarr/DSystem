/* $Id: PrefetchList.h,v 1.7 1997/03/20 15:49:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef PrefetchList_h
#define PrefetchList_h

#include <libs/support/lists/SinglyLinkedList.h>

#ifndef mh_ast_h
#include <libs/Memoria/include/mh_ast.h>
#endif

class PrefetchListEntry : public SinglyLinkedListEntry {

  AST_INDEX Node;

public:

  PrefetchListEntry(AST_INDEX n)  {Node = n;};
  AST_INDEX GetValue() {return Node;};

 };

class PrefetchList : public SinglyLinkedList {
public:
  void append_entry(AST_INDEX n) 
    {
     PrefetchListEntry *e = new PrefetchListEntry(n);
     SinglyLinkedList::Append(e);
    };
  PrefetchListEntry *first_entry()
    {return (PrefetchListEntry *) SinglyLinkedList::First();};
  PrefetchListEntry *last_entry()
    {return (PrefetchListEntry *) SinglyLinkedList::Last();};
  void free_head()
    { Delete(SinglyLinkedList::First());};
  Boolean NullList()
    {return BOOL(first_entry() == NULL);}
 };

class PrefetchListIterator : public SinglyLinkedListIterator {
public:
        PrefetchListIterator(PrefetchList *P) : SinglyLinkedListIterator(P)
	  {
	  }
	PrefetchListEntry *current()
	 {return (PrefetchListEntry *)SinglyLinkedListIterator::Current();};

};
#endif
