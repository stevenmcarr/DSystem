/* $Id: Sparse_Set.C,v 1.6 1997/03/11 14:37:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/**********************************************************************
 * Sparse set containing arbitrary unsigned integers (which might be,
 * for example, AST_INDEX's).
 */
/**********************************************************************
 * Revision History:
 * $Log: Sparse_Set.C,v $
 * Revision 1.6  1997/03/11 14:37:20  carr
 * newly checked in as revision 1.6
 *
Revision 1.6  94/01/05  14:21:24  johnmc
add HashFunct and EntryCompare to override incorrect
inherited defaults

Revision 1.4  93/09/28  18:01:32  curetonk
bug fixes for the iterators

Revision 1.3  93/09/28  12:30:13  curetonk
added reset function to iterators
changed iterator functionality so that they reset
automatically after encountering a null entry.

Revision 1.2  93/09/28  11:19:29  curetonk
replaced CCHashTable with HashTable.
minor changes to remove Solaris warnings.

Revision 1.1  93/06/22  16:53:45  reinhard
Initial revision

 */

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

#include <libs/support/sets/Sparse_Set.h>

STATIC(int, ElementCompare, (const Element* first, const Element* second));

/**********************************************************************
 * Sparse_Set() Constructor                                           *
 **********************************************************************/
Sparse_Set::Sparse_Set() : HashTable()
{
  HashTable::Create(sizeof(Element), 8);
}

/**********************************************************************
 * Sparse_Set() Destructor                                            *
 **********************************************************************/
Sparse_Set::~Sparse_Set()
{
  HashTable::Destroy();
}


uint Sparse_Set::HashFunct(const void* entry, const uint size)
{
  return *((uint *) entry) % size;
}

int Sparse_Set::EntryCompare(const void* e1, const void* e2)
{
  return *((uint *) e1) - *((uint *) e2);
}

/**********************************************************************
 * query_entry()  true iff <elmt> is in set
 **********************************************************************/
Boolean 
Sparse_Set::query_entry(Element elmt) const
{
  void* found = HashTable::QueryEntry((const void*)&elmt);

  return (Boolean)(found != 0);
}

/**********************************************************************
 * operator+=()  Add element to set.
 **********************************************************************/
Sparse_Set& 
Sparse_Set::operator+= (const Element& elmt)
{
  //3/4/93 RvH: casting const-ness away should be legal here ...
  HashTable::AddEntry((void*)&elmt);

  return *this;
}
  
/**********************************************************************
 * operator-=()  Remove element from set.
 **********************************************************************/
Sparse_Set& 
Sparse_Set::operator-= (const Element& elmt)
{
  //3/4/93 RvH: casting const-ness away should be legal here ...
  HashTable::DeleteEntry((void*)&elmt);

  return *this;
}


/**********************************************************************
 * operator+=   Add <set> to own set.
 *              Does NOT delete other set.
 **********************************************************************/
Sparse_Set& 
Sparse_Set::operator += (const Sparse_Set& set)
{
  Element elmt;

  for (Sparse_Set_Iter iter(set); elmt = iter(); *this += (elmt));

  return *this;
}


/**********************************************************************
 * count()  # of entries
 **********************************************************************/
unsigned int 
Sparse_Set::count() const
{
  return HashTable::NumberOfEntries();
}

  
/**********************************************************************
 * get_entry_index()  Get index of <elmt>.
 **********************************************************************/
int 
Sparse_Set::get_entry_index(Element elmt) const
{
  return HashTable::GetEntryIndex((const void*)&elmt);
}
  

/**********************************************************************
 * get_entry_by_index()   Get the <index>-th Element if available
 *                        0 otherwise.
 **********************************************************************/
Element 
Sparse_Set::get_entry_by_index(int index) const
{
  void* entry = HashTable::GetEntryByIndex((const unsigned int)index);
  Element elmt = entry ? (*(Element*)entry) : 0;

  return elmt;
}


/**********************************************************************
 * Sparse_Set_Iter  constructor.
 **********************************************************************/
Sparse_Set_Iter::Sparse_Set_Iter(const Sparse_Set& my_set)
  : HashTableIterator((const HashTable*)&my_set)
{
}

/**********************************************************************
 * Sparse_Set_Iter  reset.
 **********************************************************************/
void 
Sparse_Set_Iter::reset()
{
  HashTableIterator::Reset();
 
  return;
}

/**********************************************************************
 * Sparse_Set_Iter  stepping function.
 **********************************************************************/
Element 
Sparse_Set_Iter::operator() ()
{
  Element* currentElement;

  currentElement = (Element*)HashTableIterator::Current();

  if (currentElement == (Element*)NULL)
    {
      HashTableIterator::Reset();
      return (Element)NULL;
    }

  HashTableIterator::operator ++();

  return *currentElement;
}

/**********************************************************************
 * ElementCompare()  Sparse_Set_Sorted_Iter default comparison function.
 **********************************************************************/
static int 
ElementCompare(const Element* first, const Element* second)
{
  return (int)((int)*first - (int)*second);
}


/**********************************************************************
 * Sparse_Set_Sorted_Iter  constructor.
 **********************************************************************/
Sparse_Set_Sorted_Iter::Sparse_Set_Sorted_Iter (const Sparse_Set& my_set, 
    ElementCompareFunctPtr my_comp = ElementCompare)
  : HashTableSortedIterator ((const HashTable*)&my_set, 
                             (EntryCompareFunctPtr)my_comp )
{
}

/**********************************************************************
 * Sparse_Set_Sorted_Iter  reset.
 **********************************************************************/
void
Sparse_Set_Sorted_Iter::reset()
{
  HashTableSortedIterator::Reset();

  return;
}

/**********************************************************************
 * Sparse_Set_Sorted_Iter  stepping function.
 **********************************************************************/
Element 
Sparse_Set_Sorted_Iter::operator() ()
{
  Element* currentElement;

  currentElement = (Element*)HashTableSortedIterator::Current();

  if (currentElement == (Element*)NULL)
    {
      HashTableSortedIterator::Reset();
      return (Element)NULL;
    }

  HashTableSortedIterator::operator ++();

  return *currentElement;
}
