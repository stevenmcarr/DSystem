/* $Id: PrefetchList.h,v 1.1 1993/06/15 14:06:39 carr Exp $ */
#ifndef PrefetchList_h
#define PrefetchList_h

#ifndef sllist_h
#include <misc/sllist.h>
#endif

#ifndef mh_ast_h
#include <mh_ast.h>
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
     SinglyLinkedList::append_entry(e);
    };
  PrefetchListEntry *first_entry()
    {return (PrefetchListEntry *) SinglyLinkedList::first_entry();};
  PrefetchListEntry *last_entry()
    {return (PrefetchListEntry *) SinglyLinkedList::last_entry();};
  void free_head()
    { delete_entry(SinglyLinkedList::first_entry());};
  Boolean NullList()
    {return first_entry() == NULL;}
 };

class PrefetchListIterator : public SinglyLinkedListIterator {
public:
	// old style
	PrefetchListEntry *next_entry()
	 {return (PrefetchListEntry *)SinglyLinkedListIterator::next_entry();};
	PrefetchListEntry *current()
	 {return (PrefetchListEntry *)SinglyLinkedListIterator::current();};

};
#endif PrefetchList_h
