/* $Id: ParameterList.h,v 1.1 1997/03/11 14:34:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ParameterList_h
#define ParameterList_h

//*****************************************************************
// Parameter List Abstraction                   September 1993
// Author: John Mellor-Crummey                                
//
// Copyright 1993, Rice University
//*****************************************************************

#include <libs/support/lists/IOSinglyLinkedList.h>

//-----------------------------------------------------
// class ParameterListEntry
//    information about a formal parameter to a 
//    procedure
//
//    values of type field are VTYPE_STAR, 
//    VTYPE_PROCEDURE, or VTYPE_UNDEFINED
//
//    this class reads/writes itself from/to a database
//    port
//-----------------------------------------------------

class ParameterListEntry : public SinglyLinkedListEntryIO {
  int ptype;    // parameter type
  char *pname;  // parameter name
public:
  // used with no args when initializing one to read from a file
  ParameterListEntry(char *pname = 0, int ptype = 0);
  ParameterListEntry(ParameterListEntry *rhs);
  virtual ~ParameterListEntry();
  
  // field access
  int type();
  char *name();
  
  // ------------ I/O ------------
  int ReadUpCall(FormattedFile& port);
  int WriteUpCall(FormattedFile& port);
};


//-----------------------------------------------------
// class ParameterList
//    a list of information about a procedure's formal
//    parameters
//
//    this class reads/writes itself from/to a database
//    port
//-----------------------------------------------------

class ParameterList : public SinglyLinkedListIO {
  // current position in the list in an enumeration
  ParameterListEntry *current; 
public:
  ParameterList();
  ParameterList(ParameterList *rhs);
  ~ParameterList();
  
  SinglyLinkedListEntryIO *NewEntry();
  
  ParameterListEntry *First() { 
    return (current = (ParameterListEntry *) 
	    SinglyLinkedList::First()); 
  };
  ParameterListEntry *Next() { 
    return current = 
      (current ? (ParameterListEntry *) current->Next() : 0); 
  };
};

#endif
