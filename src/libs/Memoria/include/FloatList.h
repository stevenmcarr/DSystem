/* $Id: FloatList.h,v 1.1 1992/12/07 10:16:34 carr Exp $ */
#ifndef FloatList_h
#define FloatList_h

#include <misc/sllist.h>

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
