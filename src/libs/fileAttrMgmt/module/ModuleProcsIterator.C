/* $Id: ModuleProcsIterator.C,v 1.1 1997/03/11 14:28:06 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
// ModuleProcsIterator.C: 
//
//  iterator to enumerate the names of procedures in a Module
//
// Author: 
//   John Mellor-Crummey                              December 1993
//
// Copyright 1993, Rice University
//******************************************************************

#include <stdlib.h>
#include <assert.h>

#include <libs/support/tables/HashTable.h>
#include <libs/support/tables/namedObject/NamedObject.h>
#include <libs/support/tables/namedObject/NamedObjectTable.h>

#include <libs/frontEnd/include/NeedProvSet.h>

#include <libs/fileAttrMgmt/module/NeedProvModAttr.h>
#include <libs/fileAttrMgmt/module/Module.h>
#include <libs/fileAttrMgmt/module/ModuleProcsIterator.h>


//***************************************************************************
// interface operations for class ModuleProcsIterator
//***************************************************************************


struct ModuleProcsIteratorS {
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


ModuleProcsIterator::ModuleProcsIterator(Module *module) 
{
  hidden = new ModuleProcsIteratorS(module);
}


ModuleProcsIterator::~ModuleProcsIterator()
{
  delete hidden;
}


const char *ModuleProcsIterator::Current() const
{
  NamedObject *no = hidden->npi.Current();
  return (no ? no->name : 0);
}

void *ModuleProcsIterator::CurrentUpCall() const
{
  return (void *) Current();
}


void ModuleProcsIterator::operator++() 
{
  hidden->npi++;
}


void ModuleProcsIterator::Reset() 
{
  hidden->npi.Reset();
}

