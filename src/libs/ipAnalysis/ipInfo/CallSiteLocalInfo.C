/* $Id: CallSiteLocalInfo.C,v 1.1 1997/03/11 14:34:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// CallSiteLocalInfo.C
//
// Author: John Mellor-Crummey                                July 1994
//
// Copyright 1994, Rice University
//***************************************************************************


#include <libs/ipAnalysis/ipInfo/CallSiteLocalInfo.h>
#include <libs/support/strings/rn_string.h>


//***************************************************************************
// class CallSiteLocalInfo interface operations
//***************************************************************************

CallSiteLocalInfo::CallSiteLocalInfo(unsigned int _id) : WordObjectIO(_id)
{
}


CallSiteLocalInfo::~CallSiteLocalInfo() 
{ 
}


int CallSiteLocalInfo::WordObjectReadUpCall(FormattedFile *file)
{ 
  return ReadUpCall(file);
}


int CallSiteLocalInfo::WordObjectWriteUpCall(FormattedFile *file)
{ 
  return WriteUpCall(file);
}


