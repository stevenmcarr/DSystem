/* $Id: ConstructorRegistry.h,v 1.1 1997/03/11 14:37:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ConstructorRegistry.h
//
// Author: John Mellor-Crummey                                June 1994
//
// Copyright 1994, Rice University
//***************************************************************************


#ifndef ConstructorRegistry_h
#define ConstructorRegistry_h

#ifndef RegistryObject_h
#include <libs/support/registry/RegistryObject.h>
#endif

#ifndef ClassInstanceRegistry_h
#include <libs/support/registry/ClassInstanceRegistry.h>
#endif


//****************************************************************************
// interface operations:
// macro  REGISTER_CONSTRUCTOR(name, className, registryPtr)
// macro  CONSTRUCT_BY_NAME_FROM_REGISTRY(registryPtr, name)
//****************************************************************************

//-----------------------------------------------------------------------
// macro REGISTER_CONSTRUCTOR adds an entry to the ClassInstanceRegistry
// given by "registryPtr" that enables an instance of "className" to be 
// constructed from "name" and "registryPtr" using the macro 
// CONSTRUCT_BY_NAME_FROM_REGISTRY.
//
// NOTE: class "className" must be derived from RegistryObject
//-----------------------------------------------------------------------
#define REGISTER_CONSTRUCTOR(name, className, registryPtr) \
class className ##  ConstructorRegistryEntry : \
public ConstructorRegistryEntry { \
public: \
className ##  ConstructorRegistryEntry(ConstructorRegistryEntryFunction cref)\
 : ConstructorRegistryEntry(cref) {};}; \
RegistryObject  * className ## Registry ## New () \
{ \
  return new className; \
} \
REGISTER_STATIC_CLASS_INSTANCE(name, className ## ConstructorRegistryEntry, (className ## Registry ## New), registryPtr)


//-----------------------------------------------------------------------
// macro CONSTRUCT_BY_NAME_FROM_REGISTRY returns an instance of the class
// identified by "name" using a ClassInstanceRegistry "registryPtr" that
// has previously been initialized by a call to REGISTER_CONSTRUCTOR
// for "name". the returned pointer is of type (RegistryObject *)
// which can be cast to the appropriate derived class.
//-----------------------------------------------------------------------
#define CONSTRUCT_BY_NAME_FROM_REGISTRY(registryPtr, name) \
((ConstructorRegistryEntry *) registryPtr->Lookup(name))->New()



//****************************************************************************
// supplemental definitions supporting macro interface operations
//****************************************************************************


typedef 
RegistryObject *(*ConstructorRegistryEntryFunction)();


//--------------------------------------------------------------------
// class ConstructorRegistryEntry
//--------------------------------------------------------------------
class ConstructorRegistryEntry : public RegistryObject {
private:
  ConstructorRegistryEntryFunction constructorFn;
public:
  ConstructorRegistryEntry(ConstructorRegistryEntryFunction cref);
  ~ConstructorRegistryEntry();
   RegistryObject *New();
};



#endif

