/* $Id: PointerMap.h,v 1.3 1997/03/11 14:37:36 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef PointerMap_h
#define PointerMap_h

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

#ifndef NameValueTable_h
#include <libs/support/tables/NameValueTable.h>
#endif

class PointerMap : private NameValueTable
{
  public:
    PointerMap();
   ~PointerMap();

    void  InsertMapping(void* oldPtr, void* newPtr);  // abort if collision
    void* Map(void* oldPtr);                          // abort if no mapping

  protected:
    virtual uint NameHashFunct (const Generic name, const uint size);
    virtual int  NameCompare (const Generic name1, const Generic name2);
};

#endif /* PointerMap_h */
