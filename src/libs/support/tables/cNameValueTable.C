/* $Id: cNameValueTable.C,v 1.5 1997/03/11 14:37:38 carr Exp $ */
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
 * See the include file cNameValueTable.h for an explanation of the data and  *
 * functions in the cNameValueTable.                                          *
 *                                                                            *
 *       Copyright 1993, Rice University, as part of the Rn/ParaScope         *
 *                    Programming Environment Project                         *
 *                                                                            *
 ******************************************************************************/

/******************************* include files ********************************/

#include <stdarg.h>

#include <libs/support/tables/NameValueTable.h>
#include <libs/support/tables/cNameValueTable.h>

/******************************* local defines ********************************/


/**************************** variable definitions ****************************/

struct Pair
   {
   Generic name;
   Generic value;
   };

/********************** cNameValueTable extern functions **********************/

//
//
cNameValueTable NameValueTableAlloc (uint initialSlots, 
                                     NameCompareCallback   UserNameCompare,
                                     NameHashFunctCallback UserNameHashFunct)
{
   NameValueTable* nvt;

   nvt = new NameValueTable();
   
   nvt->Create (initialSlots, 
                (NameHashFunctFunctPtr)UserNameHashFunct, 
                (NameCompareFunctPtr)UserNameCompare, 
                (NameValueCleanupFunctPtr)0);

   return (cNameValueTable)nvt;
}

//
//
void NameValueTableFree (cNameValueTable nvt)
{
   ((NameValueTable*)nvt)->Destroy ();

   delete (NameValueTable*)nvt;

   return;
}

//
//
Boolean NameValueTableAddPair (cNameValueTable nvt, 
                               Generic name, Generic value, Generic* oldValue)
{
   return ((NameValueTable*)nvt)->AddNameValue (name, value, oldValue); 
}

//
//
Boolean NameValueTableDeletePair (cNameValueTable nvt, Generic name, 
                                  Generic* nameInTable, Generic* valueInTable)
{
   return ((NameValueTable*)nvt)->DeleteNameValue (name, nameInTable, valueInTable); 
}

//
//
Boolean NameValueTableQueryPair (cNameValueTable nvt, Generic name, Generic* value)
{
   return ((NameValueTable*)nvt)->QueryNameValue (name, value);
}

//
//
uint NameValueTableNumPairs (cNameValueTable nvt)
{
   return ((NameValueTable*)nvt)->NumberOfNameValues();
}

//
//
void NameValueTableForAll (cNameValueTable nvt, 
                           NameValueTableForAllCallback UserForAll, Generic extraArg)
{
   Generic currentName;
   Generic currentValue;
   NameValueTableIterator anIterator ((NameValueTable*)nvt);

   anIterator.Reset();

   for (int i = 0; i < NameValueTableNumPairs(nvt); i++, anIterator++)
      {
      currentName  = anIterator.name;
      currentValue = anIterator.value;
      UserForAll (currentName, currentValue, extraArg);
      }

   return;
}

//
//
void NameValueTableForAllV (cNameValueTable nvt, 
                            NameValueTableForAllCallbackV UserForAll, ...)
{
   va_list argList;
   Generic currentName;
   Generic currentValue;
   NameValueTableIterator anIterator ((NameValueTable*)nvt);

   anIterator.Reset();

   for (int i = 0; i < NameValueTableNumPairs(nvt); i++, anIterator++)
      {
      va_start (argList, UserForAll);
      currentName  = anIterator.name;
      currentValue = anIterator.value;
      UserForAll (currentName, currentValue, argList);
      va_end (argList);
      }

   return;
}

//
//
int NameValueTableIntCompare (Generic intKey1, Generic intKey2)
{
   return (int)((int)intKey1 != (int)intKey2);
}

//
//
uint NameValueTableIntHash (Generic intKey, uint size)
{
   return (uint)((int)intKey % size);
}

