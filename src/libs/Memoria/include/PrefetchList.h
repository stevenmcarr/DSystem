/* $Id: PrefetchList.h,v 1.6 1994/07/20 11:31:52 carr Exp $ */
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
