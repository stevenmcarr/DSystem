/* $Id: modref.h,v 1.11 1997/03/11 14:34:51 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************
 * Mod/Ref Information for a Scope             September 1991     *
 * Author: John Mellor-Crummey                                    *
 *                                                                *
 * this file contains definitions that support an external        *
 * representation of summary of mod/ref for variables. all        *
 * information is represented in a form based on variable         *
 * equivalence classes.                                           *
 *                                                                *
 * Copyright 1991, Rice University, as part of the ParaScope      *
 * Programming Environment Project                                *
 *                                                                *
 ******************************************************************/

#ifndef modrefht_h
#define modrefht_h

#include <string.h>

#include <libs/support/misc/general.h>

#include <libs/support/tables/HashTable.h>
#include <libs/support/database/OBSOLETE/AsciiDbioPort.h>
#include <libs/support/strings/rn_string.h>

#include <libs/ipAnalysis/ipInfo/iptypes.h>

#include <libs/support/lists/IOSinglyLinkedList.h>


//-----------------------------------------------------
// class PairListEntry
//    information about a mod or ref to an equivalence
//    class of variables
//
//    sz represents the size of the equivalence 
//    class member accessed
//
//    for local or common variables, off represents the 
//    offset from the equivalence leader. for 
//    formal parameters, off represents the parameter
//    position.
//
//    this class reads/writes itself from/to a database
//    port
//-----------------------------------------------------

class PairListEntry: public SinglyLinkedListEntryWithDBIO {
public:
  // default arguments used by PairList::NewEntry()
  PairListEntry(int offset=0, int size=0) { off = offset; sz = size; };
  
  int Offset() { return off; }
  int Size() { return sz; }
  
  int ReadUpCall(FormattedFile& port);
  int WriteUpCall(FormattedFile& port);

private:
  int off; // offset/parameter position
  int sz;  // size
};


//-----------------------------------------------------
// class PairList
//    a list pairs containing information about 
//    accesses to an equivalence class of variables
//
//    this class reads/writes itself from/to a database
//    port
//-----------------------------------------------------

class PairList: public SinglyLinkedListWithDBIO {
public:
  PairList() { current = 0; };
  virtual SinglyLinkedListEntryWithDBIO *NewEntry();
  
  void Append(int offset, int size) {
    PairListEntry *e = new PairListEntry(offset,size);
    SinglyLinkedList::Append((SinglyLinkedListEntry *) e);
  };
  void Push(int offset, int size) {
    PairListEntry *e = new PairListEntry(offset,size);
    SinglyLinkedList::Push((SinglyLinkedListEntry *)e);
  };
  PairListEntry *Pop() {
    return (PairListEntry *) SinglyLinkedList::Pop();
  };
  PairListEntry *First() { 
    return current = (PairListEntry *) SinglyLinkedList::First(); 
  };
  PairListEntry *Next() { 
    return current = 
      (current ? (PairListEntry *) current->Next() : 0);
  };
  
private:
  // current position in the list in an enumeration
  PairListEntry *current;
};


//-----------------------------------------------------
// class ModRefHashTableEntry
//    an entry in a ModRefInfo HashTable that represents
//    mod/ref information to an equivalence class of
//    variables
//
//    this class reads/writes itself from/to a database
//    port
//-----------------------------------------------------

class ModRefHashTableEntry {
public:
  void init(char *leader = 0, unsigned int vtype = 0);
  void DeleteEntry();
  
  unsigned int hash(unsigned int size) { 
    return hash_string(leader, size);
  };
  int compare(ModRefHashTableEntry *m) { 
    return strcmp(this->leader, m->leader); 
  };
  int Read(FormattedFile& port);
  int Write(FormattedFile& port);

  friend class ModRefInfo;

private:
  char *leader;         // equivalence class leader name
  int  type;            // VTYPE information 
  PairList *modref[2];  // mod and ref lists 
};


//-----------------------------------------------------
// class ModRefInfo
//    a HashTable for a particular scope that contains 
//    an entry for each variable equivalence class that 
//    contains information about modifications or 
//    references to some member of that class
//
//    this class reads/writes itself from/to a database
//    port
//-----------------------------------------------------

class ModRefInfo : private HashTable {
public:
  ModRefInfo(); 
  
  // add information to the mod or ref set
  void AddModRef(ModRefType mut, char *leader, int offset, unsigned int size,
		 unsigned int vtype);
  
  unsigned int ModRefCount(ModRefType mut, char *leader);
  
  // ------------- query functions -------------
  
  Boolean GetFirstModRef(ModRefType mut, char *leader, int *offset, 
			 unsigned int *size, unsigned int *vtype);
  
  Boolean GetNextModRef(ModRefType mut, char *leader, int *offset, 
			unsigned int *size, unsigned int *vtype);
  
  // enumerate equivalence leaders
  unsigned int LeaderCount();
  
  char *GetNextLeader(unsigned int &vtype); 
  char *GetNextLeader();  // deprecated -- JMC 2/93

  char *GetFirstLeader(unsigned int &vtype);
  char *GetFirstLeader(); // deprecated -- JMC 2/93
  
  // ------------- I/O -------------
  int Write(FormattedFile& port);
  int Read(FormattedFile& port);

private:
  int current_index; // for enumerating entries in the table

  virtual unsigned int HashFunct(const void *entry, const unsigned int size);
  virtual void EntryCleanup(void *entry);
  virtual int EntryCompare(const void *e1, const void *e2);
};

#endif /* modrefht_h */
