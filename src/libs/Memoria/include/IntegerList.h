/* $Id: IntegerList.h,v 1.1 1995/03/13 15:07:06 carr Exp $ */
#ifndef IntegerList_h
#define IntegerList_h

#ifndef sllist_h
#include <misc/sllist.h>
#endif

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

       (*this)++;
       return e;
    } 

};

#endif
