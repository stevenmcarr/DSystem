/* $Id: CallSitesLocalInfo.C,v 1.1 1997/03/11 14:34:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// CallSitesLocalInfo.C
//
// Author: John Mellor-Crummey                                November 1993
//
// Copyright 1993, Rice University
//***************************************************************************


#include <libs/support/file/FormattedFile.h>

#include <libs/ipAnalysis/ipInfo/CallSiteLocalInfo.h>
#include <libs/ipAnalysis/ipInfo/CallSitesLocalInfo.h>



//***************************************************************************
// class CallSitesLocalInfo interface operations
//***************************************************************************

CallSitesLocalInfo::CallSitesLocalInfo()
{
}


CallSitesLocalInfo::~CallSitesLocalInfo() 
{
  Destroy();
}


void CallSitesLocalInfo::AddCallSiteEntry(CallSiteLocalInfo *pli) 
{
  AddEntry(pli);
}


CallSiteLocalInfo *CallSitesLocalInfo::GetCallSiteEntry
(unsigned int callSiteId) 
{
  return (CallSiteLocalInfo *) QueryEntry(callSiteId);
}


//***************************
// private operations 
//***************************


void CallSitesLocalInfo::Destroy() 
{
  WordObjectTable::Destroy();
}


int CallSitesLocalInfo::Create() 
{
  WordObjectTable::Create();
  return 0;
}


CallSiteLocalInfo *CallSitesLocalInfo::NewCallSiteEntry() 
{
  // provide placeholder function for this base class so that
  // it is possible to read and write empty CallSitesLocalInfo
  // classes if there is no call site information
  return 0;
}

WordObjectIO *CallSitesLocalInfo::NewWordObjectIO() 
{
  return  NewCallSiteEntry();
}



//***************************************************************************
// class CallSitesLocalInfoIterator interface operations
//***************************************************************************

CallSitesLocalInfoIterator::CallSitesLocalInfoIterator(const CallSitesLocalInfo *cli) : 
WordObjectTableIterator((WordObjectTable *) cli)
{
}


CallSitesLocalInfoIterator::~CallSitesLocalInfoIterator() 
{ 
}


CallSiteLocalInfo *CallSitesLocalInfoIterator::Current() const
{
  return (CallSiteLocalInfo *) WordObjectTableIterator::Current();
}


