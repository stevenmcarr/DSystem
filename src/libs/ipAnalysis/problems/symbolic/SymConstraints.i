/* $Id: SymConstraints.i,v 1.2 1997/03/11 14:35:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef SymConstraints_i
#define SymConstraints_i

#include <libs/support/tables/HashTable.h>

#include <libs/ipAnalysis/problems/symbolic/SymConstraints.h>

//  Structure for elements of the table associating base values and variables
//
class BaseMapEntry {
  public:
    ValNumber base;
    int index;
    int coeff;
    int addend;

    BaseMapEntry(ValNumber b, int i, int c, int a)
      : base(b), index(i), coeff(c), addend(a) {;};

    ~BaseMapEntry() {;};

    unsigned int hash(unsigned int size)
    {
        return base % size;
    };

    int compare(BaseMapEntry *e)
    {
        return (this->base != e->base);
        // if different, any non-zero value suffices
    };

    void BaseMapEntry::Dump(void);
};

//  Map from base values (no constant coefficient or addend) to indices
//  of variables which are linearly derived therefrom.
//
class BaseMap : private HashTable 
{
  public:
    BaseMap() : HashTable () 
    {
      HashTable::Create (sizeof(BaseMapEntry), 8);
    };

      // add to the table
    void add_entry(ValNumber b, int i, int c, int a)
    {
        BaseMapEntry e(b, i, c, a);
        HashTable::AddEntry(&e);
    };

      //  query entry equivalent
    BaseMapEntry *operator[](ValNumber b)     // get entry for given base
    {
        BaseMapEntry e(b, UNUSED, 0, 0);
        BaseMapEntry *found = (BaseMapEntry *) HashTable::QueryEntry(&e);
	return found;
    }

  private:
    virtual unsigned int HashFunct(const void *entry, const unsigned int size)
    {
        return ((BaseMapEntry *)entry)->hash(size);
    }

    virtual int EntryCompare(const void *e1, const void *e2)
    {
        return ((BaseMapEntry *)e1)->compare((BaseMapEntry *)e2);
    }

    virtual void EntryCleanup(void *entry)
    {
      return;
    };
};

#endif
