/* $Id: FloatList.h,v 1.3 1993/06/30 19:46:02 johnmc Exp $ */
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
     SinglyLinkedList::Append(e);
    };
  FloatListEntry *first_entry()
    {return (FloatListEntry *) SinglyLinkedList::First();};
  FloatListEntry *last_entry()
    {return (FloatListEntry *) SinglyLinkedList::Last();};
  void free_head()
    { Delete(SinglyLinkedList::First());};
 };

#endif FloatList_h
