/* $Id: RegistryObject.h,v 1.1 1997/03/11 14:37:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// RegistryObject.h
//
// a base class for items stored in an instance of class 
// ClassInstanceRegistry.
//
// Author: John Mellor-Crummey                                June 1994
//
// Copyright 1994, Rice University
//***************************************************************************


#ifndef RegistryObject_h
#define RegistryObject_h

//--------------------------------------------------------------------
// class RegistryObject
//--------------------------------------------------------------------
class RegistryObject {
public:
  RegistryObject();
  virtual ~RegistryObject();
};


#endif
