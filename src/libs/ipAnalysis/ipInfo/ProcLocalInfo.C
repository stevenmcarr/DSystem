/* $Id: ProcLocalInfo.C,v 1.1 1997/03/11 14:34:45 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ProcLocalInfo.C
//
// Author: John Mellor-Crummey                                November 1993
//
// Copyright 1993, Rice University
//***************************************************************************


#include <libs/ipAnalysis/ipInfo/ProcLocalInfo.h>
#include <libs/support/strings/rn_string.h>


//***************************************************************************
// class ProcLocalInfo interface operations
//***************************************************************************


CLASS_NAME_IMPL(ProcLocalInfo);

ProcLocalInfo::ProcLocalInfo(const char *_name) :  
NamedObjectIO(_name)
{
}


ProcLocalInfo::~ProcLocalInfo() 
{ 
}

int ProcLocalInfo::NamedObjectReadUpCall(FormattedFile *file)
{ 
  return ReadUpCall(file);
}

int ProcLocalInfo::NamedObjectWriteUpCall(FormattedFile *file)
{ 
  return WriteUpCall(file);
}


