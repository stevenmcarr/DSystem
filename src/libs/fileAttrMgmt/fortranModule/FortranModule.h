/* $Id: FortranModule.h,v 1.1 1997/03/11 14:27:58 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef FortranModule_h
#define FortranModule_h

#ifdef __cplusplus

// ***************************************************************************
// FortranModule.h
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
// ***************************************************************************


#include <sys/types.h>


#ifndef Module_h
#include <libs/fileAttrMgmt/module/Module.h>
#endif 

#ifndef AttributeConstructorRegistry_h
#include <libs/fileAttrMgmt/attributedFile/AttributeConstructorRegistry.h>
#endif

class AttributeConstructor;
class AttributeConstructorRegistry;
class NeedProvModAttr;    

#define FORTRAN_FILE_SUFFIX ".f"

//--------------------------------------------------------------------
// class FortranModule
//--------------------------------------------------------------------
class FortranModule : public Module {
private:
  //--------------------------------------------------------------------
  // AttributedFile uses this to get access to constructors for 
  // Composition Attributes
  //--------------------------------------------------------------------
  AttributeConstructorRegistry *GetAttributeConstructorRegistry();
public:
  static AttributeConstructorRegistry *fortModuleAttrConstructorRegistry;

  virtual ~FortranModule();
  
#if 0
  //--------------------------------------------------------------------
  // interface to register a constructor for each Attribute available
  // for a Composition
  //--------------------------------------------------------------------
  static void RegisterAttributeConstructor(AttributeConstructor *attrConstructor);
#endif

  CLASS_NAME_FDEF(FortranModule);	

  virtual time_t GetLastModificationTime();
  static time_t GetLastModificationTime(const char *const srcFileName);
};

CLASS_NAME_EDEF(FortranModule);	

#define REGISTER_FORTRAN_MODULE_ATTRIBUTE(classname) \
CLASS_NAME_IMPL(classname); \
REGISTER_ATTRIBUTE_CONSTRUCTOR(CLASS_NAME(classname), classname, \
			       FortranModule::fortModuleAttrConstructorRegistry)

#define REGISTER_FORTRAN_MODULE_DERIVED_ATTRIBUTE(classname, basename) \
REGISTER_ATTRIBUTE_CONSTRUCTOR(CLASS_NAME(classname), classname, \
			       FortranModule::fortModuleAttrConstructorRegistry)

#else

typedef void *FortranModule;

#endif


#endif
