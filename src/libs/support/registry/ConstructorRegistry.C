/* $Id: ConstructorRegistry.C,v 1.1 1997/03/11 14:37:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ConstructorRegistry.C
//
// Author: John Mellor-Crummey                                June 1994
//
// Copyright 1994, Rice University
//***************************************************************************

#include <stdlib.h>
#include <assert.h>

#include <libs/support/registry/ConstructorRegistry.h>


//***************************************************************************
// class ConstructorRegistryEntry implementation
//***************************************************************************

ConstructorRegistryEntry::ConstructorRegistryEntry
(ConstructorRegistryEntryFunction cref) 
{
  constructorFn = cref;
} 


ConstructorRegistryEntry::~ConstructorRegistryEntry()
{
} 


RegistryObject *ConstructorRegistryEntry::New()
{
  return constructorFn();
} 

