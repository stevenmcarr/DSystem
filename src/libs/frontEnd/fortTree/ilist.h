/* $Id: ilist.h,v 1.7 1997/06/24 17:53:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ilist_h
#define ilist_h

#include <libs/support/misc/general.h>
#include <libs/support/lists/SinglyLinkedList.h>

class InvocationListEntry: public SinglyLinkedListEntry {
  AST_INDEX invocation_node;
public:
  InvocationListEntry(AST_INDEX n) { invocation_node = n; };
  
  int node() { return invocation_node; }
  InvocationListEntry *Next() {
    return (InvocationListEntry *) SinglyLinkedListEntry::Next();
  };
};

class InvocationList: public SinglyLinkedList {
public:
  void Append(AST_INDEX node) {
    InvocationListEntry *e = new InvocationListEntry(node);
    SinglyLinkedList::Append(e);
  };
  InvocationListEntry *First() { 
    return (InvocationListEntry *) SinglyLinkedList::First(); 
  };
};

#endif
