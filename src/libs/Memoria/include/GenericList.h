/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef GenericList_h
#define GenericList_h

#include <libs/support/lists/SinglyLinkedList.h>

class GenericListEntry : public SinglyLinkedListEntry {

  Generic value;

public:

  GenericListEntry(Generic i)  {value = i;};
  Generic GetValue() {return value;};

};

class GenericList : public SinglyLinkedList {
private:
  friend class GenericListIter;
public:
  GenericList(void) : SinglyLinkedList () {};
  GenericList* operator+=(Generic i) 
    {
     GenericListEntry *e = new GenericListEntry(i);
     SinglyLinkedList::Append(e);
     return this;
    };
  void Append(Generic i) {SinglyLinkedList::Append(new GenericListEntry(i));};
  GenericListEntry *First()
    {return (GenericListEntry *) SinglyLinkedList::First();};
  GenericListEntry *Last()
    {return (GenericListEntry *) SinglyLinkedList::Last();};

  Boolean QueryEntry(Generic i);

  GenericListEntry* GetEntry(Generic i);
  
};

class GenericListIter : public SinglyLinkedListIterator {
public:
  GenericListIter(GenericList *l) : 
    SinglyLinkedListIterator(l)
  {
  }

  GenericListIter(GenericList &l) : 
    SinglyLinkedListIterator(l)
  {
  }
  
  GenericListEntry* operator() ()
    {
     GenericListEntry *e = 
       (GenericListEntry *) SinglyLinkedListIterator::Current();

       ++(*this);
       return e;
    } 

};

#endif
