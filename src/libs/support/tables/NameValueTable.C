/* $Id: NameValueTable.C,v 1.3 1997/03/11 14:37:35 carr Exp $ */
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
 * See the include file NameValueTable.h for an explanation of the data and   *
 * functions in the NameValueTable class.                                     *
 *                                                                            *
 *       Copyright 1993, Rice University, as part of the Rn/ParaScope         *
 *                    Programming Environment Project                         *
 *                                                                            *
 ******************************************************************************/

/******************************* include files ********************************/

#include <stdio.h>

#include <libs/support/tables/NameValueTable.h>

/******************************* local defines ********************************/

static const int MY_TRUE  = 1;
static const int MY_FALSE = 0;

/**************************** variable definitions ****************************/

struct Pair
{
  Generic name;
  Generic value;
};

/***************** NameValueTable static function prototypes ******************/

static uint DefaultNameHashFunct (const Generic name, const uint size);
static int  DefaultNameCompare (const Generic name1, const Generic name2);
static void DefaultNameValueCleanup (Generic name, Generic value);

static void LocalAddPair (Pair* currentPair, Pair* newPair, va_list argList);
static void LocalDeletePair (Pair* pairToDelete, va_list argList);

/****************** NameValueTable public member functions ********************/

//
//
NameValueTable::NameValueTable () : HashTable ()
{
  NameHashFunctCallback    = (NameHashFunctFunctPtr)NULL;
  NameCompareCallback      = (NameCompareFunctPtr)NULL;
  NameValueCleanupCallback = (NameValueCleanupFunctPtr)NULL;

  return;
}

//
//
NameValueTable::~NameValueTable ()
{
  return;
}

//
//
void NameValueTable::Create (uint initialSlots,
                             NameHashFunctFunctPtr    const _NameHashFunctCallback,
                             NameCompareFunctPtr      const _NameCompareCallback,
                             NameValueCleanupFunctPtr const _NameValueCleanupCallback)
{
  if (_NameHashFunctCallback)    NameHashFunctCallback    = _NameHashFunctCallback;
  else                           NameHashFunctCallback    = DefaultNameHashFunct;

  if (_NameCompareCallback)      NameCompareCallback      = _NameCompareCallback;
  else                           NameCompareCallback      = DefaultNameCompare;

  if (_NameValueCleanupCallback) NameValueCleanupCallback = _NameValueCleanupCallback;
  else                           NameValueCleanupCallback = DefaultNameValueCleanup;

  HashTable::Create (sizeof(Pair), initialSlots);

  return;
}

//
//
void NameValueTable::Destroy ()
{
  HashTable::Destroy ();

  return;
}

//
//
Boolean NameValueTable::AddNameValue (Generic name, Generic newValue,
                                      Generic* oldValue)
{
  Pair newPair;
  Pair oldPair;
  uint foundPair = MY_FALSE;
   
  oldPair.name  = 0;
  oldPair.value = 0;

  newPair.name  = name;
  newPair.value = newValue;

  HashTable::AddEntry ((void *)&newPair, (AddEntryFunctPtr)LocalAddPair, 
                                           &foundPair, &oldPair);

  if (foundPair)
    {  
       if (oldValue) *oldValue = oldPair.value;

       return true;
    }
  else 
    {
       return false;
    }
}

//
//
Boolean NameValueTable::DeleteNameValue (Generic name, Generic* deletedName, 
                                         Generic* deletedValue)
{
  Pair thePair;
  Pair deletedPair;
  uint foundPair = MY_FALSE;

  thePair.name  = name;
  thePair.value = 0;

  deletedPair.name  = 0;
  deletedPair.value = 0;

  HashTable::DeleteEntry ((void*)&thePair, (DeleteEntryFunctPtr)LocalDeletePair,
                                            &foundPair, &deletedPair);

  if (foundPair)
    {
       if (deletedName)  *deletedName  = deletedPair.name;
       if (deletedValue) *deletedValue = deletedPair.value;
      
       return true;
    }
  else 
    {
       return false;
    }
}

//
//
Boolean NameValueTable::QueryNameValue (const Generic name, Generic* theValue) const
{
  Pair  thePair;
  Pair* queriedPair = NULL;

  thePair.name  = name;
  thePair.value = 0;

  queriedPair = (Pair*)HashTable::QueryEntry ((void*)&thePair);

  if (queriedPair) 
    {
       if (theValue) *theValue = queriedPair->value;

       return true;
    }
  else
    {
       return false;
    }
}

//
//
uint NameValueTable::NumberOfNameValues () const
{
  return HashTable::NumberOfEntries ();
}

/***************** NameValueTable protected member functions ******************/

