/* $Id: StringBuffer.h,v 1.4 1997/03/11 14:37:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef StringBuffer_h
#define StringBuffer_h

#include <stdio.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif 

//************************************************************************
// StringBuffer.h -- a simple utility for building strings of arbitrary
//                   length through a sequence of Append operations.
//
// Author:
//   John Mellor-Crummey                                 February 1993
//
// Copyright 1993 as part of the ParaScope Programming Environment 
// project, Rice University.
//************************************************************************

class StringBuffer {
  char *buffer;
  unsigned int buflen; // current length of the buffer
  unsigned int slen; // current length of the string

  //----------------------------------------------------------------------
  // calls realloc if additional space is needed during an Append
  // operation; doubles the size of the buffer if that will suffice, 
  // but only allocates as much as needed if the buffer must grow by more
  // than double
  //______________________________________________________________________
  void GetSpace(const int howmuch);
  
public:
  //----------------------------------------------------------------------
  // constructor -- init_len is the length of the string allocated 
  // initially.  If init_len is not specified, a
  // string of length 80 will be allocated.  The string can grow to
  // arbitrary length (exceeding init_len) through a sequence of Append
  // operations. 
  //----------------------------------------------------------------------
  StringBuffer(const unsigned int init_len = 80); 
  ~StringBuffer(); 

  //----------------------------------------------------------------------
  // reinitialize the StringBuffer, discarding the current string if any.
  // If init_len is not specified, Reset will allocate a buffer
  // of size 80.
  //----------------------------------------------------------------------
  void Reset(const unsigned int init_len = 80);

  //----------------------------------------------------------------------
  // extend the string according to arguments which are in a form suitable
  // for use by vsprintf
  //----------------------------------------------------------------------
  void Append(const char *format, ...); 

  //----------------------------------------------------------------------
  // extend the string with a single character
  //----------------------------------------------------------------------
  void Append(const char c);

  //----------------------------------------------------------------------
  // append two string buffer entries
  //----------------------------------------------------------------------
  void Append(const StringBuffer *buf);

  //----------------------------------------------------------------------
  // return the constructed string which must be deallocated by the caller
  // Flush reallocates the returned string into a block of memory just 
  // large enough to hold the returned string so no space is wasted.
  //----------------------------------------------------------------------
  char *Flush(); 

  //----------------------------------------------------------------------
  // depercated interface
  //----------------------------------------------------------------------
  char *Finalize();
  
};

#endif /* StringBuffer_h */
