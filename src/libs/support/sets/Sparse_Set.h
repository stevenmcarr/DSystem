/* $Id: Sparse_Set.h,v 1.7 1997/03/27 20:51:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef Sparse_Set_h
#define Sparse_Set_h

/**********************************************************************
 * Sparse set containing arbitrary unsigned integers (which might be,
 * for example, AST_INDEX's).
 *
 * This is NOT a bit set, which assumes elements to be inserted by their
 * (relatively small) number, but instead handles arbitrary integers
 * to be inserted.
 *
 * NOTE:
 * This set is not based on bit vectors, which grow linearly with
 * the value of the largest element inserted, but instead is based on
 * a sparser data structure (for now a hash table).
 * Consequently, we can expect set operations like union and intersectio
 * to be slower than for bit sets which contain the same # of elements.
 */
/**********************************************************************
 * Revision history:
 * $Log: Sparse_Set.h,v $
 * Revision 1.7  1997/03/27 20:51:49  carr
 * Alpha
 *
 * Revision 1.6  1997/03/11  14:37:20  carr
 * newly checked in as revision 1.6
 *
 * Revision 1.6  94/01/05  14:22:13  johnmc
 * >> add HashFunct and EntryCompare to override incorrect
 * >> inherited defaults
 * 
 * Revision 1.4  93/09/28  12:31:39  curetonk
 * added reset function to iterators
 * changed iterator functionality so that they reset
 * automatically after encountering a null entry.
 * 
 * Revision 1.3  93/09/28  11:21:18  curetonk
 *  replaced CCHashTable with HashTable.
 * minor changes to remove Solaris warnings.
 * 
 * Revision 1.2  93/06/23  10:29:54  reinhard
 * Updated comments.
 * 
 * Revision 1.1  93/06/22  16:35:34  reinhard
 * Initial revision
 * 
 */

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef HashTable_h
#include <libs/support/tables/HashTable.h>
#endif

/*------------------- TYPES ---------------------------------*/

//typedef unsigned int Element;
typedef Generic Element;

typedef FUNCTION_POINTER(int, ElementCompareFunctPtr, (const Element* first, 
                                                       const Element* second));


/*********************************************************************/
/*** Declaration of class Sparse_Set *********************************/
/*********************************************************************/
class Sparse_Set : public HashTable 
{
  public:
    Sparse_Set();
   ~Sparse_Set();
  
    Boolean      query_entry(Element elmt) const;  
    Sparse_Set&  operator+=(const Element& elmt);
    Sparse_Set&  operator-=(const Element& elmt);
    Sparse_Set&  operator+=(const Sparse_Set& other_set);
    unsigned int count() const;  
    int          get_entry_index(Element elmt) const;
    Element      get_entry_by_index(int index) const;

  private:
    virtual uint HashFunct(const void* entry, const uint size);
    virtual int EntryCompare(const void* e1, const void* e2);
    
    friend class Sparse_Set_Iter;
};


/**********************************************************************
 * An Iterator.  Usage:
 *
 *    Sparse_Set set;
 *    Element    elmt;
 *    ...
 *    for (Sparse_Set_Iter iter(set); elmt = iter();) { ... }
 */
class Sparse_Set_Iter : public HashTableIterator
{
  public:
    Sparse_Set_Iter(const Sparse_Set& my_set);
    void reset();
    Element operator() (); // Step
};


/**********************************************************************
 * A sorted Iterator.  Upon construction, the iterator creates a
 *                     sorted list of the elements in <set>, using
 *                     comparison function <comp>.
 *                     The step function retrieves the elements of the
 *                     sorted list one at a time.
 *                     Destruction deletes the sorted list.
 * Usage:
 *
 *    Sparse_Set set;
 *    Element    elmt;
 *    ...
 *    for (Sparse_Set_Sorted_Iter iter(set); elmt = iter();) { ... }
 */
class Sparse_Set_Sorted_Iter : public HashTableSortedIterator
{
  public:
    Sparse_Set_Sorted_Iter(const Sparse_Set& my_set, ElementCompareFunctPtr my_comp);
    void reset();
    Element operator() (); // Step
};

#endif
