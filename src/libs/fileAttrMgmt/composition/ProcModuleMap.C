/* $Id: ProcModuleMap.C,v 1.2 2001/09/14 18:20:52 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
// ProcModuleMap.C: 
//
//  maps the name of an entry point to its interface and a module
//  name. an instance of this map is used to record needs or provides 
//  information for a composition.
//
// Author: 
//   John Mellor-Crummey                              October 1993
//
// Copyright 1993, Rice University
//******************************************************************

#include <libs/support/strings/rn_string.h>
#include <libs/support/strings/StringIO.h>

#include <libs/fileAttrMgmt/composition/ProcModuleMap.h>

//***************************************************************************
// interface operations for class ProcModuleMapEntry
//***************************************************************************

ProcModuleMapEntry::ProcModuleMapEntry(const char *_moduleName, ProcInterface *pi) :
ProcInterface(*pi), moduleName(ssave(_moduleName)) 
{
}


ProcModuleMapEntry::ProcModuleMapEntry() : moduleName(0) 
{
}


ProcModuleMapEntry::~ProcModuleMapEntry()
{
  sfree((char *) moduleName);
}


//-------------------------------------------------------
// I/O support
//-------------------------------------------------------


int ProcModuleMapEntry::NamedObjectWriteUpCall(FormattedFile *file) 
{
  return WriteString(moduleName, file) || ProcInterface::Write(file);
}


int ProcModuleMapEntry::NamedObjectReadUpCall(FormattedFile *file) 
{
  return ReadString((char **) &moduleName, file) || ProcInterface::Read(file);
}


//***************************************************************************
// interface operations for class ProcModuleMap
//***************************************************************************

// keep base class constructor code from getting inlined
ProcModuleMap::ProcModuleMap() 
{ 
}


ProcModuleMap::~ProcModuleMap()
{
  this->Destroy();
}


NamedObjectIO *ProcModuleMap::NewEntry()
{
  return new ProcModuleMapEntry;
}


ProcModuleMapEntry *ProcModuleMap::QueryEntry(const char *_name)
{
  return (ProcModuleMapEntry *) NamedObjectTable::QueryEntry(_name);
}


//-------------------------------------------------------
// I/O support
//-------------------------------------------------------


int ProcModuleMap::Read(FormattedFile *file)
{
  return NamedObjectTableRead(file);
}


int ProcModuleMap::Write(FormattedFile *file)
{
  return NamedObjectTableWrite(file);
}



//*****************************************************
// interface operations for class ProcModuleMapIterator
//*****************************************************



ProcModuleMapIterator::ProcModuleMapIterator(ProcModuleMap *set) :
NamedObjectTableIterator((NamedObjectTable *) set)
{
}


ProcModuleMapIterator::~ProcModuleMapIterator()
{
}


ProcModuleMapEntry *ProcModuleMapIterator::Current() const
{
  return (ProcModuleMapEntry *) NamedObjectTableIterator::Current();
}
