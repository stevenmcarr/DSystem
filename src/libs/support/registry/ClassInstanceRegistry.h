/* $Id: ClassInstanceRegistry.h,v 1.1 1997/03/11 14:37:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ClassInstanceRegistry.h
//
// a class for storing items of type RegistryObject so that they can be
// looked up by name. 
//
// Author: John Mellor-Crummey                                June 1994
//
// Copyright 1994, Rice University
//***************************************************************************


#ifndef ClassInstanceRegistry_h
#define ClassInstanceRegistry_h

#ifndef NamedObjectTable_h
#include <libs/support/tables/namedObject/NamedObjectTable.h>
#endif

class RegistryObject; // minimal external declaration


//--------------------------------------------------------------------
// macro REGISTER_STATIC_CLASS_INSTANCE registers an instance of 
// class "className" created with constructor arguments "classDeclArgs" 
// by the name "name" in ClassInstanceRegistry pointed to by 
// "registryPtr"
//--------------------------------------------------------------------
#define REGISTER_STATIC_CLASS_INSTANCE(name, className, classDeclArgs, registryPtr) \
static DoClassInstanceRegister \
className ## DoClassInstanceRegister(name, new className classDeclArgs, \
(ClassInstanceRegistry **) &registryPtr)


//--------------------------------------------------------------------
// macro LOOKUP_STATIC_CLASS_INSTANCE retrieves an instance of 
// class "className" indexed by the name "name" in the registry
// pointed to by "registryPtr"
//--------------------------------------------------------------------
#define LOOKUP_STATIC_CLASS_INSTANCE(name, className, registryPtr) \
(className *) registryPtr->Lookup(name)


//--------------------------------------------------------------------
// class ClassInstanceRegistry
//--------------------------------------------------------------------
class ClassInstanceRegistry: private NamedObjectTable {
public:
  ClassInstanceRegistry();
  ~ClassInstanceRegistry();

  void Register(const char *const name, RegistryObject *ro);
  RegistryObject *Lookup(const char *const name);
};


//--------------------------------------------------------------------
// class DoClassInstanceRegister
//--------------------------------------------------------------------
class DoClassInstanceRegister {
public:
  DoClassInstanceRegister(const char *const name, RegistryObject *ro, 
			  ClassInstanceRegistry **registry);
  ~DoClassInstanceRegister();
};


#endif

