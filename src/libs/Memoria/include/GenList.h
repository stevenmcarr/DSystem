#ifndef GenList_h
#define GenList_h

#ifndef sllist_h
#include <misc/sllist.h>
#endif

#ifndef sr_h
#include <sr.h>
#endif

class GenListEntry : public SinglyLinkedListEntry {

  scalar_info_type *sptr;

public:

  GenListEntry(scalar_info_type *s)  {sptr = s;};
  scalar_info_type *GetValue() {return sptr;};

 };

class GenList : public SinglyLinkedList {
public:
  void Append(scalar_info_type *s) 
    {
     GenListEntry *e = new GenListEntry(s);
     SinglyLinkedList::Append(e);
    };
  GenListEntry *First()
    {return (GenListEntry *) SinglyLinkedList::First();};
  GenListEntry *Last()
    {return (GenListEntry *) SinglyLinkedList::Last();};
  void FreeHead()
    { Delete(SinglyLinkedList::First());};
  Boolean NullList()
    {return BOOL(First() == NULL);}
  void Clear()
    {
     while(NOT(NullList()))
       FreeHead();
    }
 };

class GenListIterator : public SinglyLinkedListIterator {
public:
        GenListIterator(GenList *G) : SinglyLinkedListIterator(G)
	  {
	  }
	GenListEntry *Current()
	 {return (GenListEntry *)SinglyLinkedListIterator::Current();};
};
#endif 
