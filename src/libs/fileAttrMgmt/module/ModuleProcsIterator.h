/* $Id: ModuleProcsIterator.h,v 1.2 1997/03/27 20:32:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
// ModuleProcsIterator.h: 
//
//  iterator to enumerate the names of procedures defined by a 
//  Module
//
// Author: 
//   John Mellor-Crummey                              December 1993
//
// Copyright 1993, Rice University
//******************************************************************

#ifndef ModuleProcIterator_h
#define ModuleProcIterator_h

#ifndef StackableIterator_h
#include <libs/support/iterators/StackableIterator.h>
#endif
#include <libs/fileAttrMgmt/module/NeedProvModAttr.h>
#include <libs/frontEnd/include/NeedProvSet.h>

class AttributedFile;        // minimal external declaration
class Module;                // minimal external declaration


//***************************************************************************
// interface operations for class ModuleProcsIterator
//***************************************************************************


class ModuleProcsIteratorS {
public:
  NeedProvSetIterator npi;
  NeedProvModAttr *npAttr;
  ModuleProcsIteratorS(Module *module) :
  npAttr((NeedProvModAttr *) 
	 module->AttachAttribute(CLASS_NAME(NeedProvModAttr))),
  npi(npAttr->provs) {};
  ~ModuleProcsIteratorS() {
    npAttr->uplinkToFile->DetachAttribute(npAttr);
  }
};


class ModuleProcsIterator : public StackableIterator {
private:
  struct ModuleProcsIteratorS *hidden;
public:
    ModuleProcsIterator(Module *module);
   ~ModuleProcsIterator();

    void operator++();
    void Reset();
    const char *Current() const;
    void *CurrentUpCall() const;
};


#endif
