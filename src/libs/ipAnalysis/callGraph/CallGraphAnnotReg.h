/* $Id: CallGraphAnnotReg.h,v 1.5 1997/03/11 14:34:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef CallGraphAnnotReg_h 
#define CallGraphAnnotReg_h 

//****************************************************************************
// CallGraphAnnotReg.h:
//    use static constructors to register a manager class for each 
//    interprocedural annotation with the program analysis driver
//
// Author: John Mellor-Crummey                              September 1994
//
// Copyright 1994, Rice University. All rights reserved.
//****************************************************************************

#ifndef ClassInstanceRegistry_h
#include <libs/support/registry/ClassInstanceRegistry.h>
#endif

//--------------------------------------------------------------------------
// REGISTER_CG_ANNOT_MGR(name, className, registryPtr)
//      associate an annotation manager class for an annotation with a
//      registry associated with a CallGraph, CallGraphNode, or CallGraphEdge 
//
//      each entity's registry is used to support demand driven computation 
//      of annotations "by name"
//--------------------------------------------------------------------------
#define REGISTER_CG_ANNOT_MGR(name, className, registryPtr) \
REGISTER_STATIC_CLASS_INSTANCE(name, className, , registryPtr) 

#endif /* CallGraphAnnotReg_h */
