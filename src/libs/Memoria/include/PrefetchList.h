/* $Id: PrefetchList.h,v 1.4 1993/06/30 22:05:48 johnmc Exp $ */
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
     SinglyLinkedList::Append(e);
    };
  PrefetchListEntry *first_entry()
    {return (PrefetchListEntry *) SinglyLinkedList::First();};
  PrefetchListEntry *last_entry()
    {return (PrefetchListEntry *) SinglyLinkedList::Last();};
  void free_head()
    { Delete(SinglyLinkedList::First());};
  Boolean NullList()
    {return first_entry() == NULL;}
 };

class PrefetchListIterator : public SinglyLinkedListIterator {
public:
	// old style
	PrefetchListEntry *next_entry()
	{ 
	  // Steve: this next_entry interface is deprecated 
	  // use ++ and Current() instead -- JMC 6/30/93
	 (*((PrefetchListIterator *)this))++; 
	 return (PrefetchListEntry *) SinglyLinkedListIterator::Current();
	 };
	PrefetchListEntry *current()
	 {return (PrefetchListEntry *)SinglyLinkedListIterator::Current();};

};
#endif PrefetchList_h
