/* $Id: EntryPoints.C,v 1.2 1997/03/27 20:40:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// EntryPoints.C
//
// Author: John Mellor-Crummey                                February 1994
//
// Copyright 1994, Rice University
//***************************************************************************


#include <libs/ipAnalysis/ipInfo/EntryPoints.h>

#include <libs/support/file/FormattedFile.h>


//***************************************************************************
// class EntryPoint interface operations
//***************************************************************************

EntryPoint::EntryPoint(const char *_name, int _nodeId) : 
NamedObjectIO(_name), nodeId(_nodeId)
{
}

EntryPoint::EntryPoint(EntryPoint *rhs) : NamedObjectIO(rhs->name), 
nodeId(rhs->nodeId), formals(rhs->formals)
{
}


EntryPoint::~EntryPoint()
{
}


int EntryPoint::NamedObjectReadUpCall(FormattedFile *ffile)
{
  return ffile->Read(nodeId) || formals.Read(ffile);
}


int EntryPoint::NamedObjectWriteUpCall(FormattedFile *ffile)
{
  return ffile->Write(nodeId) || formals.Write(ffile);
}



//***************************************************************************
// class EntryPoints interface operations
//***************************************************************************


EntryPoints::EntryPoints()
{
}


EntryPoints::~EntryPoints()
{
  Destroy();
}


void EntryPoints::AddEntry(EntryPoint *entry)
{
  NamedObjectTable::AddEntry(entry);
}


void EntryPoints::operator =(EntryPoints &rhs)
{
  // precondition LHS is empty
  NamedObjectTableIterator entries((NamedObjectTable *) &rhs);
  EntryPoint *entry;
  for (; entry = (EntryPoint *) entries.Current(); ++entries) {
    AddEntry(new EntryPoint(entry));
  }
}


NamedObjectIO* EntryPoints::NewEntry()
{
  return new EntryPoint();
}


EntryPoint* EntryPoints::QueryEntry(const char *name) const
{
  return (EntryPoint *) NamedObjectTable::QueryEntry(name);
}


int EntryPoints::Read(FormattedFile *ffile)
{
  return NamedObjectTableRead(ffile);
}


int EntryPoints::Write(FormattedFile *ffile)
{
  return NamedObjectTableWrite(ffile);
}


//***************************************************************************
// class EntryPointsIterator interface operations
//***************************************************************************



EntryPointsIterator::EntryPointsIterator(const EntryPoints *set) :
NamedObjectTableIterator((NamedObjectTable *) set)
{
}


EntryPointsIterator::~EntryPointsIterator()
{ 
}


EntryPoint *EntryPointsIterator::Current() const
{ 
  return (EntryPoint *) NamedObjectTableIterator::Current();
}

