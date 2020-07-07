/* $Id: port.C,v 1.13 2001/10/12 19:34:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * Code for portable compiling
 *
 * Author: Cliff Click, Copyright (C) 1991, 1992
 * This material is released for public use, but is not placed in the public
 * domain.  You are free to use/mangle/copy this code, but since I retain
 * ownership you cannot copyright or patent this code.	Please leave this
 * copyright notice in copies of this code, so that future programmers cannot
 * be denied using it.
 */

#include <iostream>

using namespace std;

#include <libs/support/misc/port.h>

//-----------------------------print-------------------------------------------
// This function prints a void pointer to a stream.  It only exists because
// gcc does not support this, but Solaris CC doesn't support a const void*
// << operator.

#ifdef __GNUC__

ostream &operator << (ostream &os, void *ptr)
{
  os << (const void*)ptr;
  return os;
}

#else

ostream &operator << (ostream &os, const void *ptr)
{
  os << (void*)ptr;
  return os;
}

#endif

//------------------------------gcd--------------------------------------------
// Greatest common divisor
int32 gcd( int32 x, int32 y )
{
  if( !x ) return y;
  else if( x == y ) return y;
  else if( x < y ) return gcd(y-x,x);
  else return gcd(y,x-y);
}
