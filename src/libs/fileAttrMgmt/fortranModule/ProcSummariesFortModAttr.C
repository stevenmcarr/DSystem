/* $Id: ProcSummariesFortModAttr.C,v 1.1 1997/03/11 14:28:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ProcSummariesFortModAttr.C
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#include <libs/support/file/FormattedFile.h>

#include <libs/ipAnalysis/ipInfo/ProcSummary.h>

#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>

#include <libs/fileAttrMgmt/module/ProcSummariesModAttr.h>

#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/ProcSummariesFortModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/ComputeProcSummary.i>
#include <libs/fileAttrMgmt/fortranModule/CollectLocalInfo.h>



//*******************************************************************
// declarations 
//*******************************************************************

CLASS_NAME_EIMPL_DERIVED(ProcSummariesFortModAttr, ProcSummariesModAttr); 


//*********************************************************************
// class ProcSummariesFortModAttr interface operations
//*********************************************************************

ProcSummariesFortModAttr::ProcSummariesFortModAttr()
{
}

ProcSummariesFortModAttr::~ProcSummariesFortModAttr()
{
}


void ProcSummariesFortModAttr::DetachUpCall()
{
  uplinkToFile->DetachAttributeIfUnreferenced(CLASS_NAME(FortTreeModAttr));
}


int ProcSummariesFortModAttr::ComputeUpCall()
{
  int code = -1;
  FortTreeModAttr *ftAttr = (FortTreeModAttr *) 
    uplinkToFile->AttachAttribute(CLASS_NAME(FortTreeModAttr));

  if (ftAttr->GetCheckedState() != ft_ERRONEOUS) 
    // state is ft_CORRECT or ft_WARNINGS_ONLY
    code = CollectLocalInfo(ftAttr->ft, this, ComputeProcSummary);

  uplinkToFile->DetachAttribute(ftAttr);
  return code;
}
