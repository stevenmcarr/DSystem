/* $Id: FormalParameters.C,v 1.1 1997/03/11 14:34:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// FormalParameters.C
//
// Author: John Mellor-Crummey                                February 1994
//
// Copyright 1994, Rice University
//***************************************************************************


#include <libs/ipAnalysis/ipInfo/FormalParameters.h>

#include <libs/support/file/FormattedFile.h>


//***************************************************************************
// class FormalParameter interface operations
//***************************************************************************

FormalParameter::FormalParameter(const char *pname, int _ptype) :
NamedObjectIO(pname), ptype(_ptype)
{
}

FormalParameter::FormalParameter(FormalParameter *rhs) :
NamedObjectIO(rhs->name), ptype(rhs->ptype)
{
}


FormalParameter::~FormalParameter()
{
}


int FormalParameter::NamedObjectReadUpCall(FormattedFile *ffile)
{
  return ffile->Read(*(int *) &ptype);
}


int FormalParameter::NamedObjectWriteUpCall(FormattedFile *ffile)
{
  return ffile->Write(ptype);
}



//***************************************************************************
// class FormalParameters interface operations
//***************************************************************************


FormalParameters::FormalParameters()
{
}


FormalParameters::FormalParameters(FormalParameters &rhs)
{
  NamedObjectTableIterator entries((NamedObjectTable *) &rhs);
  FormalParameter *entry;
  for (; entry = (FormalParameter *) entries.Current(); entries++) {
    AddEntry(new FormalParameter(entry));
  }
}


FormalParameters::~FormalParameters()
{
  Destroy();
}


void FormalParameters::Append(FormalParameter *formal)
{
  NamedObjectTable::AddEntry(formal);
}


NamedObjectIO* FormalParameters::NewEntry()
{
  return (NamedObjectIO *) new FormalParameter();
}


FormalParameter* FormalParameters::QueryEntry(const char *name) const
{
  return (FormalParameter *) NamedObjectTable::QueryEntry(name);
}


FormalParameter *FormalParameters::GetMember(unsigned int cindex)
{ 
  return (FormalParameter *) NamedObjectTable::GetEntryByIndex(cindex);
}


int FormalParameters::GetMemberIndex(const char *name)
{ 
  return NamedObjectTable::GetEntryIndex(name);
}


int FormalParameters::Read(FormattedFile *ffile)
{
  return NamedObjectTableRead(ffile);
}


int FormalParameters::Write(FormattedFile *ffile)
{
  return NamedObjectTableWrite(ffile);
}



//***************************************************************************
// class FormalParametersIterator interface operations
//***************************************************************************


FormalParametersIterator::FormalParametersIterator(const FormalParameters *s):
NamedObjectTableIterator((NamedObjectTable *) s)
{
}


FormalParametersIterator::~FormalParametersIterator()
{ 
}


FormalParameter *FormalParametersIterator::Current() const
{ 
  return (FormalParameter *) NamedObjectTableIterator::Current();
}

