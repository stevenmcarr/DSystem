/* $Id: ResizableArray.C,v 1.7 1997/06/27 17:44:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
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

#include <string.h>
#include <iostream.h>

#include <include/bstring.h>

#include <libs/support/arrays/ResizableArray.h>

//------------------------------Array----------------------------------------
// Deep-copy an Array
Array::Array( Array &a ) : len(a.len)
{
  max = a.max;
  count = a.count;
  data = (char *)malloc(max*len);
  bcopy( (const char *)a.data, (char *)data, max*len );
}

//------------------------------Array----------------------------------------
// Deep-copy an Array
Array &Array::operator =( Array &a )
{
  free(data);			// Toss old data
  max = a.max;
  count = a.count;
  len = a.len;
  data = (char *)malloc(max*len);
  bcopy( (const char *)a.data, (char *)data, max*len );
  return *this;
}

//------------------------------grow-----------------------------------------
// Arrays of objects which grow as needed.  Here's where they grow.
void Array::grow( uint size )
{
  if( !data ) { 		// Create data area
    max = 1;
    data = (char *)calloc(max,len);
  }
  uint olen = max;		// Old size
  while( size >= max ) max <<= 1;  // Double to size
  data = (char *)realloc(data,max*len);
  bzero( data+olen*len, (max-olen)*len);
}

//------------------------------insert---------------------------------------
// Arrays of objects which grow as needed.  Here's where they grow.
void Array::insert( void *datum, uint where )
{
  assert( len == 4 );		// Convience for arrays of ptrs
  if( where > count ) count = where; // Past end of real data?
  count++;			// Inserting somewheres
  if( count > max ) grow(count);// Need to grow?
  void **home = &((void **)data)[where];
  bcopy((const char *) home, (char *)home+1, (count-where-1)*sizeof(void *) );
  *home = datum;
}

//------------------------------insert---------------------------------------
// Arrays of objects which grow as needed.  Here's where they grow.
void Array::insert( char &datum, uint where )
{
  if( where > count ) count = where; // Past end of real data?
  count++;			// Inserting somewheres
  if( count > max ) grow(count);// Need to grow?
  char *home = data+where*len;
  bcopy((const char *) home, (char *)home+len, (count-where-1)*len );
  bcopy((const char *) &datum, (char *)home, len);
}

//------------------------------remove-----------------------------------------
// Remove an item from the array.
void Array::remove( uint where )
{
  if( where >= count ) return;	// Deleting past end?
  count--;			// One fewer items in array
  char *home = data+where*len;	// Address being wiped out
  bcopy((const char *) home+len, (char *)home, (count-where)*len );
}

//------------------------------find-----------------------------------------
uint Array::operator []( void *datum ) const
{
  assert( len == 4 );		// Convience for arrays of pointers
  for( uint i=0; i<count; i++ ) {
    if( datum == ((void **)data)[i] )
      return i;
  }
  return i;
}

//------------------------------find-----------------------------------------
uint Array::operator []( char &datum ) const
{
  char *home = data;
  for( uint i=0; i<count; i++ ) {
    if( !bcmp(&datum,home,len) )
      return i;
    home += len;
  }
  return i;
}

//------------------------------print-----------------------------------------
// Print out an array.
ostream & operator << (ostream &os, Array &a)
{
  register uint i, j;
  char buf[3];
  os << "Array@" << ((void *)(&a)) << "[" << a.count << "] = {";
  switch( a.len ) {
  case 1:
    for( i=0; i<a.count-1; i++ )
      os << a.data[i] << ',';
    os << a.data[i];
    break;
  case 2:
    for( i=0; i<a.count-1; i++ )
      os << *(int16*)(a.data+i*2) << ',';
    os << *(int16*)(a.data+i*2);
    break;
  case 4:
    for( i=0; i<a.count-1; i++ )
      os << *(int32*)(a.data+i*4) << ',';
    os << *(int32*)(a.data+i*4);
    break;
  default:
    register char *s;
    os << "0x";
    for( i=0; i<a.count-1; i++ ) {
      s = a.data+i*a.len;
      for( j=0; j<a.len; j++ ) {
	sprintf(buf,"%02.2X",*s++);
	os << buf;
      }
      os << ",0x";
    }
    s = a.data+i*a.len;
    for( j=0; j<a.len; j++ ) {
      sprintf(buf,"%02.2X",*s++);
      os << buf;
    }
    break;
  }
  os << "}\n";
  return os;
}

