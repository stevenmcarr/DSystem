/* $Id: IntList.h,v 1.1 1997/03/11 14:35:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef IntList_h
#define IntList_h
#include <libs/support/lists/SinglyLinkedList.h>

class IntListEntry: public SinglyLinkedListEntry 
{
  public:
    IntListEntry() : SinglyLinkedListEntry() {};
    IntListEntry(int i) : item(i), SinglyLinkedListEntry() {};
    int item;
};
class IntList: public SinglyLinkedList
{
  public:
    IntList() : SinglyLinkedList() {};
    int First() { return ((IntListEntry *)SinglyLinkedList::First())->item; };
    int Last()  { return ((IntListEntry *)SinglyLinkedList::Last())->item; };

    // add/remove items from list
    void Append(int i)
    {
	SinglyLinkedList::Append((SinglyLinkedListEntry *)new IntListEntry(i));
    };
    void Push(int i)
    {
	SinglyLinkedList::Push((SinglyLinkedListEntry *)new IntListEntry(i));
    };
    int Pop()
    {
	IntListEntry *ip = (IntListEntry *) SinglyLinkedList::Pop();
	int i = ip->item;
	delete ip;
	return i;
    };
};
#endif // IntList_h
