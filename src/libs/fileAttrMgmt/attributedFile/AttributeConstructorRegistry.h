/* $Id: AttributeConstructorRegistry.h,v 1.1 1997/03/11 14:27:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef AttributeConstructorRegistry_h
#define AttributeConstructorRegistry_h

#ifndef ConstructorRegistry_h
#include <libs/support/registry/ConstructorRegistry.h>
#endif


//--------------------------------------------------------------------
// macro to associate an attribute with a name in a registry so that
// the attribute can be constructed by name
//--------------------------------------------------------------------
#define REGISTER_ATTRIBUTE_CONSTRUCTOR(name, className, registryPtr) \
REGISTER_CONSTRUCTOR(name, className, registryPtr)
     
class AttributeConstructorRegistry : public ClassInstanceRegistry {};

#endif

