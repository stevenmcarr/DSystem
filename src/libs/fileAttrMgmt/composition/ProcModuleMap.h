/* $Id: ProcModuleMap.h,v 1.1 1997/03/11 14:27:51 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
// ProcModuleMap.h: 
//
//  maps the name of an entry point to its interface and a module
//  name. an instance of this map is used to record needs or provides 
//  information for a composition.
//
// Author: 
//   John Mellor-Crummey                              October 1993
//
// Copyright 1993, Rice University
//******************************************************************

#ifndef ProcModuleMap_h
#define ProcModuleMap_h

#include <libs/frontEnd/include/NeedProvSet.h>

class ProcModuleMapEntry : public ProcInterface {
public:
  const char *moduleName; 

  ProcModuleMapEntry();
  ProcModuleMapEntry(const char *moduleName, ProcInterface *pi);
  ~ProcModuleMapEntry();
  
  int NamedObjectReadUpCall(FormattedFile* port);
  int NamedObjectWriteUpCall(FormattedFile* port);
  // void Dump(void);
};


class ProcModuleMap : private NamedObjectTableIO {
public:
  
  ProcModuleMap();
  ~ProcModuleMap();

  NamedObjectIO *NewEntry();
  
  ProcModuleMapEntry *QueryEntry(const char *name);
  NamedObjectTableIO::AddEntry;

  int Read(FormattedFile *file);
  int Write(FormattedFile *file);

friend class ProcModuleMapIterator;
};



class ProcModuleMapIterator : private NamedObjectTableIterator {
public:
    ProcModuleMapIterator(ProcModuleMap *map);
   ~ProcModuleMapIterator();

    NamedObjectTableIterator::operator++;
    NamedObjectTableIterator::Reset;
    ProcModuleMapEntry* Current() const;
};

#endif

