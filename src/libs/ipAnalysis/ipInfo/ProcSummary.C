/* $Id: ProcSummary.C,v 1.1 1997/03/11 14:34:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ProcSummary.C
//
// Author: John Mellor-Crummey                                December 1993
//
// Copyright 1993, Rice University
//***************************************************************************


#include <libs/support/strings/rn_string.h>

#include <libs/ipAnalysis/ipInfo/CallSite.h>
#include <libs/ipAnalysis/ipInfo/ProcSummary.h>
#include <libs/ipAnalysis/ipInfo/ParameterList.h>


//***************************************************************************
// class ProcSummary interface operations
//***************************************************************************


ProcSummary::ProcSummary(const char *_name, ProcType type) : 
ProcLocalInfo(_name), procType(type) 
{
  calls = new CallSites;
}


ProcSummary::ProcSummary() : ProcLocalInfo(0), procType(ProcType_ILLEGAL) 
{
  calls = new CallSites;
}


ProcSummary::~ProcSummary()
{
  delete calls;
}


int ProcSummary::ReadUpCall(FormattedFile *ffile)
{ 
  return ffile->Read(*(unsigned int *) &procType) || calls->Read(*ffile) 
    || entryPoints.Read(ffile);
}


int ProcSummary::WriteUpCall(FormattedFile *ffile)
{ 
  return ffile->Write((unsigned int) procType) || calls ->Write(*ffile) 
    || entryPoints.Write(ffile);
}

