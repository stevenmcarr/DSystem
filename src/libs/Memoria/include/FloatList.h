/* $Id: FloatList.h,v 1.5 1997/03/20 15:49:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef FloatList_h
#define FloatList_h

#include <libs/support/lists/SinglyLinkedList.h>

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

#endif
