/* $Id: VectorSet.C,v 1.4 2001/09/17 01:33:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * Vector Sets - An Abstract Data Type
 *
 * Author: Cliff Click, Copyright (C) 1991, 1992
 * This material is released for public use, but is not placed in the public
 * domain.  You are free to use/mangle/copy this code, but since I retain
 * ownership you cannot copyright or patent this code.	Please leave this
 * copyright notice in copies of this code, so that future programmers cannot
 * be denied using it.
 */

#include <string.h>

#include <libs/support/sets/VectorSet.h>

static const uint MAXUINT32 = (uint)-1L;

// BitsInByte is a lookup table which tells the number of bits that
// are in the looked-up number.  It is very useful in VectorSet_Size.

static uint8 bitsInByte[256] = {
  0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

//------------------------------VectorSet--------------------------------------
// Create a new, empty Set.
VectorSet::VectorSet()
{
  size = 2;                     // Small initial size
  data = (uint32*)malloc(size*sizeof(uint32));
  data[0] = 0;                  // No elements
  data[1] = 0;
}

//------------------------------operator=--------------------------------------
Set &VectorSet::operator = (const Set &s)
{
  if( &s == this ) return *this;
  free((char*)data);
  slamin(*(const VectorSet *)&s);
  return *this;
}

//------------------------------slamin-----------------------------------------
// Initialize one set with another.  No regard is made to the existing Set.
void VectorSet::slamin(const VectorSet& s)
{
  size = s.size;                // Use new size
  data = (uint32*)malloc(size*sizeof(uint32)); // Make array of required size
  memcpy(data, s.data, size*sizeof(uint32) );  // Fill the array
}

//------------------------------grow-------------------------------------------
// Expand the existing set to a bigger size
void VectorSet::grow(register uint newsize) 
{
  newsize = (newsize+31) >> 5;  // Convert to longwords
  data = (uint32 *)realloc((char*)data,newsize*sizeof(uint32));  // Get bigger size
  memset((data + size), 0, (newsize - size)*sizeof(uint32));
  size = newsize;
}

//------------------------------operator<<=------------------------------------
// Insert a member into an existing Set.
Set &VectorSet::operator <<= (uint elem)
{
  register uint word = elem >> 5;            // Get the longword offset
  register uint32 mask = 1L << (elem & 31);  // Get bit mask

  if( word >= size )            // Need to grow set?
    grow(elem+1);               // Then grow it
  data[word] |= mask;           // Set new bit
  return *this;
}

//------------------------------operator>>=------------------------------------
// Delete a member from an existing Set.
Set &VectorSet::operator >>= (uint elem)
{
  register uint word = elem >> 5; // Get the longword offset
  if( word >= size )              // Beyond the last?
    return *this;                 // Then it's clear & return clear
  register uint32 mask = 1L << (elem & 31);     // Get bit mask
  data[word] &= ~mask;            // Clear bit
  return *this;
}

//------------------------------operator&=-------------------------------------
// Intersect one set into another.
VectorSet &VectorSet::operator &= (const VectorSet &s)
{
  // NOTE: The intersection is never any larger than the smallest set.
  size = min(size,s.size);      // Get smaller size
  register uint32 *u1 = data;   // Pointer to the destination data
  register uint32 *u2 = s.data; // Pointer to the source data
  for( uint i=0; i<size; i++)   // For data in set
    *u1++ &= *u2++;             // Copy and AND longwords
  return *this;                 // Return set
}

//------------------------------operator&=-------------------------------------
Set &VectorSet::operator &= (const Set &s)
{
  // The cast is a virtual function that checks that "s" is a VectorSet.
  return (*this) &= (*(const VectorSet *)&s);
}

//------------------------------operator|=-------------------------------------
// Union one set into another.
VectorSet &VectorSet::operator |= (const VectorSet &s)
{
  uint cnt = min(size,s.size);  // This many words must be unioned
  register uint32 *u1 = data;   // Pointer to the destination data
  register uint32 *u2 = s.data; // Pointer to the source data
  for( uint i=0; i<cnt; i++)    // Copy and OR the two sets
    *u1++ |= *u2++;

  if( cnt < s.size ) {          // Is set 2 larger than set 1?
    // Extend result by larger set
    grow(max(size,s.size)*sizeof(uint32)*8); 
    memcpy(data, s.data, (s.size - cnt)*sizeof(uint32));
    }
  return *this;                 // Return result set
}

//------------------------------operator|=-------------------------------------
Set &VectorSet::operator |= (const Set &s)
{
  // The cast is a virtual function that checks that "s" is a VectorSet.
  return (*this) |= (*(const VectorSet *)&s);
}

//------------------------------operator-=-------------------------------------
// Difference one set from another.
VectorSet &VectorSet::operator -= (const VectorSet &s)
{
  register uint cnt = min(size,s.size); // Get smaller size
  register uint32 *u1 = data;   // Pointer to the destination data
  register uint32 *u2 = s.data; // Pointer to the source data
  for( uint i=0; i<cnt; i++ )   // For data in set
    *u1++ &= ~(*u2++);          // A <-- A & ~B  with longwords
  return *this;                 // Return new set
}

//------------------------------operator-=-------------------------------------
Set &VectorSet::operator -= (const Set &s)
{
  // The cast is a virtual function that checks that "s" is a VectorSet.
  return (*this) -= (*(const VectorSet *)&s);
}

//------------------------------compare----------------------------------------
// Compute 2 booleans: bits in A not B, bits in B not A.
// Return X0 --  A is not a subset of B
//        X1 --  A is a subset of B
//        0X --  B is not a subset of A
//        1X --  B is a subset of A
int VectorSet::compare (const VectorSet &s) const 
{
  register uint32 *u1 = data;   // Pointer to the destination data
  register uint32 *u2 = s.data; // Pointer to the source data
  register uint32 AnotB = 0, BnotA = 0;
  register uint cnt = min(size,s.size);

  // Get bits for both sets
  uint i;
  for(i=0; i<cnt; i++ ) { // For data in BOTH sets
    register uint32 A = *u1++;  // Data from one guy
    register uint32 B = *u2++;  // Data from other guy
    AnotB |= (A & ~B);          // Compute bits in A not B
    BnotA |= (B & ~A);          // Compute bits in B not A
  }

  // Get bits from bigger set
  if( size < s.size ) {
    for( ; i<s.size; i++ )      // For data in larger set
      BnotA |= *u2++;           // These bits are in B not A
  } else {
    for( ; i<size; i++ )        // For data in larger set
      AnotB |= *u1++;           // These bits are in A not B
  }

  // Set & return boolean flags
  return ((!BnotA)<<1) + (!AnotB);
}

//------------------------------operator==-------------------------------------
// Test for set equality
int VectorSet::operator == (const VectorSet &s) const
{
  return compare(s) == 3;       // TRUE if A and B are mutual subsets
}

//------------------------------operator==-------------------------------------
int VectorSet::operator == (const Set &s) const
{
  // The cast is a virtual function that checks that "s" is a VectorSet.
  return (*this) == (*(const VectorSet *)&s);
}

//------------------------------disjoint---------------------------------------
// Check for sets being disjoint.
int VectorSet::disjoint(const Set &set) const
{
  const VectorSet &s = (*(const VectorSet *)&set);  // Checking cast operator

  // NOTE: The intersection is never any larger than the smallest set.
  uint smallsize = min(size,s.size); // Get smaller size
  register uint32 *u1 = data;   // Pointer to the destination data
  register uint32 *u2 = s.data; // Pointer to the source data
  for( uint i=0; i<smallsize; i++)   // For data in set
    if( *u1++ & *u2++ )         // If any elements in common
      return 0;                 // Then not disjoint
  return 1;                     // Else disjoint
}

//------------------------------operator<--------------------------------------
// Test for strict subset
int VectorSet::operator < (const VectorSet &s) const
{
  return compare(s) == 1;       // A subset B, B not subset A
}

//------------------------------operator<--------------------------------------
int VectorSet::operator < (const Set &s) const
{
  // The cast is a virtual function that checks that "s" is a VectorSet.
  return (*this) < (*(const VectorSet *)&s);
}

//------------------------------operator<=-------------------------------------
// Test for subset
int VectorSet::operator <= (const VectorSet &s) const
{
  return compare(s) & 1;        // A subset B
}

//------------------------------operator<=-------------------------------------
int VectorSet::operator <= (const Set &s) const
{
  // The cast is a virtual function that checks that "s" is a VectorSet.
  return (*this) <= (*(const VectorSet *)&s);
}

//------------------------------operator[]-------------------------------------
// Test for membership.  A Zero/Non-Zero value is returned!
int VectorSet::operator[](uint elem) const
{
  register uint word = elem >> 5; // Get the longword offset
  if( word >= size )              // Beyond the last?
    return 0;                     // Then it's clear
  register uint32 mask = 1L << (elem & 31);  // Get bit mask
  return (data[word] & mask);     // Return the sense of the bit
}

//------------------------------getelem----------------------------------------
// Get any element from the set.
uint VectorSet::getelem(void) const
{
  uint i;
  int j;
  for( i=0; i<size; i++ )
    if( data[i] )
      break;
  uint word = data[i];
  for( j= -1; word; j++, word>>=1 );
  return (i<<5)+j;
}

//------------------------------Size-------------------------------------------
// Return number of elements in a Set
uint VectorSet::Size(void) const
{
  uint sum = 0;                 // Cumulative size so far. 
  uint8 *currByte = (uint8*)data;
  for( uint32 i = 0; i < (size<<2); i++) // While have bytes to process
    sum += bitsInByte[*currByte++];      // Add bits in current byte to size.
  return sum;
}

//------------------------------Sort-------------------------------------------
// Sort the elements for the next forall statement
void VectorSet::Sort(void)
{
}

//------------------------------iterate----------------------------------------
SetI_ *VectorSet::iterate(uint &elem) const
{
  VSetI_ *foo = (new VSetI_((const VectorSet *) this));
  elem = foo->next();
  return foo;
}

//=============================================================================
//------------------------------VSetI_-----------------------------------------
// Initialize the innards of a VectorSet iterator
VSetI_::VSetI_( const VectorSet *vset ) : s(vset)
{
  i = (uint)-1L;
  j = 0;
  mask = (1L<<31);
}

//------------------------------next-------------------------------------------
// Find and return the next element of a vector set, or return garbage and
// make "VSetI_::test()" fail.
uint VSetI_::next(void)
{
  j++;                          // Next element in word
  mask<<=1;                     // Next bit in word
  do {                          // Do While still have words
    while( mask ) {             // While have bits in word
      if( s->data[i] & mask ) { // If found a bit
        return (i<<5)+j;        // Return the bit address
      }
      j++;                      // Skip to next bit
      mask<<=1;
    }
    j = 0;                      // No more bits in word; setup for next word
    mask = 1;
    for( i++; (i<s->size) && (!s->data[i]); i++ ); // Skip to non-zero word
  } while( i<s->size );
  return MAXUINT32;             // No element, iterated them all
}

