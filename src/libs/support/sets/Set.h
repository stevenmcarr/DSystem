/* $Id: Set.h,v 1.2 1997/03/11 14:37:19 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef _SET_
#define _SET_

/*
 * Sets - An Abstract Data Type
 *
 * Author: Cliff Click, Copyright (C) 1991, 1992
 * This material is released for public use, but is not placed in the public
 * domain.  You are free to use/mangle/copy this code, but since I retain
 * ownership you cannot copyright or patent this code.	Please leave this
 * copyright notice in copies of this code, so that future programmers cannot
 * be denied using it.
 */

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _PORT_
#include <libs/support/misc/port.h>
#endif _PORT_
class ostream;
class SetI_;
class SparseSet;
class VectorSet;
class ListSet;
class CoSet;

// These sets can grow or shrink, based on the initial size and the largest
// element currently in them.  Basically, they allow a bunch of bits to be
// grouped together, tested, set & cleared, intersected, etc.  The basic
// Set class is an abstract class, and cannot be constructed.  Instead,
// one of VectorSet, SparseSet, or ListSet is created.  Each variation has 
// different asymptotic running times for different operations, and different
// constants of proportionality as well.  
// {n = number of elements, N = largest element}

// 		VectorSet	SparseSet	ListSet
// Create	O(N)		O(1)		O(1)
// Clear	O(N)		O(1)		O(1)
// Insert	O(1)		O(1)		O(log n)
// Delete	O(1)		O(1)		O(log n)
// Member	O(1)		O(1)		O(log n)
// Size		O(N)		O(1)		O(1)
// Copy		O(N)		O(n)		O(n)
// Union	O(N)		O(n)		O(n)
// Intersect	O(N)		O(n)		O(n)
// Difference	O(N)		O(n)		O(n)
// Equal	O(N)		O(n)		O(n)
// ChooseMember	O(N)		O(1)		O(1)
// Sort		O(1)		O(n log n)	O(1)
// Forall	O(N)		O(n)		O(n)
// Complement	O(1)		O(1)		O(1)

// TIME:	N/32		n		8*n	Accesses
// SPACE:	N/8		4*N+4*n		8*n	Bytes

// Create:	Make an empty set
// Clear:	Remove all the elements of a Set
// Insert:	Insert an element into a Set; duplicates are ignored
// Delete:	Removes an element from a Set
// Member:	Tests for membership in a Set
// Size:	Returns the number of members of a Set
// Copy:	Copy or assign one Set to another
// Union:	Union 2 sets together
// Intersect:	Intersect 2 sets together
// Difference:	Compute A & !B; remove from set A those elements in set B
// Equal:	Test for equality between 2 sets
// ChooseMember Pick a random member
// Sort:	If no other operation changes the set membership, a following
//		Forall will iterate the members in ascending order.
// Forall:	Iterate over the elements of a Set.  Operations that modify
//		the set membership during iteration work, but the iterator may
//		skip any member or duplicate any member.
// Complement:	Only supported in the Co-Set variations.  It adds a small
//		constant-time test to every Set operation.
//
// PERFORMANCE ISSUES:
// If you "cast away" the specific set variation you are using, and then do
// operations on the basic "Set" object you will pay a virtual function call
// to get back the specific set variation.  On the other hand, using the
// generic Set means you can change underlying implementations by just 
// changing the initial declaration.  Examples:
//      void foo(VectorSet vs1, VectorSet vs2) { vs1 |= vs2; }
// "foo" must be called with a VectorSet.  The vector set union operation
// is called directly.
//      void foo(Set vs1, Set vs2) { vs1 |= vs2; }
// "foo" may be called with *any* kind of sets; suppose it is called with
// VectorSets.  Two virtual function calls are used to figure out the that vs1
// and vs2 are VectorSets.  In addition, if vs2 is not a VectorSet then a 
// temporary VectorSet copy of vs2 will be before the union proceeds.
// 
// VectorSets have a small constant.  Time and space are proportional to the
//   largest element.  Fine for dense sets and largest element < 10,000.
// SparseSets have a medium constant.  Time is proportional to the number of
//   elements, space is proportional to the largest element.
//   Fine (but big) with the largest element < 100,000.
// ListSets have a big constant.  Time *and space* are proportional to the
//   number of elements.  They work well for a few elements of *any* size 
//   (i.e. sets of pointers)!

