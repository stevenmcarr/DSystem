/* $Id: VectorSet.h,v 1.3 1997/03/11 14:37:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef _VECTOR_SET_
#define _VECTOR_SET_

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

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef _SET_
#include <libs/support/sets/Set.h>
#endif _SET_

// These sets can grow or shrink, based on the initial size and the largest
// element currently in them.  Slow and bulky for sparse sets, these sets
// are super for dense sets.  They are fast and compact when dense.

// TIME:
// O(1) - Insert, Delete, Member, Sort
// O(max_element) - Create, Clear, Size, Copy, Union, Intersect, Difference,
//                  Equal, ChooseMember, Forall

// SPACE: (max_element)/(8*sizeof(int))


//------------------------------VectorSet--------------------------------------
class VectorSet : public Set {
friend class VectorSetI;	// Friendly iterator class
protected:
  uint size;                    // Size of data IN LONGWORDS (32bits)
  uint32 *data;                 // The data, bit packed

  void slamin( const VectorSet& s );	 // Initialize one set with another
  int compare(const VectorSet &s) const; // Compare set contents
  void grow(uint newsize);               // Grow vector to required bitsize

public:
  VectorSet();			        // Creates a new, empty set.
  VectorSet(const VectorSet &s) {slamin(s);}	// Set clone; deep-copy guts
  Set &operator =(const Set &s);                // Set clone; deep-copy guts
  VectorSet &operator =(const VectorSet &s)     // Set clone; deep-copy guts
  { if( &s == this ) return *this; free((char*)data); slamin(s); return *this; }
  ~VectorSet() {free((char*)data);}
  Set &clone(void) const { return *(new VectorSet(*this)); }
  static Set &Construct(void) { return *(new VectorSet()); }

  Set &operator <<=(uint elem);          // Add member to set
  VectorSet operator << (uint elem)      // Add member to new set
  { VectorSet foo(*this); foo <<= elem; return foo; }
  Set &operator >>=(uint elem);          // Delete member from set
  VectorSet operator >> (uint elem)      // Delete member from new set
  { VectorSet foo(*this); foo >>= elem; return foo; }

  VectorSet &operator &=(const VectorSet &s); // Intersect sets into first set
  Set       &operator &=(const Set       &s); // Intersect sets into first set
  VectorSet operator & (const VectorSet &s) const
  { VectorSet foo(*this); foo &= s; return foo; }

  VectorSet &operator |=(const VectorSet &s); // Intersect sets into first set
  Set       &operator |=(const Set       &s); // Intersect sets into first set
  VectorSet operator | (const VectorSet &s) const
  { VectorSet foo(*this); foo |= s; return foo; }

  VectorSet &operator -=(const VectorSet &s); // Intersect sets into first set
  Set       &operator -=(const Set       &s); // Intersect sets into first set
  VectorSet operator - (const VectorSet &s) const
  { VectorSet foo(*this); foo -= s; return foo; }

  int operator ==(const VectorSet &s) const;  // True if sets are equal
  int operator ==(const Set       &s) const;  // True if sets are equal
  int disjoint   (const Set       &s) const;  // True if sets are disjoint
  int operator < (const VectorSet &s) const;  // True if strict subset
  int operator < (const Set       &s) const;  // True if strict subset
  int operator <=(const VectorSet &s) const;  // True if subset relation holds.
  int operator <=(const Set       &s) const;  // True if subset relation holds.

  int operator [](uint elem) const; // Test for membership
  uint getelem(void) const;         // Return a random element
  uint Size(void) const;            // Number of elements in the Set
  int  isEmpty() const { return !Size(); }  // Is Set empty ?
  void Sort(void);                  // Sort before iterating

  operator VectorSet* (void) const { return *this; }

private:
  friend class VSetI_;
  SetI_ *iterate(uint&) const;
};

//------------------------------Iteration--------------------------------------
// Loop thru all elements of the set, setting "elem" to the element numbers
// in random order.  Inserted or deleted elements during this operation may
// or may not be iterated over; untouched elements will be affected once.
// Usage:  for( VectorSetI i(s); i.test(); i++ ) { body = i.elem; }

class VSetI_ : public SetI_ {
  private:
    const VectorSet *s;
    uint i, j;
    uint32 mask;

    friend class VectorSet;
    friend class VectorSetI;

  public:
    VSetI_(const VectorSet *vset);
    ~VSetI_() {};

    uint next(void);
    int test(void) { return i < s->size; }
};

class VectorSetI : public SetI {
  public:
    VectorSetI( const VectorSet *s ) : SetI(s) { }
    void operator ++(void) { elem = ((VSetI_*)impl)->next(); }
    int test(void) { return ((VSetI_*)impl)->test(); }
};

#endif
