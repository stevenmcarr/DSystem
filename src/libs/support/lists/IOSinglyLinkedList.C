/* $Id: IOSinglyLinkedList.C,v 1.3 1997/03/11 14:36:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************
 * I/O of a Singly Linked List Abstraction     September 1991     *
 * Author: John Mellor-Crummey                                    *
 *                                                                *
 * this file contains support for reading and writing a general   *
 * purpose singly linked list abstraction. this abstraction is    *
 * useless in its own right since the list and its entries        *
 * contain no information other than what is needed to describe   *
 * the structure. to make use of this abstraction, derive a       *
 * list entry class that contains some useful data, and           *
 * derive a corresponding list class composed of elements         *
 * of the derived list entry class. input and output of           *
 * structural information is performed using the functions        *
 * provided here. a client merely has to read/write the data      *
 * contained in a list entry.                                     *
 *                                                                *
 * Copyright 1991, Rice University, as part of the ParaScope      *
 * Programming Environment Project                                *
 *                                                                *
 ******************************************************************/

#include <stdio.h>
#include <assert.h>
#include <libs/support/misc/general.h>

#include <libs/support/file/FormattedFile.h>
#include <libs/support/lists/IOSinglyLinkedList.h>


// ***************************************************************************
// class SinglyLinkedListIO
// ***************************************************************************


//-----------------------------------------------
// read a list of entries from a database
// port. this function is meant to be used
// directly by derived classes
//-----------------------------------------------
int SinglyLinkedListIO::Read(FormattedFile& port) 
{
  int code = ReadUpCall(port);
  if (code) return code;

  int entries_in_list;
  code = port.Read(entries_in_list); 
  if (code) return code;

  while (entries_in_list-- > 0) { 
    SinglyLinkedListEntryIO *e = NewEntry();
    code = e->ReadUpCall(port);
    if (code) return code;
    Append(e);
  }
  return 0;
}


//-----------------------------------------------
// write a list of entries to a database
// port. this function is meant to be used
// directly by derived classes
//-----------------------------------------------
int SinglyLinkedListIO::Write(FormattedFile& port) 
{
  int code = WriteUpCall(port);
  if (code) return code;

  code = port.Write(Count()); 
  if (code) return code;
  
  SinglyLinkedListEntryIO *e = 
    (SinglyLinkedListEntryIO *) First();
  
  // write each entry in the list
  while (e != 0) {
    code = e->WriteUpCall(port);
    if (code) return code;
    e = (SinglyLinkedListEntryIO *) e->Next();
  }
  return 0;
}

int SinglyLinkedListIO::WriteUpCall(FormattedFile&)
{ 
  return 0; 
} 
int SinglyLinkedListIO::ReadUpCall(FormattedFile&)
{ 
  return 0;
} 
