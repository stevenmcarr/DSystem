/* $Id: NameValueTable.h,v 1.4 1997/03/11 14:37:35 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************************
 *                                                                            *
 * File:    C++ Name Value Table Utility                                      *
 * Author:  Kevin Cureton                                                     *
 * Date:    March 1993                                                        *
 *                                                                            *
 **************************** NameValueTable Public ***************************
 *                                                                            *
 * Constructor:                                                               *
 *                                                                            *
 *                                                                            *
 * Destructor:                                                                *
 *                                                                            *
 *                                                                            *
 * Create:                                                                    *
 *                                                                            *
 *                                                                            *
 * Destroy:                                                                   *
 *                                                                            *
 *                                                                            *
 * AddNameValue:                                                              *
 *                                                                            *
 *                                                                            *
 * DeleteNameValue:                                                           *
 *                                                                            *
 *                                                                            *
 * QueryNameValue:                                                            *
 *                                                                            *
 *                                                                            *
 * NumberOfNameValues:                                                        *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *************************** NameValueTable Protected *************************
 *                                                                            *
 * Create:                                                                    *
 *                                                                            *
 * NameHashFunct:                                                             *
 *                                                                            *
 * NameCompare:                                                               *
 *                                                                            *
 * NameValueCleanup:                                                          *
 *                                                                            *
 *                                                                            *
 *************************** NameValueTable Private ***************************
 *                                                                            *
 * HashFunct:                                                                 *
 *    entry                                                                   *
 *    size                                                                    *
 *                                                                            *
 * EntryCompare:                                                              *
 *    entry1                                                                  * 
 *    entry2                                                                  *
 *                                                                            *
 * EntryCleanup:                                                              *
 *    entry                                                                   *
 *                                                                            *
 * ChooseNameHashFunct:                                                       *
 *    entry                                                                   *
 *    size                                                                    *
 *                                                                            *
 * ChooseNameCompare:                                                         *
 *    entry1                                                                  *
 *    entry2                                                                  *
 *                                                                            *
 * ChooseNameValueCleanup:                                                    *
 *    entry                                                                   *
 *                                                                            *
 * NameHashFunctCallback:                                                     *
 *                                                                            *
 * NameCompareCallback:                                                       *
 *                                                                            *
 * NameValueCleanupCallback:                                                  *
 *                                                                            *
 *                                                                            *
 *************************** NameValueTableIterator ***************************
 *                                                                            *
 * const Generic name:                                                        *
 *                                                                            *
 * const Generic value:                                                       *
 *                                                                            *
 * NameValueTableIterator:                                                    *
 *    theNameValueTable                                                       *
 *                                                                            *
 * NameValueTableIterator ():                                                 *
 *                                                                            *
 * operator ++():                                                             *
 *                                                                            *
 * Reset ():                                                                  *
 *                                                                            *
 ******************************************************************************
 *                                                                            *
 *       Copyright 1993, Rice University, as part of the Rn/ParaScope         *
 *                    Programming Environment Project                         *
 *                                                                            *
 ******************************************************************************/

#ifndef NameValueTable_h
#define NameValueTable_h

/******************************* include files ********************************/

#include <sys/types.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef HashTable_h
#include <libs/support/tables/HashTable.h>
#endif

/******************** NameValueTable function prototypes **********************/

typedef FUNCTION_POINTER (uint, NameHashFunctFunctPtr, (const Generic, const uint));
           /* Generic name, uint size */

typedef FUNCTION_POINTER (int, NameCompareFunctPtr, (const Generic, const Generic));
           /* Generic name1, Generic name2 */

typedef FUNCTION_POINTER (void, NameValueCleanupFunctPtr, (Generic, Generic));
           /* Generic name, Generic value */

/********************* NameValueTable class definitions ***********************/

class NameValueTable : private HashTable
{
  public:
    NameValueTable ();
    virtual ~NameValueTable ();

        // Must be called after creating object
    void Create (uint initialSlots,
                         NameHashFunctFunctPtr    const NameHashFunctCallback,
                         NameCompareFunctPtr      const NameCompareCallback,
                         NameValueCleanupFunctPtr const NameValueCleanupCallback);

        // Must be called before deleting object
    void Destroy ();
   
    Boolean AddNameValue (Generic name, 
                          Generic newValue, Generic* oldValue);

    Boolean DeleteNameValue (Generic name, 
                             Generic* deletedName, Generic* deletedValue);

    Boolean QueryNameValue (const Generic name, Generic* theValue) const;

    uint    NumberOfNameValues () const;

    friend class NameValueTableIterator;

  protected:
        // Must be called after creating object
    void Create (uint initialSlots);

    virtual uint NameHashFunct (const Generic name, const uint size);
    virtual int  NameCompare (const Generic name1, const Generic name2);
    virtual void NameValueCleanup (Generic name, Generic value);

  private:
    virtual uint HashFunct (const void* entry, const uint size);
    virtual int  EntryCompare (const void* entry1, const void* entry2);
    virtual void EntryCleanup (void* entry);

    NameHashFunctFunctPtr    NameHashFunctCallback;
    NameCompareFunctPtr      NameCompareCallback;
    NameValueCleanupFunctPtr NameValueCleanupCallback;

    uint ChooseNameHashFunct (const void* entry, const uint size);
    int  ChooseNameCompare (const void* entry1, const void* entry2);
    void ChooseNameValueCleanup (void* entry);
};

/***************** NameValueTableIterator class definitions *******************/

class NameValueTableIterator : private HashTableIterator
{
  public:
    const Generic name;
    const Generic value;

    NameValueTableIterator (const NameValueTable* theNameValueTable);
   ~NameValueTableIterator ();

    void    operator ++();
    void    Reset ();
};

#endif
