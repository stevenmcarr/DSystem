/* $Id: FileSuffixRegistry.h,v 1.1 1997/03/11 14:27:45 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef FileSuffixRegistry_h
#define FileSuffixRegistry_h

//******************************************************************
// FileSuffixRegistry.h: 
//
//  map from a file suffix to a handle that provides a set of key
//  operations for files of that type. this registry frees other
//  modules from needing to know details about particular file types.
//
// Author: 
//   John Mellor-Crummey                              December 1993
//
// Copyright 1993, Rice University
//    
//******************************************************************

#include <assert.h>

#ifndef NamedObject_h
#include <libs/support/tables/namedObject/NamedObject.h>
#endif


#ifndef NamedObjectTable_h
#include <libs/support/tables/namedObject/NamedObjectTable.h>
#endif

class Attribute;                     // minimal external declaration
class AttributedFile;                // minimal external declaration
class FileSuffixHandle;              // forward declaration


//--------------------------------------------------------------------
// types for functions synthesized by REGISTER_FILE_SUFFIX macro
//--------------------------------------------------------------------
typedef AttributedFile *(*FileConstructorFunction)();
typedef time_t (*FileModificationTimeFunction)(const char *fileName);
typedef void (*FileSuffixRegistryRegisterFn)(FileSuffixHandle *fsh);


//--------------------------------------------------------------------
// REGISTER_FILE_SUFFIX(suffix, classname, registryClass)
//
// macro to associate an attributed file type with a file suffix 
// in a registry so that file structures can be synthesized from a 
// suffix
//--------------------------------------------------------------------

#define REGISTER_FILE_SUFFIX(suffix, classname, registryClass) \
AttributedFile * classname ## SuffixRegistry ## New() \
{ \
  return new classname; \
} \
time_t  classname ## LastModTime (const char * /* fileName */) \
{ \
  assert(0); \
  return 0; \
  /* return classname::GetLastModificationTime(fileName); */ \
} \
static FileSuffixHandle \
classname ## FileSuffixHandle(suffix, classname ## SuffixRegistry ## New, \
			      classname ## LastModTime, \
			      registryClass::RegisterFileSuffixHandle)


//--------------------------------------------------------------------
// class FileSuffixHandle
//--------------------------------------------------------------------
class FileSuffixHandle : public NamedObject {
private:
  FileConstructorFunction fileConstructorFunction;
  FileModificationTimeFunction fileModificationTimeFunction;
public:
  //--------------------------------------------------------------------
  // constructor/destructor
  //--------------------------------------------------------------------
  FileSuffixHandle(const char *const fileSuffix, FileConstructorFunction fcf,
		   FileModificationTimeFunction fmtf,
		   FileSuffixRegistryRegisterFn rf);
  ~FileSuffixHandle();

  //--------------------------------------------------------------------
  // interface operations
  //--------------------------------------------------------------------
  AttributedFile *New();
  time_t GetLastModificationTime(const char *fileName);
};


//--------------------------------------------------------------------
// class FileSuffixRegistry
//--------------------------------------------------------------------
class FileSuffixRegistry: private NamedObjectTable {
public:
  //--------------------------------------------------------------------
  // constructor/destructor
  //--------------------------------------------------------------------
  FileSuffixRegistry();
  ~FileSuffixRegistry();

  //--------------------------------------------------------------------
  // interface operations
  //--------------------------------------------------------------------
  void Register(FileSuffixHandle *fsHandle);
  FileSuffixHandle *Lookup(const char *suffix);
};


//--------------------------------------------------------------------
// const char *ExtractFileSuffix(const char *fileName)
//
// find the last period in fileName and return the substring beginning
// with the last period 
//--------------------------------------------------------------------
const char *ExtractFileSuffix(const char *fileName);


#endif
