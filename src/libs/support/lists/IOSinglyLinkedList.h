/* $Id: IOSinglyLinkedList.h,v 1.2 1997/06/25 15:16:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef SinglyLinkedListIO_h
#define SinglyLinkedListIO_h


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

#include <libs/support/lists/SinglyLinkedList.h>

class FormattedFile;


//-----------------------------------------------------
// class SinglyLinkedListEntryIO
//    a derived class of SinglyLinkedListEntry 
//    that knows how to read and write itself from/to
//    database ports. this data structure is useful
//    only as a base class for structures that have 
//    useful information stored in the list records.
//-----------------------------------------------------

class SinglyLinkedListEntryIO : public SinglyLinkedListEntry {
public:
  // read data for a derived list entry 
  virtual int ReadUpCall(FormattedFile&) = 0;
  
  // write data for a derived list entry 
  virtual int WriteUpCall(FormattedFile&) = 0;
};


//-----------------------------------------------------
// class SinglyLinkedListIO
//    a derived class of SinglyLinkedList 
//    that knows how to read and write itself from/to
//    database ports. 
//-----------------------------------------------------

class SinglyLinkedListIO : public SinglyLinkedList {
public:
  // create a new list element for a derived list class 
  virtual SinglyLinkedListEntryIO *NewEntry() = 0;
  
  // initiate I/O 
  int Read(FormattedFile& port);
  int Write(FormattedFile& port);

  // perform I/O of data associated with a derived list class 
  virtual int ReadUpCall(FormattedFile& port);
  virtual int WriteUpCall(FormattedFile& port);
};

#endif 
