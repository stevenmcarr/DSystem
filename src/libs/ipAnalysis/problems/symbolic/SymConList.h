/* $Id: SymConList.h,v 1.1 1997/03/11 14:35:19 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef SymConList_h
#define SymConList_h
#include <libs/support/lists/SinglyLinkedList.h>
#include <libs/ipAnalysis/problems/symbolic/SymConAnnot.h>

class SymConListEntry: public SinglyLinkedListEntry 
{
  public:
    SymConListEntry() : SinglyLinkedListEntry() {};
    SymConListEntry(ConstraintItem *i) : SinglyLinkedListEntry()
    {
	this->con = i;
    };
    ConstraintItem *con;
};
class SymConList: public SinglyLinkedList
{
  public:
    SymConList() : SinglyLinkedList() {};
    ConstraintItem *First() { return ((SymConListEntry *)
				      (SinglyLinkedList::First()))->con; };
    ConstraintItem *Last()  { return ((SymConListEntry *)
				      (SinglyLinkedList::Last()))->con; };
    // add/remove items from list
    void Append(ConstraintItem *i)
    {
	SinglyLinkedList::Append(new SymConListEntry(i));
    };
    void Append(int par, int my, int pc, int add)
    {
	SinglyLinkedList::Append(new SymConListEntry(new 
						     ConstraintItem(par, my, 
								    pc, add)));
    };
    void Push(ConstraintItem *i)
    {
	SinglyLinkedList::Push(new SymConListEntry(i));
    };
    void Push(int par, int my, int pc, int add)
    {
	SinglyLinkedList::Push(new SymConListEntry(new 
						   ConstraintItem(par, my, 
								  pc, add)));
    };
    ConstraintItem *Pop()
    {
	SymConListEntry *p = (SymConListEntry *) SinglyLinkedList::Pop();
	ConstraintItem *ip = p->con;
	delete p;
	return ip;
    };
    void Pop(int &par, int &my, int &pc, int &add)
    {
	SymConListEntry *p = (SymConListEntry *) SinglyLinkedList::Pop();
	ConstraintItem *ip = p->con;
	delete p;
	par = ip->parent;
	my  = ip->myCoeff;
	pc  = ip->pCoeff;
	add = ip->addend;
	delete ip;
	return;
    };
};
#endif // SymConList_h
