/* $Id: cNameValueTable.h,v 1.5 1999/06/11 21:07:19 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************************
 *                                                                            *
 * File:    C Name Value Table Utility                                        *
 * Author:  Kevin Cureton                                                     *
 * Date:    March 1993                                                        *
 *                                                                            *
 * This code provides a C interface to the C++ Hash Table.                    *
 *                                                                            *
 * Example code using these routines can be found in the file nvtTest.c.      *
 *                                                                            *
 ******************************* cNameValueTable ******************************
 *                                                                            *
 * NameValueTableAlloc:                                                       *
 *   Allocate and initialize a new hash table.  The table will expand         *
 *   automatically as needed.                                                 *
 *                                                                            *
 *   initialSlots - initial number of slots in the table.                     *
 *   EntryCompare - user supplied funct to compare entries.  Must return 0    *
 *                  if names are equivalent, a non-zero number otherwise.     *
 *                  A default function exists and will be used if NULL is     *
 *                  entered for the value.                                    *
 *   HashFunct    - hash funct to map name into [0, htSize).  A default       *
 *                  function exists and will be used if NULL is entered       *
 *                  for the value.                                            *
 *                                                                            *
 *   Returns a handle to the hash table.  This handle should be used in       *
 *   subsequent calls for the hash table.  If handle is negative              *
 *   then a hash table cannot be allocated.                                   *
 *                                                                            *
 * NameValueTableFree:                                                        *
 *   Deallocates all storage for the hash table.                              *
 *                                                                            *
 *   nvt - hash table id returned by NameValueTableAlloc.                     *
 *                                                                            *
 *   Returns nothing.                                                         *
 *                                                                            *
 * NameValueTableAddPair:                                                     *
 *   Associates an name with a value and places the name into the hash table. *
 *                                                                            *
 *   nvt      - hash table id returned by NameValueTableAlloc.                *
 *   name     - pointer to the name to be hashed.                             *
 *   value    - the value to be stored in the table.                          *
 *   oldValue - address of the location in which to store the old value.      *
 *                                                                            *
 *   Returns true if an old pair was found, false if otherwise.               *
 *                                                                            *
 * NameValueTableDeletePair:                                                  *
 *   Remove an association from the hash table and reclaim the storage used   *
 *   by the name.                                                             *
 *                                                                            *
 *   nvt          - hash table id returned by NameValueTableAlloc.            *
 *   name         - pointer to the name to be deleted.                        *
 *   nameInTable  - address of the location in which to store the deleted     *
 *                  name.                                                     *
 *   valueInTable - address of the location in which to store the deleted     *
 *                  value.                                                    *
 *                                                                            *
 *   Returns true if a pair to delete was found, false if otherwise.          *
 *                                                                            *
 * NameValueTableQueryPair:                                                   *
 *   Query to determine if an association exists in the table.                *
 *                                                                            *
 *   nvt   - hash table id returned by NameValueTableAlloc.                   *
 *   name  - pointer to the name to be queried.                               *
 *   value - address of the location in which to store the value.             *
 *                                                                            *
 *   Returns true if the entry was found, false if it was not.                *
 *                                                                            *
 * NameValueTableNumPairs:                                                    *
 *   Determine the number of entries in the hash table.                       *
 *                                                                            *
 *   nvt - hash table id returned by NameValueTableAlloc.                     *
 *                                                                            *
 *   Returns the number of entries in the hash table.                         *
 *                                                                            *
 * NameValueTableForAll:                                                      *
 * NameValueTableForAllV:                                                     *
 *   Execute a function on all entries in the hash table.                     *
 *                                                                            *
 *   nvt         - hash table id returned by NameValueTableAlloc.             *
 *   value       - the value to be stored int the table.                      *
 *   extraArg    - an argument to be passed to the ForAll funct.              *
 *   UserForAll  - pointer to the function to execute.                        *
 *   ...         - the arguments to be passed to the ForAllV funct when it    * 
 *                 is called.                                                 *      
 *                                                                            *
 *   Returns nothing.                                                         *
 *                                                                            *
 *   Example NameValueTableForAllCallbackV function using varargs.            *
 *                                                                            *
 *      void UserForAll (Generic name, Generic value, va_list argList)        *
 *      {                                                                     *
 *         while (STILL_MORE_ARGUMENTS_TO_GET)                                *
 *           {                                                                *
 *              theArgValue = va_arg (argList, argType);                      *
 *           }                                                                *
 *      }                                                                     *
 *                                                                            *
 ******************************************************************************
 *                                                                            *
 * NameValueTableIntNotEqual:                                                 *
 *   Simple comparison function useful for HashTables with integer keys.      *
 *                                                                            *
 *   intKey1, intKey2 - pair of integer keys to compare                       *
 *                                                                            *
 *   Returns 0 if the values are equal, non-zero if otherwise.                *
 *                                                                            *
 * NameValueTableIntHash:                                                     *
 *   Simple hash function useful for creating HashTables with integer keys.   *
 *                                                                            *
 *   intKey - integer value to hash.                                          *
 *   size   - the size of the HashTable.                                      *
 *                                                                            *
 *   Returns the hash value of intKey.                                        *
 *                                                                            *
 ******************************************************************************
 *                                                                            *
 *       Copyright 1993, Rice University, as part of the Rn/ParaScope         *
 *                    Programming Environment Project                         *
 *                                                                            *
 ******************************************************************************/

#ifndef cNameValueTable_h
#define cNameValueTable_h

/******************************* include files ********************************/

#include <sys/types.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef rn_varargs_h
#include <include/rn_varargs.h>
#endif

/**************************** variable definitions ****************************/

typedef Generic cNameValueTable;

/******************** cNameValueTable function prototypes *********************/

typedef FUNCTION_POINTER (int, NameCompareCallback, (Generic name1, Generic name2));

typedef FUNCTION_POINTER (unsigned int, NameHashFunctCallback, (Generic name, unsigned int size));

typedef FUNCTION_POINTER (void, NameValueTableForAllCallback, (Generic name, 
                                                               Generic value, 
                                                               Generic extraArg));

typedef FUNCTION_POINTER (void, NameValueTableForAllCallbackV, (Generic name, 
                                                                Generic value,
                                                                va_list argList));

/********************** cNameValueTable function externs **********************/

EXTERN (cNameValueTable, NameValueTableAlloc, (unsigned int initialSlots, 
                                               NameCompareCallback   UserNameCompare,
                                               NameHashFunctCallback UserHashFunct));

EXTERN (void, NameValueTableFree, (cNameValueTable nvt));

EXTERN (Boolean, NameValueTableAddPair, (cNameValueTable nvt, 
                                         Generic name,
                                         Generic value, 
                                         Generic* oldValue));

EXTERN (Boolean, NameValueTableDeletePair, (cNameValueTable nvt, 
                                            Generic name, 
                                            Generic* nameInTable, 
                                            Generic* valueInTable));

EXTERN (Boolean, NameValueTableQueryPair, (cNameValueTable nvt, 
                                           Generic name, 
                                           Generic* value));

EXTERN (unsigned int, NameValueTableNumPairs, (cNameValueTable nvt));


EXTERN (void, NameValueTableForAll, (cNameValueTable nvt, 
                                     NameValueTableForAllCallback UserForAll, 
                                     Generic extraArg));

EXTERN (void, NameValueTableForAllV, (cNameValueTable nvt, 
                                      NameValueTableForAllCallbackV UserForAll, ...));

EXTERN (int, NameValueTableIntCompare, (Generic intKey1, Generic intKey2));

EXTERN (unsigned int, NameValueTableIntHash, (Generic intKey, unsigned int size));

#endif
