/* $Id: NamedObject.C,v 1.2 1997/03/11 14:37:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*************************************************************************** 
//
// NamedObject
//                                                                          
//   base class for named entities
//
// Author:  John Mellor-Crummey                            June 1993
//
// Copyright 1993, Rice University
//                                                                          
//***************************************************************************

#include <stdio.h>
#include <stdlib.h>

#include <libs/support/tables/namedObject/NamedObject.h>
#include <libs/support/strings/rn_string.h>
#include <libs/support/strings/StringIO.h>
#include <libs/support/msgHandlers/DumpMsgHandler.h>


//***************************************************************************
// class NamedObject interface operations
//***************************************************************************

CLASS_NAME_IMPL(NamedObject);


NamedObject::NamedObject(const char *const objectName) : name(objectName)
{
}


void NamedObject::Create(const char *const objectName) 
{
  // sidestep const nature  
  *(char **) &name = *(char **) &objectName;
}


NamedObject::~NamedObject()
{
}


void NamedObject::NamedObjectDump()
{
  dumpHandler.Dump("%s(name = %s)\n", ClassName(), name);
  dumpHandler.BeginScope();
  NamedObjectDumpUpCall();
  dumpHandler.EndScope();
}


void NamedObject::NamedObjectDumpUpCall()
{
}


//***************************************************************************
// class NamedObjectIO interface operations
//***************************************************************************

CLASS_NAME_IMPL(NamedObjectIO);


NamedObjectIO::NamedObjectIO(const char *const objectName) : 
NamedObject(objectName ? ssave(objectName) : 0)
{
  
}


NamedObjectIO::~NamedObjectIO()
{
  if (name) sfree((char *) name);
}


int NamedObjectIO::NamedObjectRead(FormattedFile *file)
{
  return ReadString((char **) &name, file) || NamedObjectReadUpCall(file);
}

int NamedObjectIO::NamedObjectReadUpCall(FormattedFile *)
{
  return 0;
}


int NamedObjectIO::NamedObjectWrite(FormattedFile *file)
{
  return WriteString(name, file) || NamedObjectWriteUpCall(file);
}

int NamedObjectIO::NamedObjectWriteUpCall(FormattedFile *)
{
  return 0;
}


