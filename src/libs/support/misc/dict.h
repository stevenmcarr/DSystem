/* $Id: dict.h,v 1.8 1997/03/11 14:36:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef _DICT_
#define _DICT_

/*
 * Dictionaries - An Abstract Data Type
 *
 * Author: Cliff Click, Copyright (C) 1991, 1992
 * This material is released for public use, but is not placed in the public
 * domain.  You are free to use/mangle/copy this code, but since I retain
 * ownership you cannot copyright or patent this code.	Please leave this
 * copyright notice in copies of this code, so that future programmers cannot
 * be denied using it.
 */

#ifndef string_h
#include <string.h>
#endif	string_h
#ifndef _PORT_
#include <libs/support/misc/port.h>
#endif _PORT_
#ifndef _MEMORY_
#include <libs/support/memMgmt/libmem.h>
#endif	_MEMORY_
class ostream;

// These dictionaries define a key-value mapping.  They can be inserted to,
// searched or deleted from.  They grow and shrink as needed.  The key is a
// pointer to something (or anything which can be stored in a pointer).  A
// key comparison routine determines if two keys are equal or not.  A hash
// function can be provided; if it's not provided the key itself is used
// instead.  A nice string hash function is included.
class Dict;
typedef int32 (*CmpKey)(const void *key1, const void *key2);
typedef int  (*Hash)(const void *key);
typedef void (*FuncDict)(const void *key, const void *val, Dict *d);

// Hashing functions
int hashstr(const void *s);	   // Nice string hash
// Slimey cheap hash function; no guarenteed performance.  Better than the
// default for pointers, especially on MS-DOS machines.
int hashptr(const void *key);
// Slimey cheap hash function; no guarenteed performance.
int hashkey(const void *key);

// Key comparators
int32 cmpstr(const void *k1, const void *k2);
// Slimey cheap key comparator.
int32 cmpkey(const void *key1, const void *key2);

class Dict {			// Dictionary structure
 private:
  const void ***bin;		// Hash table is array of ptrs to void *'s
  uint size;			// Size (# of slots) in hash table
  uint32 cnt;			// Number of key-value pairs in hash table
  const Hash hash;		// Hashing function
  const CmpKey cmp;		// Key comparison function
  void doubhash( void );	// Double hash table size

 public:
  friend class DictI;		 // Friendly iterator function

// Size is a hint to the starting size (number of key-value) pairs of the
// dictionary; 0 works fine.  cmp is a key comparision routine; the default
// just compares the key pointers themselves for equality.  Hash is a routine
// to hash a key; the default hash routine uses the key itself; a nice
// string hasher is provided for text keys.
// A sample call for just using the key pointers: Dict();
// A sample call for using text keys: Dict(cmpstr,hashstr);
  Dict(CmpKey cmp = cmpkey, Hash hash = hashkey, uint size = 0);
  Dict( Dict & );		// Deep-copy guts

  ~Dict();     			// Delete hash table guts

  Dict &operator =( Dict & );

// Return # of key-value pairs in dict
  uint32 Size(void) const { return cnt; }

// Insert inserts the given key-value pair into the dictionary.  The prior
// value of the key is returned; NULL if the key was not previously defined.
  void *Insert(const void *key, const void *val); // A new key-value
  void *Delete(const void *key);		  // Delete & return old

// Find finds the value of a given key; or NULL if not found.  The
// dictionary is NOT changed.
  void *operator [](const void *key) const;  // Do a lookup

// == compares two dictionaries; they must have the same keys (their keys
// must match using CmpKey) and they must have the same values (pointer
// comparison).  If so 1 is returned, if not 0 is returned.
  int32 operator ==(const Dict &d) const;   // Compare dictionaries for equal

// For passes each key-value pair to the given function, along with the
// dictionary.	The function can do most anything, INCLUDING calling Insert!
// A key is only visited once per call to For, and any NEW keys inserted
// during processing MAY or MAY NOT be given to the function.  Specifically
// changing or deleting a key-value pair is OK, inserting a new key may not
// behave as desired.  Also finding a key may cause some other key to be
// skipped or iterated twice; this is an undetected error.
  void For(FuncDict fd);	      // Iterate function over dictionary

// Print out the dictionary contents as key-value pairs
  friend ostream & operator << (ostream &os, Dict &d);
  void print();

  void **_get_bin(uint i) { return (void **)bin[i]; }
  uint _get_size() { return size; }
};

// The class of dictionary iterators.  Fails in the presences of modifica-
// tions to the dictionary during iteration (including searches).  Due to
// it's nature it can't handle NULL keys (these are used to end the loop).
// Usage:  for( DictI i(dict); i.k; i++ ) { body = i.k; body = i.v; }
class DictI {
 private:
  const Dict *d;
  uint i;
  const void **p;
  int bcnt;
 public:
  const void *k, *v;
  DictI( Dict *dict );
  void operator ++(void);
  void reset();
};

// forDict is given a pointer to a dictionary.	It declares a local variable
// `k' to hold the key pointer, and local `v' to value pointer.  This macro
// loops thru the following statement with the key and value variables set
// to all the key-value pairs in the dictionary.  This macro is NOT robust
// in the face of insertions or deletions during the looping; such behavior
// will NOT work, and is an (undetected) error.
#define forDict(d) { \
  register uint _i;  Dict *D = d;\
  for(_i=0; _i < D->_get_size(); _i++) { \
    void ** _p = D->_get_bin(_i); \
    if( !_p ) continue; \
    register int _bcnt = (int)(*(int32 *)_p++); \
    while( _bcnt-- ) { void *k = *_p++; void *v = *_p++; k=k; v=v;

#define endDict }}}

#endif _DICT_