//
//
void NameValueTable::Create (uint initialSlots)
{
  HashTable::Create (sizeof(Pair), initialSlots);

  return;
}

//
//
uint NameValueTable::NameHashFunct (const Generic name, const uint size)
{
  return DefaultNameHashFunct (name, size);
}

//
//
int NameValueTable::NameCompare (const Generic name1, const Generic name2)
{
  return DefaultNameCompare (name1, name2);
}

//
//
void NameValueTable::NameValueCleanup (Generic name, Generic value)
{
  DefaultNameValueCleanup (name, value);

  return;
}

/****************** NameValueTable private member functions *******************/

//
//
uint NameValueTable::HashFunct (const void* entry, const uint size)
{
  const Pair* thePair = (Pair*)entry;

  if (NameHashFunctCallback) return NameHashFunctCallback (thePair->name, size);
  else                       return NameHashFunct (thePair->name, size);
}

//
//
int NameValueTable::EntryCompare (const void* entry1, const void* entry2)
{
  const Pair* thePair1 = (Pair*)entry1;
  const Pair* thePair2 = (Pair*)entry2;

  if (NameCompareCallback) return NameCompareCallback (thePair1->name, thePair2->name);
  else                     return NameCompare (thePair1->name, thePair2->name);
}

//
//
void NameValueTable::EntryCleanup (void* entry)
{
  const Pair* thePair = (Pair*)entry;

  if (NameValueCleanupCallback) NameValueCleanupCallback (thePair->name, thePair->value);
  else                          NameValueCleanup (thePair->name, thePair->value);

  return;
}

/********************** NameValueTable static functions ***********************/

//
//
static uint DefaultNameHashFunct (const Generic name, const uint size)
{
# ifdef DEBUG
    fprintf (stdout, "\tNameValueTable::DefaultNameHashFunct\t%d\n", (uint)((int)name % size));
# endif

  return (uint)((int)name % size);
}

//
//
static int  DefaultNameCompare (const Generic name1, const Generic name2)
{
# ifdef DEBUG
    fprintf (stdout, "\tNameValueTable::DefaultNameCompare\t%d\n", ((int)name1 != (int)name2));
# endif

  return ((int)name1 != (int)name2);
}

//
//
static void DefaultNameValueCleanup (Generic name, Generic value)
{
# ifdef DEBUG
    fprintf (stdout, "\tNameValueTable::DefaultNameValueCleanup\t%d\t%d\n", 
             (int)name, (int)value);
# endif

  return;
}

//
//
static void LocalAddPair (Pair* currentPair, Pair* newPair, va_list argList)
{
  uint* foundPair = va_arg (argList, uint*);
  Pair* oldPair = va_arg (argList, Pair*);

  oldPair->name  = currentPair->name;
  oldPair->value = currentPair->value;

  currentPair->name  = newPair->name;
  currentPair->value = newPair->value;

  *foundPair = MY_TRUE;

  return;
}

//
//
static void LocalDeletePair (Pair* pairToDelete, va_list argList)
{
  uint* foundPair = va_arg (argList, uint*);
  Pair* deletedPair = va_arg (argList, Pair*);

  deletedPair->name  = pairToDelete->name;
  deletedPair->value = pairToDelete->value;

  *foundPair = MY_TRUE;

  return;
}

/*************** NameValueTableIterator public member functions ***************/

//
//
NameValueTableIterator::NameValueTableIterator (const NameValueTable* theNameValueTable) :
   HashTableIterator ((HashTable*)theNameValueTable), name(0), value(0)
{
  NameValueTableIterator::Reset ();

  return;
}

//
//
NameValueTableIterator::~NameValueTableIterator ()
{
  return;
}

//
//
void NameValueTableIterator::operator ++()
{
  Pair* thePair = (Pair*)NULL;
  Generic* namePtr  = (Generic*)&name;
  Generic* valuePtr = (Generic*)&value;

  HashTableIterator::operator ++();

  thePair = (Pair*)HashTableIterator::Current ();

  if (thePair)
    {
       *namePtr  = thePair->name;
       *valuePtr = thePair->value;
    }
  else 
    {
       *namePtr  = 0;
       *valuePtr = 0;
    }

  return;
}

//
//
void NameValueTableIterator::Reset ()
{
  Pair* thePair = (Pair*)NULL;
  Generic* namePtr  = (Generic*)&name;
  Generic* valuePtr = (Generic*)&value;

  HashTableIterator::Reset ();

  thePair = (Pair*)HashTableIterator::Current ();

  if (thePair)
    {
       *namePtr  = thePair->name;
       *valuePtr = thePair->value;
    }
  else 
    {
       *namePtr  = 0;
       *valuePtr = 0;
    }

  return;
}

