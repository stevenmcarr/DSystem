/* $Id: PointerMap.C,v 1.4 1997/03/11 14:37:36 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***********************************************************************//
//    PointerMap.h:                                                      //
//                                                                       //
//      unidirectional pointer translation map: (void*) --> (void*)      //
//                                                                       //
//    Author:                                                            //
//      John Mellor-Crummey                               May 1993       //
//                                                                       //
//    Copyright 1993, Rice University                                    //
//                                                                       //
//***********************************************************************//

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <libs/support/tables/PointerMap.h>

PointerMap::PointerMap() : NameValueTable()
{
  NameValueTable::Create(8);
} 

PointerMap::~PointerMap()
{
  NameValueTable::Destroy();
}

void PointerMap::InsertMapping(void* oldPtr, void* newPtr)
{
  Boolean collision;
  Generic dummy;

  collision = NameValueTable::AddNameValue((Generic)oldPtr, (Generic)newPtr, &dummy);

     // fail if mapping exists
  assert(collision == false);

  return;
}

void* PointerMap::Map(void* oldPtr)
{
  Boolean success; 
  Generic newPtr;

  success = NameValueTable::QueryNameValue((const Generic)oldPtr, &newPtr);

     // fail if no mapping found
  assert(success == true);

  return (void*)newPtr;
}

uint PointerMap::NameHashFunct(const Generic name, const uint size)
{
  return (uint)((int)name % size);
}

int  PointerMap::NameCompare(const Generic name1, const Generic name2)
{
  return (int)((int)name1 != (int)name2);
}
