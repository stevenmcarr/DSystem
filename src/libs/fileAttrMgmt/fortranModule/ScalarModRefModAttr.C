/* $Id: ScalarModRefModAttr.C,v 1.1 1997/03/11 14:28:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ScalarModRefModAttr.C
//
// Author: John Mellor-Crummey                                January 1994
//
// Copyright 1994, Rice University
//***************************************************************************


#include <libs/support/file/FormattedFile.h>

#include <libs/ipAnalysis/ipInfo/ProcScalarModRefInfo.h>

#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>

#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/ScalarModRefModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/ComputeModRef.i>
#include <libs/fileAttrMgmt/fortranModule/CollectLocalInfo.h>



//*******************************************************************
// declarations 
//*******************************************************************


REGISTER_FORTRAN_MODULE_ATTRIBUTE(ScalarModRefModAttr);


//*********************************************************************
// class ScalarModRefModAttr interface operations
//*********************************************************************

ScalarModRefModAttr::ScalarModRefModAttr()
{
}

ScalarModRefModAttr::~ScalarModRefModAttr()
{
}


void ScalarModRefModAttr::DetachUpCall()
{
  uplinkToFile->DetachAttributeIfUnreferenced(CLASS_NAME(FortTreeModAttr));
}


int ScalarModRefModAttr::ComputeUpCall()
{
  FortTreeModAttr *ftAttr = (FortTreeModAttr *) 
    uplinkToFile->AttachAttribute(CLASS_NAME(FortTreeModAttr));

  int code = CollectLocalInfo(ftAttr->ft, this, ComputeProcScalarModRefInfo);

  uplinkToFile->DetachAttribute(ftAttr);
  return code;
}


ProcLocalInfo *ScalarModRefModAttr::NewProcEntry()
{
  return new ProcScalarModRefInfo;
}

