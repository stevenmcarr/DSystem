/* $Id: FileSuffixRegistry.C,v 1.1 1997/03/11 14:27:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
// FileSuffixRegistry.C: 
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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <libs/support/msgHandlers/ErrorMsgHandler.h>

#include <libs/fileAttrMgmt/attributedFile/FileSuffixRegistry.h>


//***********************************************************************
// interface operations for class FileSuffixHandle
//***********************************************************************


FileSuffixHandle::FileSuffixHandle
(const char *const fileSuffix, FileConstructorFunction fcf,
 FileModificationTimeFunction fmtf, FileSuffixRegistryRegisterFn rf) : 
 NamedObject(fileSuffix)
{
  fileConstructorFunction = fcf;
  fileModificationTimeFunction = fmtf;
  rf(this);
} 


FileSuffixHandle::~FileSuffixHandle()
{
} 


AttributedFile *FileSuffixHandle::New()
{
  return fileConstructorFunction();
} 

time_t FileSuffixHandle::GetLastModificationTime(const char *fileName)
{
  return fileModificationTimeFunction(fileName);
} 



//***********************************************************************
// interface operations for class FileSuffixRegistry
//***********************************************************************

void FileSuffixRegistry::Register(FileSuffixHandle *fsHandle)
{
  NamedObjectTable::AddEntry(fsHandle);
}


FileSuffixHandle *FileSuffixRegistry::Lookup(const char *fileSuffix)
{
  if (fileSuffix == 0) {
    errorMsgHandler.HandleMsg("Missing file suffix\n");
    return 0;
  }

  FileSuffixHandle *fileSuffixHandle = (FileSuffixHandle *) 
    NamedObjectTable::QueryEntry(fileSuffix);

  if (fileSuffixHandle == 0)
    errorMsgHandler.HandleMsg("Unknown file suffix '%s'\n", fileSuffix);

  return fileSuffixHandle;
}


FileSuffixRegistry::~FileSuffixRegistry()
{
}


FileSuffixRegistry::FileSuffixRegistry()
{
}


//***********************************************************************
// externally visible operations
//***********************************************************************

const char *ExtractFileSuffix(const char *fileName)
{
  return strrchr(fileName, '.');
}

