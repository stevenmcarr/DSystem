/* $Id: ClassInstanceRegistry.C,v 1.1 1997/03/11 14:37:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ClassInstanceRegistry.C
//
// Author: John Mellor-Crummey                                June 1994
//
// Copyright 1994, Rice University
//***************************************************************************

#include <libs/support/registry/ClassInstanceRegistry.h>
#include <libs/support/tables/namedObject/NamedObject.h>
#include <libs/support/registry/RegistryObject.h>

//***************************************************************************
// class NamedRegistryObject implementation
//***************************************************************************

class NamedRegistryObject : public NamedObject {
public:
  RegistryObject *ro;
  NamedRegistryObject(const char *const _name, RegistryObject *_ro) : 
  NamedObject(_name), ro(_ro) {};
  ~NamedRegistryObject() { delete ro; };
};


//***************************************************************************
// class DoClassInstanceRegister implementation
//***************************************************************************

DoClassInstanceRegister::DoClassInstanceRegister
(const char *const name, RegistryObject *ro, ClassInstanceRegistry **registry)
{
  if (!*registry) *registry = new ClassInstanceRegistry;
  (*registry)->Register(name, ro);
} 


DoClassInstanceRegister::~DoClassInstanceRegister()
{
} 


//***************************************************************************
// class ClassInstanceRegistry implementation
//***************************************************************************

ClassInstanceRegistry::ClassInstanceRegistry()
{
}


ClassInstanceRegistry::~ClassInstanceRegistry()
{
  Destroy();
}


void ClassInstanceRegistry::Register(const char *const name, 
				     RegistryObject *ro)
{
  if (!NamedObjectTable::QueryEntry(name)) {
    NamedObject *no = new NamedRegistryObject(name, ro);
    NamedObjectTable::AddEntry(no);
  }
}


RegistryObject *ClassInstanceRegistry::Lookup(const char *const name)
{
  NamedRegistryObject *nro = 
    (NamedRegistryObject *) NamedObjectTable::QueryEntry(name);
  return (nro ? nro->ro : 0);
}
