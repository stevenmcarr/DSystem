/* $Id: Set.C,v 1.5 1997/03/11 14:37:19 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * Sets - An Abstract Data Type
 *
 * Author: Cliff Click, Copyright (C) 1991, 1992
 * This material is released for public use, but is not placed in the public
 * domain.  You are free to use/mangle/copy this code, but since I retain
 * ownership you cannot copyright or patent this code.	Please leave this
 * copyright notice in copies of this code, so that future programmers cannot
 * be denied using it.
 *
 * Change history:
 * 29 June 93 -- Added returns to avoid compiler warning messages. --paco
 */


#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <iostream.h>
#include <libs/support/sets/Set.h>

// 6/22/93: The following is missing from stdlib.h:
extern "C" long unsigned int strtoul(const char *s, char **end, int base);

//-------------------------Virtual Functions-----------------------------------
// These functions MUST be implemented by the inheriting class.
Set &Set::operator =  (const Set &)       { abort(); return *this; }
Set &Set::clone       (void)        const { abort(); return *this; }
Set &Set::operator <<= (uint)             { abort(); return *this; }
Set &Set::operator >>= (uint)             { abort(); return *this; }
int  Set::operator []  (uint)       const { abort(); return 0; }
Set &Set::operator &= (const Set &)       { abort(); return *this; }
Set &Set::operator |= (const Set &)       { abort(); return *this; }
Set &Set::operator -= (const Set &)       { abort(); return *this; }
int  Set::operator == (const Set &) const { abort(); return 0; }
int  Set::operator <  (const Set &) const { abort(); return 0; }
int  Set::operator <= (const Set &) const { abort(); return 0; }
int  Set::disjoint    (const Set &) const { abort(); return 0; }
void Set::Sort(void)                      { abort(); }
uint Set::getelem(void)             const { abort(); return 0; }
uint Set::Size(void)                const { abort(); return 0; }
Set::operator SparseSet* (void)     const { abort(); return NULL; }
Set::operator VectorSet* (void)     const { abort(); return NULL; }
Set::operator ListSet  * (void)     const { abort(); return NULL; }
Set::operator CoSet    * (void)     const { abort(); return NULL; }
//Set  Set::operator <<  (uint)             { abort(); }
//Set  Set::operator >>  (uint)             { abort(); }
//Set  Set::operator &  (const Set &) const { abort(); }
//Set  Set::operator |  (const Set &) const { abort(); }
//Set  Set::operator -  (const Set &) const { abort(); }

SetI_ *Set::iterate(uint&) const { abort(); return NULL; }
uint SetI_::next(void)           { abort(); return 0; }
int  SetI_::test(void)           { abort(); return 0; }

//------------------------------setstr-----------------------------------------
// Create a string with a printable representation of a set.
// The caller must deallocate the string.
char *Set::setstr() const
{
  if( !this ) return strdup("{no set}");
  Set &set = clone();		// Virtually copy the basic set.
  set.Sort();                   // Sort elements for in-order retrieval

  int len = 100;                // Remaining string space
  char *buf = (char*)malloc(len);// Some initial string space
  
  register char *s = buf;       // Current working string pointer
  *s++ = '{';

  // For all elements of the Set
  uint hi = (uint)-2L, lo = (uint)-2L;
  for( SetI i(&set); i.test(); i++ ) {	
    if( hi+1 == i.elem ) {      // Moving sequentially thru range?
      hi = i.elem;              // Yes, just update hi end of range
    } else {                    // Else range ended
      if( buf+len-s < 25 ) {    // Generous trailing space for upcoming number
	int offset = s-buf;     // Not enuf space; compute offset into buffer
	len <<= 1;              // Double string size
	buf = (char*)realloc(buf,len); // Reallocate doubled size
	s = buf+offset;         // Get working pointer into new bigger buffer
      }
      if( lo != -2L ) {         // Startup?  No!  Then print previous range.
	if( lo != hi ) sprintf(s,"%ld-%ld,",lo,hi);
	else sprintf(s,"%ld,",lo);
	s += strlen(s);         // Advance working string
      }
      hi = lo = i.elem;
    }
  }
  if( lo != -2L ) {
    if( lo != hi ) sprintf(s,"%ld-%ld}",lo,hi);
    else sprintf(s,"%ld}",lo);
  } else {
    // strcat(s,"}");      // 4/13/93 RvH: this caused "}}}}}} ..."
    *s++ = '}';
    *s = 0;
  }
  delete &set;                  // Nuke the temporary set
  return buf;
}

//------------------------------print------------------------------------------
ostream & operator << (ostream &os, const Set &s)
{
  char *printable_set = s.setstr();
  os << printable_set;
  free(printable_set);
  return os;
}

//------------------------------print------------------------------------------
// Handier print routine
void Set::print() const
{
  cout << (*this);
}

//------------------------------parse------------------------------------------
// Convert a textual representation of a Set, to a Set and union into "this"
// Set.  Return the amount of text parsed in "len", or zero in "len".
int Set::parse(const char *s)
{
  register char c;		// Parse character
  register const char *t = s;	// Save the starting position of s.

  do c = *s++;			// Skip characters
  while( c && (c <= ' ') );     // Till no more whitespace or EOS
  if( c != '{' ) return 0;      // Oops, not a Set openner
  if( *s == '}' ) return 2;     // The empty Set

  // Sets are filled with values of the form "xx," or "xx-yy," with the comma
  // a "}" at the very end.
  while(1) 
    {     // While have elements in the Set
       char *u;                               // Pointer to character ending parse
       uint elem = (uint)strtoul(s, &u, 10);  // Get element

       if( u == s ) return 0;                 // Bogus crude
       s = u;                                 // Skip over the number
       c = *s++;                              // Get the number seperator
       switch(c) 
         {                                    // Different seperators
            case '}':                         // Last simple element
            case ',':                         // Simple element
              (*this) <<= elem;               // Insert the simple element into the Set
            break;                            // Go get next element
 
            case '-':                         // Range
              {
                 uint hi = (uint)strtoul(s,&u,10);// Get element
                 if( u == s ) return 0;           // Bogus crude
                 for(uint i=elem; i<=hi; i++) (*this) <<= i;// Insert entire range into Set
                 s = u;			// Skip over the number
                 c = *s++; 		// Get the number seperator
              }
            break;
         }
       if( c == '}' ) break;       // End of the Set
       if( c != ',' ) return 0;    // Bogus garbage
    }
  return (int)(s-t);		// Return length parsed
}

