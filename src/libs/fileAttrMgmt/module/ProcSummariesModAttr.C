/* $Id: ProcSummariesModAttr.C,v 1.1 1997/03/11 14:28:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ProcSummariesModAttr.C
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#include <libs/support/file/FormattedFile.h>

#include <libs/ipAnalysis/ipInfo/ProcSummary.h>

#include <libs/fileAttrMgmt/module/ProcSummariesModAttr.h>



//*******************************************************************
// declarations 
//*******************************************************************

CLASS_NAME_EIMPL(ProcSummariesModAttr);


//*********************************************************************
// class ProcSummariesModAttr interface operations
//*********************************************************************

ProcSummariesModAttr::ProcSummariesModAttr()
{
}

ProcSummariesModAttr::~ProcSummariesModAttr()
{
}


void ProcSummariesModAttr::DetachUpCall()
{
}


ProcLocalInfo *ProcSummariesModAttr::NewProcEntry()
{
  return new ProcSummary;
}



