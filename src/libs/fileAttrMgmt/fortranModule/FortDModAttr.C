/* $Id: FortDModAttr.C,v 1.1 1997/03/11 14:27:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// FortDModAttr.C
//
// Author: John Mellor-Crummey                                January 1994
//
// Copyright 1994, Rice University
//***************************************************************************


#include <libs/support/file/FormattedFile.h>

#include <libs/ipAnalysis/ipInfo/ProcFortDInfo.h>

#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>

#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortDModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/ComputeFortD.i>
#include <libs/fileAttrMgmt/fortranModule/CollectLocalInfo.h>



//*******************************************************************
// declarations 
//*******************************************************************


REGISTER_FORTRAN_MODULE_ATTRIBUTE(FortDModAttr);


//*********************************************************************
// class FortDModAttr interface operations
//*********************************************************************

FortDModAttr::FortDModAttr()
{
}

FortDModAttr::~FortDModAttr()
{
}


void FortDModAttr::DetachUpCall()
{
  uplinkToFile->DetachAttributeIfUnreferenced(CLASS_NAME(FortTreeModAttr));
}


int FortDModAttr::ComputeUpCall()
{
  FortTreeModAttr *ftAttr = (FortTreeModAttr *) 
    uplinkToFile->AttachAttribute(CLASS_NAME(FortTreeModAttr));

  int code = CollectLocalInfo(ftAttr->ft, this, ComputeFortDInfo);

  uplinkToFile->DetachAttribute(ftAttr);
  return code;
}


ProcLocalInfo *FortDModAttr::NewProcEntry()
{
  return new ProcFortDInfo;
}

