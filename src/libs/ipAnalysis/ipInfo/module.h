/* $Id: module.h,v 1.8 1997/03/11 14:34:51 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ModuleIPinfoList_h
#define ModuleIPinfoList_h

#include <libs/support/file/FormattedFile.h>

/******************************************************************
 * Module IP Information Abstraction           September 1991     *
 * Author: John Mellor-Crummey                                    *
 *                                                                *
 * this file contains definitions that support an external        *
 * representation of summary interprocedural information for      *
 * all of the entry points in a module.                           *
 *                                                                *
 * Copyright 1991, Rice University, as part of the ParaScope      *
 * Programming Environment Project                                *
 *                                                                *
 ******************************************************************/

#include <libs/support/lists/IOSinglyLinkedList.h>
#include <libs/ipAnalysis/ipInfo/iptree.h>

//-----------------------------------------------------
// class ModuleIPinfoListEntry
//    a list record for an entry point in a module 
//    that contains summary interprocedural information 
//    for the entry.
//
//    this class reads/writes itself from/to a database
//    port
//-----------------------------------------------------

class ModuleIPinfoListEntry: public SinglyLinkedListEntryWithDBIO {
public:
  IPinfoTree *info;
  
  // default argument used by ModuleIPinfoList::NewEntry
  ModuleIPinfoListEntry(IPinfoTree *tree = 0) { 
    info = (tree ? tree : new IPinfoTree);  // new tree if none provided
  };
  virtual ~ModuleIPinfoListEntry();
  
  ModuleIPinfoListEntry *Next() { 
    return (ModuleIPinfoListEntry *) 
      SinglyLinkedListEntryWithDBIO::Next();
  };
  
  // ------------ I/O ------------
  int ReadUpCall(FormattedFile& port);
  int WriteUpCall(FormattedFile& port);
};

//-----------------------------------------------------
// class ModuleIPinfoList
//    a list containing a record for each entry point
//    of a module. 
//
//    this class reads/writes itself from/to a database
//    port
//-----------------------------------------------------

class ModuleIPinfoList: public SinglyLinkedListWithDBIO {
  ModuleIPinfoListEntry *current;
public:
  
  ModuleIPinfoList() { current = 0; };
  
  // function that returns a new element of type ModuleIPinfoListEntry
  SinglyLinkedListEntryWithDBIO *NewEntry();
  
  // support FIFO addition of elements
  void Append(IPinfoTree *info) {
    ModuleIPinfoListEntry *e = new ModuleIPinfoListEntry(info);
    SinglyLinkedList::Append((SinglyLinkedListEntry *) e);
  };
  
  // support FIFO removal of elements
  ModuleIPinfoListEntry *Pop() {
    return (ModuleIPinfoListEntry *) SinglyLinkedList::Pop();
  };
  
  // routines to traverse the list
  ModuleIPinfoListEntry *First() { 
    return (current = 
	    (ModuleIPinfoListEntry *) SinglyLinkedList::First()); 
  };
  
  ModuleIPinfoListEntry *Next() { 
    return current = (current ?
		      (ModuleIPinfoListEntry *) current->Next(): 0);
  };
};

#endif
