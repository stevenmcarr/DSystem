/* $Id: IntegerList.h,v 1.3 1997/03/20 15:49:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef IntegerList_h
#define IntegerList_h

#include <libs/support/lists/SinglyLinkedList.h>

class IntegerListEntry : public SinglyLinkedListEntry {

  int value;

public:

  IntegerListEntry(int i)  {value = i;};
  int GetValue() {return value;};

};

class IntegerList : public SinglyLinkedList {
private:
  friend class IntegerListIter;
public:
  IntegerList* operator+=(int i) 
    {
     IntegerListEntry *e = new IntegerListEntry(i);
     SinglyLinkedList::Append(e);
     return this;
    };
  IntegerListEntry *First()
    {return (IntegerListEntry *) SinglyLinkedList::First();};
  IntegerListEntry *Last()
    {return (IntegerListEntry *) SinglyLinkedList::Last();};

  Boolean QueryEntry(int i);

  IntegerListEntry* GetEntry(int i);
  
};

class IntegerListIter : public SinglyLinkedListIterator {
public:
  IntegerListIter(IntegerList *l) : 
    SinglyLinkedListIterator(l)
  {
  }

  IntegerListIter(IntegerList &l) : 
    SinglyLinkedListIterator(l)
  {
  }
  
  IntegerListEntry* operator() ()
    {
     IntegerListEntry *e = 
       (IntegerListEntry *) SinglyLinkedListIterator::Current();

       ++(*this);
       return e;
    } 

};

#endif
