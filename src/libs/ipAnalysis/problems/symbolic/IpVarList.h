/* $Id: IpVarList.h,v 1.1 1997/03/11 14:35:18 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef IpVarList_h
#define IpVarList_h
#include <libs/support/lists/SinglyLinkedList.h>
#include <libs/support/strings/rn_string.h>

class IpVarListEntry: public SinglyLinkedListEntry 
{
  public:
    IpVarListEntry() : name(NULL), offset(0), length(0),
                       SinglyLinkedListEntry()
    {};
    IpVarListEntry(char *n, int o, int len) : offset(o), length(len),
                                              SinglyLinkedListEntry()
    {
	name = ssave(n);
    };
    ~IpVarListEntry()
    {
	sfree(name);
    };
    char *name;
    int offset;
    int length;
};
class IpVarList: public SinglyLinkedList
{
friend class IpVarListIter;
  public:
    IpVarList() : SinglyLinkedList() {};

    // add/remove items from list
    void Append(char *n, int o, int len)
    {
	SinglyLinkedList::Append((SinglyLinkedListEntry *)
				 new IpVarListEntry(n, o, len));
    };
    void Push(char *n, int o, int len)
    {
	SinglyLinkedList::Push((SinglyLinkedListEntry *)
			       new IpVarListEntry(n, o, len));
    };
};

class IpVarListIter: public SinglyLinkedListIterator 
{
public:
  IpVarListIter(IpVarList *l): SinglyLinkedListIterator(l) {};
  IpVarListIter(IpVarList &l): SinglyLinkedListIterator(l) {};
  
  IpVarListEntry *Current() 
  {
      return (IpVarListEntry *)SinglyLinkedListIterator::Current();
  };
  char *Current(int &o, int & len)
  {
      IpVarListEntry *e = (IpVarListEntry *)SinglyLinkedListIterator::Current();
      if (e)
      {
	  o    = e->offset;
	  len  = e->length;
	  return e->name;
      }
      else
	  return (char *) 0;
  };
};

#endif // IpVarList_h
