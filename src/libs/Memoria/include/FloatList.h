/* $Id: FloatList.h,v 1.2 1992/12/11 11:19:38 carr Exp $ */
#ifndef FloatList_h
#define FloatList_h

#ifndef sllist_h
#include <misc/sllist.h>
#endif

class FloatListEntry : public SinglyLinkedListEntry {

  float value;

public:

  FloatListEntry(float f)  {value = f;};
  float GetValue() {return value;};
  void IncrementValue() {value += 1.0;};
  void AddValue(float f) {value += f;};

 };

class FloatList : public SinglyLinkedList {
public:
  void append_entry(float f) 
    {
     FloatListEntry *e = new FloatListEntry(f);
     SinglyLinkedList::append_entry(e);
    };
  FloatListEntry *first_entry()
    {return (FloatListEntry *) SinglyLinkedList::first_entry();};
  FloatListEntry *last_entry()
    {return (FloatListEntry *) SinglyLinkedList::last_entry();};
  void free_head()
    { delete_entry(SinglyLinkedList::first_entry());};
 };

#endif FloatList_h