//------------------------------Set--------------------------------------------
class Set {
 public:

  // Creates a new, empty set.
  // DO NOT CONSTRUCT A Set.  THIS IS AN ABSTRACT CLASS, FOR INHERITENCE ONLY
  Set() {};

  // Creates a new set from an existing set 
  // DO NOT CONSTRUCT A Set.  THIS IS AN ABSTRACT CLASS, FOR INHERITENCE ONLY
  Set(const Set &s) {};

  // Set assignment; deep-copy guts
  virtual Set &operator =(const Set &s);
  virtual Set &clone(void) const;

  // Virtual destructor
  virtual ~Set() {};

  // Add member to set
  virtual Set &operator <<=(uint elem); 
  // virtual Set  operator << (uint elem);

  // Delete member from set
  virtual Set &operator >>=(uint elem); 
  // virtual Set  operator >> (uint elem);

  // Membership test.  Result is Zero (absent)/ Non-Zero (present)
  virtual int operator [](uint elem) const;

  // Intersect sets
  virtual Set &operator &=(const Set &s); 
  // virtual Set  operator & (const Set &s) const;

  // Union sets
  virtual Set &operator |=(const Set &s); 
  // virtual Set  operator | (const Set &s) const;

  // Difference sets
  virtual Set &operator -=(const Set &s);
  // virtual Set  operator - (const Set &s) const;

  // Tests for equality.  Result is Zero (false)/ Non-Zero (true)
  virtual int operator ==(const Set &s) const;
  int operator !=(const Set &s) const { return !(*this == s); }
  virtual int disjoint(const Set &s) const;

  // Tests for strict subset.  Result is Zero (false)/ Non-Zero (true)
  virtual int operator < (const Set &s) const;
  int operator > (const Set &s) const { return s < *this; }

  // Tests for subset.  Result is Zero (false)/ Non-Zero (true)
  virtual int operator <=(const Set &s) const;
  int operator >=(const Set &s) const { return s <= *this; }

  // Return any member of the Set.  Undefined if the Set is empty.
  virtual uint getelem(void) const;

  // Return the number of members in the Set
  virtual uint Size(void) const;

  // If an iterator follows a "Sort()" without any Set-modifying operations
  // inbetween then the iterator will visit the elements in ascending order.
  virtual void Sort(void);

  // Convert a set to printable string in an allocated buffer.  
  // The caller must deallocate the string.
  virtual char *setstr(void) const;

  // Print the Set on the ostream
  friend ostream & operator << (ostream &, const Set &);

  // Print the Set on "stdout".  Can be conveniently called in the debugger
  void print() const;

  // Parse text from the string into the Set.  Return length parsed.
  virtual int parse(const char *s);

  // Convert a generic Set to a specific Set
  virtual operator SparseSet* (void) const;
  virtual operator VectorSet* (void) const;
  virtual operator ListSet  * (void) const;
  virtual operator CoSet    * (void) const;

protected:
  friend class SetI;
  virtual SetI_ *iterate(uint&) const;
};
typedef Set&((*Set_Constructor)(void));

//------------------------------Iteration--------------------------------------
// Loop thru all elements of the set, setting "elem" to the element numbers
// in random order.  Inserted or deleted elements during this operation may
// or may not be iterated over; untouched elements will be affected once.

// Usage:  for( SetI  i(s); i.test(); i++ ) { body = i.elem; }   ...OR...
//         for( i.reset(s); i.test(); i++ ) { body = i.elem; }

class SetI_ { 
  private:
    friend class SetI; 

  public:
    SetI_() {};
    virtual ~SetI_() {};
    virtual uint next(void);
    virtual int test(void); 
};

class SetI {
protected:
  SetI_ *impl;
public:
  uint elem;                    // The publically accessible element

  SetI( const Set *s ) { impl = s->iterate(elem); }
  ~SetI() { delete impl; }
  void reset( const Set *s ) { delete impl; impl = s->iterate(elem); }
  void operator ++(void) { elem = impl->next(); }
  int test(void) { return impl->test(); }
};

#endif _SET_
