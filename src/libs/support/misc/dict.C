/* $Id: dict.C,v 1.10 1997/06/26 17:32:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
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

#include <assert.h>
#include <iostream.h>
#include <stdio.h>
#include <stream.h>

#include <libs/support/misc/dict.h>

//------------------------------data-----------------------------------------
#define MAXID 20
static char initflag = 0;	// True after 1st initialization
static char shft[MAXID] = {1,2,3,4,5,6,7,1,2,3,4,5,6,7,1,2,3,4,5,6};
static short xsum[MAXID];

//------------------------------Dict-----------------------------------------
// The dictionary is kept has a hash table.  The hash table is a even power
// of two, for nice modulo operations.	Each bucket in the hash table points
// to a linear list of key-value pairs; each key & value is just a (void *).
// The list starts with a count.  A hash lookup finds the list head, then a
// simple linear scan finds the key.  If the table gets too full, it's
// doubled in size; the total amount of EXTRA times all hash functions are
// computed for the doubling is no more than the current size - thus the
// doubling in size costs no more than a constant factor in speed.
Dict::Dict(CmpKey initcmp, Hash inithash, uint initsize)
: cmp(initcmp), hash(inithash)
{
  register int i;

// Precompute table of null character hashes
  if( !initflag ) {		// Not initializated yet?
    xsum[0] = (1<<shft[0])+1;	// Initialize
    for(i=1; i<MAXID; i++) {
      xsum[i] = (1<<shft[i])+1+xsum[i-1];
    }
    initflag = 1;		// Never again
  }

  if( initsize < 16 ) initsize = 16;	    // Correct for small sizes
  initsize--;				    // Correct for round up
  for( i=0; initsize; i++, initsize >>= 1); // i = lg(initsize)
  initsize = 1<<i;		// initsize = 2**i
  size = initsize;		// Size is next smallest power of 2
  cnt = 0;
  bin = (const void ***)calloc(size,sizeof(void **));
  assert(bin != NULL);			// Check for out-of-memory
}

Dict::~Dict()          // Delete hash table guts
{
  for( register uint i=0; i<size; i++ )
    if( bin[i] ) free( (char *)bin[i] );
  free( (char *)bin );
}



//------------------------------doubhash-------------------------------------
// Double hash table size.  If can't do so, just suffer.  If can, then run
// thru old hash table, moving things to new table.  Note that since hash
// table doubled, exactly 1 new bit is exposed in the mask - so everything
// in the old table ends up on 1 of two lists in the new table; a hi and a
// lo list depending on the value of the bit.
void Dict::doubhash(void)
{
  register const void ***nb;	// New hash bins
  register const void **l, **h; // Pointer into list of key-value pairs
  const void **ptr;
  register const void *kv;	// Key of a key-value pair
  register uint i,j,k;		// Index into hash table

  if( !size ) return;		// Can't double beyond 64K
  j = size;			// Fast size access
  k = j << 1;			// New mask value
  nb = (const void ***)malloc(k*sizeof(void **));
  assert(nb != NULL);			// Check for out-of-memory
  for( i=0; i < j; i++) {	// For complete OLD table do
    ptr = bin[i];		// Get list of key-value pairs
    nb[i] = ptr;		// New table uses 1/2 of same list
    if( ptr ) { 		// Any old list?
      int32 bcnt, lcnt, hcnt;
      bcnt = *(int32 *)ptr++;	// Get count of items
      // Allocate worst-case space
      nb[i+j] = (const void **)malloc((uint)((bcnt+bcnt+1)*sizeof(void *)));
      assert(nb[i+j] != NULL);		// Check for out-of-memory
      h = nb[i+j]+1;
      l = ptr;			// Start at head of list
      lcnt = hcnt = 0;		// Reset new counts
      while( bcnt-- ) { 	// Till copied em all
	kv = *ptr++;		// Get key of a key-value pair
	if( (hash( kv ) & (k-1)) == i ) {    // Hi or low list?
	  *l++ = kv;		// Place on low list
	  *l++ = *ptr++;	// Copy value
	  lcnt++;
	} else {
	  *h++ = kv;		// Place on high list
	  *h++ = *ptr++;	// Copy value also
	  hcnt++;
	}
      } 			// Repeat till did 'em all
      *(int32 *)(nb[i  ]) = lcnt;// Save new counts
      *(int32 *)(nb[i+j]) = hcnt;
      if( !lcnt ) {		// Empty low list?
	free( (char*)nb[i] );		// Then free low list
	nb[i] = NULL;
      }
      if( !hcnt ) {		// Empty high list?
	free( (char*)nb[i+j] );	// Then empty high list
	nb[i+j] = NULL;
      }
    } else nb[i+j] = NULL;	// Empty bucket in old list is empty in new
  }				// End of for complete OLD table do...
  free( (char*)bin );			// Toss old hash table
  bin = nb;			// Keep new
  size = k;			// Double size
}

//------------------------------Dict-----------------------------------------
// Deep copy a dictionary.
Dict::Dict( Dict &d ) : size(d.size), cnt(d.cnt), hash(d.hash), cmp(d.cmp)
{
  bin = (const void ***)calloc(size,sizeof(void **));
  assert(bin != NULL);			 // OOM check
  for( register uint i=0; i<size; i++ ) {
    const void **ptr = d.bin[i]; // Get list of key-value pairs
    if( ptr ) { 		 // Anything in old dictionary?
      int32 bcnt = *(int32 *)ptr;// Get count of key/value pairs
      int32 len = (bcnt+bcnt+1)*sizeof(void*); // Length in bytes
      bin[i] = (const void **)malloc((uint)len);
      assert(bin[i] != NULL);		 // OOM check
      bcopy((const char *)ptr, (char *)bin[i], (int)len);// Copy key-value pairs
    }
  }
}

//------------------------------Dict-----------------------------------------
// Deep copy a dictionary.
Dict &Dict::operator =( Dict &d )
{
  for( register uint i=0; i<size; i++ )
    if( bin[i] ) free( (char *)bin[i] );
  free( (char *)bin );
  size = d.size;		// Copy other dictionary parts
  cnt = d.cnt;
  *(Hash*)(&hash) = d.hash;
  *(CmpKey*)(&cmp) = d.cmp;
  bin = (const void ***)calloc(size,sizeof(void **));
  assert(bin != NULL);			 // OOM check
  for( i=0; i<size; i++ ) {
    const void **ptr = d.bin[i]; // Get list of key-value pairs
    if( ptr ) { 		 // Anything in old dictionary?
      int32 bcnt = *(int32 *)ptr;// Get count of key/value pairs
      int32 len = (bcnt+bcnt+1)*sizeof(void*); // Length in bytes
      bin[i] = (const void **)malloc((uint)len);
      assert(bin[i] != NULL);		 // OOM check
      bcopy((const char *)ptr, (char *)bin[i], (int)len);// Copy key-value pairs
    }
  }
  return *this;
}

//------------------------------Insert---------------------------------------
// Insert or replace a key/value pair in the given dictionary.	If the
// dictionary is too full, it's size is doubled.  The prior value being
// replaced is returned (NULL if this is a 1st insertion of that key).	If
// an old value is found, it's swapped with the prior key-value pair on the
// list.  This moves a commonly searched-for value towards the list head.
void *Dict::Insert(const void *key, const void *val)
{
  register const void **ptr, *kv; // Follow lists of key-value pairs
  register uint i;		// Index into hash table
  register int bcnt, j;

  i = hash( key ) & (size-1);	// Get hash key, corrected for size
  ptr = bin[i]; 		// Point to hash table
  if( !ptr ) {			// Bucket empty?
    ptr = bin[i] = (const void **)malloc(3*sizeof(void *));
    assert(ptr != NULL);		// Check for out-of-memory
    *ptr = NULL;		// Make a small empty bucket
  }
  j = bcnt = (int)(*(int32 *)ptr++);// Get size of bucket
  while( j-- ) {		// While not checked all of bucket
    kv = *ptr++;		// Get next key from bucket
    if( !cmp(kv,key) ) {	// Check for match
      if( bin[i]+2 != ptr ) {	// At head of list?
	ptr[-1] = ptr[-3];	// Put prior key in current
	ptr[-3] = kv;		// Put current key in prior
	kv = *ptr;		// Get old current value
	*ptr = ptr[-2]; 	// Put prior value over current
	ptr -= 2;		// Their now swapped; backup to new current
      } else {			// At head of list; just get value
	ptr[-1] = kv;		// Matched!  Store new key
	kv = *ptr;		// Matched!  Get old value
      } 			// End of if at head of list...
      *ptr = val;		// Save new value
      return (void *)kv;	// Return old value
    } else ptr++;		// Not matched, skip value and scan on
  }				// While not at list end do...
  bcnt++;			// Get size of bucket plus 1 pair
  bin[i] = (const void **)realloc( (char *) bin[i], (bcnt+bcnt+1)*sizeof(void *) );
  assert( bin[i] != NULL );		// Check for out-of-memory
  ptr = bin[i]; 		// Point to head of list
  *(int32 *)ptr++ = bcnt;	// Raise count in bucket
  ptr += (int)(bcnt+bcnt-2);	// Point to end of list
  *ptr++ = key; 		// Add key/value to list end
  *ptr++ = val;
  cnt++;			// Raise count of things in dictionary
  if( cnt > size )		// Table too full?
    doubhash(); 		// Double hash table size
  return NULL;			// Nothing found prior
}

//------------------------------Delete---------------------------------------
// Find & remove a value from dictionary. Return old value.
void *Dict::Delete(const void *key)
{
  register const void **ptr, *kv; // Follow lists of key-value pairs
  register uint i;		// Index into hash table
  register int bcnt, j;

  i = hash( key ) & (size-1);	// Get hash key, corrected for size
  ptr = bin[i]; 		// Point to hash table
  if( !ptr ) return NULL;	// Bucket empty?  Nothing to delete
  j = bcnt = (int)(*(int32 *)ptr++);// Get size of bucket
  while( j-- ) {		// While not checked all of bucket
    kv = *ptr++;		// Get next key from bucket
    if( !cmp(kv,key) ) {	// Check for match
      cnt--;			// Lower count in hash table
      kv = *ptr;		// Get old value
      ptr[-1] = ptr[j+j-1];	// Get LAST key
      *ptr = ptr[j+j];		// Get LAST value
      *(int32 *)bin[i] = --bcnt;// Lower count in bucket
      if( !bcnt ) {		// Killed last bucket element?
	free( (char*)bin[i] ); 	// Free empty bucket
	bin[i] = NULL;		// Mark as free
      }
      return (void *)kv;	// Return old value
    } else ptr++;		// Not matched, skip value and scan on
  }				// While not at list end do...
  return NULL;			// Deleting non-existant key
}

//------------------------------FindDict-------------------------------------
// Find a key-value pair in the given dictionary.  If not found, return NULL.
// If found, move key-value pair towards head of list.
void *Dict::operator [](const void *key) const
{
  register const void **ptr, *kv;// Pointer to list of key-value pairs
  register uint i;		// Index into hash table
  int32 bcnt;

  i = ((const Hash)hash)( key ) & (size-1);
  ptr = bin[i]; 		// Point to hash table
  if( !ptr ) return NULL;	// Empty bucket, no match
  bcnt = *(int32 *)ptr++;	// Get size of bucket
  while( bcnt-- ) {		// While not at list end do...
    kv = *ptr++;		// Get next key from bucket
    if( !((const CmpKey)cmp)(kv,key) ) { // Check for match
      if( bin[i]+2 != ptr ) {	// At head of list?
	ptr[-1] = ptr[-3];	// Put prior key in current
	ptr[-3] = kv;		// Put current key in prior
	kv = *ptr;		// Get the current value
	*ptr = ptr[-2]; 	// Put prior value over current
	ptr[-2] = kv;		// Put current value over prior value
      } else			// At head of list; just get value
	kv = *ptr;		// Matched!  Get value
      return (void *)kv;	// Return value found
    } else ptr++;		// Not matched, skip value and scan on
  }				// While not at list end do...
  return NULL;			// Save me from lint
}

//------------------------------CmpDict--------------------------------------
// CmpDict compares two dictionaries; they must have the same keys (their
// keys must match using CmpKey) and they must have the same values (pointer
// comparison).  If so 1 is returned, if not 0 is returned.
int32 Dict::operator ==(const Dict &d2) const
{
  register const void **ptr, *kv;// Follow lists of key-value pairs
  register uint i;

  if( cnt != d2.cnt ) return 0;
  for( i=0; i < size; i++) {	// For complete hash table do
    ptr = bin[i];		// Get linear list of key-value pairs
    if( !ptr ) continue;	// Nothing to do
    int32 bcnt = *(int32 *)ptr++;  // Get size of bucket
    while( bcnt-- ) {		// Check for end
      kv = *ptr++;		// Get next key
      if( d2[kv] != *ptr++ )	// Check that values match
	return 0;		// No match, bounce out
    }
  }
  return 1;			// All match, is OK
}

//------------------------------DictI----------------------------------------
// Create an iterator and initialize the first variables.
DictI::DictI( Dict *dict ) : d(dict)
{
  reset();
}

void DictI::reset()
{
  for( i=0; i<d->size; i++ ) {
    p = d->bin[i];
    if( !p ) continue;
    bcnt = (int)(*(int32 *)p++);
    while( bcnt-- ) {
      k = *p++;
      v = *p++;
      return;
    }
  }
  k = v = NULL;
}

//------------------------------next-----------------------------------------
// Find the next key-value pair in the dictionary, or return a NULL key and
// value.
void DictI::operator ++(void)
{
  while( bcnt-- ) {
    k = *p++;
    v = *p++;
    return;
  }
  while( ++i < d->size ) {
    p = d->bin[i];
    if( !p ) continue;
    bcnt = (int)(*(int32 *)p++);
    if( !bcnt-- ) continue;
    k = *p++;
    v = *p++;
    return;
  }
  k = v = NULL;
}

//------------------------------ForDict--------------------------------------
// Perform function FuncDict on every key-value pair in Dict *d.  Deletion
// works, but insertion is weird.
void Dict::For(FuncDict fd)
{
  register const void **ptr;	// Follow lists of key-value pairs
  register uint i;

  for( i=0; i < size; i++) {	// For complete hash table do
    ptr = bin[i];		// Get linear list of key-value pairs
    if( !ptr ) continue;	// Ignore empty bin
    int bcnt = ((int)*(int32 *)ptr++); // Get size of bucket
    while( bcnt-- ) {		// While have a list do
      ptr = bin[i];		// Get linear list of key-value pairs
      if( !ptr ) break; 	// Ignore empty bin
      fd(ptr[bcnt+bcnt+1],ptr[bcnt+bcnt+2],this);
    }
  }
}

//------------------------------print----------------------------------------
// Print out the dictionary as key-value pairs.
ostream & operator << (ostream &os, Dict &d)
{
  os << "Dict@" << (void*)(&d) << "[" << d.cnt << "] = {";
  forDict(&d) {
    os << "(" << (void*)k << ',' << (void*)v << "),";
  } endDict;
  os << "}\n";
  return os;
}

//------------------------------print------------------------------------------
// Handier print routine
void Dict::print()
{
  cout << (*this);
}

//------------------------------Hashing Functions----------------------------
// Convert string to hash key.	This algorithm implements a universal hash
// function with the multipliers frozen (ok, so it's not universal).  The
// multipliers (and allowable characters) are all odd, so the resultant sum
// is odd - guarenteed not divisible by any power of two, so the hash tables
// can be any power of two with good results.  Also, I choose multipliers
// that have only 2 bits set (the low is always set to be odd) so
// multiplication requires only shifts and adds.  Characters are required to
// be in the range 0-127 (I double & add 1 to force oddness).  Keys are
// limited to MAXID characters in length.  Experimental evidence on 150K of
// C text shows excellent spreading of values for any size hash table.
Generic hashstr(const void *t)
{
  register char c, k = 0;
  register int32 sum = 0;
  register const char *s = (const char *)t;

  while( (c = *s++) != '\0' ) { // Get characters till nul
    c = (c<<1)+1;		// Characters are always odd!
    sum += c + (c<<shft[k++]);	// Universal hash function
  }
  return (Generic)((sum+xsum[k]) >> 1); // Hash key, un-modulo'd table size
}

//------------------------------hashptr--------------------------------------
// Slimey cheap hash function; no guarenteed performance.  Better than the
// default for pointers, especially on MS-DOS machines.
Generic hashptr(const void *key)
{
#ifdef __TURBOC__
    return (Generic)((const int32)key >> 16);
#else  __TURBOC__
    return (Generic)((const int32)key >> 2);
#endif
}

// Slimey cheap hash function; no guarenteed performance.
Generic hashkey(const void *key)
{
  return (Generic)key;
}

//------------------------------Key Comparator Functions---------------------
int32 cmpstr(const void *k1, const void *k2)
{
  return strcmp((const char *)k1,(const char *)k2);
}

// Slimey cheap key comparator.
int32 cmpkey(const void *key1, const void *key2)
{
  return (const int32)key1 - (const int32)key2;
}

