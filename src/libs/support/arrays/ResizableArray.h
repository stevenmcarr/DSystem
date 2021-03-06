/* $Id: ResizableArray.h,v 1.9 2001/10/12 19:35:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef _ARRAY_
#define _ARRAY_

/*
 * Self-sizing arrays - An Abstract Data Type
 *
 * Author: Cliff Click, Copyright (C) 1991, 1992
 * This material is released for public use, but is not placed in the public
 * domain.  You are free to use/mangle/copy this code, but since I retain
 * ownership you cannot copyright or patent this code.	Please leave this
 * copyright notice in copies of this code, so that future programmers cannot
 * be denied using it.
 */

#include <ostream>
#include <stdio.h>
#include <assert.h>
#include <libs/support/misc/port.h>
#include <libs/support/memMgmt/libmem.h>
//class ostream;

using namespace std;

//------------------------------Array------------------------------------------
// Arrays of objects which grow as needed.
class Array {
 private:
  uint max;
  uint count;
  uint16 len;
  void grow( uint size );
 public:
  char *data;
  Array( uint16 llen = sizeof(void *) ) : len(llen)
    { max = count = 0; data = NULL; }
  virtual ~Array( ) { if( data ) free(data); }
  Array( Array & );
  Array &operator =( Array & );

  uint cnt( void ) const { return count; }
  void insert( void *datum, uint where );
  void insert( char &datum, uint where );
  void replace( void *datum, uint where )
    { if( where >= count ) count = where+1;	   // Past end of real data?
      if( where >= max ) grow(where); ((void**)data)[where] = datum; }
  void replace( char &datum, uint where )
    { if( where >= count ) count = where+1;	   // Past end of real data?
      if( where >= max ) grow(where); bcopy(&datum,data+where*len,len); }
  void operator += ( void *datum ) { insert( datum, count ); }
  void operator += ( char &datum ) { insert( datum, count ); }
  void operator -= ( uint where ) { remove( where ); }
  void remove( uint where );
  char *operator [] ( uint where )
    { if( where >= count ) count = where+1;	   // Past end of data?
      if( where >= max ) grow(where); return data+where*len; }
  uint operator [] ( void *datum ) const;
  uint operator [] ( char &datum ) const;

  friend ostream & operator << (ostream &, Array &a);
};

#endif 
